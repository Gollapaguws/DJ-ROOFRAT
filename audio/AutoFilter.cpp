#include "audio/AutoFilter.h"

#include <algorithm>
#include <cmath>

namespace dj {

AutoFilter::AutoFilter() {
    updateCoefficients();
}

void AutoFilter::setSampleRate(int sampleRate) {
    sampleRate_ = sampleRate;
    updateCoefficients();
}

void AutoFilter::setAttack(float attackMs) {
    attackMs_ = std::clamp(attackMs, 1.0f, 100.0f);
    updateCoefficients();
}

void AutoFilter::setRelease(float releaseMs) {
    releaseMs_ = std::clamp(releaseMs, 10.0f, 1000.0f);
    updateCoefficients();
}

void AutoFilter::setSensitivity(float sens) {
    sensitivity_ = std::clamp(sens, 0.0f, 1.0f);
}

void AutoFilter::setResonance(float resonance) {
    resonance_ = std::clamp(resonance, 0.0f, 0.9f);
}

void AutoFilter::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

void AutoFilter::updateCoefficients() {
    // Calculate envelope follower coefficients
    // Attack: time to reach 63% of target from zero (1 time constant)
    float attackSamples = (attackMs_ / 1000.0f) * sampleRate_;
    attackCoeff_ = std::exp(-1.0f / (attackSamples + 1.0f));
    
    // Release: time to decay to 37% of peak
    float releaseSamples = (releaseMs_ / 1000.0f) * sampleRate_;
    releaseCoeff_ = std::exp(-1.0f / (releaseSamples + 1.0f));
}

float AutoFilter::resonantLowpass(float input, FilterState& state, float cutoffHz) {
    // Simple one-pole lowpass filter for stability
    if (cutoffHz <= 20.0f) cutoffHz = 20.0f;
    if (cutoffHz >= 20000.0f) cutoffHz = 20000.0f;
    
    // Calculate filter coefficient (one-pole lowpass)
    float wc = 2.0f * 3.14159265359f * cutoffHz / sampleRate_;
    if (wc > 1.0f) wc = 1.0f;
    
    // Resonance adds feedback-like behavior
    float resonantGain = 1.0f + resonance_ * 2.0f;
    
    // Simple one-pole IIR: y[n] = alpha*x[n] + (1-alpha)*y[n-1]
    float alpha = wc;
    float output = alpha * input + (1.0f - alpha) * state.z1;
    state.z1 = output;
    
    // Apply resonance by mixing back (feedback)
    output = output * resonantGain;
    output = std::clamp(output, -5.0f, 5.0f);  // Clamp to prevent overflow
    
    return output;
}

std::array<float, 2> AutoFilter::process(const std::array<float, 2>& input) {
    // Detect envelope from input signal (peak detection)
    float inputEnergy = std::abs(input[0]) + std::abs(input[1]);
    inputEnergy /= 2.0f;  // Average
    
    // Envelope follower with attack/release
    if (inputEnergy > envelope_) {
        // Attack: fast rise to new envelope
        envelope_ = attackCoeff_ * envelope_ + (1.0f - attackCoeff_) * inputEnergy;
    } else {
        // Release: slow decay back to zero
        envelope_ = releaseCoeff_ * envelope_;
    }
    
    // Map envelope to filter cutoff frequency
    // Min cutoff: 200 Hz, Max cutoff: 5000 Hz (modulated by sensitivity)
    float minCutoff = 200.0f;
    float maxCutoff = 5000.0f;
    float cutoff = minCutoff + (maxCutoff - minCutoff) * envelope_ * sensitivity_;
    
    // Apply resonant lowpass filter to both channels
    float filteredLeft = resonantLowpass(input[0], filterLeft_, cutoff);
    float filteredRight = resonantLowpass(input[1], filterRight_, cutoff);
    
    // Mix dry and wet signals
    float dryLeft = input[0] * (1.0f - mix_);
    float dryRight = input[1] * (1.0f - mix_);
    
    float wetLeft = filteredLeft * mix_;
    float wetRight = filteredRight * mix_;
    
    return {
        dryLeft + wetLeft,
        dryRight + wetRight
    };
}

void AutoFilter::reset() {
    envelope_ = 0.0f;
    lastLeft_ = 0.0f;
    lastRight_ = 0.0f;
    filterLeft_ = {0.0f, 0.0f};
    filterRight_ = {0.0f, 0.0f};
}

} // namespace dj
