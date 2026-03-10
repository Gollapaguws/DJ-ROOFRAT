#include "crowdAI/CrowdStateMachine.h"

#include <algorithm>

namespace dj {

namespace {

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

CrowdOutput CrowdStateMachine::update(float bpm, float transitionSmoothness, float trackEnergy) {
    const float bpmNorm = std::clamp((bpm - 80.0f) / 90.0f, 0.0f, 1.0f);
    const float smoothNorm = std::clamp(transitionSmoothness, 0.0f, 1.0f);
    const float energyNorm = std::clamp(trackEnergy, 0.0f, 1.0f);

    const float target = (bpmNorm * 0.35f) + (smoothNorm * 0.30f) + (energyNorm * 0.35f);
    energy_ += (target - energy_) * 0.18f;

    if (smoothNorm < 0.2f) {
        energy_ *= 0.94f;
    }

    energy_ = std::clamp(energy_, 0.0f, 1.0f);

    if (energy_ < 0.28f) {
        mood_ = CrowdMood::Unimpressed;
    } else if (energy_ < 0.55f) {
        mood_ = CrowdMood::Calm;
    } else if (energy_ < 0.8f) {
        mood_ = CrowdMood::Grooving;
    } else {
        mood_ = CrowdMood::Hyped;
    }

    CrowdOutput output;
    output.energyMeter = energy_;
    output.mood = mood_;
    output.reaction = selectReaction(mood_, smoothNorm);
    return output;
}

} // namespace dj
