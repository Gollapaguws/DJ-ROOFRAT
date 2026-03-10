#pragma once

#include <string>

namespace dj {

enum class CrowdMood {
    Unimpressed,
    Calm,
    Grooving,
    Hyped,
};

enum class CrowdPersonality {
    Default,
    Rave,
    Jazz,
    EDM,
};

struct CrowdOutput {
    float energyMeter = 0.0f;
    std::string reaction;
    CrowdMood mood = CrowdMood::Calm;
};

class CrowdStateMachine {
public:
    explicit CrowdStateMachine(CrowdPersonality personality = CrowdPersonality::Default);

    // Phase 6: Extended signature with beatmatch delta input
    CrowdOutput update(float bpm, float transitionSmoothness, float trackEnergy, float beatmatchDelta);

    // Phase 5 backward compatibility: default beatmatchDelta = 2.0f (neutral)
    CrowdOutput update(float bpm, float transitionSmoothness, float trackEnergy) {
        return update(bpm, transitionSmoothness, trackEnergy, 2.0f);
    }

private:
    CrowdPersonality personality_;
    float energy_ = 0.35f;
    CrowdMood mood_ = CrowdMood::Calm;
    float energyBuffer_ = 0.35f;
    int hysteresisCounter_ = 0;
    int reactionIndex_ = 0;  // For round-robin reaction selection
};

} // namespace dj
