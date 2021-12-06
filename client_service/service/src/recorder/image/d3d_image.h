#pragma once

#ifdef _WIN32

#include <Windows.h>
#include <d3d11.h>
#include <memory>
#include <unordered_map>
#include "recorder/image/image.h"
#include "renderer/d3d11_renderer.h"
#include "renderer/d3d11_context.h"
#include "renderer/d3d11_texture.h"

namespace service::recorder::image {

class D3dImageRendererBundle {
public:
    D3dImageRendererBundle(service::renderer::D3d11SharedContext* inputContext, service::renderer::D3d11SharedContext* outputContext, ID3D11Texture2D* output, bool sameDevice);
    ~D3dImageRendererBundle();

    // Assume that the input image is on the same device as the shared context.
    void render(ID3D11Texture2D* image, DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_IDENTITY);
    void forceNoCopy() { _forceNoCopy = true; }

    service::renderer::D3d11Renderer* renderer() { return _renderer.get(); }
    ID3D11DeviceContext* context() const { return _context; }
private:
    ID3D11Texture2D* rawOutputTexture() const;

    service::renderer::D3d11SharedContext* _shared = nullptr;
    service::renderer::D3d11RendererPtr _renderer;
    service::renderer::D3d11ModelPtr _fullscreenQuad;
    service::renderer::SharedD3d11TextureHandlePtr _sharedOutput;
    ID3D11Texture2D* _outputTexture = nullptr;
    ID3D11RenderTargetView* _renderTarget = nullptr;
    ID3D11DeviceContext* _context = nullptr;
    bool _sameDevice = false;
    bool _forceNoCopy = false;
};

using D3dImageRendererBundlePtr = std::unique_ptr<D3dImageRendererBundle>;

class D3dImage {
public:
    explicit D3dImage(service::renderer::D3d11SharedContext* shared);
    ~D3dImage();

    bool isInit() const { return _init; };
    void initializeImage(size_t width, size_t height, bool shared = false);
    void copyFromCpu(const Image& image);
    void copyFromGpu(ID3D11Texture2D* image, DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_IDENTITY);
    void copyFromSharedGpu(service::renderer::D3d11SharedContext* imageContext, ID3D11Texture2D* image, DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_IDENTITY);

    size_t width() const { return _width; }
    size_t height() const { return _height; }

    Image cpuImage();
    ID3D11Texture2D* rawTexture() const { return _hwTexture; }
    service::renderer::D3d11SharedContext* context() const { return _shared; }
private:
    service::renderer::D3d11SharedContext* _shared = nullptr;
    bool _init = false;
    size_t _width = 0;
    size_t _height = 0;

    std::unordered_map<service::renderer::D3d11SharedContext*, D3dImageRendererBundlePtr> _renderBundles;
    D3dImageRendererBundle* getRendererBundle(service::renderer::D3d11SharedContext* context);

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

using D3dImagePtr = std::unique_ptr<D3dImage>;

}
#endif