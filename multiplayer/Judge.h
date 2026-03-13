#pragma once

namespace dj {

struct PerformanceMetrics {
    float beatmatchDelta;       // BPM delta between decks
    float transitionSmoothness; // 0.0-1.0
    float crowdEnergy;          // 0.0-1.0
    int blockNumber;            // Time index for creativity scoring
};

struct JudgmentScores {
    float beatmatchScore = 0.0f;
    float transitionScore = 0.0f;
    float energyScore = 0.0f;
    float creativityScore = 0.0f;
    float crowdScore = 0.0f;
};

class Judge {
public:
    Judge();
    
    // Score individual performance
    JudgmentScores evaluatePerformance(const PerformanceMetrics& metrics);
    
    // Score components (0.0-100.0 each)
    float scoreBeatmatching(float bpmDelta);
    float scoreTransitions(float smoothness);
    float scoreEnergyManagement(float crowdEnergy);
    float scoreCreativity(int variationCount);  // Simplified: track EQ/filter changes
    float scoreCrowdResponse(float crowdEnergy);
    
private:
    int variationCount_;  // Track creativity over session
};

} // namespace dj
