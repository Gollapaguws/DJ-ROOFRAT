#include "visuals/ComputeShader.h"

#include <fstream>
#include <iterator>

namespace dj {

ComputeShader::ComputeShader() = default;

ComputeShader::~ComputeShader() = default;

bool ComputeShader::compile(ID3D11Device* device, const std::string& shaderName,
                            const std::string& entryPoint, std::string* errorOut) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!device) {
        if (errorOut) *errorOut = "Device is null";
        return false;
    }

    // Load shader source from file
    std::string filePath = "shaders/" + shaderName + ".hlsl";
    
    std::ifstream shaderFile(filePath, std::ios::binary);
    if (!shaderFile.is_open()) {
        if (errorOut) {
            *errorOut = "Failed to open compute shader file: " + filePath;
        }
        return false;
    }

    // Read entire file into string
    std::string hlslSource((std::istreambuf_iterator<char>(shaderFile)),
                           std::istreambuf_iterator<char>());
    shaderFile.close();

    if (hlslSource.empty()) {
        if (errorOut) {
            *errorOut = "Compute shader file is empty: " + filePath;
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

    // Compile compute shader (cs_5_0 target)
    HRESULT hr = D3DCompile(
        hlslSource.c_str(),
        hlslSource.size(),
        filePath.c_str(),
        nullptr,
        nullptr,
        entryPoint.c_str(),
        "cs_5_0",
        compileFlags,
        0,
        computeShaderBlob_.GetAddressOf(),
        &errorBlob
    );

    if (FAILED(hr)) {
        if (errorOut && errorBlob) {
            *errorOut = static_cast<const char*>(errorBlob->GetBufferPointer());
            errorBlob->Release();
        } else if (errorOut) {
            *errorOut = "Compute shader compilation failed for: " + filePath;
        }
        return false;
    }

    // Create compute shader
    hr = device->CreateComputeShader(
        computeShaderBlob_->GetBufferPointer(),
        computeShaderBlob_->GetBufferSize(),
        nullptr,
        computeShader_.GetAddressOf()
    );

    if (FAILED(hr)) {
        if (errorOut) {
            *errorOut = "Failed to create compute shader object";
        }
        return false;
    }

    return true;
#else
    (void)device;
    (void)shaderName;
    (void)entryPoint;
    if (errorOut) {
        *errorOut = "Compute shader compilation not available (graphics disabled)";
    }
    return false;
#endif
}

void ComputeShader::dispatch(ID3D11DeviceContext* context, uint32_t groupCountX,
                             uint32_t groupCountY, uint32_t groupCountZ) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!context || !computeShader_) {
        return;
    }

    // Set compute shader
    context->CSSetShader(computeShader_.Get(), nullptr, 0);
    
    // Dispatch thread groups
    context->Dispatch(groupCountX, groupCountY, groupCountZ);
    
    // Unbind resources after dispatch
    ID3D11UnorderedAccessView* nullUAV = nullptr;
    context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
    context->CSSetShader(nullptr, nullptr, 0);
#endif
}

} // namespace dj
