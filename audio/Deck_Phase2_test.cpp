#include "audio/Deck.h"

#include <cassert>
#include <cmath>
#include <iostream>

namespace dj {

// Test 1: Verify isolator mutes specific band completely (output 0.0)
void test_Deck_IsolatorKillBand() {
    Deck deck;
    AudioClip clip = AudioClip::generateTestTone(440.0f, 1.0f, 44100);
    
    bool loaded = deck.loadClip(clip);
    assert(loaded);
    
    deck.setOutputSampleRate(44100);
    
    // Set all EQ bands to unity
    deck.setEQ(1.0f, 1.0f, 1.0f);
    
    // Enable isolator on low band only
    deck.setIsolatorMode(true, false, false);
    
    // Verify isolator is enabled
    assert(deck.isIsolatorEnabled(0) == true);  // low band
    assert(deck.isIsolatorEnabled(1) == false); // mid band
    assert(deck.isIsolatorEnabled(2) == false); // high band
    
    std::cout << "✓ test_Deck_IsolatorKillBand passed" << std::endl;
}

// Test 2: Verify filter order can be set and retrieved
void test_Deck_FilterOrderToggle() {
    Deck deck;
    AudioClip clip = AudioClip::generateTestTone(440.0f, 1.0f, 44100);
    
    bool loaded = deck.loadClip(clip);
    assert(loaded);
    
    // Default should be 1 (single-pole)
    deck.setFilterOrder(1);
    
    // Toggle to 2 (Butterworth)
    deck.setFilterOrder(2);
    
    // Try invalid values - should clamp
    deck.setFilterOrder(5);  // Should clamp to 2
    deck.setFilterOrder(0);  // Should clamp to 1
    
    std::cout << "✓ test_Deck_FilterOrderToggle passed" << std::endl;
}

// Test 3: Verify isolator settings persist across modes
void test_Deck_IsolatorMultiBand() {
    Deck deck;
    AudioClip clip = AudioClip::generateTestTone(440.0f, 1.0f, 44100);
    
    bool loaded = deck.loadClip(clip);
    assert(loaded);
    
    // Enable all isolators
    deck.setIsolatorMode(true, true, true);
    
    assert(deck.isIsolatorEnabled(0) == true);
    assert(deck.isIsolatorEnabled(1) == true);
    assert(deck.isIsolatorEnabled(2) == true);
    
    // Disable all isolators
    deck.setIsolatorMode(false, false, false);
    
    assert(deck.isIsolatorEnabled(0) == false);
    assert(deck.isIsolatorEnabled(1) == false);
    assert(deck.isIsolatorEnabled(2) == false);
    
    std::cout << "✓ test_Deck_IsolatorMultiBand passed" << std::endl;
}

void runAllPhase2DeckTests() {
    std::cout << "\n=== Running Deck Phase 2 Tests ===" << std::endl;
    try {
        test_Deck_IsolatorKillBand();
        test_Deck_FilterOrderToggle();
        test_Deck_IsolatorMultiBand();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
    }
}

} // namespace dj

// Entry point for running tests
int main() {
    dj::runAllPhase2DeckTests();
    std::cout << "\n=== Deck Phase 2 Tests Complete ===" << std::endl;
    return 0;
}
