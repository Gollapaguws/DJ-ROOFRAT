#include "multiplayer/Judge.h"

#include <algorithm>

namespace dj {

Judge::Judge()
    : variationCount_(0) {
}

JudgmentScores Judge::evaluatePerformance(const PerformanceMetrics& metrics) {
    JudgmentScores scores;
    scores.beatmatchScore = scoreBeatmatching(metrics.beatmatchDelta);
    scores.transitionScore = scoreTransitions(metrics.transitionSmoothness);
    scores.energyScore = scoreEnergyManagement(metrics.crowdEnergy);
    scores.creativityScore = scoreCreativity(variationCount_);
    scores.crowdScore = scoreCrowdResponse(metrics.crowdEnergy);
    return scores;
}

float Judge::scoreBeatmatching(float bpmDelta) {
    if (bpmDelta < 0.5f) {
        return 100.0f;  // Perfect
    } else if (bpmDelta < 2.0f) {
        return 80.0f;   // Good
    } else if (bpmDelta < 5.0f) {
        return 50.0f;   // Okay
    } else {
        return 0.0f;    // Poor
    }
}

float Judge::scoreTransitions(float smoothness) {
    return smoothness * 100.0f;
}

float Judge::scoreEnergyManagement(float crowdEnergy) {
    return crowdEnergy * 100.0f;
}

float Judge::scoreCreativity(int variationCount) {
    float score = variationCount * 5.0f;
    return std::clamp(score, 0.0f, 100.0f);
}

float Judge::scoreCrowdResponse(float crowdEnergy) {
    return crowdEnergy * 100.0f;
}

} // namespace dj
