#include "visuals/Enhanced3DScene.h"

#include <algorithm>
#include <cmath>

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include "visuals/Shader.h"
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
    // Clean up D3D11 resources if needed
    // Note: device_ and context_ are owned by GraphicsContext, not this class
    device_ = nullptr;
    context_ = nullptr;
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

} // namespace dj
