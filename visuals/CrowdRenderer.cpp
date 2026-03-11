#include "visuals/CrowdRenderer.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace dj {

CrowdRenderer::CrowdRenderer()
    : motionAmplitude_(0.0f), density_(0.0f), visibleSilhouettes_(0), animationPhase_(0.0f) {}

void CrowdRenderer::update(int mood, float energy, float dtSeconds) {
    // Clamp inputs to safe ranges
    const float clampedEnergy = std::clamp(energy, 0.0f, 1.0f);
    const float clampedDt = std::clamp(dtSeconds, 0.0f, 1.0f);

    // Clamp mood to valid range [0, 3] representing (Unimpressed, Calm, Grooving, Hyped)
    const int clampedMood = std::clamp(mood, 0, 3);

    // Map mood to amplitude scaling factor
    // 0 (Unimpressed) -> 0.1, 1 (Calm) -> 0.3, 2 (Grooving) -> 0.6, 3 (Hyped) -> 1.0
    float moodFactor = 0.0f;
    switch (clampedMood) {
    case 0:  // Unimpressed
        moodFactor = 0.1f;
        break;
    case 1:  // Calm
        moodFactor = 0.3f;
        break;
    case 2:  // Grooving
        moodFactor = 0.6f;
        break;
    case 3:  // Hyped
        moodFactor = 1.0f;
        break;
    }

    // Motion amplitude driven by mood * energy
    motionAmplitude_ = std::clamp(moodFactor * clampedEnergy, 0.0f, 1.0f);

    // Density driven mainly by energy with mood modulation
    density_ = std::clamp(clampedEnergy * (0.5f + moodFactor * 0.5f), 0.0f, 1.0f);

    // Update animation phase for smooth motion
    animationPhase_ += clampedDt * 3.0f;  // ~3 cycles per second base rate
    if (animationPhase_ > 2.0f * std::numbers::pi_v<float>) {
        animationPhase_ -= 2.0f * std::numbers::pi_v<float>;
    }

    // Visible silhouettes range from 0 to ~500 based on mood and energy
    // Hyped mood at full energy -> ~500 silhouettes
    // Unimpressed mood at zero energy -> ~0 silhouettes
    const float maxSilhouettes = 500.0f;
    visibleSilhouettes_ = std::max(0, static_cast<int>(
        moodFactor * clampedEnergy * maxSilhouettes +
        std::abs(std::sin(animationPhase_)) * moodFactor * 50.0f  // Add some oscillation
    ));
}

float CrowdRenderer::motionAmplitude() const noexcept {
    return motionAmplitude_;
}

float CrowdRenderer::density() const noexcept {
    return density_;
}

int CrowdRenderer::visibleSilhouettes() const noexcept {
    return visibleSilhouettes_;
}

} // namespace dj
