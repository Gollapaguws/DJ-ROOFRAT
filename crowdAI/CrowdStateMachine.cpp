#include "crowdAI/CrowdStateMachine.h"

#include <algorithm>

namespace dj {

namespace {

struct PersonalityConfig {
    float bpmWeight;
    float smoothnessWeight;
    float energyWeight;
    float decayFactor;
    float moodUpThreshold;
    float moodDownThreshold;
    float hysteresisFrames;
};

PersonalityConfig getConfig(CrowdPersonality personality) {
    switch (personality) {
    case CrowdPersonality::Rave:
        return {0.60f, 0.20f, 0.20f, 0.88f, 0.52f, 0.42f, 5.0f};
    case CrowdPersonality::Jazz:
        return {0.15f, 0.45f, 0.40f, 0.96f, 0.70f, 0.55f, 10.0f};
    case CrowdPersonality::EDM:
        return {0.50f, 0.20f, 0.30f, 0.92f, 0.45f, 0.40f, 3.0f};
    case CrowdPersonality::Default:
    default:
        return {0.35f, 0.30f, 0.35f, 0.94f, 0.55f, 0.50f, 8.0f};
    }
}

std::string selectReaction(CrowdMood mood, float transitionSmoothness) {
    if (transitionSmoothness < 0.2f) {
        return "Boos";
    }

    switch (mood) {
    case CrowdMood::Unimpressed:
        return "Murmurs";
    case CrowdMood::Calm:
        return "Head nods";
    case CrowdMood::Grooving:
        return "Cheers";
    case CrowdMood::Hyped:
        return "Roaring crowd";
    }

    return "Neutral";
}

} // namespace

CrowdStateMachine::CrowdStateMachine(CrowdPersonality personality)
    : personality_(personality) {}

CrowdOutput CrowdStateMachine::update(float bpm, float transitionSmoothness, float trackEnergy) {
    const PersonalityConfig config = getConfig(personality_);

    const float bpmNorm = std::clamp((bpm - 80.0f) / 90.0f, 0.0f, 1.0f);
    const float smoothNorm = std::clamp(transitionSmoothness, 0.0f, 1.0f);
    const float energyNorm = std::clamp(trackEnergy, 0.0f, 1.0f);

    const float target = (bpmNorm * config.bpmWeight) + (smoothNorm * config.smoothnessWeight) + (energyNorm * config.energyWeight);
    energy_ += (target - energy_) * 0.18f;

    if (smoothNorm < 0.2f) {
        energy_ *= config.decayFactor;
    }

    energy_ = std::clamp(energy_, 0.0f, 1.0f);

    // Apply hysteresis to prevent mood flicker
    CrowdMood newMood = mood_;
    if (energy_ < 0.28f) {
        newMood = CrowdMood::Unimpressed;
    } else if (energy_ < config.moodUpThreshold) {
        newMood = CrowdMood::Calm;
    } else if (energy_ < 0.8f) {
        newMood = CrowdMood::Grooving;
    } else {
        newMood = CrowdMood::Hyped;
    }

    // Hysteresis: only update mood if we've been stable for enough frames
    // or if we're changing due to significant energy jump
    if (newMood != mood_) {
        const float energyDelta = std::abs(energy_ - energyBuffer_);
        const bool isSignificantChange = energyDelta > 0.1f || newMood == CrowdMood::Hyped || newMood == CrowdMood::Unimpressed;

        if (isSignificantChange || hysteresisCounter_ >= static_cast<int>(config.hysteresisFrames)) {
            mood_ = newMood;
            hysteresisCounter_ = 0;
        } else {
            hysteresisCounter_++;
        }
    } else {
        hysteresisCounter_ = 0;
    }

    energyBuffer_ = energy_;

    CrowdOutput output;
    output.energyMeter = energy_;
    output.mood = mood_;
    output.reaction = selectReaction(mood_, smoothNorm);
    return output;
}

} // namespace dj

