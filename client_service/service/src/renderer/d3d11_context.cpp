#include "renderer/d3d11_context.h"
#include "shared/errors/error.h"

#ifdef _WIN32
#pragma comment(lib, "dxgi")
#include <VersionHelpers.h>
namespace service::renderer {

D3d11ImmediateContextGuard::D3d11ImmediateContextGuard(std::unique_lock<std::recursive_mutex>&& guard, ID3D11DeviceContext* context):
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

D3d11SharedContext::D3d11SharedContext(size_t flags, HMONITOR monitor) {
    UINT deviceFlags = 
#ifndef NDEBUG
        D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT
#else
        D3D11_CREATE_DEVICE_BGRA_SUPPORT
#endif
    ;

    if (flags & CONTEXT_FLAG_USE_ST) {
        deviceFlags |= D3D11_CREATE_DEVICE_SINGLETHREADED;
    }

    std::vector<D3D_FEATURE_LEVEL> features = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    if (IsWindows8OrGreater() && (flags & CONTEXT_FLAG_USE_D3D11_1)) {
        features.push_back(D3D_FEATURE_LEVEL_11_1);
    }

    HRESULT hr = S_OK;
    UINT lastAdapterIndex = 0;
    while (!_device) {
        IDXGIFactory1* factory = nullptr;
        IDXGIAdapter1* adapter = nullptr;
        IDXGIOutput* dxgiOutput = nullptr;

        if (monitor != NULL) {
            hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&factory));
            if (hr != S_OK) {
                THROW_ERROR("Failed to create DXGI factory: " << hr);
            }

            while (factory->EnumAdapters1(lastAdapterIndex++, &adapter) != DXGI_ERROR_NOT_FOUND) {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                std::wstring strDesc(desc.Description);
                LOG_INFO("Found DXGI Adapter: " << strDesc << std::endl
                    << "\tDedicated Video Memory: " << desc.DedicatedVideoMemory << std::endl
                    << "\tDedicated System Memory: " << desc.DedicatedSystemMemory << std::endl
                    << "\tShared System Memory: " << desc.SharedSystemMemory << std::endl
                );
                
                DXGI_OUTPUT_DESC outputDesc;
                UINT outputIndex = 0;
                while (adapter->EnumOutputs(outputIndex++, &dxgiOutput) != DXGI_ERROR_NOT_FOUND) {
                    hr = dxgiOutput->GetDesc(&outputDesc);
                    if (hr != S_OK) {
                        continue;
                    }

                    if (outputDesc.Monitor == monitor) {
                        break;
                    }

                    dxgiOutput->Release();
                    dxgiOutput = nullptr;
                }

                // Found an adapter that has the desired monitor output.
                if (dxgiOutput) {
                    break;
                }
            }

            factory->Release();
        }

        try {
            hr = D3D11CreateDevice(
                adapter,
                adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,
                nullptr,
                deviceFlags,
                features.data(),
                features.size(),
                D3D11_SDK_VERSION,
                &_device,
                nullptr,
                &_context
            );

            if (adapter) {
                adapter->Release();
            }

            if (hr != S_OK) {
                if (dxgiOutput) {
                    dxgiOutput->Release();
                }
                THROW_ERROR("Failed to create D3D11 device: " << hr);
            }

            hr = _device->QueryInterface(__uuidof(ID3D11Device1), (void**)&_device1);
            if (hr != S_OK) {
                LOG_WARNING("No ID3D11Device1 available - Windows 7?" << std::endl);
                _device1 = nullptr;
            }

            if (flags & CONTEXT_FLAG_VERIFY_DUPLICATE_OUTPUT) {
                IDXGIOutput1* output1 = nullptr;
                hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), (void**)&output1);
                if (hr != S_OK) {
                    dxgiOutput->Release();
                    THROW_ERROR("Failed to get IDXGIOutput1.");
                }

                IDXGIOutputDuplication* dupl = nullptr;
                hr = output1->DuplicateOutput(_device, &dupl);
                output1->Release();
                if (hr != S_OK) {
                    THROW_ERROR("Duplicate output failure: " << hr);
                }
                dupl->Release();
            }

            if (dxgiOutput) {
                dxgiOutput->Release();
            }
        } catch (std::exception& ex) {
            LOG_WARNING("Failed to create D3D11 device or verify its use: " << ex.what() << std::endl);
            if (_device) {
                _device->Release();
                _device = nullptr;
            }

            if (_device1) {
                _device1->Release();
                _device1 = nullptr;
            }
            continue;
        }
    }

    if (!_device) {
        THROW_ERROR("Failed to create D3D11 shared context.");
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
    std::unique_lock<std::recursive_mutex> lock(_contextMutex);
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