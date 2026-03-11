#include "audio/Phaser.h"

#include <algorithm>
#include <cmath>

namespace dj {

void Phaser::AllpassFilter::setFrequency(float freq, int sampleRate) {
    frequency = std::clamp(freq, 200.0f, 2000.0f);
    
    // Calculate IIR coefficient for first-order allpass:
    // a1 = (tan(pi*freq/sr) - 1) / (tan(pi*freq/sr) + 1)
    float pi = 3.14159265359f;
    float wc = pi * frequency / sampleRate;
    float tanWc = std::tan(wc);
    a1 = (tanWc - 1.0f) / (tanWc + 1.0f);
}

float Phaser::AllpassFilter::process(float input) {
    // First-order allpass filter: H(z) = (a1 + z^-1) / (1 + a1*z^-1)
    // Difference equation: y[n] = a1*x[n] + x[n-1] - a1*y[n-1]
    float output = a1 * input + stateX - a1 * stateY;
    stateX = input;
    stateY = output;
    return output;
}

void Phaser::AllpassFilter::reset() {
    stateX = 0.0f;
    stateY = 0.0f;
}

Phaser::Phaser() {
    // Initialize filter stages (4 by default)
    allpassFiltersLeft_.resize(12);   // Max stages
    allpassFiltersRight_.resize(12);
    
    for (auto& f : allpassFiltersLeft_) {
        f.setFrequency(500.0f, sampleRate_);
    }
    for (auto& f : allpassFiltersRight_) {
        f.setFrequency(500.0f, sampleRate_);
    }
}

void Phaser::setSampleRate(int sampleRate) {
    sampleRate_ = sampleRate;
    
    // Update all filter coefficients
    for (auto& f : allpassFiltersLeft_) {
        f.setFrequency(f.frequency, sampleRate_);
    }
    for (auto& f : allpassFiltersRight_) {
        f.setFrequency(f.frequency, sampleRate_);
    }
}

void Phaser::setStages(int stages) {
    numStages_ = std::clamp(stages, 4, 12);
}

void Phaser::setLFORate(float rateHz) {
    lfoRate_ = std::clamp(rateHz, 0.1f, 10.0f);
}

void Phaser::setFeedback(float feedback) {
    feedback_ = std::clamp(feedback, 0.0f, 0.95f);
}

void Phaser::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

float Phaser::sineLFO() {
    float value = std::sin(2.0f * 3.14159265359f * lfoPhase_);
    
    float phaseDelta = lfoRate_ / sampleRate_;
    lfoPhase_ += phaseDelta;
    
    if (lfoPhase_ >= 1.0f) {
        lfoPhase_ -= 1.0f;
    }
    
    return value;
}

std::array<float, 2> Phaser::process(const std::array<float, 2>& input) {
    // Get LFO value and modulate allpass frequencies
    float lfo = sineLFO();
    
    // Map LFO [-1, 1] to frequency range [MIN_FREQ, MAX_FREQ]
    float centerFreq = (MIN_FREQ + MAX_FREQ) / 2.0f;
    float freqRange = (MAX_FREQ - MIN_FREQ) / 2.0f;
    float modulatedFreq = centerFreq + (lfo * freqRange);
    
    // Update all stage frequencies
    for (auto& f : allpassFiltersLeft_) {
        f.setFrequency(modulatedFreq, sampleRate_);
    }
    for (auto& f : allpassFiltersRight_) {
        f.setFrequency(modulatedFreq, sampleRate_);
    }
    
    // Process left channel: feed output back to input (true feedback)
    float chainInputLeft = input[0] + feedbackStateLeft_ * feedback_;
    float phaseLeft = chainInputLeft;
    for (int i = 0; i < numStages_; ++i) {
        phaseLeft = allpassFiltersLeft_[i].process(phaseLeft);
    }
    feedbackStateLeft_ = phaseLeft;  // Store output for next iteration
    
    // Process right channel: same pattern
    float chainInputRight = input[1] + feedbackStateRight_ * feedback_;
    float phaseRight = chainInputRight;
    for (int i = 0; i < numStages_; ++i) {
        phaseRight = allpassFiltersRight_[i].process(phaseRight);
    }
    feedbackStateRight_ = phaseRight;  // Store output for next iteration
    
    // Mix dry and wet (phase-shifted) signals
    float dryLeft = input[0] * (1.0f - mix_);
    float dryRight = input[1] * (1.0f - mix_);
    
    float wetLeft = phaseLeft * mix_;
    float wetRight = phaseRight * mix_;
    
    return {
        std::clamp(dryLeft + wetLeft, -2.0f, 2.0f),
        std::clamp(dryRight + wetRight, -2.0f, 2.0f)
    };
}

void Phaser::reset() {
    for (auto& f : allpassFiltersLeft_) {
        f.reset();
    }
    for (auto& f : allpassFiltersRight_) {
        f.reset();
    }
    feedbackStateLeft_ = 0.0f;
    feedbackStateRight_ = 0.0f;
    lfoPhase_ = 0.0f;
}

} // namespace dj
