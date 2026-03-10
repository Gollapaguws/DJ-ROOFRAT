#pragma once

#include <string>

namespace dj {

enum class CrowdMood {
    Unimpressed,
    Calm,
    Grooving,
    Hyped,
};

struct CrowdOutput {
    float energyMeter = 0.0f;
    std::string reaction;
    CrowdMood mood = CrowdMood::Calm;
};

class CrowdStateMachine {
public:
    CrowdOutput update(float bpm, float transitionSmoothness, float trackEnergy);

private:
    float energy_ = 0.35f;
    CrowdMood mood_ = CrowdMood::Calm;
};

} // namespace dj
