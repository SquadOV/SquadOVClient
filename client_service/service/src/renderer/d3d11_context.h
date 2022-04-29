#pragma once

#ifdef _WIN32

#include <Windows.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <memory>
#include <mutex>
#include <wil/com.h>

namespace service::renderer {

constexpr size_t CONTEXT_FLAG_USE_D3D11_1 = 0b001;
constexpr size_t CONTEXT_FLAG_USE_ST = 0b010;
constexpr size_t CONTEXT_FLAG_VERIFY_DUPLICATE_OUTPUT = 0b100;

enum class D3d11Device {
    GPU,
    CPU
};

class D3d11ImmediateContextGuard {
public:
    friend class D3d11SharedContext;

    D3d11ImmediateContextGuard(D3d11ImmediateContextGuard&& o);
    D3d11ImmediateContextGuard(const D3d11ImmediateContextGuard& o) = delete;
    ~D3d11ImmediateContextGuard();
    ID3D11DeviceContext* context() const { return _context.get(); }

protected:
    D3d11ImmediateContextGuard(std::unique_lock<std::recursive_mutex>&& guard, const wil::com_ptr<ID3D11DeviceContext>& context);

private:
    std::unique_lock<std::recursive_mutex> _guard;
    wil::com_ptr<ID3D11DeviceContext> _context;
};

class D3d11SharedContext {
public:

    D3d11SharedContext(size_t flags = CONTEXT_FLAG_USE_D3D11_1, HMONITOR monitor = NULL, D3d11Device device = D3d11Device::GPU);
    ~D3d11SharedContext();

    ID3D11Device* device() const { return _device.get(); }
    ID3D11Device1* device1() const { return _device1.get(); }

    D3d11ImmediateContextGuard immediateContext();
    ID3D11DeviceContext* deferredContext();
    D3d11Device deviceClass() const { return _deviceClass; }

    std::wstring adapterName() const;

    void execute(ID3D11DeviceContext* deferredContext);
    void execute(ID3D11CommandList* list);

private:
    wil::com_ptr<ID3D11Device> _device;
    wil::com_ptr<ID3D11Device1> _device1;
    wil::com_ptr<ID3D11DeviceContext> _context;
    std::recursive_mutex _contextMutex;
    D3d11Device _deviceClass;
};

using D3d11SharedContextPtr = std::shared_ptr<D3d11SharedContext>;
D3d11SharedContext* getSharedD3d11Context();

}

#endif