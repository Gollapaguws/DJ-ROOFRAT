#include "visuals/Shader.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>

namespace dj {

Shader::Shader() = default;

Shader::~Shader() {
    // Clean up OpenGL resources
    if (shaderProgram_ != 0) {
        glDeleteProgram(shaderProgram_);
    }
    if (vertexShaderGL_ != 0) {
        glDeleteShader(vertexShaderGL_);
    }
    if (fragmentShaderGL_ != 0) {
        glDeleteShader(fragmentShaderGL_);
    }
}

// ========== OpenGL Implementation (Phase 3+) ==========

std::string Shader::readFileAsString(const char* filePath, std::string* errorOut) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        if (errorOut) {
            *errorOut = std::string("File not found: ") + filePath;
        }
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}

bool Shader::compileShaderSource(const char* source, GLenum shaderType, 
                                 GLuint& outShader, std::string* errorOut) {
    if (!source || !*source) {
        if (errorOut) {
            *errorOut = "Shader source is null or empty";
        }
        return false;
    }

    outShader = glCreateShader(shaderType);
    glShaderSource(outShader, 1, &source, nullptr);
    glCompileShader(outShader);

    // Check compilation status
    GLint compileStatus;
    glGetShaderiv(outShader, GL_COMPILE_STATUS, &compileStatus);

    if (compileStatus == GL_FALSE) {
        // Get error log
        GLint infoLogLength;
        glGetShaderiv(outShader, GL_INFO_LOG_LENGTH, &infoLogLength);

        if (infoLogLength > 0) {
            std::string infoLog(infoLogLength, '\0');
            glGetShaderInfoLog(outShader, infoLogLength, nullptr, &infoLog[0]);
            if (errorOut) {
                *errorOut = infoLog;
            }
        } else if (errorOut) {
            const char* typeStr = (shaderType == GL_VERTEX_SHADER) ? "vertex" : "fragment";
            *errorOut = std::string(typeStr) + " shader compilation failed (no info log)";
        }

        glDeleteShader(outShader);
        outShader = 0;
        return false;
    }

    return true;
}

bool Shader::compile(const char* vertexSource, const char* fragmentSource, 
                     const char* shaderType, std::string* errorOut) {
    // Compile vertex shader if source is provided
    if (vertexSource && shaderType && std::string(shaderType) == "vertex") {
        return compileShaderSource(vertexSource, GL_VERTEX_SHADER, vertexShaderGL_, errorOut);
    }

    // Compile fragment shader if source is provided
    if (fragmentSource && shaderType && std::string(shaderType) == "fragment") {
        return compileShaderSource(fragmentSource, GL_FRAGMENT_SHADER, fragmentShaderGL_, errorOut);
    }

    if (errorOut && shaderType) {
        *errorOut = std::string("Unknown shader type: ") + shaderType;
    } else if (errorOut) {
        *errorOut = "No shader source provided";
    }
    return false;
}

bool Shader::link(std::string* errorOut) {
    // Check if we have both shaders compiled
    if (vertexShaderGL_ == 0 || fragmentShaderGL_ == 0) {
        if (errorOut) {
            *errorOut = "Both vertex and fragment shaders must be compiled before linking";
        }
        return false;
    }

    // Create shader program
    shaderProgram_ = glCreateProgram();
    glAttachShader(shaderProgram_, vertexShaderGL_);
    glAttachShader(shaderProgram_, fragmentShaderGL_);
    glLinkProgram(shaderProgram_);

    // Check link status
    GLint linkStatus;
    glGetProgramiv(shaderProgram_, GL_LINK_STATUS, &linkStatus);

    if (linkStatus == GL_FALSE) {
        // Get error log
        GLint infoLogLength;
        glGetProgramiv(shaderProgram_, GL_INFO_LOG_LENGTH, &infoLogLength);

        if (infoLogLength > 0) {
            std::string infoLog(infoLogLength, '\0');
            glGetProgramInfoLog(shaderProgram_, infoLogLength, nullptr, &infoLog[0]);
            if (errorOut) {
                *errorOut = infoLog;
            }
        } else if (errorOut) {
            *errorOut = "Shader program linking failed (no info log)";
        }

        glDeleteProgram(shaderProgram_);
        shaderProgram_ = 0;
        return false;
    }

    // Successfully linked - we can delete the individual shaders now
    // (they're attached to the program and will be deleted when the program is deleted)
    glDeleteShader(vertexShaderGL_);
    glDeleteShader(fragmentShaderGL_);
    vertexShaderGL_ = 0;
    fragmentShaderGL_ = 0;

    return true;
}

void Shader::use() const {
    if (shaderProgram_ != 0) {
        glUseProgram(shaderProgram_);
    }
}

GLint Shader::getUniformLocation(const char* name) const {
    if (shaderProgram_ == 0) {
        return -1;
    }
    return glGetUniformLocation(shaderProgram_, name);
}

bool Shader::loadFromFiles(const char* vertexPath, const char* fragmentPath, 
                           std::string* errorOut) {
    // Read vertex shader source
    std::string vertexSource = readFileAsString(vertexPath, errorOut);
    if (vertexSource.empty()) {
        return false;
    }

    // Read fragment shader source
    std::string fragmentSource = readFileAsString(fragmentPath, errorOut);
    if (fragmentSource.empty()) {
        return false;
    }

    // Compile vertex shader
    if (!compile(vertexSource.c_str(), nullptr, "vertex", errorOut)) {
        return false;
    }

    // Compile fragment shader
    if (!compile(nullptr, fragmentSource.c_str(), "fragment", errorOut)) {
        return false;
    }

    // Link program
    return link(errorOut);
}

// ========== Legacy DirectX Implementation (for backward compatibility) ==========

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS) && !defined(DJROOFRAT_OPENGL_MIGRATION)

bool Shader::compile(const std::string& entryPoint, const std::string& target, std::string* errorOut) {
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
}

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

#endif // defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS) && !defined(DJROOFRAT_OPENGL_MIGRATION)

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS) && !defined(DJROOFRAT_OPENGL_MIGRATION)

bool Shader::compile(const std::string& shaderName, const std::string& entryPoint, const std::string& target, std::string* errorOut) {
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
}

#endif // defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS) && !defined(DJROOFRAT_OPENGL_MIGRATION)

} // namespace dj


