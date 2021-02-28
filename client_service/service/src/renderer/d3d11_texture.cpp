#include "renderer/d3d11_texture.h"
#include "shared/errors/error.h"

#ifdef _WIN32
namespace service::renderer {

SharedD3d11TextureHandle::SharedD3d11TextureHandle(D3d11SharedContext* destContext, ID3D11Texture2D* srcTexture, bool canWrite) {
    D3D11_TEXTURE2D_DESC desc;
    srcTexture->GetDesc(&desc);

    _isNtHandle = (desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED_NTHANDLE);

    HRESULT hr = S_OK;
    if (_isNtHandle) {
        hr = srcTexture->QueryInterface(__uuidof(IDXGIResource1), (void**)&_frameResource1);
        if (hr != S_OK) {
            THROW_ERROR("Failed to get frame DXGI resource (NT).");
        }

        DWORD access = DXGI_SHARED_RESOURCE_READ;
        if (canWrite) {
            access |= DXGI_SHARED_RESOURCE_WRITE;
        }

        hr = _frameResource1->CreateSharedHandle(nullptr, access, nullptr, &_frameHandle);
        if (hr != S_OK) {
            THROW_ERROR("Failed to get frame shared handle (NT).");
        }

        if (!destContext->device1()) {
            THROW_ERROR("ID3D11Device1 interface not available.");
        }

        hr = destContext->device1()->OpenSharedResource1(_frameHandle, __uuidof(ID3D11Texture2D), (void**)&_sTexture);
        if (hr != S_OK) {
            THROW_ERROR("Failed to open shared resource (NT).");
        }
    } else {
        hr = srcTexture->QueryInterface(__uuidof(IDXGIResource), (void**)&_frameResource);
        if (hr != S_OK) {
            THROW_ERROR("Failed to get frame DXGI resource.");
        }

        hr = _frameResource->GetSharedHandle(&_frameHandle);
        if (hr != S_OK) {
            THROW_ERROR("Failed to get frame shared handle.");
        }

        hr = destContext->device()->OpenSharedResource(_frameHandle, __uuidof(ID3D11Texture2D), (void**)&_sTexture);
        if (hr != S_OK) {
            THROW_ERROR("Failed to open shared resource.");
        }
    }
}

SharedD3d11TextureHandle::~SharedD3d11TextureHandle() {
    if (_frameResource) {
        _frameResource->Release();
        _frameResource = nullptr;
    }

    if (_frameResource1) {
        _frameResource1->Release();
        _frameResource1 = nullptr;
    }
    
    if (_sTexture) {
        _sTexture->Release();
        _sTexture = nullptr;
    }

    if (_isNtHandle) {
        CloseHandle(_frameHandle);
        _frameHandle = nullptr;
    }

    
}

}
#endif