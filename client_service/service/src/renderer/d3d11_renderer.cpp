#include "renderer/d3d11_renderer.h"
#include "shared/errors/error.h"

#ifdef _WIN32
namespace service::renderer {

D3d11Renderer::D3d11Renderer(D3d11SharedContext* shared):
    _shared(shared) {
    // Use a deferred context so multiple renderers can render at the same time.
    _context = _shared->deferredContext();
    _shader = loadShaderFromDisk(std::filesystem::path("simple.vs.fxc"), std::filesystem::path("simple.ps.fxc"));
    if (!_shader) {
        THROW_ERROR("Failed to load shader.");
    }
    _shader->initialize(_shared->device());

    D3D11_BLEND_DESC blendDesc;
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    HRESULT hr = _shared->device()->CreateBlendState(&blendDesc, &_blendState);
    if (hr != S_OK) {
        THROW_ERROR("Failed to create blend state: " << hr);
    }
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

ID3D11Device* D3d11Renderer::device() const {
    if (!_shared) {
        return nullptr;
    }
    return _shared->device();
}

ID3D11Texture2D* D3d11Renderer::createTexture2D(const D3D11_TEXTURE2D_DESC& desc) {
    ID3D11Texture2D* texture = nullptr;
    HRESULT hr = _shared->device()->CreateTexture2D(&desc, nullptr, &texture);
    if (hr != S_OK) {
        LOG_WARNING("Failed to create texture: " << hr << std::endl);
        return nullptr;
    }
    return texture;
}

ID3D11RenderTargetView* D3d11Renderer::createRenderTarget(ID3D11Resource* resource, const D3D11_RENDER_TARGET_VIEW_DESC& desc) {
    ID3D11RenderTargetView* view = nullptr;
    HRESULT hr = _shared->device()->CreateRenderTargetView(resource, &desc, &view);
    if (hr != S_OK) {
        LOG_WARNING("Failed to create render target view: " << hr << std::endl);
        return nullptr;
    }
    return view;
}

void D3d11Renderer::prepRenderTargetForRender(ID3D11DeviceContext* context, ID3D11RenderTargetView* target) {
    context->OMSetRenderTargets(1, &target, nullptr);

    FLOAT blendFactor[4] = {0.f, 0.f, 0.f, 0.f};
    context->OMSetBlendState(_blendState, blendFactor, 0xffffffff);
    if (_clearBeforeRender) {
        const float clearColor[4] = { 0.f, 1.f, 0.f, 1.f };
        context->ClearRenderTargetView(target, clearColor);
    }

    context->RSSetViewports(1, &_viewport);
    context->RSSetState(_state);
}

bool D3d11Renderer::renderSceneToRenderTarget(ID3D11RenderTargetView* target) {
    if (!target) {
        LOG_WARNING("Received nullptr for render target." << std::endl);
        return false;
    }

    std::lock_guard<std::mutex> guard(_renderMutex);
    prepRenderTargetForRender(_context, target);
 
    return renderScene();
}

bool D3d11Renderer::renderScene() {
    // Stupid simple rendering loop because our use-cases aren't super fancy at the moment.
    for (const auto& model : _models) {
        if (!model->visible()) {
            continue;
        }

        // Load the model's vertex/index buffers.
        model->render(_context);

        // Setup shader
        _shader->render(_context, model.get());

        // Render
        _context->DrawIndexed(model->numIndices(), 0, 0);
    }

    _shared->execute(_context);
    return true;
}

}
#endif