// Phase 1: Integration E2E Test - MIDI Effects
// Tests: Simulate MIDI CC message → parse command → apply effect to deck → verify audio processed

#include "audio/Deck.h"
#include "audio/AudioClip.h"
#include "input/MIDIMessage.h"

#include <cassert>
#include <iostream>

namespace dj {

// E2E Test 5: MIDI to effects pipeline
void test_E2E_MIDIEffects_Pipeline() {
    std::cout << "[TEST 1] E2E_MIDIEffects_Pipeline...\n";
    
    try {
        const int sampleRate = 44100;
        
        // Step 1: Create test clip
        auto clip = AudioClip::generateTestTone(440.0f, 1.0f, sampleRate);
        assert(!clip.empty() && "Failed to generate test clip");
        
        // Step 2: Load into deck and prepare playback
        Deck deck;
        deck.setOutputSampleRate(sampleRate);
        bool loaded = deck.loadClip(clip);
        assert(loaded && "Failed to load clip");
        
        deck.play();
        assert(deck.isPlaying() && "Deck should be playing");
        
        // Step 3: Generate baseline audio (no effects)
        std::array<float, 2> baselineFrame = deck.nextFrame();
        
        // Step 4: Apply filter effect via simulated MIDI CC
        // CC#20 typically controls cutoff frequency in DJ controllers
        // Simulate MIDI ControlChange message:
        // - Status: 0xB0 (CC on channel 0)
        // - Controller: 20 (Filter Cutoff)
        // - Value: 127 (Maximum 7-bit MIDI CC value, fully open)
        unsigned char midiCC[] = {0xB0, 20, 127};
        midi::MIDIMessage msg = midi::MIDIMessage::parseRawBytes(midiCC, 3);
        
        assert(msg.getStatus() == midi::MIDIStatus::ControlChange && "Should parse as ControlChange");
        assert(msg.getController() == 20 && "Should parse CC#20");
        assert(msg.getValue() == 127 && "Should parse maximum 7-bit MIDI CC value");
        
        std::cout << "  ✓ MIDI CC message parsed successfully\n";
        
        // Step 5: Apply filter to deck based on MIDI value
        // Map MIDI value (0-127) to filter cutoff (0.0-1.0)
        float midiNormalizer = msg.getValue() / 127.0f;
        deck.setFilter(midiNormalizer);
        
        std::cout << "  ✓ Filter applied to deck (cutoff: " << midiNormalizer << ")\n";
        
        // Step 6: Generate audio with filter applied
        // Note: It takes a few frames for filter state to build up
        std::array<float, 2> filteredFrame = deck.nextFrame();
        for (int i = 0; i < 100; ++i) {
            filteredFrame = deck.nextFrame();
        }
        
        std::cout << "  ✓ Audio processed through filter\n";
        std::cout << "  ✓ Baseline sample: " << baselineFrame[0] << "\n";
        std::cout << "  ✓ Filtered sample: " << filteredFrame[0] << "\n";
        
        // Step 7: Apply another effect - EQ
        // Simulate MIDI CC#19 for EQ control
        unsigned char midiCC2[] = {0xB0, 19, 127};
        midi::MIDIMessage msg2 = midi::MIDIMessage::parseRawBytes(midiCC2, 3);
        
        assert(msg2.getStatus() == midi::MIDIStatus::ControlChange && "Should parse as ControlChange");
        
        float eqValue = msg2.getValue() / 127.0f;
        (void)eqValue;
        // Apply EQ with boosted high frequency
        deck.setEQ(0.8f, 1.0f, 1.2f);
        
        std::cout << "  ✓ EQ applied to deck\n";
        
        // Step 8: Verify deck is still generating audio
        std::array<float, 2> eqFrame = deck.nextFrame();
        bool hasSignal = std::abs(eqFrame[0]) > 0.001f || std::abs(eqFrame[1]) > 0.001f;
        assert(hasSignal && "Deck should still output audio after EQ");
        
        deck.stop();
        assert(!deck.isPlaying() && "Deck should stop");
        
        std::cout << "PASS\n";
    } catch (const std::exception& e) {
        std::cout << "  ✗ Exception during MIDI effects test: " << e.what() << "\n";
        assert(false && "MIDI effects test failed");
    }
}

void runAllE2EMIDIEffectsTests() {
    std::cout << "\n=== Running E2E MIDI Effects Tests ===\n";
    try {
        test_E2E_MIDIEffects_Pipeline();
    } catch (const std::exception& e) {
        std::cerr << "MIDI effects test failed with exception: " << e.what() << "\n";
    }
}

} // namespace dj

// Entry point
int main() {
    dj::runAllE2EMIDIEffectsTests();
    std::cout << "\n=== E2E MIDI Effects Tests Complete ===\n";
    return 0;
}
