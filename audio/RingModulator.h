#pragma once

#include <array>

namespace dj {

class RingModulator {
public:
    RingModulator();
    
    void setSampleRate(int sampleRate);
    void setCarrierFrequency(float frequencyHz);  // 20 Hz - 5000 Hz
    void setMix(float mix);                       // 0.0 = dry, 1.0 = wet
    
    std::array<float, 2> process(const std::array<float, 2>& input);
    void reset();

private:
    int sampleRate_ = 44100;
    float carrierFrequency_ = 440.0f;
    float mix_ = 0.5f;
    
    // Carrier oscillator phase
    float carrierPhase_ = 0.0f;
    
    float carrierOscillator();  // Sine wave carrier
};

} // namespace dj
