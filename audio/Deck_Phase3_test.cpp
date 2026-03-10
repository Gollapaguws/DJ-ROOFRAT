#include "audio/Deck.h"

#include <cassert>
#include <iostream>

namespace dj {

// Test 1: Validate 8/16/32-beat loop sizing calculation
void test_Deck_LoopBeatsConfiguration() {
    Deck deck;
    AudioClip clip = AudioClip::generateTestTone(440.0f, 1.0f, 44100);
    
    bool loaded = deck.loadClip(clip);
    assert(loaded);
    
    // Test default is 16 beats
    assert(deck.loopBeats() == 16);
    
    // Test setting to 8 beats
    deck.setLoopBeats(8);
    assert(deck.loopBeats() == 8);
    
    // Test setting to 32 beats
    deck.setLoopBeats(32);
    assert(deck.loopBeats() == 32);
    
    // Test setting back to 16 beats
    deck.setLoopBeats(16);
    assert(deck.loopBeats() == 16);
    
    // Test invalid values are clamped to valid options
    deck.setLoopBeats(5);    // Should clamp to closest or default
    assert(deck.loopBeats() == 8 || deck.loopBeats() == 16);  // At least a valid value
    
    deck.setLoopBeats(100);  // Should clamp to closest or default
    assert(deck.loopBeats() == 16 || deck.loopBeats() == 32);  // At least a valid value
    
    std::cout << "✓ test_Deck_LoopBeatsConfiguration passed" << std::endl;
}

// Test 2: Confirm both decks apply tempo independently
void test_Deck_TempoScaling() {
    Deck deckA;
    Deck deckB;
    
    AudioClip clipA = AudioClip::generateTestTone(440.0f, 1.0f, 44100);
    AudioClip clipB = AudioClip::generateTestTone(880.0f, 1.0f, 44100);
    
    bool loadedA = deckA.loadClip(clipA);
    bool loadedB = deckB.loadClip(clipB);
    assert(loadedA && loadedB);
    
    // Set different tempo for each deck
    deckA.setTempoPercent(5.0f);
    deckB.setTempoPercent(-5.0f);
    
    // Verify they are independent
    assert(deckA.tempoPercent() == 5.0f);
    assert(deckB.tempoPercent() == -5.0f);
    
    // Verify bounds checking (-50 to +50)
    deckA.setTempoPercent(60.0f);  // Should clamp to 50.0f
    assert(deckA.tempoPercent() == 50.0f);
    
    deckB.setTempoPercent(-60.0f);  // Should clamp to -50.0f
    assert(deckB.tempoPercent() == -50.0f);
    
    // Verify reset works independently
    deckA.setTempoPercent(0.0f);
    deckB.setTempoPercent(10.0f);
    assert(deckA.tempoPercent() == 0.0f);
    assert(deckB.tempoPercent() == 10.0f);
    
    std::cout << "✓ test_Deck_TempoScaling passed" << std::endl;
}

// Test 3: Verify loop beats cycle correctly
void test_Deck_LoopBeatsCycle() {
    Deck deck;
    AudioClip clip = AudioClip::generateTestTone(440.0f, 1.0f, 44100);
    
    bool loaded = deck.loadClip(clip);
    assert(loaded);
    
    // Start at default 16
    assert(deck.loopBeats() == 16);
    
    // Cycle: 16 → 32
    deck.setLoopBeats(32);
    assert(deck.loopBeats() == 32);
    
    // Cycle: 32 → 8
    deck.setLoopBeats(8);
    assert(deck.loopBeats() == 8);
    
    // Cycle: 8 → 16
    deck.setLoopBeats(16);
    assert(deck.loopBeats() == 16);
    
    std::cout << "✓ test_Deck_LoopBeatsCycle passed" << std::endl;
}

void runAllPhase3DeckTests() {
    std::cout << "\n=== Running Deck Phase 3 Tests ===" << std::endl;
    try {
        test_Deck_LoopBeatsConfiguration();
        test_Deck_TempoScaling();
        test_Deck_LoopBeatsCycle();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
    }
}

} // namespace dj

// Entry point for running deck tests
int main() {
    dj::runAllPhase3DeckTests();
    std::cout << "\n=== Deck Phase 3 Tests Complete ===" << std::endl;
    return 0;
}
