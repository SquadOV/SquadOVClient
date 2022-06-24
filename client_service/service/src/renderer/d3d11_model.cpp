#define _USE_MATH_DEFINES
#include <cmath>

#include "renderer/d3d11_model.h"
#include "shared/errors/error.h"

#ifdef _WIN32

#include "recorder/image/image.h"

using namespace DirectX;
namespace service::renderer {

D3d11Model::D3d11Model(
    ID3D11Device* device,
    const std::vector<D3d11Vertex>& vertices,
    const std::vector<unsigned int>& indices) {
    _numVertices = vertices.size();
    _numIndices = indices.size();

    D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(D3d11Vertex) * _numVertices;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
    vertexBufferData.pSysMem = vertices.data();

    {
        HRESULT hr = device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &_vertexBuffer);
        if (hr != S_OK) {
            THROW_ERROR("Failed to create vertex buffer.");
        }
    }

    D3D11_BUFFER_DESC indexBufferDesc = { 0 };
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned int) * _numIndices;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
    indexBufferData.pSysMem = indices.data();

    {
        HRESULT hr = device->CreateBuffer(&indexBufferDesc, &indexBufferData, &_indexBuffer);
        if (hr != S_OK) {
            THROW_ERROR("Failed to create index buffer.");
        }
    }

    clearXform();
}

D3d11Model::~D3d11Model() {
    if (_vertexBuffer) {
        _vertexBuffer->Release();
        _vertexBuffer = nullptr;
    }

    if (_indexBuffer) {
        _indexBuffer->Release();
        _indexBuffer = nullptr;
    }

    freeShaderResource();
}

void D3d11Model::render(ID3D11DeviceContext* context) {
    UINT stride = sizeof(D3d11Vertex);
    UINT offset = 0;

    context->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void D3d11Model::freeShaderResource() {
    if (_shaderResource) {
        _shaderResource->Release();
        _shaderResource = nullptr;
    }

    if (_shaderTexture) {
        _shaderTexture->Release();
        _shaderTexture = nullptr;
    }
}

void D3d11Model::getTextureDims(float& width, float& height) {
    if (!_shaderTexture) {
        return;
    }

    D3D11_TEXTURE2D_DESC shaderDesc;
    _shaderTexture->GetDesc(&shaderDesc);
    width = static_cast<float>(shaderDesc.Width);
    height = static_cast<float>(shaderDesc.Height);
}

void D3d11Model::setTexture(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Texture2D* texture, bool noCopy) {
    bool needRecreate = false;
    D3D11_TEXTURE2D_DESC textureDesc;
    texture->GetDesc(&textureDesc);

    if (_shaderTexture || noCopy) {
        if (_shaderTexture) {
            _shaderTexture->GetDesc(&_cachedShaderDesc);
        }

        needRecreate = 
            (textureDesc.Width != _cachedShaderDesc.Width) ||
            (textureDesc.Height != _cachedShaderDesc.Height) ||
            (textureDesc.Format != _cachedShaderDesc.Format);
        
        if (noCopy) {
            _cachedShaderDesc = textureDesc;
        }
    } else {
        needRecreate = true;
    }

    if (needRecreate) {
        freeShaderResource();

        if (!noCopy) {
            textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.MiscFlags = 0;
            textureDesc.CPUAccessFlags = 0;

            HRESULT hr = device->CreateTexture2D(&textureDesc, nullptr, &_shaderTexture);
            if (hr != S_OK) {
                THROW_ERROR("Failed to create shader texture.");
            }
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC desc;
        desc.Format = textureDesc.Format;
        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MostDetailedMip = 0;
        desc.Texture2D.MipLevels = 1;

        HRESULT hr = device->CreateShaderResourceView(noCopy ? texture : _shaderTexture, &desc, &_shaderResource);
        if (hr != S_OK) {
            THROW_ERROR("Failed to create shader resource view.");
        }
    }

    if (!noCopy) {
        context->CopyResource(_shaderTexture, texture);
    }
}

void D3d11Model::clearXform() {
    _modelXform = XMMatrixIdentity();
}

void D3d11Model::setScale(DirectX::XMFLOAT3 scale) {
    _modelXform = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&scale)) * _modelXform;
}

void D3d11Model::setTranslation(DirectX::XMFLOAT3 delta) {
    _modelXform = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&delta)) * _modelXform;
}

void D3d11Model::setZRotation(float degrees) {
    _modelXform = XMMatrixRotationZ(degrees * M_PI / 180.f) * _modelXform;
}

void D3d11Model::setOpacity(float v) {
    _opacity = v;
    _useOpacity = true;
}

D3d11ModelPtr createFullScreenQuad(ID3D11Device* device) {
    // Top left, top right, bottom left, bottom right
    const std::vector<D3d11Vertex> vertices = {
        D3d11Vertex{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.f, 0.f), XMFLOAT4(1.f, 1.f, 1.f, 1.f)},
        D3d11Vertex{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.f, 0.f), XMFLOAT4(1.f, 1.f, 1.f, 1.f)},
        D3d11Vertex{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.f, 1.f), XMFLOAT4(1.f, 1.f, 1.f, 1.f)},
        D3d11Vertex{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.f, 1.f), XMFLOAT4(1.f, 1.f, 1.f, 1.f)}
    };
    const std::vector<unsigned int> indices = {
        0, 1, 2,
        1, 3, 2
    };
    return std::make_shared<D3d11Model>(device, vertices, indices);
}

D3d11ModelPtr createSolidColorQuad(ID3D11Device* device, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color) {
    const float x = -1.f + 2.f * pos.x;
    const float y = 1.f - 2.f * pos.y;
    const float width = 2.f * size.x;
    const float height = -2.f * size.y;

    const std::vector<D3d11Vertex> vertices = {
        D3d11Vertex{ XMFLOAT3(x, y, 1.0f), XMFLOAT2(0.f, 0.f), color},
        D3d11Vertex{ XMFLOAT3(x + width, y, 1.0f), XMFLOAT2(1.f, 0.f), color},
        D3d11Vertex{ XMFLOAT3(x, y + height, 1.0f), XMFLOAT2(0.f, 1.f), color},
        D3d11Vertex{ XMFLOAT3(x + width, y + height, 1.0f), XMFLOAT2(1.f, 1.f), color}
    };
    const std::vector<unsigned int> indices = {
        0, 1, 2,
        1, 3, 2
    };
    return std::make_shared<D3d11Model>(device, vertices, indices);
}

}

#endif