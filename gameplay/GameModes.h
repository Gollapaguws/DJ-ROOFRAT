#pragma once

#include <string>

namespace dj {

enum class PlayMode {
    Career,
    Sandbox,
};

class ScoringSystem {
public:
    void reset();
    int update(float crowdEnergy, float transitionSmoothness);
    int score() const;

private:
    int score_ = 0;
};

class CareerProgression {
public:
    void reset();
    void update(float crowdEnergy);
    int tier() const;
    std::string currentVenueName() const;

private:
    float reputation_ = 10.0f;
    int peakTier_ = 0;
};

} // namespace dj
