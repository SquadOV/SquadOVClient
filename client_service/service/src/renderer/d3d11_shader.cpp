#include "renderer/d3d11_shader.h"

#ifdef _WIN32
#include "shared/env.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"
#include "shared/filesystem/utility.h"

namespace fs = std::filesystem;
namespace service::renderer {

D3d11ShaderPtr loadShaderFromDisk(const fs::path& vertexShader, const fs::path& pixelShader) {
    const fs::path shaderBasePath(shared::getEnvW(L"SQUADOV_SHADERS"));
    if (!fs::exists(shaderBasePath)) {
        LOG_WARNING("Shader path doesn't exist: " << shaderBasePath << std::endl);
        return nullptr;
    }

    const auto vertexPath = shaderBasePath / vertexShader;
    if (!fs::exists(vertexPath)) {
        LOG_WARNING("Vertex shader does not exist: " << vertexPath << std::endl);
        return nullptr;
    }

    const auto pixelPath = shaderBasePath / pixelShader;
    if (!fs::exists(pixelPath)) {
        LOG_WARNING("Pixel shader does not exist: " << pixelPath << std::endl);
        return nullptr;
    }

    return std::make_shared<D3d11Shader>(vertexPath, pixelPath);
}

D3d11Shader::D3d11Shader(const std::filesystem::path& vertexShaderPath, const std::filesystem::path& pixelShaderPath):
    _vertexShaderPath(vertexShaderPath),
    _pixelShaderPath(pixelShaderPath) {
}

void D3d11Shader::initialize(ID3D11Device* device) {
    std::vector<char> vertexBlob;
    std::vector<char> pixelBlob;

    {
        shared::filesystem::readBinaryData(vertexBlob, _vertexShaderPath);
        HRESULT hr = device->CreateVertexShader(reinterpret_cast<const void*>(vertexBlob.data()), vertexBlob.size(), nullptr, &_vertexShader);
        if (hr != S_OK) {
            THROW_ERROR("Failed to create vertex shader: " << hr);
        }
    }

    {
        shared::filesystem::readBinaryData(pixelBlob, _pixelShaderPath);
        HRESULT hr = device->CreatePixelShader(reinterpret_cast<const void*>(pixelBlob.data()), pixelBlob.size(), nullptr, &_pixelShader);
        if (hr != S_OK) {
            THROW_ERROR("Failed to create pixel shader: " << hr);
        }
    }

    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementsDesc;
    inputElementsDesc.resize(2);
    inputElementsDesc[0].SemanticName = "POSITION";
    inputElementsDesc[0].SemanticIndex = 0;
    inputElementsDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementsDesc[0].InputSlot = 0;
    inputElementsDesc[0].AlignedByteOffset = 0;
    inputElementsDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    inputElementsDesc[0].InstanceDataStepRate = 0;

    inputElementsDesc[1].SemanticName = "TEXCOORD";
    inputElementsDesc[1].SemanticIndex = 0;
    inputElementsDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementsDesc[1].InputSlot = 0;
    inputElementsDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    inputElementsDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    inputElementsDesc[1].InstanceDataStepRate = 0;

    HRESULT hr = device->CreateInputLayout(inputElementsDesc.data(), 2, reinterpret_cast<const void*>(vertexBlob.data()), vertexBlob.size(), &_inputLayout);
    if (hr != S_OK) {
        THROW_ERROR("Failed to create input layout: " << hr);
    }

    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = device->CreateSamplerState(&samplerDesc, &_sampler);
    if (hr != S_OK) {
        THROW_ERROR("Failed to create sampler state: " << hr);
    }
}

D3d11Shader::~D3d11Shader() {
    if (_vertexShader) {
        _vertexShader->Release();
        _vertexShader = nullptr;
    }

    if (_pixelShader) {
        _pixelShader->Release();
        _pixelShader = nullptr;
    }

    if (_sampler) {
        _sampler->Release();
        _sampler = nullptr;
    }

    if (_inputLayout) {
        _inputLayout->Release();
        _inputLayout = nullptr;
    }
}

void D3d11Shader::setTexture(ID3D11DeviceContext* context, unsigned int texIndex, ID3D11ShaderResourceView* texture) {
    context->PSSetShaderResources(texIndex, 1, &texture);
}

void D3d11Shader::render(ID3D11DeviceContext* context) {
    context->IASetInputLayout(_inputLayout);
    context->VSSetShader(_vertexShader, nullptr, 0);
    context->PSSetShader(_pixelShader, nullptr, 0);
    context->PSSetSamplers(0, 1, &_sampler);
}

}

#endif