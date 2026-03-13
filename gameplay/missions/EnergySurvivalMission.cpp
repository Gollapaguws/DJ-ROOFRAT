#include "gameplay/missions/EnergySurvivalMission.h"

#include <algorithm>

namespace dj {

EnergySurvivalMission::EnergySurvivalMission()
    : startBlock_(0), energySustainBlocks_(0), complete_(false), failed_(false) {}

void EnergySurvivalMission::setup() {
    startBlock_ = 0;
    energySustainBlocks_ = 0;
    complete_ = false;
    failed_ = false;
}

void EnergySurvivalMission::update(int blockNumber) {
    // Placeholder: validation done via validateEnergy
}

void EnergySurvivalMission::teardown() {
    // Cleanup if needed
}

bool EnergySurvivalMission::isComplete() const {
    return complete_;
}

bool EnergySurvivalMission::isFailed() const {
    return failed_;
}

std::string EnergySurvivalMission::getName() const {
    return "EnergySurvival";
}

std::string EnergySurvivalMission::getDescription() const {
    return "Keep crowd energy > 0.5 for 60 seconds";
}

float EnergySurvivalMission::getScore() const {
    return static_cast<float>(energySustainBlocks_);
}

float EnergySurvivalMission::getProgress() const {
    return std::clamp(static_cast<float>(energySustainBlocks_) / DURATION_BLOCKS, 0.0f, 1.0f);
}

void EnergySurvivalMission::validateEnergy(float crowdEnergy, int blockNumber) {
    if (crowdEnergy > ENERGY_THRESHOLD) {
        energySustainBlocks_++;
        if (energySustainBlocks_ >= DURATION_BLOCKS) {
            complete_ = true;
        }
    } else {
        // Energy dropped below threshold - instant failure
        failed_ = true;
    }
}

}  // namespace dj
