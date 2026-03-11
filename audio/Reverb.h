#pragma once

#include <array>
#include <vector>

namespace dj {

class Reverb {
public:
    Reverb();
    
    void setSampleRate(int sampleRate);
    void setRoomSize(float roomSize);
    void setDamping(float damping);
    void setWetDryMix(float wetDryMix);  // 0.0 = dry, 1.0 = wet
    
    std::array<float, 2> process(const std::array<float, 2>& input);
    void reset();

private:
    struct CombFilter {
        std::vector<float> buffer;
        std::size_t bufferIndex = 0;
        float filterStore = 0.0f;
        float damp1 = 0.0f;
        float damp2 = 0.0f;
        float feedback = 0.0f;
        
        void setDamping(float damping);
        float process(float input);
        void reset();
    };
    
    struct AllpassFilter {
        std::vector<float> buffer;
        std::size_t bufferIndex = 0;
        float feedback = 0.5f;
        
        float process(float input);
        void reset();
    };
    
    // Freeverb: 8 parallel comb filters + 4 series allpass filters
    // Separate L/R filter banks for true stereo with no crosstalk
    // Comb filter delays (in samples @ 44.1kHz)
    static constexpr int COMB_TUNINGS[8] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617};
    // Right channel uses offset tunings (classical Freeverb stereo separation)
    static constexpr int COMB_TUNINGS_RIGHT[8] = {1139, 1211, 1300, 1379, 1445, 1514, 1580, 1640};
    static constexpr int ALLPASS_TUNINGS[4] = {556, 441, 341, 225};
    
    // Separate filter banks for each channel
    std::array<CombFilter, 8> combFiltersLeft_;
    std::array<CombFilter, 8> combFiltersRight_;
    std::array<AllpassFilter, 4> allpassFiltersLeft_;
    std::array<AllpassFilter, 4> allpassFiltersRight_;
    
    float roomSize_ = 0.5f;
    float damping_ = 0.5f;
    float wetDryMix_ = 0.5f;
    int sampleRate_ = 44100;
};

} // namespace dj
