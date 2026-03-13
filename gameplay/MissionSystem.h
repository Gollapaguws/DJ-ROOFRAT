#pragma once

#include <memory>
#include <vector>
#include "gameplay/Mission.h"

namespace dj {

enum class MissionState {
    Idle,      // Not started
    Running,   // Currently active
    Complete,  // Success
    Failed     // Failed to meet requirements
};

class MissionSystem {
public:
    MissionSystem();
    
    // Mission lifecycle
    void startMission(std::shared_ptr<Mission> mission);
    void updateMission(int blockNumber);
    bool isMissionActive() const;
    MissionState getMissionState() const;
    std::shared_ptr<Mission> getCurrentMission() const;
    
    // Score/time tracking
    float getMissionScore() const;
    int getMissionElapsedBlocks() const;
    
private:
    std::shared_ptr<Mission> currentMission_;
    MissionState state_;
    int startBlock_;
};

}  // namespace dj
