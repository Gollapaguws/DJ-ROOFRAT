#include "audio/Mixer.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace dj {

void Mixer::setCrossfader(float value) {
    crossfader_ = std::clamp(value, -1.0f, 1.0f);
}

float Mixer::crossfader() const {
    return crossfader_;
}

void Mixer::setDeckTrim(float a, float b) {
    deckTrimA_ = std::clamp(a, 0.0f, 2.0f);
    deckTrimB_ = std::clamp(b, 0.0f, 2.0f);
}

void Mixer::setMasterGain(float gain) {
    masterGain_ = std::clamp(gain, 0.0f, 2.0f);
}

std::pair<float, float> Mixer::crossfadeGains() const {
    const float t = (crossfader_ + 1.0f) * 0.5f;
    const float gainA = std::cos(t * static_cast<float>(std::numbers::pi) * 0.5f);
    const float gainB = std::sin(t * static_cast<float>(std::numbers::pi) * 0.5f);
    return {gainA, gainB};
}

std::vector<float> Mixer::mixBlock(Deck& deckA, Deck& deckB, std::size_t frames, MixMetrics& metrics) {
    std::vector<float> out(frames * 2U, 0.0f);
    const auto [fadeA, fadeB] = crossfadeGains();

    float energyAccumulator = 0.0f;

    for (std::size_t i = 0; i < frames; ++i) {
        const auto a = deckA.nextFrame();
        const auto b = deckB.nextFrame();

        const float left = ((a[0] * fadeA * deckTrimA_) + (b[0] * fadeB * deckTrimB_)) * masterGain_;
        const float right = ((a[1] * fadeA * deckTrimA_) + (b[1] * fadeB * deckTrimB_)) * masterGain_;

        out[(i * 2U)] = std::clamp(left, -1.0f, 1.0f);
        out[(i * 2U) + 1U] = std::clamp(right, -1.0f, 1.0f);

        energyAccumulator += 0.5f * ((out[(i * 2U)] * out[(i * 2U)]) + (out[(i * 2U) + 1U] * out[(i * 2U) + 1U]));
    }

    const float velocity = std::abs(crossfader_ - previousCrossfader_);
    metrics.transitionSmoothness = std::clamp(1.0f - (velocity * 6.0f), 0.0f, 1.0f);
    metrics.rms = std::sqrt(energyAccumulator / static_cast<float>(std::max<std::size_t>(1, frames)));
    metrics.deckAEnergy = deckA.recentEnergy();
    metrics.deckBEnergy = deckB.recentEnergy();
    previousCrossfader_ = crossfader_;
    return out;
}

} // namespace dj
