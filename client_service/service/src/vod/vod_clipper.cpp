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

constexpr int64_t CLIPPING_TIME_BASE = 1000;

struct InputStreamContainer {
    AVCodecContext* codecContext = nullptr;

    ~InputStreamContainer();
};
using InputStreamContainerPtr = std::unique_ptr<InputStreamContainer>;

struct OutputStreamContainer {
    AVStream* stream = nullptr;
    AVCodecContext* codecContext = nullptr;

    // Audio only
    AVAudioFifo* fifo = nullptr;
    SwrContext* swr = nullptr;
    uint8_t** samplesStorage = nullptr;
    int sampleLineSize = 0;
    int64_t maxSamples = 0;
    
    bool firstFrame = false;
    int64_t startPts = 0;
    int64_t lastDts = AV_NOPTS_VALUE;

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
    void encode(OutputStreamContainer* container, AVFrame* frame);

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
    if (codecContext) {
        avcodec_free_context(&codecContext);
    }

    if (samplesStorage) {
        av_freep(&samplesStorage[0]);
    }

    if (fifo) {
        av_audio_fifo_free(fifo);
    }

    if (swr) {
        swr_free(&swr);
    }
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

void VodClipper::encode(OutputStreamContainer* container, AVFrame* frame) {
    auto ret = avcodec_send_frame(container->codecContext, frame);
    if (ret < 0) {
        THROW_ERROR("Failed to send frame: " << ret);
    }

    while (true) {
        AVPacket packet = { 0 };
        const auto ret = avcodec_receive_packet(container->codecContext, &packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            THROW_ERROR("Failed to receive encoded packet.");
        }

        av_packet_rescale_ts(&packet, container->codecContext->time_base, container->stream->time_base);
        packet.stream_index = container->stream->index;

        const auto maxDts = container->lastDts + !(_outputFormat->flags & AVFMT_TS_NONSTRICT);
        if (container->lastDts != AV_NOPTS_VALUE && packet.dts < maxDts) {
            LOG_WARNING("Bad packet: non-monotonically increasing DTS...Recovering" << std::endl);
            packet.dts = maxDts;
        }

        if (packet.dts > packet.pts) {
            LOG_WARNING("Bad packet: DTS > PTS...Recovering" << std::endl);
            packet.pts = packet.dts;
        }

        container->lastDts = packet.dts;
        if (av_interleaved_write_frame(_outputContext, &packet) < 0) {
            THROW_ERROR("Failed to write to output.");
        }

        av_packet_unref(&packet);
    }
}

void VodClipper::openInput() {
    _inputFormat = av_find_input_format(_request.inputFormat.c_str());
    if (!_inputFormat) {
        THROW_ERROR("Failed to find the input format:" << _request.inputFormat);
    }

    _inputContext = nullptr;

    AVDictionary* options = nullptr;
    // To parallel what we have setup for video processing already.
    av_dict_set(&options, "analyzeduration", "100000000", 0);
    av_dict_set(&options, "probesize", "100000000", 0);
    if (_request.inputFormat == "mpegts") {
        av_dict_set(&options, "scan_all_pmts", "1", 0);
    }

    if (avformat_open_input(&_inputContext, _request.source.c_str(), _inputFormat, &options) < 0) {
        THROW_ERROR("Failed to allocate input AV context.");
    }
    av_dict_free(&options);

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

    // The start/end time comes in milliseconds so we need to make sure we convert the timestamp
    // using the millisecond timebase.
    const int64_t scaledTs = av_rescale_q(_request.start, AVRational{1, CLIPPING_TIME_BASE}, vstream->time_base);
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

    const std::string utf8Output = shared::filesystem::pathUtf8(_output);
    _outputContext->url = av_strndup(utf8Output.c_str(), utf8Output.size());
}

void VodClipper::openInputOutputCodecPairs() {
    for (unsigned int i = 0; i < _inputContext->nb_streams; ++i) {
        auto* istream = _inputContext->streams[i];
        if (istream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO && istream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
            continue;
        }

        if (istream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && !_request.audio) {
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

    av_dump_format(_inputContext, 0, _request.source.c_str(), 0);
    av_dump_format(_outputContext, 0, path.c_str(), 1);

    AVDictionary* hopts = nullptr;
    av_dict_set(&hopts, "movflags", "faststart", 0);
    if (avformat_write_header(_outputContext, &hopts) < 0) {
        THROW_ERROR("Failed to write header");
    }
    av_dict_free(&hopts);

    AVPacket packet;
    AVFrame* frame = av_frame_alloc();
    AVFrame* outputAudioFrame = nullptr;
    int64_t audioPts = 0;

    bool finished = false;

    while (!finished) {
        if (av_read_frame(_inputContext, &packet) < 0) {
            break;
        }

        const auto streamIndex = packet.stream_index;
        const auto streamType = _inputContext->streams[streamIndex]->codecpar->codec_type;
        const auto it = _streamPairs.find(streamIndex);
        if (it != _streamPairs.end()) {
            const auto* inputContainer = it->second.first.get();
            auto* outputContainer = it->second.second.get();

            if (_request.fullCopy) {
                av_packet_rescale_ts(&packet, _inputContext->streams[streamIndex]->time_base, outputContainer->stream->time_base);
                if (outputContainer->lastDts != AV_NOPTS_VALUE && packet.dts <= outputContainer->lastDts) {
                    if (packet.pts >= packet.dts) {
                        packet.pts = std::max(packet.pts, outputContainer->lastDts + 1);
                    }
                    packet.dts = outputContainer->lastDts + 1;
                }

                if (packet.dts > packet.pts) {
                    packet.pts = packet.dts;
                }

                outputContainer->lastDts = packet.dts;
                if (av_interleaved_write_frame(_outputContext, &packet) < 0) {
                    THROW_ERROR("Failed to write to output.");
                }
            } else {
                av_packet_rescale_ts(&packet, _inputContext->streams[streamIndex]->time_base, inputContainer->codecContext->time_base);    
                int ret = avcodec_send_packet(inputContainer->codecContext, &packet);
                if (ret < 0) {
                    THROW_ERROR("Failed to decode frame.");
                    break;
                }

                while (true) {
                    ret = avcodec_receive_frame(inputContainer->codecContext, frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        break;
                    } else if (ret < 0) {
                        THROW_ERROR("Failed to receive decoded frame.");
                        break;
                    }

                    if (av_compare_ts(frame->best_effort_timestamp, inputContainer->codecContext->time_base, _request.start, AVRational{1, CLIPPING_TIME_BASE}) < 0) {
                        continue;
                    } else if (av_compare_ts(frame->best_effort_timestamp, inputContainer->codecContext->time_base, _request.end, AVRational{1, CLIPPING_TIME_BASE}) >= 0)  {
                        finished = true;
                        break;
                    }

                    if (!outputContainer->firstFrame) {
                        outputContainer->startPts = frame->best_effort_timestamp;
                        outputContainer->firstFrame = true;
                    }

                    frame->pts = av_rescale_q(frame->best_effort_timestamp - outputContainer->startPts, inputContainer->codecContext->time_base, outputContainer->codecContext->time_base);
                    
                    if (streamType == AVMEDIA_TYPE_VIDEO) {
                        encode(outputContainer, frame);
                    } else {
                        const auto numConverted = swr_convert(
                            outputContainer->swr,
                            outputContainer->samplesStorage,
                            static_cast<int>(outputContainer->maxSamples),
                            (const uint8_t**)&frame->data[0],
                            frame->nb_samples
                        );

                        const auto newFifoSize = av_audio_fifo_space(outputContainer->fifo) + numConverted;
                        if (av_audio_fifo_realloc(outputContainer->fifo, newFifoSize) < 0) {
                            THROW_ERROR("Failed to reallocate FIFO queue.");
                        }

                        if (av_audio_fifo_write(outputContainer->fifo, (void**)outputContainer->samplesStorage, numConverted) < numConverted) {
                            THROW_ERROR("Failed to write to FIFO queue.");
                        }

                        if (!outputAudioFrame) {
                            outputAudioFrame = av_frame_alloc();
                            outputAudioFrame->nb_samples = outputContainer->codecContext->frame_size;
                            outputAudioFrame->channel_layout = outputContainer->codecContext->channel_layout;
                            outputAudioFrame->format = outputContainer->codecContext->sample_fmt;
                            outputAudioFrame->sample_rate = outputContainer->codecContext->sample_rate;
                            if (av_frame_get_buffer(outputAudioFrame, 0) < 0) {
                                THROW_ERROR("Failed to get audio buffer.");
                            }
                        }

                        if (av_frame_make_writable(outputAudioFrame) < 0) {
                            THROW_ERROR("Failed to make output audio frame writable.");
                        }

                        const auto frameSize = outputContainer->codecContext->frame_size;
                        if (av_audio_fifo_size(outputContainer->fifo) >= frameSize) {
                            if (av_audio_fifo_read(outputContainer->fifo, (void**)outputAudioFrame->data, frameSize) < frameSize) {
                                THROW_ERROR("Failed to read from FIFO queue.");
                            }

                            outputAudioFrame->pts = audioPts;
                            audioPts += frameSize;
                            encode(outputContainer, outputAudioFrame);
                        }
                    }
                }
            }
        }

        av_packet_unref(&packet);
    }
    av_packet_unref(&packet);

    av_frame_free(&outputAudioFrame);
    av_frame_free(&frame);

    for (const auto& kvp: _streamPairs) {
        encode(kvp.second.second.get(), nullptr);
    }

    const auto ret = av_write_trailer(_outputContext);
    if (ret < 0) {
        THROW_ERROR("Failed to write trailer: " << ret << std::endl);
    }

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
    return metadata;
}

std::unique_ptr<InputStreamContainer> VodClipper::handleInputStream(AVStream* stream) const {
    auto container = std::make_unique<InputStreamContainer>();

    const AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);
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
    const auto isVideo = (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO);
    auto container = std::make_unique<OutputStreamContainer>();
    std::vector<std::string> codecsToTry;
    AVCodec* codec = nullptr;
    if (isVideo) {
        codecsToTry = {
            "h264_amf",
            "h264_nvenc",
            "libopenh264"
        };
    } else {
        codecsToTry = { "aac" };
    }

    unsigned int codecTag = 0;
    for (const auto& c: codecsToTry) {
        try {
            codec = avcodec_find_encoder_by_name(c.c_str());
            if (!codec) {
                THROW_ERROR("Failed to find encoder.");
            }

            container->codecContext = avcodec_alloc_context3(codec);
            if (!container->codecContext) {
                THROW_ERROR("Failed to alloc encoder context for output stream.");
            }

            AVDictionary *options = nullptr;
            if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                container->codecContext->width = icontainer.codecContext->width;
                container->codecContext->height = icontainer.codecContext->height;
                container->codecContext->sample_aspect_ratio = icontainer.codecContext->sample_aspect_ratio;
                container->codecContext->pix_fmt = icontainer.codecContext->pix_fmt;
                container->codecContext->time_base = av_inv_q(icontainer.codecContext->framerate);
                container->codecContext->bit_rate = icontainer.codecContext->bit_rate;
                container->codecContext->rc_max_rate = container->codecContext->bit_rate;
                container->codecContext->rc_buffer_size = container->codecContext->rc_max_rate * 2;
                container->codecContext->gop_size = container->codecContext->time_base.den * 5;

                if (c == "h264_amf") {
                    container->codecContext->max_b_frames = 0;
                } else {
                    container->codecContext->max_b_frames = 3;
                }

                av_dict_set(&options, "preset", "medium", 0);
                if (c == "h264_amf") {
                    av_dict_set(&options, "qp_i", "28", 0);
                    av_dict_set(&options, "qp_p", "28", 0);
                    av_dict_set(&options, "qp_b", "28", 0);
                    av_dict_set(&options, "rc", "cqp", 0);
                } else if (c == "libopenh264") {
                    container->codecContext->qmin = 15;
                    container->codecContext->qmax = 28;
                    av_dict_set(&options, "rc_mode", "quality", 0);
                } else if (c == "h264_nvenc") {
                    av_dict_set(&options, "qp", "28", 0);
                    av_dict_set(&options, "rc", "constqp", 0);
                }
            } else {
                container->codecContext->sample_rate = icontainer.codecContext->sample_rate;
                container->codecContext->channel_layout = icontainer.codecContext->channel_layout;
                container->codecContext->channels = av_get_channel_layout_nb_channels(container->codecContext->channel_layout);
                container->codecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
                container->codecContext->time_base = AVRational{1, container->codecContext->sample_rate};
            }

            codecTag = container->codecContext->codec_tag;
            if (_request.fullCopy && avcodec_parameters_to_context(container->codecContext, stream->codecpar) < 0) {
                THROW_ERROR("Failed to copy stream parameters to codec context.");
            }

            if (_outputFormat->flags & AVFMT_GLOBALHEADER) {
                container->codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            }

            if (avcodec_open2(container->codecContext, codec, &options) < 0) {
                av_dict_free(&options);
                THROW_ERROR("Failed to open output codec.");
            }
            av_dict_free(&options);
            break;
        } catch (std::exception& ex) {
            LOG_INFO("Failed to load codec: " << c << " :: " << ex.what() << std::endl);
            if (container->codecContext) {
                avcodec_free_context(&container->codecContext);
                container->codecContext = nullptr;
            }
        }
    }

    if (!container->codecContext) {
        THROW_ERROR("Failed to find appropriate codec.");
    }

    AVStream* ostream = avformat_new_stream(_outputContext, nullptr);
    if (avcodec_parameters_from_context(ostream->codecpar, container->codecContext) < 0) {
        THROW_ERROR("Failed to copy context parameters to encoding stream.")
    }
    ostream->codecpar->codec_tag = codecTag;

    if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
        container->swr = swr_alloc_set_opts(nullptr,
            container->codecContext->channel_layout, // out_ch_layout
            container->codecContext->sample_fmt, // out_sample_fmt
            container->codecContext->sample_rate, // out_sample_rate
            icontainer.codecContext->channel_layout, // in_ch_layout
            icontainer.codecContext->sample_fmt, // in_sample_fmt
            icontainer.codecContext->sample_rate, // in_sample_rate,
            0, // log_offset,
            nullptr // log_ctx
        );
        if (!container->swr) {
            THROW_ERROR("Failed to alloc swr and set options.");
        }

        if (swr_init(container->swr) < 0) {
            THROW_ERROR("Failed to init swr.");
        }

        container->fifo = av_audio_fifo_alloc(
            container->codecContext->sample_fmt,
            container->codecContext->channels,
            container->codecContext->frame_size * 2
        );

        if (!container->fifo) {
            THROW_ERROR("Failed to create audio fifo");
        }

        container->maxSamples = icontainer.codecContext->frame_size ? av_rescale_rnd(icontainer.codecContext->frame_size, container->codecContext->sample_rate, icontainer.codecContext->sample_rate, AV_ROUND_UP) : 48000;
        if (av_samples_alloc_array_and_samples(
            &container->samplesStorage,
            &container->sampleLineSize,
            container->codecContext->channels,
            static_cast<int>(container->maxSamples),
            container->codecContext->sample_fmt, 0) < 0) {
            THROW_ERROR("Failed to allocate converted samples.");
        }
    }

    ostream->time_base = container->codecContext->time_base;
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
    clip.fullCopy = false;
    clip.inputFormat = obj.value("inputFormat", "mp4");
    clip.audio = obj.value("audio", true);
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