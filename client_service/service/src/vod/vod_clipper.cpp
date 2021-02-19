#include "vod/vod_clipper.h"
#include "shared/filesystem/common_paths.h"
#include "shared/filesystem/utility.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"

#include <sstream>
#include <unordered_map>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/audio_fifo.h>
#include <libswresample/swresample.h>
}

namespace fs = std::filesystem;
namespace service::vod {
namespace {

struct InputStreamContainer {
    AVCodecContext* codecContext = nullptr;

    ~InputStreamContainer();
};
using InputStreamContainerPtr = std::unique_ptr<InputStreamContainer>;

struct OutputStreamContainer {
    AVStream* stream = nullptr;
    
    bool firstFrame = false;
    int64_t startPts = 0;

    ~OutputStreamContainer();
};
using OutputStreamContainerPtr = std::unique_ptr<OutputStreamContainer>;

class VodClipper {
public:
    VodClipper(const fs::path& output, const VodClipRequest& request);
    ~VodClipper();

    shared::squadov::VodMetadata run();

private:
    fs::path _output;
    VodClipRequest _request;

    void openInputOutputCodecPairs();
    std::unordered_map<int, std::pair<InputStreamContainerPtr, OutputStreamContainerPtr>> _streamPairs;

    // Output
    AVOutputFormat* _outputFormat = nullptr;
    AVFormatContext* _outputContext = nullptr;
    OutputStreamContainerPtr createOutputStreamForInput(AVStream* stream, const InputStreamContainer& container) const;
    void openOutput();

    // Input
    AVInputFormat* _inputFormat = nullptr;
    AVFormatContext* _inputContext = nullptr;
    InputStreamContainerPtr handleInputStream(AVStream* stream) const;
    void openInput();
};

InputStreamContainer::~InputStreamContainer() {
    if (codecContext) {
        avcodec_free_context(&codecContext);
    }
}

OutputStreamContainer::~OutputStreamContainer() {
}

VodClipper::VodClipper(const fs::path& output, const VodClipRequest& request):
    _output(output),
    _request(request) {
}

VodClipper::~VodClipper() {
    if (_outputContext) {
        if (_outputContext->pb) {
            avio_flush(_outputContext->pb);
            avio_closep(&_outputContext->pb);
        }
        avformat_free_context(_outputContext);
    }

    if (_inputContext) {
        avformat_close_input(&_inputContext);
    }
}

void VodClipper::openInput() {
    _inputFormat = av_find_input_format("mp4");
    if (!_inputFormat) {
        THROW_ERROR("Failed to find the MP4 input format.");
    }

    _inputContext = nullptr;
    if (avformat_open_input(&_inputContext, _request.source.c_str(), _inputFormat, nullptr) < 0) {
        THROW_ERROR("Failed to allocate input AV context.");
    }

    if (avformat_find_stream_info(_inputContext, nullptr) < 0) {
        THROW_ERROR("Failed to find input stream info.");
    }

    // Seek to correct time in the video stream. Should be stream 0.
    if (_inputContext->nb_streams == 0) {
        THROW_ERROR("No input streams.");
    }

    const auto* vstream = _inputContext->streams[0];
    if (vstream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO) {
        THROW_ERROR("0-index stream is not a video. Did we make this?");
    }

    const int64_t scaledTs = av_rescale_q(_request.start * AV_TIME_BASE, AVRational{1, AV_TIME_BASE}, vstream->time_base);
    if (avformat_seek_file(_inputContext, 0, INT64_MIN, scaledTs, scaledTs, 0) < 0) {
        THROW_ERROR("Failed to seek to specified start time in input.");
    }
}

void VodClipper::openOutput() {
    _outputFormat = av_guess_format("mp4", nullptr, nullptr);
    if (!_outputFormat) {
        THROW_ERROR("Failed to find the MP4 output format.");
    }

    _outputContext = nullptr;
    if (avformat_alloc_output_context2(&_outputContext, _outputFormat, nullptr, nullptr) < 0) {
        THROW_ERROR("Failed to allocate output AV context.");
    }
}

void VodClipper::openInputOutputCodecPairs() {
    for (unsigned int i = 0; i < _inputContext->nb_streams; ++i) {
        auto* istream = _inputContext->streams[i];
        if (istream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO && istream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
            continue;
        }

        auto inputContainer = handleInputStream(istream);
        auto outputContainer = createOutputStreamForInput(istream, *inputContainer);

        auto pair = std::make_pair(static_cast<int>(i), std::make_pair(std::move(inputContainer), std::move(outputContainer)));
        _streamPairs.insert(std::move(pair));
    }
}


shared::squadov::VodMetadata VodClipper::run() {
    // Clipping the video is effectively just doing a transcoding of a sub-part of the input video.
    // So all we need to do is
    //  1) Open the input (with a delay for a set duration) as well as the output.
    //  2) Read in the input packets
    //  3) Change the packet pts and write to the output (effectively doing a -c:v copy and -c:a copy).
    // We don't need to do any fancy multi-threading like we do with the ffmpeg av encoder since
    // we don't need satisfy any sort of realtime constraints so this can all be single threaded
    // which makes things so much easier.
    openInput();
    openOutput();
    openInputOutputCodecPairs();

    const auto path = shared::filesystem::pathUtf8(_output);
    if (avio_open2(&_outputContext->pb, path.c_str(), AVIO_FLAG_WRITE, nullptr, nullptr) < 0) {
        THROW_ERROR("Failed to open video for output: " << path);
    }

    // I can't figure out why we can't successfully write the trailer with the faststart flag.
    // I'm assuming this clip is going to go through the same video processing pipeline as regular
    // VODS though so it's probably fine.
    if (avformat_write_header(_outputContext, nullptr) < 0) {
        THROW_ERROR("Failed to write header");
    }

    av_dump_format(_inputContext, 0, _request.source.c_str(), 0);
    av_dump_format(_outputContext, 0, path.c_str(), 1);

    AVPacket packet;
    int64_t audioPts = 0;
    bool finished = false;

    while (!finished) {
        if (av_read_frame(_inputContext, &packet) < 0) {
            break;
        }

        const auto streamIndex = packet.stream_index;
        const auto it = _streamPairs.find(streamIndex);
        if (it != _streamPairs.end()) {
            const auto* inputContainer = it->second.first.get();
            auto* outputContainer = it->second.second.get();

            
            av_packet_rescale_ts(&packet, _inputContext->streams[streamIndex]->time_base, inputContainer->codecContext->time_base);
            if (!outputContainer->firstFrame) {
                outputContainer->startPts = packet.pts;
                outputContainer->firstFrame = true;
            }


            packet.pts = av_rescale_q(packet.pts - outputContainer->startPts, inputContainer->codecContext->time_base, outputContainer->stream->time_base);
            packet.dts = av_rescale_q(packet.dts - outputContainer->startPts, inputContainer->codecContext->time_base, outputContainer->stream->time_base);

            if (av_compare_ts(packet.pts, outputContainer->stream->time_base, (_request.end - _request.start) * AV_TIME_BASE, AVRational{1, AV_TIME_BASE}) > 0) {
                finished = true;
                break;
            }

            if (av_interleaved_write_frame(_outputContext, &packet) < 0) {
                THROW_ERROR("Failed to write to output.");
            }
        }

        av_packet_unref(&packet);
    }
    av_packet_unref(&packet);

    shared::squadov::VodMetadata metadata;
    metadata.id = "source";
    metadata.videoUuid = "";

    for (const auto& kvp: _streamPairs) {
        const auto* inputContainer = kvp.second.first.get();
        const auto* stream = _inputContext->streams[kvp.first];

        if (inputContainer->codecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
            // Bandwidth
            {
                const auto* props = reinterpret_cast<const AVCPBProperties*>(av_stream_get_side_data(
                    stream,
                    AV_PKT_DATA_CPB_PROPERTIES,
                    nullptr
                ));

                if (props) {
                    metadata.avgBitrate = props->avg_bitrate;
                    metadata.maxBitrate = props->max_bitrate;
                    metadata.minBitrate = props->min_bitrate;
                } else if (stream->codecpar->bit_rate) {
                    metadata.avgBitrate = stream->codecpar->bit_rate;
                    metadata.maxBitrate = stream->codecpar->bit_rate;
                    metadata.minBitrate = stream->codecpar->bit_rate;
                }
            }

            // Resolution
            {
                metadata.resX = inputContainer->codecContext->width;
                metadata.resY = inputContainer->codecContext->height;
            }

            // FPS
            metadata.fps = static_cast<int>(static_cast<double>(inputContainer->codecContext->time_base.den) / inputContainer->codecContext->time_base.num);
        }
    }
    
    const auto ret = av_write_trailer(_outputContext);
    if (ret < 0) {
        THROW_ERROR("Failed to write trailer: " << ret << std::endl);
    }
    return metadata;
}

std::unique_ptr<InputStreamContainer> VodClipper::handleInputStream(AVStream* stream) const {
    auto container = std::make_unique<InputStreamContainer>();

    AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!decoder) {
        THROW_ERROR("Failed to find decoder for input stream.");
    }

    container->codecContext = avcodec_alloc_context3(decoder);
    if (!container->codecContext) {
        THROW_ERROR("Failed to create decoder context for input stream.");
    }

    if (avcodec_parameters_to_context(container->codecContext, stream->codecpar)) {
        THROW_ERROR("Failed to obtain input codec context.");
    }

    if (container->codecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
        container->codecContext->framerate = av_guess_frame_rate(_inputContext, stream, nullptr);
    }

    if (avcodec_open2(container->codecContext, decoder, nullptr) < 0) {
        THROW_ERROR("Failed to open decoder codec.");
    }

    return container;
}

std::unique_ptr<OutputStreamContainer> VodClipper::createOutputStreamForInput(AVStream* stream, const InputStreamContainer& icontainer) const {
    auto container = std::make_unique<OutputStreamContainer>();

    AVStream* ostream = avformat_new_stream(_outputContext, nullptr);
    if (avcodec_parameters_copy(ostream->codecpar, stream->codecpar) < 0) {
        THROW_ERROR("Failed to copy context parameters to encoding stream.")
    }
    container->stream = ostream;
    return container;
}

}

VodClipRequest VodClipRequest::fromJson(const nlohmann::json& obj) {
    VodClipRequest clip;
    clip.task = obj["task"].get<std::string>();
    clip.source = obj["source"].get<std::string>();
    clip.start = obj["start"].get<int64_t>();
    clip.end = obj["end"].get<int64_t>();
    return clip;
}

nlohmann::json VodClipResponse::toJson() const {
    return {
        { "task", task },
        { "path", path },
        { "success", success },
        { "metadata", metadata.toJson() }
    };
}

VodClipResponse vodClip(const VodClipRequest& request) {
    // Generate a temporary file to store the clip in locally.
    const auto parentFolder = shared::filesystem::getSquadOvClipFolder();
    if (!fs::exists(parentFolder)) {
        fs::create_directories(parentFolder);
    }

    std::ostringstream filename;
    filename << request.task << ".mp4";

    const auto clipFile = parentFolder / fs::path(filename.str());

    VodClipResponse response;
    response.task = request.task;
    response.success = false;

    try {
        VodClipper clipper(clipFile, request);
        response.metadata = clipper.run();
        response.success = true;
    } catch(std::exception& ex) {
        LOG_WARNING("Failed to perform VOD clip: " << ex.what() << std::endl);
    }
   
    response.path = shared::filesystem::pathUtf8(clipFile);
    return response;
}

}