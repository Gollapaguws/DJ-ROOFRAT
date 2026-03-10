#pragma once

#include <array>

namespace dj {

class ThreeBandEQ {
public:
    void setSampleRate(int sampleRate);
    void setGains(float low, float mid, float high);
    void setLowMidCrossover(float hz);
    void setMidHighCrossover(float hz);
    
    static float gainTodB(float linear);
    static float dBToGain(float dB);
    
    std::array<float, 2> process(const std::array<float, 2>& input);
    void reset();

private:
    struct ChannelState {
        float low = 0.0f;
        float prevInput = 0.0f;
        float prevHigh = 0.0f;
    };

    float processSingle(float input, ChannelState& state, float alphaLow, float alphaHigh);

    int sampleRate_ = 44100;
    float lowGain_ = 1.0f;
    float midGain_ = 1.0f;
    float highGain_ = 1.0f;
    float lowMidCrossoverHz_ = 220.0f;
    float midHighCrossoverHz_ = 2200.0f;

    ChannelState left_;
    ChannelState right_;
};

} // namespace dj
