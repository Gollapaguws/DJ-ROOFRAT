#pragma once

#include "gameplay/Mission.h"

namespace dj {

class EnergySurvivalMission : public Mission {
public:
    EnergySurvivalMission();
    
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
    void validateEnergy(float crowdEnergy, int blockNumber);
    
private:
    static constexpr float ENERGY_THRESHOLD = 0.5f;
    static constexpr int DURATION_BLOCKS = 2646000;  // 60 seconds at 44.1kHz
    
    int startBlock_;
    int energySustainBlocks_;
    bool complete_;
    bool failed_;
};

}  // namespace dj
