#pragma once

#include <array>
#include <vector>

namespace dj {

class Delay {
public:
    Delay();
    
    void setSampleRate(int sampleRate);
    void setTempoSync(float bpm, float noteValue);  // noteValue: 1.0=quarter, 0.5=eighth, 0.25=sixteenth
    void setFeedback(float feedback);  // 0.0-0.95 (clamped)
    void setWetDryMix(float wetDryMix);  // 0.0 = dry, 1.0 = wet
    
    std::array<float, 2> process(const std::array<float, 2>& input);
    void reset();

private:
    // Circular buffer for stereo delay lines
    std::vector<float> delayBufferLeft_;
    std::vector<float> delayBufferRight_;
    
    int writeIndex_ = 0;
    
    float feedback_ = 0.0f;
    float wetDryMix_ = 0.5f;
    int sampleRate_ = 44100;
    
    // Max delay time: 4 seconds @ 44.1kHz = 176400 samples
    static constexpr int MAX_DELAY_SAMPLES = 4 * 44100;
    
    int delayTimeInSamples_ = 22050;  // Default: ~500ms @ 44.1kHz
};

} // namespace dj
