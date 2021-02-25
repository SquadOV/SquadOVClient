#pragma once

#include <memory>
#include <unordered_map>

#ifdef _WIN32
#include "renderer/d3d11_context.h"

namespace service::renderer {

class D3d11VideoProcessor {
public:
    explicit D3d11VideoProcessor(D3d11SharedContext* shared);
    ~D3d11VideoProcessor();

    void process(ID3D11Texture2D* input, ID3D11Texture2D* output);
private:
    D3d11SharedContext* _shared = nullptr;

    ID3D11VideoDevice* _vdevice = nullptr;
    // Note that this is queries off the input shared context's immediate context.
    // Therefore when we use it we need to grab the shared context's immediate context guard first.
    ID3D11VideoContext* _vcontext = nullptr;

    D3D11_TEXTURE2D_DESC _inputDesc;
    D3D11_TEXTURE2D_DESC _outputDesc;

    void freeProcessorIfExists();
    ID3D11VideoProcessor* _processor = nullptr;
    ID3D11VideoProcessorEnumerator* _processorEnum = nullptr;

    void freeOutputViews();
    std::unordered_map<ID3D11Texture2D*, ID3D11VideoProcessorOutputView*> _outputViews;
};

using D3d11VideoProcessorPtr = std::unique_ptr<D3d11VideoProcessor>;

}

#endif