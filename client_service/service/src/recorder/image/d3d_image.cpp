#include "recorder/image/d3d_image.h"
#include "shared/errors/error.h"

namespace service::recorder::image {

D3dImage::D3dImage(service::renderer::D3d11SharedContext* shared):
    _shared(shared),
    _renderer(new service::renderer::D3d11Renderer(shared)) {

    _context = _shared->deferredContext();
    _fullscreenQuad = service::renderer::createFullScreenQuad(_shared->device());
    _renderer->addModelToScene(_fullscreenQuad);
}

D3dImage::~D3dImage() {
    if (_context) {
        _context->Release();
        _context = nullptr;
    }
    releaseTextureIfExists();
    releaseOutputTextureIfExists();
    releaseInputTextureIfExists();
}

void D3dImage::releaseTextureIfExists() {
    if (_renderTarget) {
        _renderTarget->Release();
        _renderTarget = nullptr;
    }

    if (_hwTexture) {
        _hwTexture->Release();
        _hwTexture = nullptr;
    }
}

void D3dImage::initializeImage(size_t width, size_t height) {
    releaseTextureIfExists();

    _width = width;
    _height = height;
    _renderer->initializeRenderer(_width, _height);

    D3D11_TEXTURE2D_DESC sharedDesc = { 0 };
    sharedDesc.Width = width;
    sharedDesc.Height = height;
    sharedDesc.MipLevels = 1;
    sharedDesc.ArraySize = 1;
    sharedDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sharedDesc.SampleDesc.Count = 1;
    sharedDesc.Usage = D3D11_USAGE_DEFAULT;
    sharedDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
    sharedDesc.CPUAccessFlags = 0;
    sharedDesc.MiscFlags = 0;

    _hwTexture = _renderer->createTexture2D(sharedDesc);
    if (!_hwTexture) {
        THROW_ERROR("Failed to create D3d image.");
    }

    // We now need to setup this texture as the render target as we'll be rendering
    // the input images to a quad and rendering it to this texture. This way we'll
    // get ez-pz GPU-based resizing.
    D3D11_RENDER_TARGET_VIEW_DESC targetDesc;
    targetDesc.Format = sharedDesc.Format;
    targetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    targetDesc.Texture2D.MipSlice = 0;

    _renderTarget = _renderer->createRenderTarget(_hwTexture, targetDesc);
    if (!_renderTarget) {
        THROW_ERROR("Failed to create render target.");
    }

    _init = true;
}

void D3dImage::copyFromCpu(const Image& image) {
    refreshInputTexture(image.width(), image.height());

    D3D11_BOX box;
    box.left = 0;
    box.right = image.width();
    box.top = 0;
    box.bottom = image.height();
    box.front = 0;
    box.back = 1;
    _context->UpdateSubresource(_inputTexture, 0, &box, image.buffer(), image.numBytesPerRow(), 0);

    copyFromGpu(_inputTexture);
}

void D3dImage::copyFromGpu(ID3D11Texture2D* image, DXGI_MODE_ROTATION rotation) {
    if (!_renderTarget) {
        LOG_WARNING("No render target to render to...ignoring." << std::endl);
        return;
    }

    // First check to see if we can just do a regular old copy.
    // For this to happen we need resolution and format to be the same as well
    // as the input specified rotation to be identity.
    D3D11_TEXTURE2D_DESC imageDesc;
    image->GetDesc(&imageDesc);

    D3D11_TEXTURE2D_DESC textureDesc;
    _hwTexture->GetDesc(&textureDesc);

    const bool canCopy = 
        (imageDesc.Width == textureDesc.Width) &&
        (imageDesc.Height == textureDesc.Height) &&
        (imageDesc.Format == textureDesc.Format) &&
        (rotation == DXGI_MODE_ROTATION_IDENTITY || rotation == DXGI_MODE_ROTATION_UNSPECIFIED);

    if (canCopy) {
        auto immediate = _shared->immediateContext();
        immediate.context()->CopyResource(_hwTexture, image);
    } else {
        _fullscreenQuad->setTexture(_shared->device(), _context, image);

        switch (rotation) {
        case DXGI_MODE_ROTATION_ROTATE90:
            _fullscreenQuad->setZRotation(90.f);
            break;
        case DXGI_MODE_ROTATION_ROTATE180:
            _fullscreenQuad->setZRotation(180.f);
            break;
        case DXGI_MODE_ROTATION_ROTATE270:
            _fullscreenQuad->setZRotation(270.f);
            break;
        default:
            _fullscreenQuad->setZRotation(0.f);
            break;
        }

        // Need to do a pre-emptive execute here since we use a separate deferred context
        // from the renderer's deferred context to load up the input texture.
        _shared->execute(_context);

        if (!_renderer->renderSceneToRenderTarget(_renderTarget)) {
            LOG_ERROR("Failed to render textured quad to render target..." << std::endl);
            return;
        }
    }
}

ID3D11Texture2D* D3dImage::createStagingTexture(size_t width, size_t height, bool forCpu) {
    D3D11_TEXTURE2D_DESC sharedDesc = { 0 };
    sharedDesc.Width = width;
    sharedDesc.Height = height;
    sharedDesc.MipLevels = 1;
    sharedDesc.ArraySize = 1;
    sharedDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sharedDesc.SampleDesc.Count = 1;
    sharedDesc.Usage = forCpu ? D3D11_USAGE_STAGING : D3D11_USAGE_DEFAULT;
    sharedDesc.BindFlags = 0;
    sharedDesc.CPUAccessFlags = forCpu ? D3D11_CPU_ACCESS_READ : 0;
    sharedDesc.MiscFlags = 0;

    ID3D11Texture2D* texture = nullptr;
    HRESULT hr = _shared->device()->CreateTexture2D(&sharedDesc, nullptr, &texture);
    if (hr != S_OK) {
        THROW_ERROR("Failed to create staging texture: " << hr);
    }
    return texture;
}

void D3dImage::releaseInputTextureIfExists() {
    if (_inputTexture) {
        _inputTexture->Release();
        _inputTexture = nullptr;
    }
}

void D3dImage::refreshInputTexture(size_t width, size_t height) {
    bool needsRefresh = true;
    if (_inputTexture) {
        D3D11_TEXTURE2D_DESC desc;
        _inputTexture->GetDesc(&desc);
        needsRefresh = (desc.Width != width || desc.Height != height);
    }

    if (!needsRefresh) {
        return;
    }

    releaseInputTextureIfExists();
    _inputTexture = createStagingTexture(width, height, false);
}

void D3dImage::releaseOutputTextureIfExists() {
    if (_outputTexture) {
        _outputTexture->Release();
        _outputTexture = nullptr;
    }
}

void D3dImage::refreshOutputTexture(size_t width, size_t height) {
    bool needsRefresh = true;
    if (_outputTexture) {
        D3D11_TEXTURE2D_DESC desc;
        _outputTexture->GetDesc(&desc);
        needsRefresh = (desc.Width != width || desc.Height != height);
    }

    if (!needsRefresh) {
        return;
    }

    releaseOutputTextureIfExists();
    _outputTexture = createStagingTexture(width, height, true);
}

Image D3dImage::cpuImage() {
    Image retImage;
    retImage.initializeImage(_width, _height);
    refreshOutputTexture(_width, _height);

    auto immediate = _shared->immediateContext();
    immediate.context()->CopyResource(_outputTexture, _hwTexture);

    retImage.loadFromD3d11Texture(immediate.context(), _outputTexture);
    return retImage;
}

}