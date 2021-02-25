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

    _modelXform = XMMatrixIdentity();
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

void D3d11Model::setTexture(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Texture2D* texture) {
    bool needRecreate = false;
    D3D11_TEXTURE2D_DESC textureDesc;
    texture->GetDesc(&textureDesc);

    if (_shaderTexture) {
        D3D11_TEXTURE2D_DESC shaderDesc;
        _shaderTexture->GetDesc(&shaderDesc);

        needRecreate = 
            (textureDesc.Width != shaderDesc.Width) ||
            (textureDesc.Height != shaderDesc.Height) ||
            (textureDesc.Format != shaderDesc.Format);
    } else {
        needRecreate = true;
    }

    if (needRecreate) {
        freeShaderResource();

        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.MiscFlags = 0;
        textureDesc.CPUAccessFlags = 0;

        HRESULT hr = device->CreateTexture2D(&textureDesc, nullptr, &_shaderTexture);
        if (hr != S_OK) {
            THROW_ERROR("Failed to create shader texture.");
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC desc;
        desc.Format = textureDesc.Format;
        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MostDetailedMip = 0;
        desc.Texture2D.MipLevels = 1;

        hr = device->CreateShaderResourceView(_shaderTexture, &desc, &_shaderResource);
        if (hr != S_OK) {
            THROW_ERROR("Failed to create shader resource view.");
        }
    }
    context->CopyResource(_shaderTexture, texture);
}

void D3d11Model::setZRotation(float degrees) {
    _modelXform = XMMatrixRotationZ(degrees * M_PI / 180.f);
}

D3d11ModelPtr createFullScreenQuad(ID3D11Device* device) {
    // Top left, top right, bottom left, bottom right
    const std::vector<D3d11Vertex> vertices = {
        D3d11Vertex{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.f, 0.f)},
        D3d11Vertex{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.f, 0.f)},
        D3d11Vertex{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.f, 1.f)},
        D3d11Vertex{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.f, 1.f)}
    };
    const std::vector<unsigned int> indices = {
        0, 1, 2,
        1, 3, 2
    };
    return std::make_shared<D3d11Model>(device, vertices, indices);
}

}

#endif