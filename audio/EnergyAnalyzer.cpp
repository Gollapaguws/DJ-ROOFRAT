#include "audio/EnergyAnalyzer.h"

#include <cmath>
#include <algorithm>

namespace dj {

EnergyAnalyzer::EnergyAnalyzer()
    : sumSquares_(0.0), sampleCount_(0), peakAmplitude_(0.0f) {}

void EnergyAnalyzer::processSamples(const float* samples, size_t count) {
    if (!samples || count == 0) {
        return;
    }
    
    for (size_t i = 0; i < count; ++i) {
        float sample = samples[i];
        
        // Accumulate sum of squares for RMS
        sumSquares_ += static_cast<double>(sample * sample);
        sampleCount_ += 1;
        
        // Track peak amplitude (absolute value)
        float absValue = std::abs(sample);
        if (absValue > peakAmplitude_) {
            peakAmplitude_ = absValue;
        }
    }
}

float EnergyAnalyzer::getRMS() const {
    if (sampleCount_ == 0) {
        return 0.0f;
    }
    
    double meanSquare = sumSquares_ / sampleCount_;
    return static_cast<float>(std::sqrt(meanSquare));
}

float EnergyAnalyzer::getPeak() const {
    return peakAmplitude_;
}

float EnergyAnalyzer::getDynamicRange() const {
    float rms = getRMS();
    
    // Avoid division by zero via epsilon check
    if (rms < 1e-6f) {
        return 0.0f;
    }
    
    float ratio = peakAmplitude_ / rms;
    
    // Convert ratio to dB scale: 20 * log10(ratio)
    return 20.0f * std::log10(ratio);
}

int EnergyAnalyzer::calculateRating(float rms) const {
    // Map RMS to 1-10 energy scale
    if (rms < 0.10f) return 1;   // Very low energy (silence, ambient pads)
    if (rms < 0.15f) return 2;   // Low energy (ambient, downtempo)
    if (rms < 0.20f) return 3;   // Low-medium (chill, lounge)
    if (rms < 0.25f) return 4;   // Low-medium (laid back)
    if (rms < 0.30f) return 5;   // Medium (warm-up, groovy)
    if (rms < 0.35f) return 6;   // Medium-high (building energy)
    if (rms < 0.40f) return 7;   // High (main set, upbeat)
    if (rms < 0.45f) return 8;   // High (peak approaching)
    if (rms < 0.50f) return 9;   // Very high (peak hour)
    return 10;                    // Maximum (bangers, drops)
}

int EnergyAnalyzer::getEnergyRating() const {
    return calculateRating(getRMS());
}

std::string EnergyAnalyzer::getEnergyLabel() const {
    int rating = getEnergyRating();
    
    if (rating <= 2) return "Ambient";
    if (rating <= 4) return "Chill";
    if (rating <= 6) return "Moderate";
    if (rating <= 8) return "Upbeat";
    return "Peak Hour";
}

void EnergyAnalyzer::reset() {
    sumSquares_ = 0.0;
    sampleCount_ = 0;
    peakAmplitude_ = 0.0f;
}

} // namespace dj
