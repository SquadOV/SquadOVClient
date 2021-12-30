#include "renderer/d3d11_video_processor.h"
#include "shared/errors/error.h"

#ifdef _WIN32

namespace service::renderer {

D3d11VideoProcessor::D3d11VideoProcessor(D3d11SharedContext* shared):
    _shared(shared) {

    HRESULT hr = _shared->device()->QueryInterface(__uuidof(ID3D11VideoDevice), (void**)&_vdevice);
    if (hr != S_OK) {
        THROW_ERROR("Failed to query video device.");
    }
}

D3d11VideoProcessor::~D3d11VideoProcessor() {
    freeOutputViews();
    freeProcessorIfExists();

    if (_vdevice) {
        _vdevice->Release();
        _vdevice = nullptr;
    }
}

bool D3d11VideoProcessor::isSupported(size_t width, size_t height) const {
    // Technically we probably need to separate out the input/output resolutions here
    // but here's to hoping that this is good enough for detecting support.
    D3D11_VIDEO_PROCESSOR_CONTENT_DESC desc;
    desc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
    desc.InputFrameRate = { 1, 1 };
    desc.InputWidth = width;
    desc.InputHeight = height;
    desc.OutputFrameRate = { 1, 1 };
    desc.OutputWidth = width;
    desc.OutputHeight = height;
    desc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

    ID3D11VideoProcessorEnumerator* penum = nullptr;
    HRESULT hr = _vdevice->CreateVideoProcessorEnumerator(&desc, &penum);
    if (hr != S_OK) {
        return false;
    }

    UINT outputFlags = D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT;
    hr = penum->CheckVideoProcessorFormat(DXGI_FORMAT_NV12, &outputFlags);
    if (hr != S_OK) {
        LOG_WARNING("No support for input NV12." << std::endl);
        penum->Release();
        return false;
    }

    UINT inputFlags = D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_INPUT;
    hr = penum->CheckVideoProcessorFormat(DXGI_FORMAT_B8G8R8A8_UNORM, &inputFlags);
    if (hr != S_OK) {
        LOG_WARNING("No support for input BGRA." << std::endl);
        penum->Release();
        return false;
    }

    penum->Release();
    return true;
}

void D3d11VideoProcessor::freeProcessorIfExists() {
    if (_processorEnum) {
        _processorEnum->Release();
        _processorEnum = nullptr;
    }

    if (_processor) {
        _processor->Release();
        _processor = nullptr;
    }
}

void D3d11VideoProcessor::freeOutputViews() {
    for (auto& kvp: _outputViews) {
        kvp.second->Release();
    }
    _outputViews.clear();
}

void D3d11VideoProcessor::process(ID3D11Texture2D* input, ID3D11Texture2D* output) {
    D3D11_TEXTURE2D_DESC inputDesc;
    input->GetDesc(&inputDesc);

    D3D11_TEXTURE2D_DESC outputDesc;
    output->GetDesc(&outputDesc);

    // Note that this is queries off the input shared context's immediate context.
    // Therefore when we use it we need to grab the shared context's immediate context guard first.
    auto immediate = _shared->immediateContext();
    ID3D11VideoContext* vcontext = nullptr;
    HRESULT hr = immediate.context()->QueryInterface(__uuidof(ID3D11VideoContext), (void**)&vcontext);
    if (hr != S_OK) {
        THROW_ERROR("Failed to query video context.");
    }

    bool needRecreate = false;
    if (_processor) {
        needRecreate =
            (inputDesc.Width != _inputDesc.Width) ||
            (inputDesc.Height != _inputDesc.Height) ||
            (inputDesc.Format != _inputDesc.Format) ||
            (outputDesc.Width != _outputDesc.Width) ||
            (outputDesc.Height != _outputDesc.Height) ||
            (outputDesc.Format != _outputDesc.Format);
    } else {
        needRecreate = true;
    }

    if (needRecreate) {
        freeOutputViews();
        freeProcessorIfExists();

        D3D11_VIDEO_PROCESSOR_CONTENT_DESC desc;
        desc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
        desc.InputFrameRate = { 1, 1 };
        desc.InputWidth = inputDesc.Width;
        desc.InputHeight = inputDesc.Height;
        desc.OutputFrameRate = { 1, 1 };
        desc.OutputWidth = outputDesc.Width;
        desc.OutputHeight = outputDesc.Height;
        desc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

        HRESULT hr = _vdevice->CreateVideoProcessorEnumerator(&desc, &_processorEnum);
        if (hr != S_OK) {
            vcontext->Release();
            THROW_ERROR("Failed to create video processor enumerator.");
        }

        hr = _vdevice->CreateVideoProcessor(_processorEnum, 0, &_processor);
        if (hr != S_OK) {
            vcontext->Release();
            THROW_ERROR("Failed to create video processor.");
        }

        if (_outputCsp != AVCOL_SPC_UNSPECIFIED) {
            if (_outputCsp == AVCOL_SPC_BT709) {
                auto immediate = _shared->immediateContext();

                D3D11_VIDEO_PROCESSOR_COLOR_SPACE csp = { 0 };
                csp.YCbCr_Matrix = 1;

                vcontext->VideoProcessorSetOutputColorSpace(_processor, &csp);
            } else {
                vcontext->Release();
                THROW_ERROR("Invalid YUV colorspace.");
            }
        }
    }

    _inputDesc = inputDesc;
    _outputDesc = outputDesc;

    // Try to reuse video processor outputs as much as possible.
    auto viewIt = _outputViews.find(output);
    ID3D11VideoProcessorOutputView* outputView = nullptr;
    if (viewIt == _outputViews.end()) {
        D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC desc;
        desc.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = 0;

        HRESULT hr = _vdevice->CreateVideoProcessorOutputView(output, _processorEnum, &desc, &outputView);
        if (hr != S_OK) {
            vcontext->Release();
            THROW_ERROR("Failed to create processor output view.");
        }

        _outputViews[output] = outputView;
    } else {
        outputView = viewIt->second;
    }

    // Create the input stream. We can maybe try to save this too since our use case we know that we'll be limited on what
    // textures we send in.
    ID3D11VideoProcessorInputView* inputView = nullptr;
    D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC desc;
    desc.FourCC = 0;
    desc.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
    desc.Texture2D.ArraySlice = 0;
    desc.Texture2D.MipSlice = 0;

    hr = _vdevice->CreateVideoProcessorInputView(input, _processorEnum, &desc, &inputView);
    if (hr != S_OK) {
        vcontext->Release();
        THROW_ERROR("Failed to create processor input view.");
    }

    D3D11_VIDEO_PROCESSOR_STREAM inputStream = { 0 };
    inputStream.Enable = true;
    inputStream.pInputSurface = inputView;

    hr = vcontext->VideoProcessorBlt(_processor, outputView, 0, 1, &inputStream);
    if (hr != S_OK) {
        inputView->Release();
        vcontext->Release();
        THROW_ERROR("Failed to do video processor blt.");
    }

    inputView->Release();
    vcontext->Release();

    immediate.context()->Flush();
}

}

#endif