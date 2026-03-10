#include "audio/ThreeBandEQ.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace dj {

namespace {

float lowPassAlpha(float sampleRate, float cutoffHz) {
    const float dt = 1.0f / sampleRate;
    const float rc = 1.0f / (2.0f * static_cast<float>(std::numbers::pi) * cutoffHz);
    return dt / (rc + dt);
}

float highPassAlpha(float sampleRate, float cutoffHz) {
    const float dt = 1.0f / sampleRate;
    const float rc = 1.0f / (2.0f * static_cast<float>(std::numbers::pi) * cutoffHz);
    return rc / (rc + dt);
}

} // namespace

void ThreeBandEQ::setSampleRate(int sampleRate) {
    sampleRate_ = std::max(8000, sampleRate);
    reset();
}

void ThreeBandEQ::setGains(float low, float mid, float high) {
    lowGain_ = std::clamp(low, 0.0f, 2.0f);
    midGain_ = std::clamp(mid, 0.0f, 2.0f);
    highGain_ = std::clamp(high, 0.0f, 2.0f);
}

void ThreeBandEQ::setLowMidCrossover(float hz) {
    lowMidCrossoverHz_ = std::clamp(hz, 50.0f, 500.0f);
}

void ThreeBandEQ::setMidHighCrossover(float hz) {
    midHighCrossoverHz_ = std::clamp(hz, 500.0f, 10000.0f);
}

float ThreeBandEQ::gainTodB(float linear) {
    if (linear <= 0.0f) {
        return -80.0f;  // Minimum dB value for silence
    }
    return 20.0f * std::log10(linear);
}

float ThreeBandEQ::dBToGain(float dB) {
    return std::pow(10.0f, dB / 20.0f);
}

std::array<float, 2> ThreeBandEQ::process(const std::array<float, 2>& input) {
    const float alphaLow = lowPassAlpha(static_cast<float>(sampleRate_), lowMidCrossoverHz_);
    const float alphaHigh = highPassAlpha(static_cast<float>(sampleRate_), midHighCrossoverHz_);

    return {
        processSingle(input[0], left_, alphaLow, alphaHigh),
        processSingle(input[1], right_, alphaLow, alphaHigh),
    };
}

void ThreeBandEQ::reset() {
    left_ = ChannelState{};
    right_ = ChannelState{};
}

float ThreeBandEQ::processSingle(float input, ChannelState& state, float alphaLow, float alphaHigh) {
    state.low += alphaLow * (input - state.low);
    const float high = alphaHigh * (state.prevHigh + input - state.prevInput);
    state.prevInput = input;
    state.prevHigh = high;

    const float mid = input - state.low - high;
    return (state.low * lowGain_) + (mid * midGain_) + (high * highGain_);
}

} // namespace dj
