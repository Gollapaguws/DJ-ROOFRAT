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
#include "visuals/StageGeometry.h"
#include "visuals/DJControllerGeometry.h"
#include "visuals/CrowdRenderer.h"  // Phase 2: Crowd visualization
#include "visuals/CrowdAnimator.h"  // Phase 2: Crowd animation
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

    // Phase 3: Initialize texture manager with procedural checkerboard (1024x1024 for high fidelity)
    if (!textureManager_) {
        textureManager_ = std::make_unique<TextureManager>();
    }
    textureManager_->setDevice(device_);
    if (!textureManager_->createCheckerboard(1024, 1024)) {
        return false;
    }

    if (!createStageGeometry()) {
        return false;
    }

    // Phase 2: Create DJ Controller geometry
    if (!createControllerGeometry()) {
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

    // Phase 2: Initialize crowd rendering
    if (!initializeCrowdRendering()) {
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
    
    // Phase 2: Update crowd animation synchronized to BPM
    if (crowdAnimator_) {
        crowdAnimator_->update(currentBPM_, deltaTime);
    }
    
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

    // Run shadow depth pre-pass before main shading when enabled.
    renderShadowDepthPass();

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
        
        matData->baseColor[0] = 0.95f;
        matData->baseColor[1] = 0.97f;
        matData->baseColor[2] = 1.0f;

        matData->metallic = 0.12f;
        
        // Set emissive color based on beat intensity (beat-reactive)
        float beatIntensity = getBeatIntensity();
        float emissiveScale = 0.18f + (0.42f * currentEnergy_);
        matData->emissiveColor[0] = 1.0f * emissiveScale;
        matData->emissiveColor[1] = 0.55f * emissiveScale;
        matData->emissiveColor[2] = 0.24f * emissiveScale;

        matData->roughness = 0.48f - (0.18f * beatIntensity);
        
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
        
        // Phase 5: Bind shadow map SRV and comparison sampler for PCF sampling
        // Register(t0) for shadow map texture, Register(s0) for comparison sampler
        if (shadowMap_ && shadowMappingEnabled_) {
            ID3D11ShaderResourceView* shadowSRV = shadowMap_->getShaderResourceView();
            ID3D11SamplerState* shadowSampler = shadowMap_->getComparisonSampler();
            if (shadowSRV) {
                context_->PSSetShaderResources(0, 1, &shadowSRV);
            }
            if (shadowSampler) {
                context_->PSSetSamplers(0, 1, &shadowSampler);
            }
        }
        
        // Phase 3: Bind texture SRV and sampler state to pixel shader for UV mapping
        if (textureManager_) {
            textureManager_->bind(context_, 1);
        }

        if (stageVertexBuffer_ && stageIndexBuffer_) {
            stageVertexBuffer_->bind(context_, 0);
            stageIndexBuffer_->bind(context_);
            context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context_->DrawIndexed(stageIndexBuffer_->getIndexCount(), 0, 0);
        }
    }

    // Render tunnel overlay/effect pass when enabled.
    if (tunnelEffectEnabled_) {
        renderTunnel();
    }

    // Phase 2: Render crowd visualization when enabled
    if (crowdVisualizationEnabled_) {
        renderCrowd(viewMatrix, projMatrix);
    }

    // Phase 2: Render DJ Controller UI geometry
    renderController(context_);
#endif
}

bool Enhanced3DScene::createStageGeometry() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (device_ == nullptr) {
        return false;
    }

    auto stageGeometry = std::make_unique<StageGeometry>();
    stageGeometry->generate();

    const auto& vertices = stageGeometry->getVertices();
    const auto& indices = stageGeometry->getIndices();
    if (vertices.empty() || indices.empty()) {
        return false;
    }

    if (!stageVertexBuffer_) {
        stageVertexBuffer_ = std::make_unique<VertexBuffer>();
    }
    if (!stageVertexBuffer_->create(device_, vertices.data(), static_cast<uint32_t>(vertices.size()), sizeof(Vertex))) {
        return false;
    }

    if (!stageIndexBuffer_) {
        stageIndexBuffer_ = std::make_unique<IndexBuffer>();
    }
    if (!stageIndexBuffer_->create(device_, indices.data(), static_cast<uint32_t>(indices.size()))) {
        return false;
    }

    return true;
#else
    return false;
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

        // Bind tunnel geometry buffers
        tunnelVertexBuffer_->bind(context_, 0);
        tunnelIndexBuffer_->bind(context_);
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
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

    // Create shadow constant buffer matching shadowdepth.hlsl TransformBuffer (b0):
    // matrix World (64 bytes) + matrix View (64 bytes) + matrix Projection (64 bytes)
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = 64 * 3;
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

    // Save current render targets and viewport so we can restore main pass state.
    ID3D11RenderTargetView* previousRTV = nullptr;
    ID3D11DepthStencilView* previousDSV = nullptr;
    context_->OMGetRenderTargets(1, &previousRTV, &previousDSV);

    // Save depth-stencil and rasterizer states changed by shadow pass.
    ID3D11DepthStencilState* previousDepthStencilState = nullptr;
    UINT previousStencilRef = 0;
    context_->OMGetDepthStencilState(&previousDepthStencilState, &previousStencilRef);

    ID3D11RasterizerState* previousRasterizerState = nullptr;
    context_->RSGetState(&previousRasterizerState);

    D3D11_VIEWPORT previousViewport = {};
    UINT viewportCount = 1;
    context_->RSGetViewports(&viewportCount, &previousViewport);

    // Bind shadow map for depth pass
    if (!shadowMap_->bindForDepthPass(context_)) {
        if (previousRTV) {
            previousRTV->Release();
        }
        if (previousDSV) {
            previousDSV->Release();
        }
        if (previousDepthStencilState) {
            previousDepthStencilState->Release();
        }
        if (previousRasterizerState) {
            previousRasterizerState->Release();
        }
        return;
    }

    // Set shadow depth shader
    context_->VSSetShader(shadowDepthShader_->getVertexShader(), nullptr, 0);
    context_->PSSetShader(shadowDepthShader_->getPixelShader(), nullptr, 0);

    // Update shadow constant buffer with world/view/projection matrices expected by shadowdepth.hlsl
    if (shadowConstantBuffer_) {
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        HRESULT hr = context_->Map(shadowConstantBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (SUCCEEDED(hr)) {
            struct ShadowBufferData {
                float world[16];
                float view[16];
                float projection[16];
            };

            ShadowBufferData* shadowData = (ShadowBufferData*)mapped.pData;

            // Identity world for scene-local geometry in this pass.
            for (int i = 0; i < 16; ++i) {
                shadowData->world[i] = (i % 5 == 0) ? 1.0f : 0.0f;
            }

            // Copy light view/projection matrices into shader expected slots.
            const float* lightView = shadowMap_->getLightViewMatrix();
            const float* lightProj = shadowMap_->getLightProjectionMatrix();
            std::copy(lightView, lightView + 16, shadowData->view);
            std::copy(lightProj, lightProj + 16, shadowData->projection);
            
            context_->Unmap(shadowConstantBuffer_.Get(), 0);
            
            // Bind constant buffer to vertex shader
            context_->VSSetConstantBuffers(0, 1, shadowConstantBuffer_.GetAddressOf());
        }
    }

    // Render available scene geometry into shadow depth map.
    if (stageVertexBuffer_ && stageIndexBuffer_) {
        stageVertexBuffer_->bind(context_, 0);
        stageIndexBuffer_->bind(context_);
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->DrawIndexed(stageIndexBuffer_->getIndexCount(), 0, 0);
    }

    if (tunnelVertexBuffer_ && tunnelIndexBuffer_) {
        tunnelVertexBuffer_->bind(context_, 0);
        tunnelIndexBuffer_->bind(context_);
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->DrawIndexed(tunnelIndexBuffer_->getIndexCount(), 0, 0);
    }

    // Unbind shadow map
    shadowMap_->unbindDepthPass(context_);

    // Restore previous render target/depth state and viewport for the main scene pass.
    context_->OMSetRenderTargets(1, &previousRTV, previousDSV);
    context_->OMSetDepthStencilState(previousDepthStencilState, previousStencilRef);
    context_->RSSetState(previousRasterizerState);
    if (viewportCount > 0) {
        context_->RSSetViewports(1, &previousViewport);
    }

    if (previousRTV) {
        previousRTV->Release();
    }
    if (previousDSV) {
        previousDSV->Release();
    }
    if (previousDepthStencilState) {
        previousDepthStencilState->Release();
    }
    if (previousRasterizerState) {
        previousRasterizerState->Release();
    }
#endif
}

// Phase 2: Initialize crowd rendering components
bool Enhanced3DScene::initializeCrowdRendering() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (device_ == nullptr) {
        return false;
    }

    // Create crowd renderer and animator instances
    if (!crowdRenderer_) {
        crowdRenderer_ = std::make_unique<CrowdRenderer>();
    }
    if (!crowdAnimator_) {
        crowdAnimator_ = std::make_unique<CrowdAnimator>();
    }

    // Initialize GPU resources for instanced rendering
    if (!crowdRenderer_->initialize(device_)) {
        return false;
    }

    return true;
#else
    return false;
#endif
}

// Phase 2: Render the crowd mesh synchronized with music
void Enhanced3DScene::renderCrowd(const float* viewMatrix, const float* projMatrix) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!crowdRenderer_ || !crowdAnimator_ || viewMatrix == nullptr || projMatrix == nullptr || 
        context_ == nullptr) {
        return;
    }

    // Map current mood based on energy level
    // Energy-based mood transition: low energy = unimpressed, high energy = hyped
    const int previousMood = currentMood_;
    if (currentEnergy_ < 0.25f) {
        currentMood_ = 0;  // Unimpressed
    } else if (currentEnergy_ < 0.5f) {
        currentMood_ = 1;  // Calm
    } else if (currentEnergy_ < 0.75f) {
        currentMood_ = 2;  // Grooving
    } else {
        currentMood_ = 3;  // Hyped
    }

    // Render instanced crowd with LOD selection based on camera distance
    float3 crowdPos(0.0f, 0.0f, 0.0f);    // Center of stage
    float3 cameraPos(0.0f, 0.0f, -5.0f);  // Camera position from view matrix (simplified)
    
    // Calculate LOD level
    int lodLevel = crowdRenderer_->calculateLODLevel(crowdPos, cameraPos);

    // Render the crowd
    if (!crowdRenderer_->renderInstanced(context_, cameraPos)) {
        // Fallback: rendering failed, silently continue
        return;
    }
#endif
}

// Phase 2: DJ Controller rendering
bool Enhanced3DScene::hasControllerGeometry() const {
    return controllerGeometry_ != nullptr;
}

DJControllerGeometry* Enhanced3DScene::getControllerGeometry() const {
    return controllerGeometry_.get();
}

void Enhanced3DScene::renderController(ID3D11DeviceContext* context) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!controllerGeometry_ || !context) {
        static int errorCount = 0;
        if (errorCount++ < 1) {
            printf("[3D Controller] ERROR: controllerGeometry_=%p context=%p\n", 
                   controllerGeometry_.get(), context);
        }
        return;
    }

    // Get vertices and indices from controller geometry
    const auto& vertices = controllerGeometry_->getVertices();
    const auto& indices = controllerGeometry_->getIndices();

    if (vertices.empty() || indices.empty()) {
        static int emptyCount = 0;
        if (emptyCount++ < 1) {
            printf("[3D Controller] ERROR: Empty geometry! vertices=%zu indices=%zu\n",
                   vertices.size(), indices.size());
        }
        return;
    }

    // Use enhanced shader for rendering controller with PBR
    if (!enhancedShader_) {
        static int shaderCount = 0;
        if (shaderCount++ < 1) {
            printf("[3D Controller] ERROR: No enhanced shader!\n");
        }
        return;
    }

    // Debug output: confirm renderController was called
    static bool once = false;
    static int frameCount = 0;
    if (!once) {
        printf("[3D Controller] ===== CONTROLLER RENDERING DIAGNOSTICS =====\n");
        printf("[3D Controller] Vertices: %zu, Indices: %zu\n", vertices.size(), indices.size());
        printf("[3D Controller] Vertex sample [0]: pos=(%.2f, %.2f, %.2f)\n",
               vertices[0].position[0], vertices[0].position[1], vertices[0].position[2]);
        printf("[3D Controller] Buffers: VB=%p IB=%p\n", 
               controllerVertexBuffer_.get(), controllerIndexBuffer_.get());
        printf("[3D Controller] Shader: VS=%p PS=%p\n",
               enhancedShader_->getVertexShader(), enhancedShader_->getPixelShader());
        once = true;
    }

    frameCount++;
    if (frameCount % 300 == 0) {
        printf("[3D Controller] Still rendering (frame %d)\n", frameCount);
    }

    // Set shaders
    context->VSSetShader(enhancedShader_->getVertexShader(), nullptr, 0);
    context->PSSetShader(enhancedShader_->getPixelShader(), nullptr, 0);

    // Bind material buffer for PBR properties
    if (materialBuffer_) {
        context->PSSetConstantBuffers(1, 1, materialBuffer_.GetAddressOf());
    }

    // Bind vertex and index buffers
    if (controllerVertexBuffer_ && controllerIndexBuffer_) {
        controllerVertexBuffer_->bind(context, 0);
        controllerIndexBuffer_->bind(context);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Draw indexed primitives
        uint32_t indexCount = controllerIndexBuffer_->getIndexCount();
        
        if (!once || frameCount == 1) {
            printf("[3D Controller] Issuing DrawIndexed(%u, 0, 0)\n", indexCount);
        }
        
        context->DrawIndexed(indexCount, 0, 0);
    } else {
        static int bufferErrorCount = 0;
        if (bufferErrorCount++ < 1) {
            printf("[3D Controller] ERROR: Buffers not created properly!\n");
        }
    }
#endif
}

// Phase 2: Initialize DJ Controller geometry and buffers
bool Enhanced3DScene::createControllerGeometry() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (device_ == nullptr || context_ == nullptr) {
        return false;
    }

    // Create controller geometry
    if (!controllerGeometry_) {
        controllerGeometry_ = std::make_unique<DJControllerGeometry>();
    }

    // Generate mesh
    controllerGeometry_->generateMesh();

    const auto& vertices = controllerGeometry_->getVertices();
    const auto& indices = controllerGeometry_->getIndices();

    if (vertices.empty() || indices.empty()) {
        printf("[3D Controller] ERROR: generateMesh produced empty geometry (vertices=%zu, indices=%zu)\n",
               vertices.size(), indices.size());
        return false;
    }

    // Create vertex buffer
    if (!controllerVertexBuffer_) {
        controllerVertexBuffer_ = std::make_unique<VertexBuffer>();
    }
    if (!controllerVertexBuffer_->create(device_, vertices.data(), static_cast<uint32_t>(vertices.size()), sizeof(Vertex))) {
        printf("[3D Controller] ERROR: Failed to create vertex buffer (%u vertices)\n", 
               static_cast<uint32_t>(vertices.size()));
        return false;
    }

    // Create index buffer
    if (!controllerIndexBuffer_) {
        controllerIndexBuffer_ = std::make_unique<IndexBuffer>();
    }
    if (!controllerIndexBuffer_->create(device_, indices.data(), static_cast<uint32_t>(indices.size()))) {
        printf("[3D Controller] ERROR: Failed to create index buffer (%u indices)\n", 
               static_cast<uint32_t>(indices.size()));
        return false;
    }

    printf("[3D Controller] Geometry created successfully: %u vertices, %u indices\n",
           static_cast<uint32_t>(vertices.size()), static_cast<uint32_t>(indices.size()));

    return true;
#else
    return false;
#endif
}

} // namespace dj

