#include "visuals/Enhanced3DScene.h"

#include <algorithm>

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

    // Stub implementation for Phase 1
    // Actual rendering will be implemented in Phase 2 with shader loading
    // For now, we just verify the parameters were passed correctly
    
    // TODO Phase 2: Implement actual rendering with shaders
    // - Render dynamic lighting effects
    // - Render particle bursts if enabled
    // - Render tunnel effect if enabled
    // - Apply bloom post-processing if enabled
#endif
}

} // namespace dj
