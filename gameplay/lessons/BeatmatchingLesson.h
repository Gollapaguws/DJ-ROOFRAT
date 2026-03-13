#pragma once

#include "gameplay/Lesson.h"

namespace dj {

class BeatmatchingLesson : public Lesson {
public:
    BeatmatchingLesson();
    ~BeatmatchingLesson() = default;
    
    void setup(float baselineBpmA, float baselineBpmB) override;
    bool validate(float bpmA, float bpmB, int blockNumber) override;
    void teardown() override;
    
    std::string getHint() const override;
    std::string getName() const override;
    std::string getObjective() const override;
    
private:
    float targetBpmA_ = 120.0f;
    float targetBpmB_ = 120.0f;
    int matchStartBlock_ = -1;  // Block where match began
    static constexpr int MATCH_DURATION_BLOCKS = 220;  // ~5 seconds at 44.1kHz
    static constexpr float TOLERANCE_PERCENT = 2.0f;    // ±2%
};

} // namespace dj
