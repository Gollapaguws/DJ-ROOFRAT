#include "gameplay/missions/SustainedBeatmatchMission.h"

#include <cmath>
#include <algorithm>

namespace dj {

SustainedBeatmatchMission::SustainedBeatmatchMission()
    : startBlock_(0), matchStartBlock_(-1), complete_(false), failed_(false) {}

void SustainedBeatmatchMission::setup() {
    startBlock_ = 0;
    matchStartBlock_ = -1;
    complete_ = false;
    failed_ = false;
}

void SustainedBeatmatchMission::update(int blockNumber) {
    // Placeholder: validation done via validateBeatmatch
}

void SustainedBeatmatchMission::teardown() {
    // Cleanup if needed
}

bool SustainedBeatmatchMission::isComplete() const {
    return complete_;
}

bool SustainedBeatmatchMission::isFailed() const {
    return failed_;
}

std::string SustainedBeatmatchMission::getName() const {
    return "SustainedBeatmatch";
}

std::string SustainedBeatmatchMission::getDescription() const {
    return "Maintain BPM delta < 2% for 30 seconds";
}

float SustainedBeatmatchMission::getScore() const {
    if (matchStartBlock_ == -1) {
        return 0.0f;
    }
    // Return the duration sustained in blocks (simplified)
    return static_cast<float>(matchStartBlock_);
}

float SustainedBeatmatchMission::getProgress() const {
    if (matchStartBlock_ == -1) {
        return 0.0f;
    }
    return std::clamp(static_cast<float>(matchStartBlock_) / DURATION_BLOCKS, 0.0f, 1.0f);
}

void SustainedBeatmatchMission::validateBeatmatch(float bpmA, float bpmB, int blockNumber) {
    // Handle zero/negative BPM
    if (bpmA <= 0.0f || bpmB <= 0.0f) {
        return;
    }
    
    // Calculate delta percentage
    float delta = (bpmA > bpmB) ? (bpmA - bpmB) : (bpmB - bpmA);
    float percentDelta = (delta / bpmA) * 100.0f;
    
    if (percentDelta < TOLERANCE_PERCENT) {
        // Within tolerance
        if (matchStartBlock_ == -1) {
            matchStartBlock_ = 1;  // Start counting - 1 block achieved
        } else {
            matchStartBlock_++;
            if (matchStartBlock_ >= DURATION_BLOCKS) {
                complete_ = true;
            }
        }
    } else {
        // Out of tolerance - reset
        matchStartBlock_ = -1;
    }
}

}  // namespace dj
