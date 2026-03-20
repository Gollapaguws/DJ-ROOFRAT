#pragma once

#include <memory>
#include <vector>

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;
#endif

namespace dj {

class Shader;
class VertexBuffer;
class IndexBuffer;

/// Enhanced3DScene manages advanced 3D visual effects synchronized to music
/// Features:
/// - Dynamic lighting that pulses with beats
/// - Music-reactive particle bursts
/// - Procedural geometry effects (waveforms, tunnels)
/// - Post-processing effects (bloom, color grading)
class Enhanced3DScene {
public:
    Enhanced3DScene();
    ~Enhanced3DScene();

    // Initialize with D3D11 device
    bool initialize(ID3D11Device* device, ID3D11DeviceContext* context);

    // Update scene state based on music
    void update(float bpm, float energy, float beatPhase, float deltaTime);

    // Render enhanced 3D scene
    void render(const float* viewMatrix, const float* projMatrix);

    // Enable/disable specific effects
    void setDynamicLighting(bool enabled) { dynamicLightingEnabled_ = enabled; }
    void setParticleBursts(bool enabled) { particleBurstsEnabled_ = enabled; }
    void setTunnelEffect(bool enabled) { tunnelEffectEnabled_ = enabled; }
    void setBloomEffect(bool enabled) { bloomEnabled_ = enabled; }

    // Get current feature states
    bool hasDynamicLighting() const { return dynamicLightingEnabled_; }
    bool hasParticleBursts() const { return particleBurstsEnabled_; }
    bool hasTunnelEffect() const { return tunnelEffectEnabled_; }
    bool hasBloom() const { return bloomEnabled_; }

private:
    // Feature flags
    bool dynamicLightingEnabled_ = true;
    bool particleBurstsEnabled_ = true;
    bool tunnelEffectEnabled_ = false;
    bool bloomEnabled_ = false;

    // Music state
    float currentBPM_ = 120.0f;
    float currentEnergy_ = 0.5f;
    float beatPhase_ = 0.0f;
    float timeAccumulator_ = 0.0f;

    // Beat detection
    float lastBeatTime_ = 0.0f;
    float lastBeatPhase_ = 0.0f;  // Track previous beatPhase for wrap detection
    float beatInterval_ = 0.5f;

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    ID3D11Device* device_ = nullptr;
    ID3D11DeviceContext* context_ = nullptr;

    // Enhanced shaders
    std::unique_ptr<Shader> enhancedShader_;
    std::unique_ptr<Shader> particleShader_;
    std::unique_ptr<Shader> tunnelShader_;

    // Particle system
    struct ParticleEmitter {
        float position[3];
        float intensity;
        float lastEmitTime;
    };
    std::vector<ParticleEmitter> emitters_;
    ComPtr<ID3D11Buffer> particleBuffer_;
    int maxParticles_ = 10000;
    
    // Dynamic lights
    struct DynamicLight {
        float position[3];
        float color[3];
        float intensity;
        float radius;
    };
    std::vector<DynamicLight> dynamicLights_;
    ComPtr<ID3D11Buffer> lightBuffer_;

    // Tunnel effect geometry
    std::unique_ptr<VertexBuffer> tunnelVertexBuffer_;
    std::unique_ptr<IndexBuffer> tunnelIndexBuffer_;

    // Post-processing
    ComPtr<ID3D11Texture2D> bloomTexture_;
    ComPtr<ID3D11RenderTargetView> bloomRTV_;
    ComPtr<ID3D11ShaderResourceView> bloomSRV_;

    // Helper methods
    void updateDynamicLights();
    void emitParticleBurst(const float* position, float intensity);
    void renderTunnel();
    void applyBloom();
    
    bool createParticleBuffers();
    bool createTunnelGeometry();
    bool createBloomResources();
#endif
};

} // namespace dj
