#include "gameplay/MissionSystem.h"

namespace dj {

MissionSystem::MissionSystem()
    : currentMission_(nullptr), state_(MissionState::Idle), startBlock_(0) {}

void MissionSystem::startMission(std::shared_ptr<Mission> mission) {
    currentMission_ = mission;
    state_ = MissionState::Running;
    startBlock_ = 0;
    if (currentMission_) {
        currentMission_->setup();
    }
}

void MissionSystem::updateMission(int blockNumber) {
    if (!currentMission_ || state_ != MissionState::Running) {
        return;
    }
    
    currentMission_->update(blockNumber);
    
    if (currentMission_->isComplete()) {
        state_ = MissionState::Complete;
    } else if (currentMission_->isFailed()) {
        state_ = MissionState::Failed;
    }
}

bool MissionSystem::isMissionActive() const {
    return state_ == MissionState::Running;
}

MissionState MissionSystem::getMissionState() const {
    return state_;
}

std::shared_ptr<Mission> MissionSystem::getCurrentMission() const {
    return currentMission_;
}

float MissionSystem::getMissionScore() const {
    if (!currentMission_) {
        return 0.0f;
    }
    return currentMission_->getScore();
}

int MissionSystem::getMissionElapsedBlocks() const {
    if (!currentMission_) {
        return 0;
    }
    // For now, return 0; can be enhanced to track elapsed blocks
    return 0;
}

}  // namespace dj
