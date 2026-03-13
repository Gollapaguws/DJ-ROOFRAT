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

BeamGeometry LaserController::getBeamGeometry(float width) const noexcept {
    BeamGeometry geom;

    // Clamp width to reasonable values
    float w = std::clamp(width, 0.01f, 10.0f);
    const float halfWidth = w / 2.0f;

    // Convert angles to radians
    const float primaryRad = primaryAngle_ * std::numbers::pi_v<float> / 180.0f;
    const float secondaryRad = secondaryAngle_ * std::numbers::pi_v<float> / 180.0f;

    // Beam extends along primary angle with some depth for volumetric effect
    const float beamLength = 10.0f;

    // Compute beam direction (primary angle dominant, secondary for swing)
    float cosAngle = std::cos(primaryRad);
    float sinAngle = std::sin(primaryRad);

    // Add secondary oscillation as rotation
    float rotX = std::sin(secondaryRad) * 0.1f;

    // Create quad geometry (4 vertices, 2 triangles)
    // Vertices arranged as quad corners
    LaserVertex v0, v1, v2, v3;

    // Front-left
    v0.position = {-halfWidth, 0.0f, 0.0f};
    v0.texCoord = {0.0f, 0.0f};
    v0.color = {1.0f, 0.0f, 0.5f, 0.8f};  // Magenta-ish laser

    // Front-right
    v1.position = {halfWidth, 0.0f, 0.0f};
    v1.texCoord = {1.0f, 0.0f};
    v1.color = {1.0f, 0.0f, 0.5f, 0.8f};

    // Back-left (extends along beam direction)
    v2.position = {-halfWidth + cosAngle * beamLength, rotX, sinAngle * beamLength};
    v2.texCoord = {0.0f, 1.0f};
    v2.color = {1.0f, 0.2f, 0.7f, 0.4f};  // Fade at end

    // Back-right
    v3.position = {halfWidth + cosAngle * beamLength, rotX, sinAngle * beamLength};
    v3.texCoord = {1.0f, 1.0f};
    v3.color = {1.0f, 0.2f, 0.7f, 0.4f};

    geom.vertices = {v0, v1, v2, v3};

    // Indices for two triangles (CCW winding)
    // Triangle 1: v0, v1, v2
    // Triangle 2: v1, v3, v2
    geom.indices = {0, 1, 2, 1, 3, 2};

    return geom;
}

} // namespace dj
