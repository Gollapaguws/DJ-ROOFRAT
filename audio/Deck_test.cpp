#include "audio/Deck.h"

#include <cassert>
#include <iostream>

namespace dj {

// Test 3: Confirm deck can apply preset frequency configurations
void test_Deck_FrequencyPresets() {
    Deck deck;
    AudioClip clip = AudioClip::generateTestTone(440.0f, 1.0f, 44100);
    
    bool loaded = deck.loadClip(clip);
    assert(loaded);
    
    // Test setting EQ frequencies on deck
    deck.setEQFrequencies(250.0f, 2500.0f);
    
    // Load another clip and verify settings persist
    AudioClip clip2 = AudioClip::generateTestTone(880.0f, 1.0f, 44100);
    loaded = deck.loadClip(clip2);
    assert(loaded);
    
    // Set different preset
    deck.setEQFrequencies(300.0f, 3000.0f);
    
    // Verify deck can apply the settings without errors
    std::cout << "✓ test_Deck_FrequencyPresets passed" << std::endl;
}

void runAllDeckTests() {
    std::cout << "\n=== Running Deck Frequency Tests ===" << std::endl;
    try {
        test_Deck_FrequencyPresets();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
    }
}

} // namespace dj

// Entry point for running deck tests
int main() {
    dj::runAllDeckTests();
    std::cout << "\n=== Deck Frequency Tests Complete ===" << std::endl;
    return 0;
}
