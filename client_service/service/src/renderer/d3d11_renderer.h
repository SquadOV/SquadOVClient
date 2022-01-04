#pragma once

#ifdef _WIN32

#include <Windows.h>
#include <atlbase.h>
#include <d3d11.h>
#include <vector>
#include "renderer/d3d11_context.h"
#include "renderer/d3d11_model.h"
#include "renderer/d3d11_shader.h"

namespace service::renderer {

class D3d11Renderer {
public:
    explicit D3d11Renderer(D3d11SharedContext* shared);
    ~D3d11Renderer();

    void initializeRenderer(size_t width, size_t height);
    void clearModels() { _models.clear(); };
    void addModelToScene(const D3d11ModelPtr& model);
    ID3D11Device* device() const;

    ID3D11Texture2D* createTexture2D(const D3D11_TEXTURE2D_DESC& desc);
    ID3D11RenderTargetView* createRenderTarget(ID3D11Resource* resource, const D3D11_RENDER_TARGET_VIEW_DESC& desc);

    void prepRenderTargetForRender(ID3D11DeviceContext* context, ID3D11RenderTargetView* target);
    bool renderSceneToRenderTarget(ID3D11RenderTargetView* target);
    void noClear() { _clearBeforeRender = false; }

private:
    bool renderScene();

    D3d11SharedContext* _shared = nullptr;
    ID3D11DeviceContext* _context = nullptr;
    D3D11_VIEWPORT _viewport;
    CComPtr<ID3D11BlendState> _blendState;

    void refreshRasterizerState();
    ID3D11RasterizerState* _state = nullptr;
    std::mutex _renderMutex;

    // Scene
    std::vector<D3d11ModelPtr> _models;
    D3d11ShaderPtr _shader;

    bool _clearBeforeRender = true;
};

using D3d11RendererPtr = std::unique_ptr<D3d11Renderer>;

}

#endif