#pragma once

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;
#endif

#include <string>

namespace dj {

class Shader {
public:
    Shader();
    ~Shader();

    // Compile HLSL shader code from file or string
    bool compile(const std::string& entryPoint, const std::string& target, std::string* errorOut = nullptr);

    // Compile specific named shader (for Phase 19: lighting, laser, bloom, colorgrade)
    bool compile(const std::string& shaderName, const std::string& entryPoint, const std::string& target, std::string* errorOut = nullptr);

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Get compiled shader blobs
    ID3DBlob* getVertexShaderBlob() const { return vertexShaderBlob_.Get(); }
    ID3DBlob* getPixelShaderBlob() const { return pixelShaderBlob_.Get(); }

    // Get shader interfaces
    ID3D11VertexShader* getVertexShader() const { return vertexShader_.Get(); }
    ID3D11PixelShader* getPixelShader() const { return pixelShader_.Get(); }

    // Create pixel shader from compiled blob
    bool createPixelShader(ID3D11Device* device);

    // Create vertex shader from compiled blob
    bool createVertexShader(ID3D11Device* device);

    // Create both vertex and pixel shaders (convenience method)
    bool createShaders(ID3D11Device* device);
#endif

private:
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    ComPtr<ID3DBlob> vertexShaderBlob_;
    ComPtr<ID3DBlob> pixelShaderBlob_;
    ComPtr<ID3D11VertexShader> vertexShader_;
    ComPtr<ID3D11PixelShader> pixelShader_;
#endif
};

} // namespace dj
