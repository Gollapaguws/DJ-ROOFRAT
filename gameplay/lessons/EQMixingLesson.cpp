#include "gameplay/lessons/EQMixingLesson.h"
#include "audio/Deck.h"
#include "audio/Mixer.h"

#include <cmath>
#include <sstream>

namespace dj {

EQMixingLesson::EQMixingLesson()
    : initialCrossfaderPos_(-1.0f)
    , transitionStartBlock_(-1)
{
}

void EQMixingLesson::setup(Deck& deckA, Deck& deckB, Mixer& mixer) {
    // Set initial deck EQs to contrasting presets
    // Deck A: neutral/subtle EQ (low=1.0, mid=1.0, high=0.5)
    deckA.setEQ(1.0f, 1.0f, 0.5f);
    
    // Deck B: different preset (low=1.0, mid=1.0, high=1.0 - boosted)
    deckB.setEQ(1.0f, 1.0f, 1.0f);
    
    // Start crossfader on deckA
    mixer.setCrossfader(-1.0f);
    initialCrossfaderPos_ = -1.0f;
    transitionStartBlock_ = -1;
}

bool EQMixingLesson::validate(Deck& deckA, Deck& deckB, Mixer& mixer, int blockNumber) {
    float currentCrossfader = mixer.crossfader();
    
    // Check if crossfader has moved significantly from start
    float crossfaderDelta = std::abs(currentCrossfader - initialCrossfaderPos_);
    
    if (crossfaderDelta > 0.1f && transitionStartBlock_ < 0) {
        // Transition has begun
        transitionStartBlock_ = blockNumber;
    } else if (currentCrossfader < initialCrossfaderPos_ + 0.1f) {
        // Reset if crossfader moved back toward start (user fidgeting)
        transitionStartBlock_ = -1;
    }
    
    // Transition is complete when:
    // 1. Crossfader is near deckB (> 0.9)
    // 2. We've had time to make the transition
    if (currentCrossfader > TRANSITION_COMPLETE_THRESHOLD && transitionStartBlock_ >= 0) {
        int transitionDuration = blockNumber - transitionStartBlock_;
        // Require at least some sustained time to complete
        if (transitionDuration > 60) {  // ~1.4 seconds
            return true;
        }
    }
    
    return false;
}

void EQMixingLesson::teardown() {
    initialCrossfaderPos_ = -1.0f;
    transitionStartBlock_ = -1;
}

std::string EQMixingLesson::getHint() const {
    std::ostringstream oss;
    oss << "Transition from Deck A to Deck B using the crossfader and EQ\n";
    oss << "Use crossfader: [ left (Deck A) | ] right (Deck B)\n";
    oss << "Adjust EQ during transition for smooth blending.";
    return oss.str();
}

std::string EQMixingLesson::getName() const {
    return "EQ Mixing & Crossfade";
}

std::string EQMixingLesson::getObjective() const {
    return "Transition from Deck A to Deck B using crossfader and EQ";
}

} // namespace dj
