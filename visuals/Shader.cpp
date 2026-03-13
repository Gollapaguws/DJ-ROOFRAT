#include "visuals/Shader.h"

#include <fstream>
#include <iterator>

namespace dj {

Shader::Shader() = default;

Shader::~Shader() = default;

bool Shader::compile(const std::string& entryPoint, const std::string& target, std::string* errorOut) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Simple HLSL shader code embedded as string
    const char* hlslSource = R"(
cbuffer ConstantBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
};

struct VS_INPUT {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
};

struct PS_INPUT {
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
};

PS_INPUT VSMain(VS_INPUT input) {
    PS_INPUT output;
    float4 worldPos = mul(float4(input.Position, 1.0), World);
    float4 viewPos = mul(worldPos, View);
    output.Position = mul(viewPos, Projection);
    output.Normal = mul(input.Normal, (float3x3)World);
    return output;
}

float4 PSMain(PS_INPUT input) : SV_TARGET {
    float3 lightDir = normalize(float3(1, 1, -1));
    float ndotl = max(dot(normalize(input.Normal), lightDir), 0.0);
    return float4(ndotl, ndotl, ndotl, 1.0);
}
    )";

    ID3DBlob* errorBlob = nullptr;

    // Determine which shader to compile
    const char* entryPoint_c = entryPoint.c_str();
    const char* target_c = target.c_str();

    uint32_t compileFlags = 0;
#ifdef _DEBUG
    compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

    // Compile vertex shader
    if (target == "vs_5_0") {
        HRESULT hr = D3DCompile(
            hlslSource,
            strlen(hlslSource),
            "shader.hlsl",
            nullptr,
            nullptr,
            entryPoint_c,
            target_c,
            compileFlags,
            0,
            vertexShaderBlob_.GetAddressOf(),
            &errorBlob
        );

        if (FAILED(hr)) {
            if (errorOut && errorBlob) {
                *errorOut = static_cast<const char*>(errorBlob->GetBufferPointer());
                errorBlob->Release();
            } else if (errorOut) {
                *errorOut = "Vertex shader compilation failed";
            }
            return false;
        }

        return true;
    }
    // Compile pixel shader
    else if (target == "ps_5_0") {
        HRESULT hr = D3DCompile(
            hlslSource,
            strlen(hlslSource),
            "shader.hlsl",
            nullptr,
            nullptr,
            entryPoint_c,
            target_c,
            compileFlags,
            0,
            pixelShaderBlob_.GetAddressOf(),
            &errorBlob
        );

        if (FAILED(hr)) {
            if (errorOut && errorBlob) {
                *errorOut = static_cast<const char*>(errorBlob->GetBufferPointer());
                errorBlob->Release();
            } else if (errorOut) {
                *errorOut = "Pixel shader compilation failed";
            }
            return false;
        }

        return true;
    }

    if (errorOut) {
        *errorOut = "Unknown shader target";
    }
    return false;
#else
    if (errorOut) {
        *errorOut = "Shader compilation not available (graphics disabled)";
    }
    return false;
#endif
}

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
bool Shader::createShaders(ID3D11Device* device) {
    if (!device) {
        return false;
    }

    // Compile both vertex and pixel shaders
    std::string error;
    if (!compile("VSMain", "vs_5_0", &error)) {
        return false;
    }
    if (!compile("PSMain", "ps_5_0", &error)) {
        return false;
    }

    return createVertexShader(device) && createPixelShader(device);
}

bool Shader::createPixelShader(ID3D11Device* device) {
    if (!device || !pixelShaderBlob_) {
        return false;
    }

    HRESULT hr = device->CreatePixelShader(
        pixelShaderBlob_->GetBufferPointer(),
        pixelShaderBlob_->GetBufferSize(),
        nullptr,
        pixelShader_.GetAddressOf()
    );

    return SUCCEEDED(hr);
}

bool Shader::createVertexShader(ID3D11Device* device) {
    if (!device || !vertexShaderBlob_) {
        return false;
    }

    HRESULT hr = device->CreateVertexShader(
        vertexShaderBlob_->GetBufferPointer(),
        vertexShaderBlob_->GetBufferSize(),
        nullptr,
        vertexShader_.GetAddressOf()
    );

    return SUCCEEDED(hr);
}

#endif // defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)

bool Shader::compile(const std::string& shaderName, const std::string& entryPoint, const std::string& target, std::string* errorOut) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Load shader source from external .hlsl file
    std::string filePath = "shaders/" + shaderName + ".hlsl";
    
    std::ifstream shaderFile(filePath, std::ios::binary);
    if (!shaderFile.is_open()) {
        if (errorOut) {
            *errorOut = "Failed to open shader file: " + filePath;
        }
        return false;
    }

    // Read entire file into string
    std::string hlslSourceStr((std::istreambuf_iterator<char>(shaderFile)),
                              std::istreambuf_iterator<char>());
    shaderFile.close();

    if (hlslSourceStr.empty()) {
        if (errorOut) {
            *errorOut = "Shader file is empty: " + filePath;
        }
        return false;
    }

    ID3DBlob* errorBlob = nullptr;
    uint32_t compileFlags = 0;
#ifdef _DEBUG
    compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

    if (target == "ps_5_0") {
        HRESULT hr = D3DCompile(
            hlslSourceStr.c_str(),
            hlslSourceStr.size(),
            filePath.c_str(),
            nullptr,
            nullptr,
            entryPoint.c_str(),
            target.c_str(),
            compileFlags,
            0,
            pixelShaderBlob_.GetAddressOf(),
            &errorBlob
        );

        if (FAILED(hr)) {
            if (errorOut && errorBlob) {
                *errorOut = static_cast<const char*>(errorBlob->GetBufferPointer());
                errorBlob->Release();
            } else if (errorOut) {
                *errorOut = "Pixel shader compilation failed for: " + filePath;
            }
            return false;
        }

        return true;
    } else if (target == "vs_5_0") {
        HRESULT hr = D3DCompile(
            hlslSourceStr.c_str(),
            hlslSourceStr.size(),
            filePath.c_str(),
            nullptr,
            nullptr,
            entryPoint.c_str(),
            target.c_str(),
            compileFlags,
            0,
            vertexShaderBlob_.GetAddressOf(),
            &errorBlob
        );

        if (FAILED(hr)) {
            if (errorOut && errorBlob) {
                *errorOut = static_cast<const char*>(errorBlob->GetBufferPointer());
                errorBlob->Release();
            } else if (errorOut) {
                *errorOut = "Vertex shader compilation failed for: " + filePath;
            }
            return false;
        }

        return true;
    }

    if (errorOut) {
        *errorOut = "Unknown shader target: " + target;
    }
    return false;
#else
    (void)shaderName;
    (void)entryPoint;
    (void)target;
    if (errorOut) {
        *errorOut = "Shader compilation not available (graphics disabled)";
    }
    return false;
#endif
}

} // namespace dj
