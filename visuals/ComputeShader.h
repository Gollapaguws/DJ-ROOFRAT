#pragma once

#include <memory>
#include <string>

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace dj {

class ComputeShader {
public:
    ComputeShader();
    ~ComputeShader();

    // Compile compute shader from file
    // shaderName: shader name (e.g., "particles" will load shaders/particles.hlsl)
    // entryPoint: function name in HLSL (e.g., "CSMain")
    // errorOut: optional error message output
    bool compile(ID3D11Device* device, const std::string& shaderName, 
                 const std::string& entryPoint, std::string* errorOut = nullptr);

    // Get compiled compute shader
    ID3D11ComputeShader* getComputeShader() const { return computeShader_.Get(); }

    // Dispatch compute shader
    // groupCountX/Y/Z: number of thread groups in each dimension
    void dispatch(ID3D11DeviceContext* context, uint32_t groupCountX, 
                  uint32_t groupCountY = 1, uint32_t groupCountZ = 1);

private:
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
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
    bool compile(void* device, const std::string& shaderName, 
                 const std::string& entryPoint, std::string* errorOut = nullptr) { return false; }
    void* getComputeShader() const { return nullptr; }
    void dispatch(void* context, uint32_t groupCountX, uint32_t groupCountY = 1, 
                  uint32_t groupCountZ = 1) {}
};
} // namespace dj

#endif
