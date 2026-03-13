#include "gameplay/missions/TransitionMission.h"

#include <algorithm>

namespace dj {

TransitionMission::TransitionMission()
    : startBlock_(0), bestSmoothness_(0.0f), complete_(false) {}

void TransitionMission::setup() {
    startBlock_ = 0;
    bestSmoothness_ = 0.0f;
    complete_ = false;
}

void TransitionMission::update(int blockNumber) {
    // Placeholder: validation done via validateTransition
}

void TransitionMission::teardown() {
    // Cleanup if needed
}

bool TransitionMission::isComplete() const {
    return complete_;
}

bool TransitionMission::isFailed() const {
    // No instant failure - allow multiple attempts
    return false;
}

std::string TransitionMission::getName() const {
    return "Transition";
}

std::string TransitionMission::getDescription() const {
    return "Execute smooth crossfade with smoothness > 0.7";
}

float TransitionMission::getScore() const {
    return bestSmoothness_;
}

float TransitionMission::getProgress() const {
    if (complete_) {
        return 1.0f;
    }
    return bestSmoothness_;  // Progress as smoothness achieved so far
}

void TransitionMission::validateTransition(float transitionSmoothness, int blockNumber) {
    if (transitionSmoothness > bestSmoothness_) {
        bestSmoothness_ = transitionSmoothness;
    }
    
    if (transitionSmoothness > SMOOTHNESS_THRESHOLD) {
        complete_ = true;
    }
}

}  // namespace dj
