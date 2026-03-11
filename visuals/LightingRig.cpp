#include "visuals/LightingRig.h"

#include <algorithm>
#include <cmath>

namespace dj {

LightingRig::LightingRig()
    : intensities_(32, 0.0f), phaseTime_(0.0f) {
}

void LightingRig::update(float bpm, float energy, float dtSeconds) {
    // Clamp inputs
    bpm = std::max(60.0f, bpm);
    energy = std::clamp(energy, 0.0f, 1.0f);
    dtSeconds = std::max(0.0f, dtSeconds);

    // Update phase time (accumulates for BPM-synced animation)
    phaseTime_ += dtSeconds;

    // BPM synchronization: beat frequency
    const float beatFrequency = bpm / 60.0f;  // beats per second
    const float barCycleSeconds = 32.0f / beatFrequency;  // time for all 32 bars to complete one cycle
    if (barCycleSeconds > 0.0f) {
        phaseTime_ = std::fmod(phaseTime_, barCycleSeconds);
    }

    // Update each bar intensity based on phase and energy
    for (std::size_t i = 0; i < intensities_.size(); ++i) {
        // Position of this bar in the cycle [0, 32)
        float barPosition = static_cast<float>(i);

        // Phase offset for animation: sweep through bars
        float phaseOffset = (phaseTime_ / (barCycleSeconds > 0.0f ? barCycleSeconds : 1.0f)) * 32.0f;
        phaseOffset = std::fmod(phaseOffset, 32.0f);

        // Distance from current phase (with wrap-around)
        float distanceToPhase = std::abs(barPosition - phaseOffset);
        if (distanceToPhase > 16.0f) {
            distanceToPhase = 32.0f - distanceToPhase;
        }

        // Gaussian-like peak around the phase position, modulated by energy
        float intensity = energy * std::exp(-distanceToPhase * distanceToPhase / 8.0f);

        // Clamp to [0, 1]
        intensities_[i] = std::clamp(intensity, 0.0f, 1.0f);
    }
}

const std::vector<float>& LightingRig::intensities() const {
    return intensities_;
}

} // namespace dj
