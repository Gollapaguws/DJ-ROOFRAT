#include "audio/RingModulator.h"

#include <algorithm>
#include <cmath>

namespace dj {

RingModulator::RingModulator() {
    // Initialize with A4 (440 Hz) as default carrier
}

void RingModulator::setSampleRate(int sampleRate) {
    sampleRate_ = sampleRate;
}

void RingModulator::setCarrierFrequency(float frequencyHz) {
    carrierFrequency_ = std::clamp(frequencyHz, 20.0f, 5000.0f);
}

void RingModulator::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

float RingModulator::carrierOscillator() {
    // Generate sine wave carrier
    float carrier = std::sin(2.0f * 3.14159265359f * carrierPhase_);
    
    // Advance phase
    float phaseDelta = carrierFrequency_ / sampleRate_;
    carrierPhase_ += phaseDelta;
    
    // Wrap phase to [0, 1)
    if (carrierPhase_ >= 1.0f) {
        carrierPhase_ -= 1.0f;
    }
    
    return carrier;
}

std::array<float, 2> RingModulator::process(const std::array<float, 2>& input) {
    // Get carrier signal
    float carrier = carrierOscillator();
    
    // Ring modulation: output = input * carrier
    float modulatedLeft = input[0] * carrier;
    float modulatedRight = input[1] * carrier;
    
    // Mix dry and wet signals
    float dryLeft = input[0] * (1.0f - mix_);
    float dryRight = input[1] * (1.0f - mix_);
    
    float wetLeft = modulatedLeft * mix_;
    float wetRight = modulatedRight * mix_;
    
    return {
        dryLeft + wetLeft,
        dryRight + wetRight
    };
}

void RingModulator::reset() {
    carrierPhase_ = 0.0f;
}

} // namespace dj
