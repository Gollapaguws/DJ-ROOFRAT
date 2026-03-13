#pragma once

#include "gameplay/Mission.h"

namespace dj {

class TransitionMission : public Mission {
public:
    TransitionMission();
    
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
    void validateTransition(float transitionSmoothness, int blockNumber);
    
private:
    static constexpr float SMOOTHNESS_THRESHOLD = 0.7f;
    
    int startBlock_;
    float bestSmoothness_;
    bool complete_;
};

}  // namespace dj
