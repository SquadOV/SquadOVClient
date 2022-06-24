#pragma once

#ifdef _WIN32
#include <windows.h>
#include <d3d11.h>
#include <directxmath.h>

#include <atomic>
#include <memory>
#include <optional>
#include <vector>

namespace service::renderer {

enum class TextureResizeMode {
    Naive,
    Bicubic,
    Multisample
};

struct D3d11Vertex {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 tex;
    DirectX::XMFLOAT4 color;
};

class D3d11Model {
public:
    D3d11Model(
        ID3D11Device* device,
        const std::vector<D3d11Vertex>& vertices,
        const std::vector<unsigned int>& indices
    );
    ~D3d11Model();

    void render(ID3D11DeviceContext* context);
    void setTexture(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Texture2D* texture, bool noCopy = false);
    void getTextureDims(float& width, float& height);

    void clearXform();
    void setScale(DirectX::XMFLOAT3 scale);
    void setTranslation(DirectX::XMFLOAT3 delta);
    void setZRotation(float degrees);

    bool hasTexture() const { return _shaderResource; }
    ID3D11ShaderResourceView* texture() const { return _shaderResource; }

    unsigned int numIndices() const { return _numIndices; }
    DirectX::XMMATRIX getModelXform() const { return _modelXform; }

    void setVisibility(bool v) { _visible = v; }
    bool visible() const { return _visible; }

    void setResizeMode(TextureResizeMode mode) { _resize = mode; };
    TextureResizeMode resize() const { return _resize; }

    void setOpacity(float v);
    float opacity() const { return _opacity; }
    bool useOpacity() const { return _useOpacity; }

private:
    size_t _numVertices = 0;
    size_t _numIndices = 0;
    float _opacity = 1.f;
    bool _useOpacity = false;
    DirectX::XMMATRIX _modelXform;
    TextureResizeMode _resize = TextureResizeMode::Naive;

    ID3D11Buffer* _vertexBuffer = nullptr;
    ID3D11Buffer* _indexBuffer = nullptr;
    std::atomic<bool> _visible = true;

    void freeShaderResource();

    // We create a duplicate texture of the input to make sure it has the correct bind flags
    // and that the data is fully copied. We do not own _srcTexture.
    ID3D11Texture2D* _shaderTexture = nullptr;
    ID3D11ShaderResourceView* _shaderResource = nullptr;
    D3D11_TEXTURE2D_DESC _cachedShaderDesc;
};

using D3d11ModelPtr = std::shared_ptr<D3d11Model>;

D3d11ModelPtr createFullScreenQuad(ID3D11Device* device);

// pos and size are given in terms of percentages assuming the screen goes from [0, 1] in both X and Y.
D3d11ModelPtr createSolidColorQuad(ID3D11Device* device, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color);

}

#endif