#pragma once

// Must include glad FIRST before any system GL headers
#ifdef DJROOFRAT_ENABLE_GRAPHICS
#include <glad/glad.h>
#endif

#include <string>

// Stub definitions when graphics is disabled
#ifndef DJROOFRAT_ENABLE_GRAPHICS
using GLuint = unsigned int;
using GLint = int;
#endif

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS) && !defined(DJROOFRAT_OPENGL_MIGRATION)
// DirectX 11 headers (legacy support)
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;
#endif

namespace dj {

class Shader {
public:
    Shader();
    ~Shader();

    // ========== OpenGL Methods (Phase 3+) ==========
    // Compile individual shader source (vertexSource or fragmentSource can be nullptr)
    // shaderType: "vertex" or "fragment"
    bool compile(const char* vertexSource, const char* fragmentSource, 
                 const char* shaderType, std::string* errorOut = nullptr);

    // Link compiled vertex and fragment shaders into a program
    bool link(std::string* errorOut = nullptr);

    // Use this shader program
    void use() const;

    // Get uniform location
    GLint getUniformLocation(const char* name) const;

    // Get shader program handle
    GLuint getProgram() const { return shaderProgram_; }

    // Load and compile shaders from files
    bool loadFromFiles(const char* vertexPath, const char* fragmentPath, 
                       std::string* errorOut = nullptr);

    // ========== Legacy DirectX Methods (for backward compatibility) ==========
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS) && !defined(DJROOFRAT_OPENGL_MIGRATION)
    // Compile HLSL shader code from file or string
    bool compile(const std::string& entryPoint, const std::string& target, std::string* errorOut = nullptr);

    // Compile specific named shader (for Phase 19: lighting, laser, bloom, colorgrade)
    bool compile(const std::string& shaderName, const std::string& entryPoint, const std::string& target, std::string* errorOut = nullptr);

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
    // OpenGL members (Phase 3+)
    GLuint vertexShaderGL_ = 0;
    GLuint fragmentShaderGL_ = 0;
    GLuint shaderProgram_ = 0;

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS) && !defined(DJROOFRAT_OPENGL_MIGRATION)
    // DirectX members (legacy)
    ComPtr<ID3DBlob> vertexShaderBlob_;
    ComPtr<ID3DBlob> pixelShaderBlob_;
    ComPtr<ID3D11VertexShader> vertexShader_;
    ComPtr<ID3D11PixelShader> pixelShader_;
#endif

    // Helper methods
    bool compileShaderSource(const char* source, GLenum shaderType, 
                            GLuint& outShader, std::string* errorOut = nullptr);
    std::string readFileAsString(const char* filePath, std::string* errorOut = nullptr);
};

} // namespace dj

