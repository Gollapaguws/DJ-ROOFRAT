#include "gameplay/GameModes.h"

#include <algorithm>

namespace dj {

namespace {

int tierFromReputation(float reputation) {
    if (reputation < 20.0f) {
        return 0;
    }
    if (reputation < 45.0f) {
        return 1;
    }
    if (reputation < 70.0f) {
        return 2;
    }
    if (reputation < 90.0f) {
        return 3;
    }
    return 4;
}

} // namespace

void ScoringSystem::reset() {
    score_ = 0;
}

int ScoringSystem::update(float crowdEnergy, float transitionSmoothness) {
    const float energy = std::clamp(crowdEnergy, 0.0f, 1.0f);
    const float smooth = std::clamp(transitionSmoothness, 0.0f, 1.0f);

    const int base = static_cast<int>((energy * 90.0f) + (smooth * 40.0f));
    const int bonus = (energy > 0.75f && smooth > 0.7f) ? 25 : 0;
    score_ += base + bonus;
    return score_;
}

int ScoringSystem::score() const {
    return score_;
}

void CareerProgression::reset() {
    reputation_ = 10.0f;
    peakTier_ = 0;
}

void CareerProgression::update(float crowdEnergy) {
    const float centered = std::clamp(crowdEnergy, 0.0f, 1.0f) - 0.45f;
    reputation_ += centered * 3.0f;
    reputation_ = std::clamp(reputation_, 0.0f, 100.0f);
    peakTier_ = std::max(peakTier_, tierFromReputation(reputation_));
}

int CareerProgression::tier() const {
    return peakTier_;
}

std::string CareerProgression::currentVenueName() const {
    switch (tier()) {
    case 0:
        return "Basement Bar";
    case 1:
        return "City Club";
    case 2:
        return "Rooftop Session";
    case 3:
        return "Arena Showcase";
    case 4:
        return "Festival Main Stage";
    default:
        return "Unknown Venue";
    }
}

float CareerProgression::reputation() const {
    return reputation_;
}

int CareerProgression::peakTier() const {
    return peakTier_;
}

bool CareerProgression::isVenueUnlocked(int venueTier) const {
    return venueTier <= peakTier_;
}

} // namespace dj
