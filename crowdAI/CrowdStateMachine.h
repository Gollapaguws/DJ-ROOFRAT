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

    CrowdOutput update(float bpm, float transitionSmoothness, float trackEnergy);

private:
    CrowdPersonality personality_;
    float energy_ = 0.35f;
    CrowdMood mood_ = CrowdMood::Calm;
    float energyBuffer_ = 0.35f;
    int hysteresisCounter_ = 0;
};

} // namespace dj
