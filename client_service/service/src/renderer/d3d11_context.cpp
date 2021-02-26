#include "renderer/d3d11_context.h"
#include "shared/errors/error.h"

#ifdef _WIN32
#include <VersionHelpers.h>
namespace service::renderer {

D3d11ImmediateContextGuard::D3d11ImmediateContextGuard(std::unique_lock<std::mutex>&& guard, ID3D11DeviceContext* context):
    _guard(std::move(guard)),
    _context(context) {
    _context->AddRef();
}

D3d11ImmediateContextGuard::D3d11ImmediateContextGuard(D3d11ImmediateContextGuard&& o):
    _guard(std::move(o._guard)),
    _context(o._context) {
    o._context = nullptr;
}

D3d11ImmediateContextGuard::~D3d11ImmediateContextGuard() {
    if (_context) {
        _context->Release();
        _context = nullptr;
    }
}

D3d11SharedContext::D3d11SharedContext() {
    UINT flags = 
#ifndef NDEBUG
        D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT
#else
        D3D11_CREATE_DEVICE_BGRA_SUPPORT
#endif
    ;

    std::vector<D3D_FEATURE_LEVEL> features = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    if (IsWindows8OrGreater()) {
        features.push_back(D3D_FEATURE_LEVEL_11_1);
    }

    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        features.data(),
        features.size(),
        D3D11_SDK_VERSION,
        &_device,
        nullptr,
        &_context
    );

    if (hr != S_OK) {
        THROW_ERROR("Failed to create D3D11 device: " << hr);
    }

    hr = _device->QueryInterface(__uuidof(ID3D11Device1), (void**)&_device1);
    if (hr != S_OK) {
        LOG_WARNING("No ID3D11Device1 available - Windows 7?" << std::endl);
        _device1 = nullptr;
    }
}

D3d11SharedContext::~D3d11SharedContext() {
    if (_context) {
        _context->Release();
        _context = nullptr;    
    }

    if (_device) {
        _device->Release();
        _device = nullptr;
    }

    if (_device1) {
        _device1->Release();
        _device1 = nullptr;
    }
}

D3d11ImmediateContextGuard D3d11SharedContext::immediateContext() {
    std::unique_lock<std::mutex> lock(_contextMutex);
    return D3d11ImmediateContextGuard{std::move(lock), _context};
}

ID3D11DeviceContext* D3d11SharedContext::deferredContext() {
    ID3D11DeviceContext* context = nullptr;
    HRESULT hr = _device->CreateDeferredContext(0, &context);
    if (hr != S_OK) {
        THROW_ERROR("Failed to create deferred context.");
    }
    return context;
}

void D3d11SharedContext::execute(ID3D11DeviceContext* deferredContext) {
    ID3D11CommandList* list = nullptr;
    HRESULT hr = deferredContext->FinishCommandList(false, &list);
    if (hr != S_OK) {
        THROW_ERROR("Failed to finish command list.");
    }
    execute(list);
    list->Release();
}

void D3d11SharedContext::execute(ID3D11CommandList* list) {
    auto immediate = immediateContext();
    immediate.context()->ExecuteCommandList(list, false);
}

D3d11SharedContext* getSharedD3d11Context() {
    static D3d11SharedContextPtr ptr = std::make_unique<D3d11SharedContext>();
    return ptr.get();
}

}

#endif