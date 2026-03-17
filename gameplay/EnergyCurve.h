#pragma once

#include <deque>
#include <vector>

namespace dj {

struct EnergySample {
    float energy;       // Energy level (0.0 - 1.0)
    double timestamp;   // Unix timestamp or elapsed time in seconds
    float rms;          // RMS amplitude
};

class EnergyCurve {
public:
    // Constructor: timeWindow in seconds (e.g., 1800 for 30 min), sampleRate in Hz (e.g., 1.0)
    EnergyCurve(double timeWindow, double sampleRate);
    
    // Add a new energy sample to the curve
    void addSample(float energy, double timestamp, float rms);
    
    // Get the full curve as a vector of samples
    std::vector<EnergySample> getCurve() const;
    
    // Get statistical measures
    float getAverage() const;
    float getPeak() const;
    float getDip() const;
    
private:
    double timeWindow_;      // Time window in seconds
    double sampleRate_;      // Sample rate in Hz
    size_t maxSamples_;      // Maximum number of samples to keep
    std::deque<EnergySample> samples_;
};

} // namespace dj
