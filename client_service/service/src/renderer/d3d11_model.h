#pragma once

#ifdef _WIN32
#include <windows.h>
#include <d3d11.h>
#include <directxmath.h>
#include <memory>
#include <optional>
#include <vector>

namespace service::renderer {

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
    void setTexture(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Texture2D* texture);
    void getTextureDims(float& width, float& height);

    void setZRotation(float degrees);

    bool hasTexture() const { return _shaderResource; }
    ID3D11ShaderResourceView* texture() const { return _shaderResource; }

    unsigned int numIndices() const { return _numIndices; }
    DirectX::XMMATRIX getModelXform() const { return _modelXform; }

private:
    size_t _numVertices = 0;
    size_t _numIndices = 0;
    DirectX::XMMATRIX _modelXform;

    ID3D11Buffer* _vertexBuffer = nullptr;
    ID3D11Buffer* _indexBuffer = nullptr;

    void freeShaderResource();

    // We create a duplicate texture of the input to make sure it has the correct bind flags
    // and that the data is fully copied. We do not own _srcTexture.
    ID3D11Texture2D* _shaderTexture = nullptr;
    ID3D11ShaderResourceView* _shaderResource = nullptr;
};

using D3d11ModelPtr = std::shared_ptr<D3d11Model>;

D3d11ModelPtr createFullScreenQuad(ID3D11Device* device);

// pos and size are given in terms of percentages assuming the screen goes from [0, 1] in both X and Y.
D3d11ModelPtr createSolidColorQuad(ID3D11Device* device, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color);

}

#endif