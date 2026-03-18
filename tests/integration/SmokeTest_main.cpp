// Phase 1: Integration Test Suite - Smoke Tests
// Tests that major systems initialize and work without crashes

#include "audio/Deck.h"
#include "audio/Mixer.h"
#include "audio/AudioClip.h"
#include "crowdAI/CrowdStateMachine.h"
#include "input/InputMapper.h"

#include <cassert>
#include <iostream>
#include <array>

namespace dj {

// Smoke Test 1: Initialize all major systems without crashes
void test_SmokeTest_AppStartup() {
    std::cout << "[TEST 1] SmokeTest_AppStartup...\n";
    
    try {
        // Initialize Mixer
        Mixer mixer;
        mixer.setCrossfader(0.0f);
        assert(mixer.crossfader() == 0.0f && "Mixer initialization failed");
        
        // Initialize Decks
        Deck deckA;
        Deck deckB;
        deckA.setOutputSampleRate(44100);
        deckB.setOutputSampleRate(44100);
        
        // Initialize CrowdStateMachine
        CrowdStateMachine crowd(CrowdPersonality::Default);
        auto crowdOutput = crowd.update(120.0f, 0.5f, 0.5f);
        assert(crowdOutput.energyMeter >= 0.0f && crowdOutput.energyMeter <= 1.0f &&
               "Crowd initialization failed");
        
        // Initialize InputMapper
        InputMapper mapper;
        auto cmd = mapper.parseKey('q');
        // Should parse to some command (possibly None, that's ok for this test)
        
        std::cout << "  ✓ All major systems initialized successfully\n";
        std::cout << "PASS\n";
    } catch (const std::exception& e) {
        std::cout << "  ✗ Exception during initialization: " << e.what() << "\n";
        assert(false && "Startup initialization failed");
    }
}

// Smoke Test 2: Generate test tone, play through Deck, verify audio generation
void test_SmokeTest_TestTonePlayback() {
    std::cout << "[TEST 2] SmokeTest_TestTonePlayback...\n";
    
    try {
        const int sampleRate = 44100;
        
        // Generate test tone (440 Hz, 1 second)
        auto clip = AudioClip::generateTestTone(440.0f, 1.0f, sampleRate);
        assert(!clip.empty() && "Test tone generation failed");
        
        // Load into Deck
        Deck deck;
        deck.setOutputSampleRate(sampleRate);
        bool loaded = deck.loadClip(clip);
        assert(loaded && "Failed to load clip into deck");
        
        // Start playback
        deck.play();
        assert(deck.isPlaying() && "Deck failed to start playing");
        
        // Generate several frames and verify non-zero audio output
        bool hasAudio = false;
        for (int i = 0; i < 1000; ++i) {
            auto frame = deck.nextFrame();
            // Check if either channel has non-negligible signal
            if (std::abs(frame[0]) > 0.001f || std::abs(frame[1]) > 0.001f) {
                hasAudio = true;
                break;
            }
        }
        assert(hasAudio && "Deck generated no audio output");
        
        // Stop playback
        deck.stop();
        assert(!deck.isPlaying() && "Deck failed to stop");
        
        std::cout << "  ✓ Test tone generated and played successfully\n";
        std::cout << "PASS\n";
    } catch (const std::exception& e) {
        std::cout << "  ✗ Exception during playback test: " << e.what() << "\n";
        assert(false && "Playback test failed");
    }
}

// Smoke Test 3: Parse keyboard commands and verify InputMapper returns correct commands
void test_SmokeTest_KeyboardInput() {
    std::cout << "[TEST 3] SmokeTest_KeyboardInput...\n";
    
    try {
        InputMapper mapper;
        
        // Test parsing specific keys (parseKey takes char, not string)
        auto cmd1 = mapper.parseKey(' ');   // Space key - Should be PlayPauseA or PlayPauseB
        auto cmd2 = mapper.parseKey('w');   // 'w' - Should be NudgeTempoAUp
        auto cmd3 = mapper.parseKey('s');   // 's' - Should be NudgeTempoADown
        auto cmd4 = mapper.parseKey('q');   // 'q' - Should be Quit
        
        // Verify we got valid commands (not all should be None)
        int validCommands = 0;
        if (cmd1 != InputCommand::None) validCommands++;
        if (cmd2 != InputCommand::None) validCommands++;
        if (cmd3 != InputCommand::None) validCommands++;
        if (cmd4 != InputCommand::None) validCommands++;
        
        assert(validCommands >= 2 && "InputMapper failed to parse expected commands");
        
        std::cout << "  ✓ Keyboard input parsing successful\n";
        std::cout << "PASS\n";
    } catch (const std::exception& e) {
        std::cout << "  ✗ Exception during input parsing: " << e.what() << "\n";
        assert(false && "Keyboard input test failed");
    }
}

void runAllSmokeTests() {
    std::cout << "\n=== Running Integration Smoke Tests ===\n";
    try {
        test_SmokeTest_AppStartup();
        test_SmokeTest_TestTonePlayback();
        test_SmokeTest_KeyboardInput();
    } catch (const std::exception& e) {
        std::cerr << "Smoke test failed with exception: " << e.what() << "\n";
    }
}

} // namespace dj

// Entry point
int main() {
    dj::runAllSmokeTests();
    std::cout << "\n=== Integration Smoke Tests Complete ===\n";
    return 0;
}
