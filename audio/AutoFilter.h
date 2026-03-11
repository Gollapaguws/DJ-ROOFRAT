#pragma once

#include <array>

namespace dj {

class AutoFilter {
public:
    AutoFilter();
    
    void setSampleRate(int sampleRate);
    void setAttack(float attackMs);      // 1 - 100 ms
    void setRelease(float releaseMs);    // 10 - 1000 ms
    void setSensitivity(float sens);     // 0.0 - 1.0 (how much envelope affects filter)
    void setResonance(float resonance);  // 0.0 - 0.9 (filter Q)
    void setMix(float mix);              // 0.0 = dry, 1.0 = wet
    
    std::array<float, 2> process(const std::array<float, 2>& input);
    void reset();

private:
    int sampleRate_ = 44100;
    float attackMs_ = 10.0f;
    float releaseMs_ = 100.0f;
    float sensitivity_ = 1.0f;
    float resonance_ = 0.5f;
    float mix_ = 0.5f;
    
    // Envelope follower
    float envelope_ = 0.0f;
    float attackCoeff_ = 0.0f;   // Fast rise
    float releaseCoeff_ = 0.0f;  // Slower fall
    
    // Peak detector state (tracks max of input)
    float lastLeft_ = 0.0f;
    float lastRight_ = 0.0f;
    
    // Resonant filter state
    struct FilterState {
        float z1 = 0.0f;  // Filter memory
        float z2 = 0.0f;
    } filterLeft_, filterRight_;
    
    void updateCoefficients();
    float resonantLowpass(float input, FilterState& state, float cutoffHz);
};

} // namespace dj
