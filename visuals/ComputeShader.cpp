#include "visuals/ComputeShader.h"
#include <fstream>
#include <iterator>
#include <iostream>

namespace dj {

ComputeShader::ComputeShader() = default;

ComputeShader::~ComputeShader() {
#if defined(DJROOFRAT_OPENGL_MIGRATION)
    if (shaderProgram_ != 0) {
        glDeleteProgram(shaderProgram_);
        shaderProgram_ = 0;
    }
#endif
}

#if defined(DJROOFRAT_OPENGL_MIGRATION)

bool ComputeShader::compileComputeShader(const char* source, GLuint& outShader, std::string* errorOut) {
    outShader = glCreateShader(GL_COMPUTE_SHADER);
    
    glShaderSource(outShader, 1, &source, nullptr);
    glCompileShader(outShader);
    
    GLint success = 0;
    glGetShaderiv(outShader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        GLint logLength = 0;
        glGetShaderiv(outShader, GL_INFO_LOG_LENGTH, &logLength);
        
        if (logLength > 0) {
            std::string log(logLength, '\0');
            glGetShaderInfoLog(outShader, logLength, nullptr, &log[0]);
            if (errorOut) {
                *errorOut = log;
            } else {
                std::cerr << "Compute shader error: " << log << std::endl;
            }
        }
        
        glDeleteShader(outShader);
        outShader = 0;
        return false;
    }
    
    return true;
}

bool ComputeShader::compileSource(const char* source, std::string* errorOut) {
    if (!source) {
        if (errorOut) *errorOut = "Source code is null";
        return false;
    }
    
    GLuint computeShader = 0;
    if (!compileComputeShader(source, computeShader, errorOut)) {
        return false;
    }
    
    // Create shader program
    if (shaderProgram_ != 0) {
        glDeleteProgram(shaderProgram_);
    }
    
    shaderProgram_ = glCreateProgram();
    glAttachShader(shaderProgram_, computeShader);
    glLinkProgram(shaderProgram_);
    
    GLint success = 0;
    glGetProgramiv(shaderProgram_, GL_LINK_STATUS, &success);
    
    if (!success) {
        GLint logLength = 0;
        glGetProgramiv(shaderProgram_, GL_INFO_LOG_LENGTH, &logLength);
        
        if (logLength > 0) {
            std::string log(logLength, '\0');
            glGetProgramInfoLog(shaderProgram_, logLength, nullptr, &log[0]);
            if (errorOut) {
                *errorOut = log;
            } else {
                std::cerr << "Shader program linking error: " << log << std::endl;
            }
        }
        
        glDeleteProgram(shaderProgram_);
        shaderProgram_ = 0;
        glDeleteShader(computeShader);
        return false;
    }
    
    glDeleteShader(computeShader);
    return true;
}

void ComputeShader::use() const {
    glUseProgram(shaderProgram_);
}

void ComputeShader::bindSSBO(GLuint ssbo, GLuint bindingPoint) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, ssbo);
}

void ComputeShader::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    if (shaderProgram_ == 0) {
        std::cerr << "Cannot dispatch invalid compute shader program" << std::endl;
        return;
    }
    
    glUseProgram(shaderProgram_);
    glDispatchCompute(groupCountX, groupCountY, groupCountZ);
}

#elif defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)

bool ComputeShader::compile(ID3D11Device* device, const std::string& shaderName,
                            const std::string& entryPoint, std::string* errorOut) {
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
}

void ComputeShader::dispatch(ID3D11DeviceContext* context, uint32_t groupCountX,
                             uint32_t groupCountY, uint32_t groupCountZ) {
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
}

} // namespace dj

#endif // defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
