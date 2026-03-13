#pragma once

#include "gameplay/Lesson.h"

namespace dj {

// Forward declarations
class Deck;
class Mixer;

class EQMixingLesson : public Lesson {
public:
    EQMixingLesson();
    ~EQMixingLesson() = default;
    
    void setup(Deck& deckA, Deck& deckB, Mixer& mixer) override;
    bool validate(Deck& deckA, Deck& deckB, Mixer& mixer, int blockNumber) override;
    void teardown() override;
    
    std::string getHint() const override;
    std::string getName() const override;
    std::string getObjective() const override;
    
private:
    // Track transition progress
    float initialCrossfaderPos_ = -1.0f;  // Start on deckA
    int transitionStartBlock_ = -1;
    
    static constexpr float TRANSITION_COMPLETE_THRESHOLD = 0.9f;  // How close to B we need to be
    static constexpr float EQ_MATCH_THRESHOLD = 0.1f;  // Tolerance for EQ similarity
};

} // namespace dj
