#pragma once

#include <memory>
#include <vector>

struct ID3D11Device;
struct ID3D11DeviceContext;

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;
#endif

namespace dj {

class Shader;
class VertexBuffer;
class IndexBuffer;
class TextureManager;
class TunnelGeometry;
class ShadowMap;  // Phase 5: Shadow mapping
class StageGeometry;
class DJControllerGeometry;  // Phase 2: DJ Controller UI rendering
class CrowdRenderer;  // Phase 2: Crowd visualization
class CrowdAnimator;  // Phase 2: Crowd animation

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
    void setShadowMapping(bool enabled) { shadowMappingEnabled_ = enabled; }  // Phase 5: Shadow mapping
    void setCrowdVisualization(bool enabled) { crowdVisualizationEnabled_ = enabled; }  // Phase 2: Crowd visualization

    // Get current feature states
    bool hasDynamicLighting() const { return dynamicLightingEnabled_; }
    bool hasParticleBursts() const { return particleBurstsEnabled_; }
    bool hasTunnelEffect() const { return tunnelEffectEnabled_; }
    bool hasBloom() const { return bloomEnabled_; }
    bool hasShadowMapping() const { return shadowMappingEnabled_; }  // Phase 5: Shadow mapping
    bool hasCrowdVisualization() const { return crowdVisualizationEnabled_; }  // Phase 2: Crowd visualization

    // Phase 2: Enhanced shader and material buffer methods
    bool loadEnhancedShader();
    bool createMaterialBuffer();
    bool hasCompiledShader() const;
    bool hasMaterialBuffer() const;
    float getBeatIntensity() const;
    float getEmissiveIntensity() const;
    
    // Phase 2: DJ Controller rendering
    bool hasControllerGeometry() const;
    void renderController(ID3D11DeviceContext* context, const float* viewMatrix, const float* projMatrix);
    DJControllerGeometry* getControllerGeometry() const;

private:
    // Feature flags
    bool dynamicLightingEnabled_ = true;
    bool particleBurstsEnabled_ = true;
    bool tunnelEffectEnabled_ = false;
    bool bloomEnabled_ = false;
    bool shadowMappingEnabled_ = false;  // Phase 5: Shadow mapping
    bool crowdVisualizationEnabled_ = true;  // Phase 2: Crowd visualization

    // Music state
    float currentBPM_ = 120.0f;
    float currentEnergy_ = 0.5f;
    float beatPhase_ = 0.0f;
    float timeAccumulator_ = 0.0f;

    // Beat detection
    float lastBeatTime_ = 0.0f;
    float lastBeatPhase_ = 0.0f;  // Track previous beatPhase for wrap detection
    float beatInterval_ = 0.5f;
    
    // Phase 4: Tunnel effect state
    float tunnelScrollOffset_ = 0.0f;
    float tunnelScrollSpeed_ = 0.5f;

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    ID3D11Device* device_ = nullptr;
    ID3D11DeviceContext* context_ = nullptr;

    // Material buffer structure (16-byte aligned to match HLSL)
    struct MaterialBufferData {
        float baseColor[3];        // 12 bytes: RGB base color
        float metallic;            // 4 bytes: metallic factor [0,1]
        float emissiveColor[3];    // 12 bytes: RGB emissive color
        float roughness;           // 4 bytes: roughness factor [0,1]
        float cameraPosition[3];   // 12 bytes: camera position in world space
        float bpm;                 // 4 bytes: current BPM
    };

    // Enhanced shaders
    std::unique_ptr<Shader> enhancedShader_;
    std::unique_ptr<Shader> particleShader_;
    std::unique_ptr<Shader> tunnelShader_;

    // Constant buffers
    ComPtr<ID3D11Buffer> materialBuffer_;
    ComPtr<ID3D11Buffer> tunnelBuffer_;   // Phase 4: Tunnel shader constant buffer

    // Phase 3: Texture management for UV-mapped geometry
    std::unique_ptr<TextureManager> textureManager_;

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
    std::unique_ptr<VertexBuffer> stageVertexBuffer_;
    std::unique_ptr<IndexBuffer> stageIndexBuffer_;
    std::unique_ptr<VertexBuffer> tunnelVertexBuffer_;
    std::unique_ptr<IndexBuffer> tunnelIndexBuffer_;

    // Post-processing
    ComPtr<ID3D11Texture2D> bloomTexture_;
    ComPtr<ID3D11RenderTargetView> bloomRTV_;
    ComPtr<ID3D11ShaderResourceView> bloomSRV_;

    // Phase 5: Shadow mapping
    std::unique_ptr<ShadowMap> shadowMap_;
    ComPtr<ID3D11Buffer> shadowConstantBuffer_;
    std::unique_ptr<Shader> shadowDepthShader_;

    // Phase 2: Crowd visualization state and rendering
    std::unique_ptr<CrowdRenderer> crowdRenderer_;
    std::unique_ptr<CrowdAnimator> crowdAnimator_;
    int currentMood_ = 0;  // 0-3: Unimpressed, Calm, Grooving, Hyped

    // Phase 2: DJ Controller geometry and rendering
    std::unique_ptr<DJControllerGeometry> controllerGeometry_;
    std::unique_ptr<VertexBuffer> controllerVertexBuffer_;
    std::unique_ptr<IndexBuffer> controllerIndexBuffer_;
    ComPtr<ID3D11InputLayout> controllerInputLayout_;  // Phase 1: Input layout for controller vertex structure

    // Helper methods
    void updateDynamicLights();
    void emitParticleBurst(const float* position, float intensity);
    void renderTunnel();
    void applyBloom();
    void renderShadowDepthPass();  // Phase 5: Shadow depth pass
    void renderCrowd(const float* viewMatrix, const float* projMatrix);  // Phase 2: Render crowd
    bool createControllerGeometry();  // Phase 2: Initialize controller geometry
    
    bool createParticleBuffers();
    bool createStageGeometry();
    bool createTunnelGeometry();
    bool createBloomResources();
    bool createShadowResources();  // Phase 5: Initialize shadow mapping
    bool initializeCrowdRendering();  // Phase 2: Initialize crowd rendering
#endif
};

} // namespace dj
