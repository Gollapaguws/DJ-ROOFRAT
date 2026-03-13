#include "gameplay/lessons/BeatmatchingLesson.h"

#include <cmath>
#include <sstream>

namespace dj {

BeatmatchingLesson::BeatmatchingLesson()
    : targetBpmA_(120.0f)
    , targetBpmB_(120.0f)
    , matchStartBlock_(-1)
{
}

void BeatmatchingLesson::setup(float baselineBpmA, float baselineBpmB) {
    targetBpmA_ = baselineBpmA;
    targetBpmB_ = baselineBpmB;
    matchStartBlock_ = -1;
}

bool BeatmatchingLesson::validate(float bpmA, float bpmB, int blockNumber) {
    // Guard against division by zero (undetected BPM)
    if (bpmA <= 0.0f || bpmB <= 0.0f) {
        matchStartBlock_ = -1;
        return false;
    }
    
    // Calculate BPM delta as percentage of deckA
    float bpmDelta = std::abs(bpmB - bpmA);
    float deltaPercent = (bpmDelta / bpmA) * 100.0f;
    
    // Check if within tolerance
    if (deltaPercent <= TOLERANCE_PERCENT) {
        // Start or continue tracking match duration
        if (matchStartBlock_ < 0) {
            matchStartBlock_ = blockNumber;
        }
        
        // Check if we've maintained the match for required duration
        int matchDuration = blockNumber - matchStartBlock_;
        if (matchDuration >= MATCH_DURATION_BLOCKS) {
            return true;  // Lesson complete
        }
    } else {
        // Out of tolerance, reset match timer
        matchStartBlock_ = -1;
    }
    
    return false;
}

void BeatmatchingLesson::teardown() {
    matchStartBlock_ = -1;
}

std::string BeatmatchingLesson::getHint() const {
    std::ostringstream oss;
    oss << "Adjust Deck B tempo to match Deck A (" << targetBpmA_ << " BPM)\n";
    oss << "Use '[' and ']' keys to adjust. Get within ±2% and hold for ~5 seconds.";
    return oss.str();
}

std::string BeatmatchingLesson::getName() const {
    return "Beatmatching Fundamentals";
}

std::string BeatmatchingLesson::getObjective() const {
    return "Match Deck B tempo to Deck A";
}

} // namespace dj
