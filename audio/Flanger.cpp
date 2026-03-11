#include "audio/Flanger.h"

#include <algorithm>
#include <cmath>

namespace dj {

Flanger::Flanger() {
    // Pre-allocate delay buffer for max delay (10ms @ 44.1kHz = 441 samples)
    // Use a small circular buffer
    int maxDelaySamples = static_cast<int>((MAX_DELAY_MS / 1000.0f) * 44100);
    delayLineLeft_.resize(maxDelaySamples, 0.0f);
    delayLineRight_.resize(maxDelaySamples, 0.0f);
}

void Flanger::setSampleRate(int sampleRate) {
    sampleRate_ = sampleRate;
    // Resize delay buffers when sample rate changes
    int maxDelaySamples = static_cast<int>((MAX_DELAY_MS / 1000.0f) * sampleRate);
    delayLineLeft_.resize(maxDelaySamples, 0.0f);
    delayLineRight_.resize(maxDelaySamples, 0.0f);
}

void Flanger::setLFORate(float rateHz) {
    lfoRate_ = std::clamp(rateHz, 0.1f, 10.0f);
}

void Flanger::setDepth(float depth) {
    depth_ = std::clamp(depth, 0.0f, 1.0f);
}

void Flanger::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

void Flanger::setFeedback(float feedback) {
    feedback_ = std::clamp(feedback, -1.0f, 1.0f);
}

float Flanger::sineLFO() {
    // Generate sine wave LFO
    float value = std::sin(2.0f * 3.14159265359f * lfoPhase_);
    
    // Advance phase
    float phaseDelta = lfoRate_ / sampleRate_;
    lfoPhase_ += phaseDelta;
    
    // Wrap phase to [0, 1)
    if (lfoPhase_ >= 1.0f) {
        lfoPhase_ -= 1.0f;
    }
    
    return value;
}

std::array<float, 2> Flanger::process(const std::array<float, 2>& input) {
    // Get LFO value (sine wave, range [-1, 1])
    float lfo = sineLFO();
    
    // Calculate modulated delay time
    // Center delay: halfway between min and max
    float centerDelayMs = (MIN_DELAY_MS + MAX_DELAY_MS) / 2.0f;
    // LFO modulates around center: depth controls how much it varies
    float delayRangeMs = (MAX_DELAY_MS - MIN_DELAY_MS) / 2.0f;
    float modulatedDelayMs = centerDelayMs + (lfo * delayRangeMs * depth_);
    
    // Convert to samples with current sample rate
    int delayInSamples = static_cast<int>((modulatedDelayMs / 1000.0f) * sampleRate_);
    delayInSamples = std::clamp(delayInSamples, 1, static_cast<int>(delayLineLeft_.size()) - 1);
    
    // Calculate read indices
    int readIdxLeft = (writeIndex_ - delayInSamples + static_cast<int>(delayLineLeft_.size())) % 
                      static_cast<int>(delayLineLeft_.size());
    int readIdxRight = (writeIndex_ - delayInSamples + static_cast<int>(delayLineRight_.size())) % 
                       static_cast<int>(delayLineRight_.size());
    
    // Read delayed samples
    float delayedLeft = delayLineLeft_[readIdxLeft];
    float delayedRight = delayLineRight_[readIdxRight];
    
    // Write new samples with optional feedback (jet flanger effect)
    delayLineLeft_[writeIndex_] = input[0] + delayedLeft * feedback_;
    delayLineRight_[writeIndex_] = input[1] + delayedRight * feedback_;
    
    // Advance write pointer
    writeIndex_ = (writeIndex_ + 1) % static_cast<int>(delayLineLeft_.size());
    
    // Mix dry and wet signals
    float dryLeft = input[0] * (1.0f - mix_);
    float dryRight = input[1] * (1.0f - mix_);
    
    float wetLeft = delayedLeft * mix_;
    float wetRight = delayedRight * mix_;
    
    return {
        dryLeft + wetLeft,
        dryRight + wetRight
    };
}

void Flanger::reset() {
    std::fill(delayLineLeft_.begin(), delayLineLeft_.end(), 0.0f);
    std::fill(delayLineRight_.begin(), delayLineRight_.end(), 0.0f);
    writeIndex_ = 0;
    lfoPhase_ = 0.0f;
}

} // namespace dj
