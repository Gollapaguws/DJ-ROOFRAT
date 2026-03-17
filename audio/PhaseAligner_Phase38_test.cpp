#include "audio/PhaseAligner.h"
#include "audio/Deck.h"
#include "audio/AudioClip.h"
#include "audio/BeatGrid.h"

#include <cassert>
#include <cmath>
#include <iostream>

namespace dj {

void test_PhaseAligner_ZeroOffset() {
    std::cout << "[TEST 1] PhaseAligner_ZeroOffset...\n";
    
    // Create two decks with identical beat grids
    auto clipA = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    auto clipB = AudioClip::generateTestTone(880.0f, 10.0f, 44100);
    
    Deck deckA, deckB;
    deckA.loadClip(clipA);
    deckB.loadClip(clipB);
    
    // Both decks at same BPM and position
    deckA.play();
    deckB.play();
    
    // Advance both by same amount
    for (int i = 0; i < 1000; ++i) {
        deckA.nextFrame();
        deckB.nextFrame();
    }
    
    PhaseAligner aligner;
    double phaseOffsetSamples = aligner.calculatePhaseOffset(deckA, deckB, 128.0, 128.0);
    
    // Offset should be near zero (within 10 samples)
    assert(std::abs(phaseOffsetSamples) < 10.0);
    
    std::cout << "  Phase offset: " << phaseOffsetSamples << " samples\n";
    std::cout << "PASS\n";
}

void test_PhaseAligner_PositiveOffset() {
    std::cout << "[TEST 2] PhaseAligner_PositiveOffset...\n";
    
    auto clipA = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    auto clipB = AudioClip::generateTestTone(880.0f, 10.0f, 44100);
    
    Deck deckA, deckB;
    deckA.loadClip(clipA);
    deckB.loadClip(clipB);
    
    deckA.play();
    deckB.play();
    
    // Advance deck A
    for (int i = 0; i < 1000; ++i) {
        deckA.nextFrame();
    }
    
    // Advance deck B further (B ahead by 0.25 beats = ~5512 samples @ 128 BPM, 44.1kHz)
    for (int i = 0; i < 6500; ++i) {
        deckB.nextFrame();
    }
    
    PhaseAligner aligner;
    double phaseOffsetSamples = aligner.calculatePhaseOffset(deckA, deckB, 128.0, 128.0);
    
    // Offset should be positive (B ahead)
    assert(phaseOffsetSamples > 1000.0);
    assert(phaseOffsetSamples < 10000.0);
    
    std::cout << "  Phase offset: " << phaseOffsetSamples << " samples (B ahead)\n";
    std::cout << "PASS\n";
}

void test_PhaseAligner_NegativeOffset() {
    std::cout << "[TEST 3] PhaseAligner_NegativeOffset...\n";
    
    auto clipA = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    auto clipB = AudioClip::generateTestTone(880.0f, 10.0f, 44100);
    
    Deck deckA, deckB;
    deckA.loadClip(clipA);
    deckB.loadClip(clipB);
    
    deckA.play();
    deckB.play();
    
    // Advance deck A further (A ahead, B behind)
    for (int i = 0; i < 6500; ++i) {
        deckA.nextFrame();
    }
    
    // Advance deck B less
    for (int i = 0; i < 1000; ++i) {
        deckB.nextFrame();
    }
    
    PhaseAligner aligner;
    double phaseOffsetSamples = aligner.calculatePhaseOffset(deckA, deckB, 128.0, 128.0);
    
    // Offset should be negative (B behind)
    assert(phaseOffsetSamples < -1000.0);
    assert(phaseOffsetSamples > -10000.0);
    
    std::cout << "  Phase offset: " << phaseOffsetSamples << " samples (B behind)\n";
    std::cout << "PASS\n";
}

void test_PhaseAligner_DifferentBPM() {
    std::cout << "[TEST 4] PhaseAligner_DifferentBPM...\n";
    
    auto clipA = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    auto clipB = AudioClip::generateTestTone(880.0f, 10.0f, 44100);
    
    Deck deckA, deckB;
    deckA.loadClip(clipA);
    deckB.loadClip(clipB);
    
    deckA.play();
    deckB.play();
    
    // Advance both by same frame count
    for (int i = 0; i < 1000; ++i) {
        deckA.nextFrame();
        deckB.nextFrame();
    }
    
    // Different BPMs: A at 120, B at 128
    PhaseAligner aligner;
    double phaseOffsetBeats = aligner.calculatePhaseOffsetBeats(deckA, deckB, 120.0, 128.0);
    
    // With different BPMs, phase offset in beats should still be calculated
    // (even if beat grids differ in tempo)
    assert(std::abs(phaseOffsetBeats) < 5.0);  // Within 5 beats reasonable for this test
    
    std::cout << "  Phase offset: " << phaseOffsetBeats << " beats\n";
    std::cout << "PASS\n";
}

void test_PhaseAligner_NoBeatGrid() {
    std::cout << "[TEST 5] PhaseAligner_NoBeatGrid...\n";
    
    auto clipA = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    auto clipB = AudioClip::generateTestTone(880.0f, 10.0f, 44100);
    
    Deck deckA, deckB;
    deckA.loadClip(clipA);
    deckB.loadClip(clipB);
    
    deckA.play();
    deckB.play();
    
    PhaseAligner aligner;
    
    // When no beat grid exists, should return 0.0 (graceful fallback)
    // In real implementation, we'll check if beat grid is valid
    double phaseOffsetSamples = aligner.calculatePhaseOffset(deckA, deckB, 128.0, 128.0);
    
    // Should not crash, offset may be 0 or position-based
    std::cout << "  Phase offset (no beat grid): " << phaseOffsetSamples << " samples\n";
    std::cout << "PASS\n";
}

void test_Deck_AlignPhase() {
    std::cout << "[TEST 6] Deck_AlignPhase...\n";
    
    auto clipA = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    auto clipB = AudioClip::generateTestTone(880.0f, 10.0f, 44100);
    
    Deck deckA, deckB;
    deckA.loadClip(clipA);
    deckB.loadClip(clipB);
    
    deckA.play();
    deckB.play();
    
    // Create phase offset (B ahead)
    for (int i = 0; i < 1000; ++i) {
        deckA.nextFrame();
    }
    for (int i = 0; i < 6000; ++i) {
        deckB.nextFrame();
    }
    
    std::size_t posBefore = deckB.currentFrame();
    
    // Align deck B to deck A
    deckB.alignPhaseWithDeck(deckA, 128.0, 128.0);
    
    std::size_t posAfter = deckB.currentFrame();
    
    // Position should have changed
    assert(posBefore != posAfter);
    
    // After alignment, phase offset should be near zero
    PhaseAligner aligner;
    double offsetAfter = aligner.calculatePhaseOffset(deckA, deckB, 128.0, 128.0);
    assert(std::abs(offsetAfter) < 100.0);
    
    std::cout << "  Position before: " << posBefore << ", after: " << posAfter << "\n";
    std::cout << "  Phase offset after alignment: " << offsetAfter << " samples\n";
    std::cout << "PASS\n";
}

void test_PhaseAligner_LiveTracking() {
    std::cout << "[TEST 7] PhaseAligner_LiveTracking...\n";
    
    auto clipA = AudioClip::generateTestTone(440.0f, 60.0f, 44100);  // Longer clip
    auto clipB = AudioClip::generateTestTone(880.0f, 60.0f, 44100);
    
    Deck deckA, deckB;
    deckA.loadClip(clipA);
    deckB.loadClip(clipB);
    
    deckA.play();
    deckB.play();
    
    // Align initially
    deckB.alignPhaseWithDeck(deckA, 128.0, 128.0);
    
    PhaseAligner aligner;
    
    // Simulate 5 seconds of playback (220,500 samples @ 44.1kHz)
    // Check phase drift every second
    for (int sec = 0; sec < 5; ++sec) {
        for (int i = 0; i < 44100; ++i) {
            deckA.nextFrame();
            deckB.nextFrame();
        }
        
        double offset = aligner.calculatePhaseOffset(deckA, deckB, 128.0, 128.0);
        std::cout << "  Second " << (sec + 1) << " offset: " << offset << " samples\n";
        
        // Phase should remain stable (< 50 sample drift)
        assert(std::abs(offset) < 50.0);
    }
    
    std::cout << "PASS (phase stable over 5 seconds)\n";
}

void test_PhaseAligner_EdgeCases() {
    std::cout << "[TEST 8] PhaseAligner_EdgeCases...\n";
    
    auto clipA = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    auto clipB = AudioClip::generateTestTone(880.0f, 10.0f, 44100);
    
    Deck deckA, deckB;
    deckA.loadClip(clipA);
    deckB.loadClip(clipB);
    
    PhaseAligner aligner;
    
    // Test 1: Both decks paused
    double offsetPaused = aligner.calculatePhaseOffset(deckA, deckB, 128.0, 128.0);
    std::cout << "  Offset (both paused): " << offsetPaused << " samples\n";
    
    // Test 2: One deck at track end
    deckA.play();
    for (int i = 0; i < 440000; ++i) {  // Near end of 10s clip
        deckA.nextFrame();
    }
    double offsetNearEnd = aligner.calculatePhaseOffset(deckA, deckB, 128.0, 128.0);
    std::cout << "  Offset (A near end): " << offsetNearEnd << " samples\n";
    
    // Test 3: Extreme BPM difference
    double offsetExtremeBPM = aligner.calculatePhaseOffsetBeats(deckA, deckB, 70.0, 180.0);
    std::cout << "  Offset (70 vs 180 BPM): " << offsetExtremeBPM << " beats\n";
    
    // All should complete without crash
    std::cout << "PASS (edge cases handled)\n";
}

} // namespace dj

int main() {
    std::cout << "=== Phase 38: Phase Alignment Engine Tests ===\n\n";
    
    dj::test_PhaseAligner_ZeroOffset();
    dj::test_PhaseAligner_PositiveOffset();
    dj::test_PhaseAligner_NegativeOffset();
    dj::test_PhaseAligner_DifferentBPM();
    dj::test_PhaseAligner_NoBeatGrid();
    dj::test_Deck_AlignPhase();
    dj::test_PhaseAligner_LiveTracking();
    dj::test_PhaseAligner_EdgeCases();
    
    std::cout << "\n=== All Phase 38 tests passed! ===\n";
    return 0;
}
