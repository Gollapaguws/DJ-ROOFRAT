#include "audio/Reverb.h"

#include <algorithm>
#include <cmath>

namespace dj {

// CombFilter implementation
void Reverb::CombFilter::setDamping(float damping) {
    damp1 = damping;
    damp2 = 1.0f - damping;
}

float Reverb::CombFilter::process(float input) {
    float bufout = buffer[bufferIndex];
    filterStore = (bufout * damp2) + (filterStore * damp1);
    buffer[bufferIndex] = input + (filterStore * feedback);
    
    bufferIndex++;
    if (bufferIndex >= buffer.size()) {
        bufferIndex = 0;
    }
    
    return bufout;
}

void Reverb::CombFilter::reset() {
    std::fill(buffer.begin(), buffer.end(), 0.0f);
    bufferIndex = 0;
    filterStore = 0.0f;
}

// AllpassFilter implementation
float Reverb::AllpassFilter::process(float input) {
    float bufout = buffer[bufferIndex];
    
    float output = -input + bufout;
    buffer[bufferIndex] = input + (bufout * feedback);
    
    bufferIndex++;
    if (bufferIndex >= buffer.size()) {
        bufferIndex = 0;
    }
    
    return output;
}

void Reverb::AllpassFilter::reset() {
    std::fill(buffer.begin(), buffer.end(), 0.0f);
    bufferIndex = 0;
}

// Reverb main implementation
Reverb::Reverb() {
    // Initialize left channel comb filters with standard Freeverb tunings
    for (int i = 0; i < 8; ++i) {
        combFiltersLeft_[i].buffer.resize(COMB_TUNINGS[i], 0.0f);
        combFiltersLeft_[i].setDamping(damping_);
    }
    
    // Initialize right channel comb filters with offset tunings for stereo separation
    for (int i = 0; i < 8; ++i) {
        combFiltersRight_[i].buffer.resize(COMB_TUNINGS_RIGHT[i], 0.0f);
        combFiltersRight_[i].setDamping(damping_);
    }
    
    // Initialize left channel allpass filters
    for (int i = 0; i < 4; ++i) {
        allpassFiltersLeft_[i].buffer.resize(ALLPASS_TUNINGS[i], 0.0f);
    }
    
    // Initialize right channel allpass filters
    for (int i = 0; i < 4; ++i) {
        allpassFiltersRight_[i].buffer.resize(ALLPASS_TUNINGS[i], 0.0f);
    }
}

void Reverb::setSampleRate(int sampleRate) {
    sampleRate_ = sampleRate;
}

void Reverb::setRoomSize(float roomSize) {
    roomSize_ = std::clamp(roomSize, 0.0f, 1.0f);
    
    // Room size affects comb filter feedback for both channels
    for (int i = 0; i < 8; ++i) {
        combFiltersLeft_[i].feedback = roomSize_;
        combFiltersRight_[i].feedback = roomSize_;
    }
}

void Reverb::setDamping(float damping) {
    damping_ = std::clamp(damping, 0.0f, 1.0f);
    
    // Update damping in all comb filters for both channels
    for (int i = 0; i < 8; ++i) {
        combFiltersLeft_[i].setDamping(damping_);
        combFiltersRight_[i].setDamping(damping_);
    }
}

void Reverb::setWetDryMix(float wetDryMix) {
    wetDryMix_ = std::clamp(wetDryMix, 0.0f, 1.0f);
}

std::array<float, 2> Reverb::process(const std::array<float, 2>& input) {
    // Process left channel through separate comb filters (parallel)
    float combOutLeft = 0.0f;
    for (int i = 0; i < 8; ++i) {
        combOutLeft += combFiltersLeft_[i].process(input[0]);
    }
    
    // Process right channel through separate comb filters (parallel)
    float combOutRight = 0.0f;
    for (int i = 0; i < 8; ++i) {
        combOutRight += combFiltersRight_[i].process(input[1]);
    }
    
    // Apply gain scaling after comb filtering (Freeverb compensation: 1/8 ≈ 0.015)
    constexpr float COMB_GAIN = 0.015f;
    combOutLeft *= COMB_GAIN;
    combOutRight *= COMB_GAIN;
    
    // Process through separate allpass filters (series) for each channel
    float allpassOutLeft = combOutLeft;
    float allpassOutRight = combOutRight;
    for (int i = 0; i < 4; ++i) {
        allpassOutLeft = allpassFiltersLeft_[i].process(allpassOutLeft);
        allpassOutRight = allpassFiltersRight_[i].process(allpassOutRight);
    }
    
    // Mix wet and dry signals
    float dryLeft = input[0] * (1.0f - wetDryMix_);
    float dryRight = input[1] * (1.0f - wetDryMix_);
    
    float wetLeft = allpassOutLeft * wetDryMix_;
    float wetRight = allpassOutRight * wetDryMix_;
    
    return {
        dryLeft + wetLeft,
        dryRight + wetRight
    };
}

void Reverb::reset() {
    for (int i = 0; i < 8; ++i) {
        combFiltersLeft_[i].reset();
        combFiltersRight_[i].reset();
    }
    
    for (int i = 0; i < 4; ++i) {
        allpassFiltersLeft_[i].reset();
        allpassFiltersRight_[i].reset();
    }
}

} // namespace dj
