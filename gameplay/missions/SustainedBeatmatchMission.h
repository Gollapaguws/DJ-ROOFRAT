#pragma once

#include "gameplay/Mission.h"

namespace dj {

class SustainedBeatmatchMission : public Mission {
public:
    SustainedBeatmatchMission();
    
    void setup() override;
    void update(int blockNumber) override;
    void teardown() override;
    
    bool isComplete() const override;
    bool isFailed() const override;
    std::string getName() const override;
    std::string getDescription() const override;
    
    float getScore() const override;
    float getProgress() const override;
    
    // Validation interface (for testing)
    void validateBeatmatch(float bpmA, float bpmB, int blockNumber);
    
private:
    static constexpr float TOLERANCE_PERCENT = 2.0f;
    static constexpr int DURATION_BLOCKS = 1323000;  // 30 seconds at 44.1kHz
    
    int startBlock_;
    int matchStartBlock_;
    bool complete_;
    bool failed_;
};

}  // namespace dj
