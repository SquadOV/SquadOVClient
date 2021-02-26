#pragma once

#ifdef _WIN32
#include "renderer/d3d11_context.h"
#include <dxgi1_2.h>

namespace service::renderer {

class SharedD3d11TextureHandle {
public:
    SharedD3d11TextureHandle(D3d11SharedContext* destContext, ID3D11Texture2D* srcTexture, bool canWrite);
    ~SharedD3d11TextureHandle();

    ID3D11Texture2D* texture() const { return _sTexture; };

private:
    bool _isNtHandle = false;
    HANDLE _frameHandle = nullptr;
    IDXGIResource1* _frameResource1 = nullptr;
    IDXGIResource* _frameResource = nullptr;
    ID3D11Resource* _sResource = nullptr;
    ID3D11Texture2D* _sTexture = nullptr;
};

}
#endif