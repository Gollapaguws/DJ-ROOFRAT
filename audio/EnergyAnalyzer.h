#pragma once

#include <string>

namespace dj {

class EnergyAnalyzer {
public:
    EnergyAnalyzer();
    ~EnergyAnalyzer() = default;
    
    // Process audio samples
    // Accumulates RMS, peak, and dynamic metrics
    void processSamples(const float* samples, size_t count);
    
    // Get energy metrics
    float getRMS() const;           // Root Mean Square energy (0.0-1.0)
    float getPeak() const;          // Peak amplitude (0.0-1.0)
    float getDynamicRange() const;  // Dynamic range in dB
    
    // Get 1-10 energy rating based on RMS
    int getEnergyRating() const;
    
    // Get descriptive energy label
    // Returns: "Ambient", "Chill", "Moderate", "Upbeat", or "Peak Hour"
    std::string getEnergyLabel() const;
    
    // Reset analyzer for new track
    void reset();
    
private:
    double sumSquares_;      // Accumulator for RMS calculation
    size_t sampleCount_;     // Number of samples processed
    float peakAmplitude_;    // Maximum absolute amplitude
    
    // Helper: Calculate energy rating from RMS value
    int calculateRating(float rms) const;
};

} // namespace dj
