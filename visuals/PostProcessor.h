#pragma once

#include <memory>

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#include <wrl.h>
#include "visuals/RenderTarget.h"
#include "visuals/Shader.h"

using Microsoft::WRL::ComPtr;

namespace dj {

// PostProcessor: Multi-pass rendering pipeline (bloom, color grading)
class PostProcessor {
public:
    PostProcessor();
    ~PostProcessor() = default;

    // Initialize post-processor with device and render target dimensions
    // Returns true if successful
    bool initialize(ID3D11Device* device, ID3D11DeviceContext* context,
                    int width, int height);

    // Apply bloom effect (two-pass Gaussian blur)
    // intensity: bloom brightness [0, 1]
    // Returns true if successful
    bool applyBloom(float intensity);

    // Apply color grading based on mood
    // mood: 0=Unimpressed (neutral), 1=Calm (blue), 2=Grooving (green), 3=Hyped (red)
    // Returns true if successful
    bool applyColorGrade(int mood);

    // Get intermediate render targets for multi-pass rendering
    RenderTarget* getBloomTarget() noexcept { return bloomTarget_.get(); }
    RenderTarget* getTempTarget() noexcept { return tempTarget_.get(); }

    int getWidth() const noexcept { return width_; }
    int getHeight() const noexcept { return height_; }

private:
    ID3D11Device* device_;
    ID3D11DeviceContext* context_;

    int width_;
    int height_;

    // Render targets for multi-pass rendering
    std::unique_ptr<RenderTarget> bloomTarget_;
    std::unique_ptr<RenderTarget> tempTarget_;

    // Shaders for post-processing
    std::unique_ptr<Shader> bloomShader_;
    std::unique_ptr<Shader> colorGradeShader_;

    // Fullscreen quad geometry for post-processing
    ComPtr<ID3D11Buffer> fullscreenQuadVB_;
    ComPtr<ID3D11Buffer> fullscreenQuadIB_;
    ComPtr<ID3D11InputLayout> inputLayout_;

    // Constant buffers for shader parameters
    ComPtr<ID3D11Buffer> bloomConstantBuffer_;
    ComPtr<ID3D11Buffer> colorGradeConstantBuffer_;

    // Sampler state for texture sampling
    ComPtr<ID3D11SamplerState> linearSampler_;

    // Helper to create fullscreen quad geometry
    bool createFullscreenQuad();

    // Helper to update and bind constant buffer
    bool updateBloomConstantBuffer(float textureWidth, float textureHeight, 
                                   float blurDirX, float blurDirY, float intensity);
    bool updateColorGradeConstantBuffer(int mood);

    // Bloom Gaussian weights (from specification)
    static constexpr float BLOOM_WEIGHTS[5] = {
        0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f
    };

    // Color grade LUTs for different moods
    struct MoodColor {
        float r, g, b;
    };

    static constexpr MoodColor MOOD_COLORS[4] = {
        {1.0f, 1.0f, 1.0f},   // Unimpressed: neutral
        {0.8f, 0.9f, 1.2f},   // Calm: blue tint
        {0.9f, 1.1f, 0.9f},   // Grooving: green tint
        {1.2f, 0.8f, 0.8f}    // Hyped: red tint
    };
};

}  // namespace dj

#else  // Not graphics enabled

namespace dj {

class PostProcessor {
public:
    PostProcessor() {}
    ~PostProcessor() = default;
    bool initialize(void*, void*, int, int) { return false; }
    bool applyBloom(float) { return false; }
    bool applyColorGrade(int) { return false; }
    void* getBloomTarget() noexcept { return nullptr; }
    void* getTempTarget() noexcept { return nullptr; }
    int getWidth() const noexcept { return 0; }
    int getHeight() const noexcept { return 0; }
};

}  // namespace dj

#endif
