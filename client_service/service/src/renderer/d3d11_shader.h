#pragma once

#ifdef _WIN32
#include "renderer/d3d11_model.h"

#include <d3d11.h>
#include <directxmath.h>
#include <filesystem>
#include <memory>

namespace service::renderer {

struct D3d11VSShaderConstants {
    DirectX::XMMATRIX modelXform;
};

struct D3d11PSShaderConstants {
    float inWidth;
    float inHeight;
    unsigned int mode;
    float padding;
};

class D3d11Shader {
public:
    D3d11Shader(const std::filesystem::path& vertexShaderPath, const std::filesystem::path& pixelShaderPath);
    ~D3d11Shader();

    void initialize(ID3D11Device* device);
    void setTexture(ID3D11DeviceContext* context, unsigned int texIndex, ID3D11ShaderResourceView* texture);
    void render(ID3D11DeviceContext* context, D3d11Model* model);
private:
    ID3D11VertexShader* _vertexShader = nullptr;
    ID3D11PixelShader* _pixelShader = nullptr;
    ID3D11SamplerState* _sampler = nullptr;
    ID3D11InputLayout* _inputLayout = nullptr;

    ID3D11Buffer* _vsConstants = nullptr;
    ID3D11Buffer* _psConstants = nullptr;

    std::filesystem::path _vertexShaderPath;
    std::filesystem::path _pixelShaderPath;
};

using D3d11ShaderPtr = std::shared_ptr<D3d11Shader>;

D3d11ShaderPtr loadShaderFromDisk(const std::filesystem::path& vertexShader, const std::filesystem::path& pixelShader);

}

#endif