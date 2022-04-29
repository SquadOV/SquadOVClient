#include "renderer/d3d11_context.h"
#include "shared/errors/error.h"

#ifdef _WIN32
#pragma comment(lib, "dxgi")
#include <VersionHelpers.h>
#include <wil/com.h>
namespace service::renderer {

D3d11ImmediateContextGuard::D3d11ImmediateContextGuard(std::unique_lock<std::recursive_mutex>&& guard, const wil::com_ptr<ID3D11DeviceContext>& context):
    _guard(std::move(guard)),
    _context(context) {
}

D3d11ImmediateContextGuard::D3d11ImmediateContextGuard(D3d11ImmediateContextGuard&& o):
    _guard(std::move(o._guard)),
    _context(o._context) {
    o._context = nullptr;
}

D3d11ImmediateContextGuard::~D3d11ImmediateContextGuard() {
}

D3d11SharedContext::D3d11SharedContext(size_t flags, HMONITOR monitor, D3d11Device device):
    _deviceClass(device)
{
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
            if (dxgiOutput) {
                LOG_INFO("...Found monitor." << std::endl);
            } else {
                LOG_INFO("...No monitor found." << std::endl);
            }

            if (!adapter) {
                THROW_ERROR("No more adapters available.");
            }
        }
        
        try {
            hr = D3D11CreateDevice(
                adapter,
                adapter ? D3D_DRIVER_TYPE_UNKNOWN : 
                    (device == D3d11Device::GPU) ? D3D_DRIVER_TYPE_HARDWARE : D3D_DRIVER_TYPE_WARP,
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

            const auto doVerify = flags & CONTEXT_FLAG_VERIFY_DUPLICATE_OUTPUT;
            if (dxgiOutput) {
                if (doVerify) {
                    IDXGIOutput1* output1 = nullptr;
                    hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), (void**)&output1);
                    if (hr != S_OK) {
                        dxgiOutput->Release();
                        THROW_ERROR("Failed to get IDXGIOutput1.");
                    }

                    IDXGIOutputDuplication* dupl = nullptr;
                    hr = output1->DuplicateOutput(_device.get(), &dupl);
                    output1->Release();
                    if (hr != S_OK) {
                        THROW_ERROR("Duplicate output failure: " << hr);
                    }
                    dupl->Release();
                }
                dxgiOutput->Release();
            } else if (doVerify) {
                THROW_ERROR("No DXGI Output found when verification requested.");
            }
        } catch (std::exception& ex) {
            LOG_WARNING("Failed to create D3D11 device or verify its use: " << ex.what() << std::endl);
            continue;
        }
    }

    if (!_device) {
        THROW_ERROR("Failed to create D3D11 shared context.");
    }
}

D3d11SharedContext::~D3d11SharedContext() {
}

std::wstring D3d11SharedContext::adapterName() const {
    if (!_device) {
        return L"";
    }

    wil::com_ptr<IDXGIDevice> dxgiDevice;
    HRESULT hr = _device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (hr != S_OK) {
        return L"";
    }

    wil::com_ptr<IDXGIAdapter> adapter = nullptr;
    hr = dxgiDevice->GetAdapter(&adapter);
    if (hr != S_OK) {
        return L"";
    }

    DXGI_ADAPTER_DESC desc;
    adapter->GetDesc(&desc);

    return std::wstring(desc.Description);
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