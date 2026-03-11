#pragma once

#include <array>
#include <vector>
#include <cmath>

namespace dj {

class Flanger {
public:
    Flanger();
    
    void setSampleRate(int sampleRate);
    void setLFORate(float rateHz);        // 0.1 - 10 Hz
    void setDepth(float depth);           // 0.0 - 1.0 (modulation amount)
    void setMix(float mix);               // 0.0 = dry, 1.0 = wet
    void setFeedback(float feedback);     // Optional: -1.0 to 1.0 for jet flanger
    
    std::array<float, 2> process(const std::array<float, 2>& input);
    void reset();

private:
    // Delay line for each channel
    std::vector<float> delayLineLeft_;
    std::vector<float> delayLineRight_;
    
    // LFO oscillator
    float lfoPhase_ = 0.0f;
    float lfoRate_ = 1.0f;      // Hz
    int sampleRate_ = 44100;
    
    // Effect parameters
    float depth_ = 0.5f;        // Modulation depth (0.0 - 1.0)
    float mix_ = 0.5f;          // Wet/dry mix
    float feedback_ = 0.0f;     // Feedback amount
    
    // Delay range: 0.5ms - 10ms (22 - 441 samples @ 44.1kHz)
    static constexpr float MIN_DELAY_MS = 0.5f;
    static constexpr float MAX_DELAY_MS = 10.0f;
    
    // Circular buffer index for both channels
    int writeIndex_ = 0;
    
    float sineLFO();  // Internal LFO generator
};

} // namespace dj
