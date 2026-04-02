#pragma once

#include <memory>
#include <string>

#if defined(DJROOFRAT_ENABLE_GRAPHICS)

#if defined(DJROOFRAT_OPENGL_MIGRATION)
    #include <glad/glad.h>
    #include <glm/glm.hpp>
#endif

#if defined(_WIN32) && !defined(DJROOFRAT_OPENGL_MIGRATION)
    #include <d3d11.h>
    #include <d3dcompiler.h>
    #include <wrl.h>
    using Microsoft::WRL::ComPtr;
#endif

namespace dj {

class ComputeShader {
public:
    ComputeShader();
    ~ComputeShader();

#if defined(DJROOFRAT_OPENGL_MIGRATION)
    // OpenGL compute shader API
    
    // Compile compute shader from source
    bool compileSource(const char* source, std::string* errorOut = nullptr);
    
    // Get shader program handle
    GLuint getProgram() const { return shaderProgram_; }
    
    // Use this compute shader
    void use() const;
    
    // Bind shader storage buffer
    void bindSSBO(GLuint ssbo, GLuint bindingPoint);
    
    // Dispatch compute shader
    // groupCountX/Y/Z: number of thread groups in each dimension
    void dispatch(uint32_t groupCountX, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);

private:
    GLuint shaderProgram_ = 0;
    
    bool compileComputeShader(const char* source, GLuint& outShader, std::string* errorOut);
    
#else
    // DirectX 11 compute shader API
    
    // Compile compute shader from file
    bool compile(ID3D11Device* device, const std::string& shaderName, 
                 const std::string& entryPoint, std::string* errorOut = nullptr);

    // Get compiled compute shader
    ID3D11ComputeShader* getComputeShader() const { return computeShader_.Get(); }

    // Dispatch compute shader
    void dispatch(ID3D11DeviceContext* context, uint32_t groupCountX, 
                  uint32_t groupCountY = 1, uint32_t groupCountZ = 1);

private:
    ComPtr<ID3DBlob> computeShaderBlob_;
    ComPtr<ID3D11ComputeShader> computeShader_;
#endif
};

} // namespace dj

#else

namespace dj {
class ComputeShader {
public:
    ComputeShader() = default;
    ~ComputeShader() = default;
    bool compile(void*, const std::string&,
                 const std::string&, std::string* = nullptr) { return false; }
    void* getComputeShader() const { return nullptr; }
    void dispatch(void*, uint32_t, uint32_t = 1,
                  uint32_t = 1) {}
};
} // namespace dj

#endif
