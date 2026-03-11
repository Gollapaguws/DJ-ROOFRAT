#pragma once

#include <array>
#include <vector>

namespace dj {

class Phaser {
public:
    Phaser();
    
    void setSampleRate(int sampleRate);
    void setStages(int stages);           // 4, 6, 8, or 12 allpass stages
    void setLFORate(float rateHz);        // 0.1 - 10 Hz
    void setFeedback(float feedback);     // 0.0 - 0.95 (resonance/intensity)
    void setMix(float mix);               // 0.0 = dry, 1.0 = wet
    
    std::array<float, 2> process(const std::array<float, 2>& input);
    void reset();

private:
    struct AllpassFilter {
        float stateX = 0.0f;  // x[n-1]
        float stateY = 0.0f;  // y[n-1]
        float frequency = 500.0f;
        float a1 = 0.0f;  // IIR coefficient
        
        void setFrequency(float freq, int sampleRate);
        float process(float input);
        void reset();
    };
    
    // Support up to 12 stages
    std::vector<AllpassFilter> allpassFiltersLeft_;
    std::vector<AllpassFilter> allpassFiltersRight_;
    
    int numStages_ = 4;
    float lfoPhase_ = 0.0f;
    float lfoRate_ = 1.0f;
    int sampleRate_ = 44100;
    
    float feedback_ = 0.5f;
    float mix_ = 0.5f;
    float feedbackStateLeft_ = 0.0f;
    float feedbackStateRight_ = 0.0f;
    
    // Allpass frequency modulation range
    static constexpr float MIN_FREQ = 200.0f;   // Hz
    static constexpr float MAX_FREQ = 2000.0f;  // Hz
    
    float sineLFO();
};

} // namespace dj
