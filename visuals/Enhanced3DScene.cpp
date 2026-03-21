#include "visuals/Enhanced3DScene.h"

#include <algorithm>
#include <cmath>

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include "visuals/Shader.h"
#include "visuals/IndexBuffer.h"
#include "visuals/VertexBuffer.h"
#include "visuals/TextureManager.h"
#include "visuals/TunnelGeometry.h"
#include "visuals/ShadowMap.h"
#endif

namespace dj {

Enhanced3DScene::Enhanced3DScene()
    : dynamicLightingEnabled_(true),
      particleBurstsEnabled_(true),
      tunnelEffectEnabled_(false),
      bloomEnabled_(false),
      currentBPM_(120.0f),
      currentEnergy_(0.5f),
      beatPhase_(0.0f),
      timeAccumulator_(0.0f),
      lastBeatTime_(0.0f),
      lastBeatPhase_(0.0f),
      beatInterval_(0.5f)
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
      , device_(nullptr)
      , context_(nullptr)
#endif
{
}

Enhanced3DScene::~Enhanced3DScene() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Clean up D3D11 resources if needed
    // Note: device_ and context_ are owned by GraphicsContext, not this class
    device_ = nullptr;
    context_ = nullptr;
#endif
}

bool Enhanced3DScene::initialize(ID3D11Device* device, ID3D11DeviceContext* context) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Validate input parameters
    if (device == nullptr || context == nullptr) {
        return false;
    }

    // Store device and context pointers
    device_ = device;
    context_ = context;

    // Initialize default music state
    currentBPM_ = 120.0f;
    currentEnergy_ = 0.5f;
    beatPhase_ = 0.0f;
    timeAccumulator_ = 0.0f;
    
    // Calculate beat interval based on BPM
    beatInterval_ = 60.0f / currentBPM_; // seconds per beat

    // Phase 2: Integrate enhanced shader compilation into initialization flow
    if (!loadEnhancedShader()) {
        return false;
    }

    // Phase 2: Integrate material buffer creation into initialization flow
    if (!createMaterialBuffer()) {
        return false;
    }

    // Phase 3: Initialize texture manager with procedural checkerboard
    if (!textureManager_) {
        textureManager_ = std::make_unique<TextureManager>();
    }
    textureManager_->setDevice(device_);
    if (!textureManager_->createCheckerboard(256, 256)) {
        return false;
    }

    // Phase 4: Create tunnel geometry and shader
    if (!createTunnelGeometry()) {
        return false;
    }

    // Phase 5: Create shadow mapping resources
    if (!createShadowResources()) {
        return false;
    }

    return true;
#else
    // Graphics not available
    return false;
#endif
}

void Enhanced3DScene::update(float bpm, float energy, float beatPhase, float deltaTime) {
    // Store music state
    currentBPM_ = std::clamp(bpm, 80.0f, 200.0f);
    currentEnergy_ = std::clamp(energy, 0.0f, 1.0f);
    beatPhase_ = std::clamp(beatPhase, 0.0f, 1.0f);
    
    // Update time accumulator
    timeAccumulator_ += deltaTime;

    // Calculate beat interval from BPM
    beatInterval_ = 60.0f / currentBPM_;

    // Detect beat transitions (when beatPhase wraps from ~1.0 to ~0.0)
    // This is useful for triggering burst effects
    if (beatPhase < lastBeatPhase_ && lastBeatPhase_ > 0.9f) {
        // Beat detected (phase wrapped around)
        lastBeatTime_ = timeAccumulator_;
    }
    lastBeatPhase_ = beatPhase;
    
    // Phase 4: Update tunnel scroll offset
    tunnelScrollOffset_ += tunnelScrollSpeed_ * deltaTime;
    tunnelScrollOffset_ = fmod(tunnelScrollOffset_, 1.0f);  // Wrap to [0, 1]
}

void Enhanced3DScene::render(const float* viewMatrix, const float* projMatrix) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Validate input parameters
    if (viewMatrix == nullptr || projMatrix == nullptr) {
        return;
    }

    // Skip rendering if dynamic lighting is disabled
    if (!dynamicLightingEnabled_) {
        return;
    }

    // Skip if shader or material buffer not initialized
    if (!enhancedShader_ || !materialBuffer_) {
        return;
    }

    // Populate material buffer with current state
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    HRESULT hr = context_->Map(materialBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (SUCCEEDED(hr)) {
        struct MaterialBufferData {
            float baseColor[3];      // 12 bytes
            float metallic;          // 4 bytes (16-byte aligned, row 1)
            float emissiveColor[3];  // 12 bytes
            float roughness;         // 4 bytes (16-byte aligned, row 2)
            float cameraPosition[3]; // 12 bytes
            float bpm;               // 4 bytes (16-byte aligned, row 3)
        };

        MaterialBufferData* matData = (MaterialBufferData*)mapped.pData;
        
        // Set base color (white by default)
        matData->baseColor[0] = 1.0f;
        matData->baseColor[1] = 1.0f;
        matData->baseColor[2] = 1.0f;
        
        // Set metallic (0.2 default)
        matData->metallic = 0.2f;
        
        // Set emissive color based on beat intensity (beat-reactive)
        float beatIntensity = getBeatIntensity();
        float emissiveScale = beatIntensity * currentEnergy_;
        matData->emissiveColor[0] = 1.0f * emissiveScale;
        matData->emissiveColor[1] = 0.5f * emissiveScale;
        matData->emissiveColor[2] = 0.2f * emissiveScale;
        
        // Set roughness (0.6 default)
        matData->roughness = 0.6f;
        
        // Set camera position (default at origin for now)
        matData->cameraPosition[0] = 0.0f;
        matData->cameraPosition[1] = 0.0f;
        matData->cameraPosition[2] = 0.0f;
        
        // Set BPM
        matData->bpm = currentBPM_;

        context_->Unmap(materialBuffer_.Get(), 0);

        // Bind material buffer to pixel shader constant buffer slot 1
        context_->PSSetConstantBuffers(1, 1, materialBuffer_.GetAddressOf());
        
        // Set shaders as active
        context_->VSSetShader(enhancedShader_->getVertexShader(), nullptr, 0);
        context_->PSSetShader(enhancedShader_->getPixelShader(), nullptr, 0);
        
        // Phase 3: Bind texture SRV and sampler state to pixel shader for UV mapping
        if (textureManager_) {
            textureManager_->bind(context_, 0);  // Bind to texture slot 0
        }
        
        // TODO Phase 3: Add actual draw call with geometry
        // For now this is a stub to test material buffer binding
    }
#endif
}

// Phase 2: Load enhanced.hlsl shader
bool Enhanced3DScene::loadEnhancedShader() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (device_ == nullptr) {
        return false;
    }

    if (!enhancedShader_) {
        enhancedShader_ = std::make_unique<Shader>();
    }

    // Compile vertex shader from enhanced.hlsl
    std::string error;
    if (!enhancedShader_->compile("enhanced", "VSMain", "vs_5_0", &error)) {
        return false;
    }

    // Compile pixel shader from enhanced.hlsl
    if (!enhancedShader_->compile("enhanced", "PSMain", "ps_5_0", &error)) {
        return false;
    }

    // Create shader objects in D3D11
    return enhancedShader_->createShaders(device_);
#else
    return false;
#endif
}

// Phase 2: Create material constant buffer
bool Enhanced3DScene::createMaterialBuffer() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (device_ == nullptr) {
        return false;
    }

    // MaterialBuffer structure: 48 bytes (3 rows of 16 bytes each)
    // This matches the MaterialBuffer cbuffer in enhanced.hlsl
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = 48;  // sizeof(MaterialBuffer) with 16-byte alignment
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = device_->CreateBuffer(&desc, nullptr, materialBuffer_.GetAddressOf());
    return SUCCEEDED(hr);
#else
    return false;
#endif
}

// Phase 2: Check if shader is compiled
bool Enhanced3DScene::hasCompiledShader() const {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    return enhancedShader_ != nullptr && 
           enhancedShader_->getVertexShader() != nullptr && 
           enhancedShader_->getPixelShader() != nullptr;
#else
    return false;
#endif
}

// Phase 2: Check if material buffer exists
bool Enhanced3DScene::hasMaterialBuffer() const {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    return materialBuffer_ != nullptr;
#else
    return false;
#endif
}

// Phase 2: Calculate beat intensity from beat phase using sine wave
// Produces smooth pulsing effect: 0 at start, 1 at middle, 0 at end of beat
float Enhanced3DScene::getBeatIntensity() const {
    // Use sine wave to create smooth pulse
    // beatPhase ranges from 0 to 1, map to 0 to PI for sine
    float radian = beatPhase_ * 3.14159265359f;  // PI
    float intensity = std::sin(radian);
    
    // Clamp to [0, 1] range just in case
    return std::clamp(intensity, 0.0f, 1.0f);
}

// Phase 2: Calculate emissive intensity based on beat phase and BPM
// Produces pulsing effect synchronized to music
float Enhanced3DScene::getEmissiveIntensity() const {
    // Similar to beat intensity but can be modulated differently
    float radian = beatPhase_ * 3.14159265359f;  // PI
    float intensity = std::sin(radian);
    
    // Modulate by BPM (faster BPM = more frequent pulses)
    // For now we use the beat phase directly, but this can be enhanced
    return std::clamp(intensity, 0.0f, 1.0f);
}

// Phase 4: Create tunnel geometry with scrolling UVs
bool Enhanced3DScene::createTunnelGeometry() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (device_ == nullptr) {
        return false;
    }

    // Generate tunnel mesh (32 segments, 16 rings, 5.0 radius)
    auto tunnelGeom = std::make_unique<TunnelGeometry>();
    tunnelGeom->generate(32, 16, 5.0f);
    
    const auto& vertices = tunnelGeom->getVertices();
    const auto& indices = tunnelGeom->getIndices();
    
    if (vertices.empty() || indices.empty()) {
        return false;
    }
    
    // Create vertex buffer
    if (!tunnelVertexBuffer_) {
        tunnelVertexBuffer_ = std::make_unique<VertexBuffer>();
    }
    if (!tunnelVertexBuffer_->create(device_, vertices.data(), static_cast<uint32_t>(vertices.size()), sizeof(Vertex))) {
        return false;
    }
    
    // Create index buffer
    if (!tunnelIndexBuffer_) {
        tunnelIndexBuffer_ = std::make_unique<IndexBuffer>();
    }
    if (!tunnelIndexBuffer_->create(device_, indices.data(), static_cast<uint32_t>(indices.size()))) {
        return false;
    }
    
    // Create tunnel shader
    if (!tunnelShader_) {
        tunnelShader_ = std::make_unique<Shader>();
    }
    
    std::string error;
    if (!tunnelShader_->compile("tunnel", "VSMain", "vs_5_0", &error)) {
        return false;
    }
    if (!tunnelShader_->compile("tunnel", "PSMain", "ps_5_0", &error)) {
        return false;
    }
    if (!tunnelShader_->createShaders(device_)) {
        return false;
    }
    
    // Create tunnel constant buffer
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = 80;  // 64 (matrix) + 4 (scrollOffset) + 4 (beatIntensity) + 8 (padding)
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    HRESULT hr = device_->CreateBuffer(&desc, nullptr, tunnelBuffer_.GetAddressOf());
    return SUCCEEDED(hr);
#else
    return false;
#endif
}

// Phase 4: Render tunnel with scrolling UVs and beat distortion
void Enhanced3DScene::renderTunnel() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!tunnelEffectEnabled_ || !tunnelShader_ || !tunnelVertexBuffer_ || 
        !tunnelIndexBuffer_ || !tunnelBuffer_) {
        return;
    }
    
    // Update tunnel constant buffer with scroll offset and beat intensity
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    HRESULT hr = context_->Map(tunnelBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (SUCCEEDED(hr)) {
        struct TunnelBufferData {
            float worldViewProj[16];  // 4x4 matrix
            float scrollOffset;
            float beatIntensity;
            float padding[2];
        };
        
        TunnelBufferData* tunnelData = (TunnelBufferData*)mapped.pData;
        
        // Set identity matrix for simplicity (camera is inside tunnel)
        for (int i = 0; i < 16; ++i) {
            tunnelData->worldViewProj[i] = (i % 5 == 0) ? 1.0f : 0.0f;  // Identity matrix
        }
        
        // Set scrolling offset
        tunnelData->scrollOffset = tunnelScrollOffset_;
        
        // Set beat intensity
        tunnelData->beatIntensity = getBeatIntensity();
        
        context_->Unmap(tunnelBuffer_.Get(), 0);
        
        // Bind tunnel constant buffer
        context_->VSSetConstantBuffers(0, 1, tunnelBuffer_.GetAddressOf());
        
        // Set tunnel shader
        context_->VSSetShader(tunnelShader_->getVertexShader(), nullptr, 0);
        context_->PSSetShader(tunnelShader_->getPixelShader(), nullptr, 0);
        
        // Bind texture
        if (textureManager_) {
            textureManager_->bind(context_, 0);
        }
        
        // Draw tunnel geometry
        uint32_t indexCount = tunnelIndexBuffer_->getIndexCount();
        context_->DrawIndexed(indexCount, 0, 0);
    }
#endif
}

// Phase 5: Create shadow mapping resources
bool Enhanced3DScene::createShadowResources() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (device_ == nullptr) {
        return false;
    }

    // Create shadow map
    if (!shadowMap_) {
        shadowMap_ = std::make_unique<ShadowMap>();
    }
    if (!shadowMap_->initialize(device_, 1024, 1024)) {
        return false;
    }

    // Create depth shader
    if (!shadowDepthShader_) {
        shadowDepthShader_ = std::make_unique<Shader>();
    }
    
    std::string error;
    if (!shadowDepthShader_->compile("shadowdepth", "VSMain", "vs_5_0", &error)) {
        return false;
    }
    if (!shadowDepthShader_->compile("shadowdepth", "PSMain", "ps_5_0", &error)) {
        return false;
    }
    if (!shadowDepthShader_->createShaders(device_)) {
        return false;
    }

    // Create shadow constant buffer (for light view/projection matrices)
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = 128 + 16;  // Two 4x4 matrices (128 bytes) + shadow settings (16 bytes)
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    HRESULT hr = device_->CreateBuffer(&desc, nullptr, shadowConstantBuffer_.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    return true;
#else
    return false;
#endif
}

// Phase 5: Render shadow depth pass before main rendering
void Enhanced3DScene::renderShadowDepthPass() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!shadowMappingEnabled_ || !shadowMap_ || !shadowDepthShader_) {
        return;
    }

    // Bind shadow map for depth pass
    if (!shadowMap_->bindForDepthPass(context_)) {
        return;
    }

    // Set shadow depth shader
    context_->VSSetShader(shadowDepthShader_->getVertexShader(), nullptr, 0);
    context_->PSSetShader(shadowDepthShader_->getPixelShader(), nullptr, 0);

    // Update shadow constant buffer with light-space matrices
    if (shadowConstantBuffer_) {
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        HRESULT hr = context_->Map(shadowConstantBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (SUCCEEDED(hr)) {
            struct ShadowBufferData {
                float lightView[16];
                float lightProj[16];
                int useShadows;
                float shadowBias;
                float texelSize[2];
            };

            ShadowBufferData* shadowData = (ShadowBufferData*)mapped.pData;
            
            // Copy light matrices
            const float* lightView = shadowMap_->getLightViewMatrix();
            const float* lightProj = shadowMap_->getLightProjectionMatrix();
            
            std::copy(lightView, lightView + 16, shadowData->lightView);
            std::copy(lightProj, lightProj + 16, shadowData->lightProj);
            
            shadowData->useShadows = shadowMappingEnabled_ ? 1 : 0;
            shadowData->shadowBias = 0.0005f;
            shadowData->texelSize[0] = 1.0f / 1024.0f;
            shadowData->texelSize[1] = 1.0f / 1024.0f;
            
            context_->Unmap(shadowConstantBuffer_.Get(), 0);
            
            // Bind constant buffer to vertex shader
            context_->VSSetConstantBuffers(0, 1, shadowConstantBuffer_.GetAddressOf());
        }
    }

    // TODO: In full implementation, would render all scene geometry to shadow map here
    // For now, just set up the infrastructure for Phase 5

    // Unbind shadow map
    shadowMap_->unbindDepthPass(context_);
#endif
}

} // namespace dj
