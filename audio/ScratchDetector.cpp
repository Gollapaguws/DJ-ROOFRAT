#include "audio/ScratchDetector.h"

#include <algorithm>
#include <cmath>

namespace dj {

void ScratchDetector::setSampleRate(int sampleRate) {
    sampleRate_ = std::max(8000, sampleRate);
}

void ScratchDetector::setSensitivity(float sensitivity) {
    sensitivity_ = std::clamp(sensitivity, 0.5f, 10.0f);
}

float ScratchDetector::sensitivity() const {
    return sensitivity_;
}

void ScratchDetector::setDeadzone(float deadzone) {
    deadzone_ = std::clamp(deadzone, 0.0f, 0.5f);
}

float ScratchDetector::deadzone() const {
    return deadzone_;
}

float ScratchDetector::processScratchInput(float scratchVelocity, float dt) {
    // Apply deadzone: small inputs are treated as zero
    if (std::abs(scratchVelocity) < deadzone_) {
        return 0.0f;
    }
    
    // Apply sensitivity scaling to the scratch velocity
    // Higher sensitivity = more pitch change per unit velocity
    float pitchMultiplier = scratchVelocity * sensitivity_;
    
    // Clamp to reasonable range (-3.0 to 3.0 for extreme scratches)
    pitchMultiplier = std::clamp(pitchMultiplier, -3.0f, 3.0f);
    
    return pitchMultiplier;
}

void ScratchDetector::reset() {
    // No persistent state to reset
}

} // namespace dj
