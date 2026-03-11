#include "visuals/LaserController.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace dj {

LaserController::LaserController()
    : primaryAngle_(0.0f), secondaryAngle_(0.0f), intensity_(0.0f), phaseTime_(0.0f) {}

void LaserController::update(float bpm, float crossfader, float dtSeconds) {
    // Clamp inputs to safe ranges
    const float clampedBpm = std::clamp(bpm, 0.0f, 300.0f);
    const float clampedCrossfader = std::clamp(crossfader, -1.0f, 1.0f);
    const float clampedDt = std::clamp(dtSeconds, 0.0f, 1.0f);

    // Primary angle follows crossfader in range [-45, +45] degrees
    // Crossfader -1.0 -> -45°, 0.0 -> 0°, +1.0 -> +45°
    primaryAngle_ = clampedCrossfader * 45.0f;

    // Secondary angle oscillates with BPM
    // Higher BPM = faster oscillation
    phaseTime_ += clampedDt * (clampedBpm / 60.0f);  // Convert to oscillations per second
    secondaryAngle_ = 90.0f * std::sin(2.0f * std::numbers::pi_v<float> * phaseTime_);

    // Intensity scales with BPM (normalized to typical range 60-200)
    // Map BPM to intensity: 60 BPM -> 0.3, 200 BPM -> 1.0
    const float normalizedBpm = (clampedBpm - 60.0f) / 140.0f;
    intensity_ = std::clamp(0.3f + normalizedBpm * 0.7f, 0.0f, 1.0f);

    // Clamp all outputs to safe ranges
    primaryAngle_ = std::clamp(primaryAngle_, -45.0f, 45.0f);
    secondaryAngle_ = std::clamp(secondaryAngle_, -180.0f, 180.0f);
    intensity_ = std::clamp(intensity_, 0.0f, 1.0f);
}

float LaserController::primaryAngleDegrees() const noexcept {
    return primaryAngle_;
}

float LaserController::secondaryAngleDegrees() const noexcept {
    return secondaryAngle_;
}

float LaserController::intensity() const noexcept {
    return intensity_;
}

} // namespace dj
