#include "audio/Delay.h"

#include <algorithm>
#include <cmath>

namespace dj {

Delay::Delay() {
    // Pre-allocate delay buffers for both channels
    delayBufferLeft_.resize(MAX_DELAY_SAMPLES, 0.0f);
    delayBufferRight_.resize(MAX_DELAY_SAMPLES, 0.0f);
}

void Delay::setSampleRate(int sampleRate) {
    sampleRate_ = sampleRate;
}

void Delay::setTempoSync(float bpm, float noteValue) {
    // Calculate delay time from BPM and note value
    // Formula: delayTimeMs = (60000.0 / bpm) * noteValue
    // noteValue: 1.0 = quarter note, 0.5 = eighth note, 0.25 = sixteenth note
    
    double delayTimeMs = (60000.0 / bpm) * noteValue;
    
    // Convert milliseconds to samples
    delayTimeInSamples_ = static_cast<int>((delayTimeMs / 1000.0) * sampleRate_);
    
    // Clamp to valid range
    delayTimeInSamples_ = std::clamp(delayTimeInSamples_, 1, MAX_DELAY_SAMPLES - 1);
}

void Delay::setFeedback(float feedback) {
    // Clamp feedback to prevent runaway (0.0 to 0.95)
    feedback_ = std::clamp(feedback, 0.0f, 0.95f);
}

void Delay::setWetDryMix(float wetDryMix) {
    wetDryMix_ = std::clamp(wetDryMix, 0.0f, 1.0f);
}

std::array<float, 2> Delay::process(const std::array<float, 2>& input) {
    // Calculate read position from current write index and delay time
    // readIndex = (writeIndex - delaySamples) wrapped around buffer size
    int readIdxLeft = (writeIndex_ - delayTimeInSamples_ + MAX_DELAY_SAMPLES) % MAX_DELAY_SAMPLES;
    int readIdxRight = (writeIndex_ - delayTimeInSamples_ + MAX_DELAY_SAMPLES) % MAX_DELAY_SAMPLES;
    
    // Read delayed samples from both channels
    float delayedLeft = delayBufferLeft_[readIdxLeft];
    float delayedRight = delayBufferRight_[readIdxRight];
    
    // Apply feedback (write back to read position in delay line)
    delayBufferLeft_[readIdxLeft] = delayedLeft * feedback_;
    delayBufferRight_[readIdxRight] = delayedRight * feedback_;
    
    // Write current input samples to write pointer
    delayBufferLeft_[writeIndex_] = input[0] + delayedLeft * feedback_;
    delayBufferRight_[writeIndex_] = input[1] + delayedRight * feedback_;
    
    // Advance write pointer ONCE per process() call
    writeIndex_ = (writeIndex_ + 1) % MAX_DELAY_SAMPLES;
    
    // Mix wet and dry signals
    float dryLeft = input[0] * (1.0f - wetDryMix_);
    float dryRight = input[1] * (1.0f - wetDryMix_);
    
    float wetLeft = delayedLeft * wetDryMix_;
    float wetRight = delayedRight * wetDryMix_;
    
    return {
        dryLeft + wetLeft,
        dryRight + wetRight
    };
}

void Delay::reset() {
    std::fill(delayBufferLeft_.begin(), delayBufferLeft_.end(), 0.0f);
    std::fill(delayBufferRight_.begin(), delayBufferRight_.end(), 0.0f);
    writeIndex_ = 0;
}

} // namespace dj
