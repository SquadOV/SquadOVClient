#pragma once

#ifdef _WIN32

#include <Windows.h>
#include <d3d11.h>
#include "recorder/image/image.h"
#include "renderer/d3d11_renderer.h"

namespace service::recorder::image {

class D3dImage {
public:
    explicit D3dImage(service::renderer::D3d11SharedContext* shared);
    ~D3dImage();

    bool isInit() const { return _init; };
    void initializeImage(size_t width, size_t height);
    void copyFromCpu(const Image& image);
    void copyFromGpu(ID3D11Texture2D* image, DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_IDENTITY);

    size_t width() const { return _width; }
    size_t height() const { return _height; }

    Image cpuImage();
    ID3D11Texture2D* rawTexture() const { return _hwTexture; }
private:
    service::renderer::D3d11RendererPtr _renderer;
    service::renderer::D3d11ModelPtr _fullscreenQuad;

    service::renderer::D3d11SharedContext* _shared = nullptr;
    ID3D11DeviceContext* _context = nullptr;

    ID3D11RenderTargetView* _renderTarget = nullptr;
    bool _init = false;
    size_t _width = 0;
    size_t _height = 0;

    void releaseTextureIfExists();
    ID3D11Texture2D* _hwTexture = nullptr;

    ID3D11Texture2D* createStagingTexture(size_t width, size_t height, bool forCpu);

    void releaseInputTextureIfExists();
    void refreshInputTexture(size_t width, size_t height);
    ID3D11Texture2D* _inputTexture = nullptr;

    void releaseOutputTextureIfExists();
    void refreshOutputTexture(size_t width, size_t height);
    ID3D11Texture2D* _outputTexture = nullptr;
};

}
#endif