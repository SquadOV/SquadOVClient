#include "renderer/d3d11_renderer.h"
#include "shared/errors/error.h"

#ifdef _WIN32
namespace service::renderer {

D3d11SharedContext::D3d11SharedContext(ID3D11Device* device, ID3D11DeviceContext* context):
    _device(device),
    _context(context) {
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
}

void D3d11SharedContext::execute(ID3D11CommandList* list) {
    std::lock_guard<std::mutex> guard(_contextMutex);
    _context->ExecuteCommandList(list, false);
}

D3d11SharedContext* getSharedD3d11Context() {
    static D3d11SharedContextPtr ptr = [](){
        ID3D11Device* device = nullptr;
        ID3D11DeviceContext* context = nullptr;

        UINT flags = 
#ifndef NDEBUG
            D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT
#else
            D3D11_CREATE_DEVICE_BGRA_SUPPORT
#endif
        ;

        HRESULT hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            flags,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &device,
            nullptr,
            &context
        );

        if (hr != S_OK) {
            THROW_ERROR("Failed to create D3D11 device: " << hr);
        }

        auto ctx = std::make_unique<D3d11SharedContext>(device, context);
        return ctx;
    }();
    return ptr.get();
}

D3d11Renderer::D3d11Renderer() {
    _shared = getSharedD3d11Context();
    
    // Use a deferred context so multiple renderers can render at the same time.
    HRESULT hr = _shared->device()->CreateDeferredContext(0, &_context);
    if (hr != S_OK) {
        THROW_ERROR("Failed to create deferred context.");
    }

    _shader = loadShaderFromDisk(std::filesystem::path("simple.vs.fxc"), std::filesystem::path("simple.ps.fxc"));
    if (!_shader) {
        THROW_ERROR("Failed to load shader.");
    }
    _shader->initialize(_shared->device());
}

D3d11Renderer::~D3d11Renderer() {
    _shader.reset();
    _models.clear();

    if (_context) {
        _context->Release();
        _context = nullptr;
    }
    
    if (_state) {
        _state->Release();
        _state = nullptr;
    }
}

void D3d11Renderer::refreshRasterizerState() {
    if (_state) {
        _state->Release();
        _state = nullptr;
    }

    D3D11_RASTERIZER_DESC desc = { 0 };
    desc.FillMode = D3D11_FILL_SOLID;
    desc.CullMode = D3D11_CULL_NONE;
    desc.FrontCounterClockwise = false;
    
    if (_shared->device()->CreateRasterizerState(&desc, &_state) != S_OK) {
        THROW_ERROR("Failed to create rasterizer state.");
    }
}

void D3d11Renderer::initializeRenderer(size_t width, size_t height) {
    _viewport.TopLeftX = 0.f;
    _viewport.TopLeftY = 0.f;
    _viewport.Width = static_cast<float>(width);
    _viewport.Height = static_cast<float>(height);
    _viewport.MinDepth = 0.f;
    _viewport.MaxDepth = 1.f;
    refreshRasterizerState();
}

void D3d11Renderer::addModelToScene(const D3d11ModelPtr& model) {
    if (!model) {
        LOG_WARNING("Trying to add Invalid model to scene - ignored." << std::endl);
        return;
    }
    _models.push_back(model);
}

ID3D11Texture2D* D3d11Renderer::createTexture2D(const D3D11_TEXTURE2D_DESC& desc) {
    ID3D11Texture2D* texture = nullptr;
    HRESULT hr = _shared->device()->CreateTexture2D(&desc, nullptr, &texture);
    if (hr != S_OK) {
        return nullptr;
    }
    return texture;
}

ID3D11RenderTargetView* D3d11Renderer::createRenderTarget(ID3D11Resource* resource, const D3D11_RENDER_TARGET_VIEW_DESC& desc) {
    ID3D11RenderTargetView* view = nullptr;
    HRESULT hr = _shared->device()->CreateRenderTargetView(resource, &desc, &view);
    if (hr != S_OK) {
        return nullptr;
    }
    return view;
}

bool D3d11Renderer::renderSceneToRenderTarget( ID3D11RenderTargetView* target) {
    if (!target) {
        LOG_WARNING("Received nullptr for render target." << std::endl);
        return false;
    }

    std::lock_guard<std::mutex> guard(_renderMutex);
    _context->OMSetRenderTargets(1, &target, nullptr);
    const float clearColor[4] = { 0.f, 1.f, 0.f, 1.f };
    _context->ClearRenderTargetView(target, clearColor);
    return renderScene();
}

bool D3d11Renderer::renderScene() {
    _context->RSSetViewports(1, &_viewport);
    _context->RSSetState(_state);

    // Stupid simple rendering loop because our use-cases aren't super fancy at the moment.
    for (const auto& model : _models) {
        // Load the model's vertex/index buffers.
        model->render(_context);

        // Setup shader
        _shader->render(_context);

        if (model->hasTexture()) {
            _shader->setTexture(_context, 0, model->texture());
        }

        // Render
        _context->DrawIndexed(model->numIndices(), 0, 0);
    }

    ID3D11CommandList* list = nullptr;
    HRESULT hr = _context->FinishCommandList(false, &list);
    if (hr != S_OK) {
        THROW_ERROR("Failed to finish command list.");
    }
    _shared->execute(list);
    list->Release();
    return true;
}

}
#endif