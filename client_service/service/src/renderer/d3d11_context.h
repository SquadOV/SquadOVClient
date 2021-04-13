#pragma once

#ifdef _WIN32

#include <Windows.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <memory>
#include <mutex>

namespace service::renderer {

constexpr size_t CONTEXT_FLAG_USE_D3D11_1 = 0b001;
constexpr size_t CONTEXT_FLAG_USE_ST = 0b010;
constexpr size_t CONTEXT_FLAG_VERIFY_DUPLICATE_OUTPUT = 0b100;

class D3d11ImmediateContextGuard {
public:
    friend class D3d11SharedContext;

    D3d11ImmediateContextGuard(D3d11ImmediateContextGuard&& o);
    D3d11ImmediateContextGuard(const D3d11ImmediateContextGuard& o) = delete;
    ~D3d11ImmediateContextGuard();
    ID3D11DeviceContext* context() const { return _context; }

protected:
    D3d11ImmediateContextGuard(std::unique_lock<std::recursive_mutex>&& guard, ID3D11DeviceContext* context);

private:
    std::unique_lock<std::recursive_mutex> _guard;
    ID3D11DeviceContext* _context;
};

class D3d11SharedContext {
public:

    D3d11SharedContext(size_t flags = CONTEXT_FLAG_USE_D3D11_1, HMONITOR monitor = NULL);
    ~D3d11SharedContext();

    ID3D11Device* device() const { return _device; }
    ID3D11Device1* device1() const { return _device1; }

    D3d11ImmediateContextGuard immediateContext();
    ID3D11DeviceContext* deferredContext();

    void execute(ID3D11DeviceContext* deferredContext);
    void execute(ID3D11CommandList* list);
private:
    ID3D11Device* _device = nullptr;
    ID3D11Device1* _device1 = nullptr;
    ID3D11DeviceContext* _context = nullptr;
    std::recursive_mutex _contextMutex;
};

using D3d11SharedContextPtr = std::unique_ptr<D3d11SharedContext>;
D3d11SharedContext* getSharedD3d11Context();

}

#endif