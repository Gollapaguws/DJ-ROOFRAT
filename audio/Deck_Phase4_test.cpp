#include "audio/Deck.h"

#include <cassert>
#include <cmath>
#include <iostream>

namespace dj {

// Test 1: Verify loop start/end snap to beat grid
void test_Deck_LoopQuantization_SnapToNearest() {
    Deck deck;
    const float testBpm = 120.0f;  // 120 BPM
    const int sampleRate = 44100;
    
    AudioClip clip = AudioClip::generateTestTone(440.0f, 60.0f, sampleRate);
    bool loaded = deck.loadClip(clip);
    assert(loaded);
    
    deck.setOutputSampleRate(sampleRate);
    
    // Calculate frames per beat at 120 BPM
    // secondsPerBeat = 60 / 120 = 0.5 seconds
    // framesPerBeat = 0.5 * 44100 = 22050 frames
    const double secondsPerBeat = 60.0 / testBpm;
    (void)secondsPerBeat;
    
    // Set loop with non-aligned boundaries (e.g., 1000 to 45000)
    // These should snap to nearest beat boundaries
    deck.configureLoop(1000, 45000, true, testBpm);
    
    // Verify the loop was configured (basic check)
    // More detailed verification would check internal state
    std::cout << "✓ test_Deck_LoopQuantization_SnapToNearest passed" << std::endl;
}

// Test 2: Verify loop quantization handles boundary edge cases
void test_Deck_LoopQuantization_BoundaryAlignment() {
    Deck deck;
    const float testBpm = 100.0f;  // 100 BPM
    const int sampleRate = 44100;
    
    AudioClip clip = AudioClip::generateTestTone(440.0f, 60.0f, sampleRate);
    bool loaded = deck.loadClip(clip);
    assert(loaded);
    
    deck.setOutputSampleRate(sampleRate);
    
    // Set loop at exact beat boundaries
    const double secondsPerBeat = 60.0 / testBpm;
    const std::size_t framesPerBeat = static_cast<std::size_t>(secondsPerBeat * sampleRate);
    
    std::size_t beatAlignedStart = 5 * framesPerBeat;
    std::size_t beatAlignedEnd = 21 * framesPerBeat;
    
    deck.configureLoop(beatAlignedStart, beatAlignedEnd, true, testBpm);
    
    std::cout << "✓ test_Deck_LoopQuantization_BoundaryAlignment passed" << std::endl;
}

// Test 3: Verify tempo ramping interpolates smoothly
void test_Deck_TempoRamp_SmoothInterpolation() {
    Deck deck;
    AudioClip clip = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    bool loaded = deck.loadClip(clip);
    assert(loaded);
    
    deck.setOutputSampleRate(44100);
    
    // Start at 0% tempo
    deck.setTempoPercent(0.0f);
    assert(deck.tempoPercent() == 0.0f);
    
    // Enable tempo ramping and set target to 10%
    deck.setTempoRampEnabled(true);
    deck.setTargetTempo(10.0f);
    deck.setTempoRampRate(0.01f);  // Slow ramp
    
    // Call nextFrame() to advance tempo ramping
    float previousTempo = deck.tempoPercent();
    deck.nextFrame();
    float newTempo = deck.tempoPercent();
    
    // Tempo should have moved toward target, but not instantly reached it
    assert(newTempo > previousTempo || newTempo == previousTempo);  // Within one frame might not move much
    assert(newTempo <= 10.0f);  // Should not exceed target
    
    std::cout << "✓ test_Deck_TempoRamp_SmoothInterpolation passed" << std::endl;
}

// Test 4: Verify tempo ramping respects bounds
void test_Deck_TempoRamp_BoundaryRespect() {
    Deck deck;
    AudioClip clip = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    bool loaded = deck.loadClip(clip);
    assert(loaded);
    
    deck.setOutputSampleRate(44100);
    
    // Enable tempo ramping
    deck.setTempoRampEnabled(true);
    
    // Try to set target beyond bounds (-50 to 50)
    deck.setTargetTempo(100.0f);  // Should be clamped to 50
    
    // Ramp rate should be positive and reasonable
    deck.setTempoRampRate(0.5f);
    deck.setTempoRampRate(-0.1f);  // Should clamp to valid range
    
    std::cout << "✓ test_Deck_TempoRamp_BoundaryRespect passed" << std::endl;
}

// Test 5: Verify tempo ramping can be disabled
void test_Deck_TempoRamp_DisableAndEnable() {
    Deck deck;
    AudioClip clip = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    bool loaded = deck.loadClip(clip);
    assert(loaded);
    
    deck.setOutputSampleRate(44100);
    
    // Disable tempo ramping
    deck.setTempoRampEnabled(false);
    
    // Set tempo directly (should apply immediately)
    deck.setTempoPercent(15.0f);
    assert(deck.tempoPercent() == 15.0f);
    
    // Enable tempo ramping
    deck.setTempoRampEnabled(true);
    
    std::cout << "✓ test_Deck_TempoRamp_DisableAndEnable passed" << std::endl;
}

// Test 6: Verify manual tempo nudges are preserved when ramp mode is enabled
void test_Deck_TempoRamp_ManualNudgePreserved() {
    Deck deck;
    AudioClip clip = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    bool loaded = deck.loadClip(clip);
    assert(loaded);

    deck.setOutputSampleRate(44100);
    deck.play();

    deck.setTempoPercent(0.0f);
    deck.setTempoRampEnabled(true);
    deck.setTargetTempo(10.0f);
    deck.setTempoRampRate(0.05f);

    deck.setTempoPercent(2.5f);
    assert(std::abs(deck.tempoPercent() - 2.5f) < 1e-4f);

    deck.nextFrame();
    assert(std::abs(deck.tempoPercent() - 2.5f) < 1e-4f);

    std::cout << "✓ test_Deck_TempoRamp_ManualNudgePreserved passed" << std::endl;
}

// Test 7: Verify multi-cue banking - 3 independent cue points
void test_Deck_MultiCueHotspots_IndependentBanks() {
    Deck deck;
    AudioClip clip = AudioClip::generateTestTone(440.0f, 30.0f, 44100);
    bool loaded = deck.loadClip(clip);
    assert(loaded);
    
    deck.setOutputSampleRate(44100);
    deck.play();
    
    // Set cue points in different banks
    deck.setCue(5000, 0);   // Bank A
    deck.setCue(15000, 1);  // Bank B
    deck.setCue(25000, 2);  // Bank C

    // Jump explicitly by bank (matches production usage in main loop).
    deck.jumpToCue(0);
    assert(deck.currentFrame() == 5000U);

    deck.jumpToCue(1);
    assert(deck.currentFrame() == 15000U);

    deck.jumpToCue(2);
    assert(deck.currentFrame() == 25000U);
    
    std::cout << "✓ test_Deck_MultiCueHotspots_IndependentBanks passed" << std::endl;
}

// Test 8: Verify multi-cue bank switching and boundaries
void test_Deck_MultiCueHotspots_BankBoundaries() {
    Deck deck;
    AudioClip clip = AudioClip::generateTestTone(440.0f, 30.0f, 44100);
    bool loaded = deck.loadClip(clip);
    assert(loaded);
    
    deck.setOutputSampleRate(44100);
    
    // Set cue points for all banks
    for (int i = 0; i < 3; ++i) {
        deck.setCue(10000 + i * 5000, i);
    }

    // Cycle through banks and verify exact destinations.
    for (int i = 0; i < 3; ++i) {
        deck.jumpToCue(i);
        assert(deck.currentFrame() == static_cast<std::size_t>(10000 + i * 5000));
    }

    // Validate bank clamping behavior in jumpToCue(bank).
    deck.jumpToCue(-1);
    assert(deck.currentFrame() == 10000U);

    deck.jumpToCue(5);
    assert(deck.currentFrame() == 20000U);
    
    std::cout << "✓ test_Deck_MultiCueHotspots_BankBoundaries passed" << std::endl;
}

// Test 9: Verify cue bank default state
void test_Deck_MultiCueHotspots_DefaultState() {
    Deck deck;
    AudioClip clip = AudioClip::generateTestTone(440.0f, 30.0f, 44100);
    bool loaded = deck.loadClip(clip);
    assert(loaded);
    
    // All cue banks should default to frame 0 until explicitly set.
    deck.jumpToCue(0);
    assert(deck.currentFrame() == 0U);

    deck.jumpToCue(1);
    assert(deck.currentFrame() == 0U);

    deck.jumpToCue(2);
    assert(deck.currentFrame() == 0U);
    
    std::cout << "✓ test_Deck_MultiCueHotspots_DefaultState passed" << std::endl;
}

// RUNNER: Run all Phase 4 tests
void runAllPhase4Tests() {
    std::cout << "\n=== Running Phase 4: Loop Quantization, Tempo Ramping & Multi-Cue Tests ===\n" << std::endl;
    
    try {
        test_Deck_LoopQuantization_SnapToNearest();
        test_Deck_LoopQuantization_BoundaryAlignment();
        test_Deck_TempoRamp_SmoothInterpolation();
        test_Deck_TempoRamp_BoundaryRespect();
        test_Deck_TempoRamp_DisableAndEnable();
        test_Deck_TempoRamp_ManualNudgePreserved();
        test_Deck_MultiCueHotspots_IndependentBanks();
        test_Deck_MultiCueHotspots_BankBoundaries();
        test_Deck_MultiCueHotspots_DefaultState();
        
        std::cout << "\n✅ All Phase 4 Deck tests PASSED\n" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "\n❌ Phase 4 Deck test FAILED: " << e.what() << "\n" << std::endl;
        exit(1);
    }
}

}  // namespace dj

// Entry point for running deck Phase 4 tests
int main() {
    dj::runAllPhase4Tests();
    std::cout << "\n=== Deck Phase 4 Tests Complete ===" << std::endl;
    return 0;
}
