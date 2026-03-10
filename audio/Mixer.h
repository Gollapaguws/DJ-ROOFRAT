#pragma once

#include <cstddef>
#include <utility>
#include <vector>

#include "audio/Deck.h"

namespace dj {

struct MixMetrics {
    float rms = 0.0f;
    float transitionSmoothness = 1.0f;
    float deckAEnergy = 0.0f;
    float deckBEnergy = 0.0f;
};

class Mixer {
public:
    void setCrossfader(float value);
    float crossfader() const;

    void setDeckTrim(float a, float b);
    void setMasterGain(float gain);

    std::pair<float, float> crossfadeGains() const;
    std::vector<float> mixBlock(Deck& deckA, Deck& deckB, std::size_t frames, MixMetrics& metrics);

private:
    float crossfader_ = -1.0f;
    float previousCrossfader_ = -1.0f;
    float deckTrimA_ = 1.0f;
    float deckTrimB_ = 1.0f;
    float masterGain_ = 1.0f;
};

} // namespace dj
