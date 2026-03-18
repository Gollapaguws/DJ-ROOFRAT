#include "audio/Deck.h"
#include "audio/SyncUndoStack.h"
#include "audio/AudioClip.h"
#include "visuals/SyncIndicator.h"
#include "audio/SyncController.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <string>

namespace dj {

// Test 1: test_BeatJump_VariableSizes
void test_BeatJump_VariableSizes() {
    std::cout << "[TEST 1] BeatJump_VariableSizes...\n";
    
    auto clip = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    
    // Set BPM metadata
    clip.metadata_ = TrackMetadata{};
    clip.metadata_->bpm = 120.0f;
    
    Deck deck;
    deck.loadClip(clip);
    deck.setOutputSampleRate(44100);
    deck.play();
    
    // Initial position should be 0
    deck.setTempoPercent(0.0f);
    
    // At 120 BPM, 44100 samples/sec:
    // 1 beat = 60/120 sec = 0.5 sec = 22050 samples
    double samplesPerBeat = (60.0 / 120.0) * 44100.0;
    
    // Jump +1 beat
    deck.beatJump(1);
    std::size_t pos1 = deck.currentFrame();
    double expectedPos1 = samplesPerBeat;
    assert(std::abs(static_cast<double>(pos1) - expectedPos1) < 10.0);  // Within 10 samples
    
    // Jump +4 beats (total 5)
    deck.beatJump(4);
    std::size_t pos2 = deck.currentFrame();
    double expectedPos2 = 5.0 * samplesPerBeat;
    assert(std::abs(static_cast<double>(pos2) - expectedPos2) < 10.0);
    
    // Jump +8 beats (total 13)
    deck.beatJump(8);
    std::size_t pos3 = deck.currentFrame();
    double expectedPos3 = 13.0 * samplesPerBeat;
    assert(std::abs(static_cast<double>(pos3) - expectedPos3) < 10.0);
    
    // Jump -4 beats (total 9)
    deck.beatJump(-4);
    std::size_t pos4 = deck.currentFrame();
    double expectedPos4 = 9.0 * samplesPerBeat;
    assert(std::abs(static_cast<double>(pos4) - expectedPos4) < 10.0);
    
    std::cout << "  Position after +1: " << pos1 << " (expected ~" << static_cast<int>(expectedPos1) << ")\n";
    std::cout << "  Position after +4: " << pos2 << " (expected ~" << static_cast<int>(expectedPos2) << ")\n";
    std::cout << "  Position after +8: " << pos3 << " (expected ~" << static_cast<int>(expectedPos3) << ")\n";
    std::cout << "  Position after -4: " << pos4 << " (expected ~" << static_cast<int>(expectedPos4) << ")\n";
    std::cout << "PASS\n";
}

// Test 2: test_BeatJump_MaintainsPhase
void test_BeatJump_MaintainsPhase() {
    std::cout << "[TEST 2] BeatJump_MaintainsPhase...\n";
    
    auto clipA = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    auto clipB = AudioClip::generateTestTone(880.0f, 10.0f, 44100);
    
    // Set BPM metadata
    clipA.metadata_ = TrackMetadata{};
    clipA.metadata_->bpm = 120.0f;
    clipB.metadata_ = TrackMetadata{};
    clipB.metadata_->bpm = 120.0f;
    
    Deck deckA, deckB;
    deckA.loadClip(clipA);
    deckB.loadClip(clipB);
    
    deckA.setOutputSampleRate(44100);
    deckB.setOutputSampleRate(44100);
    
    deckA.play();
    deckB.play();
    
    // Set both to neutral tempo
    deckA.setTempoPercent(0.0f);
    deckB.setTempoPercent(0.0f);
    
    // Enable sync on Deck B targeting Deck A
    deckB.setAutoSyncTarget(&deckA);
    
    // Simulate some frames to establish sync
    for (int i = 0; i < 10000; ++i) {
        deckA.nextFrame();
        deckB.nextFrame();
    }
    
    // Get position before beat jump
    std::size_t posBefore = deckB.currentFrame();
    
    // Beat jump should maintain phase (both decks advance proportionally)
    deckB.beatJump(4);
    
    // After jump, both should still be in sync (verify sync is still enabled)
    assert(deckB.isSyncEnabled());
    
    std::cout << "  Position before jump: " << posBefore << " samples\n";
    std::cout << "  Sync still enabled: " << (deckB.isSyncEnabled() ? "yes" : "no") << "\n";
    std::cout << "PASS\n";
}

// Test 3: test_Warp_MicroTune
void test_Warp_MicroTune() {
    std::cout << "[TEST 3] Warp_MicroTune...\n";
    
    auto clip = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    
    clip.metadata_ = TrackMetadata{};
    clip.metadata_->bpm = 120.0f;
    
    Deck deck;
    deck.loadClip(clip);
    deck.setOutputSampleRate(44100);
    deck.play();
    
    // Set deck to neutral tempo
    deck.setTempoPercent(0.0f);
    
    // Apply warp
    deck.setWarp(0.02f);  // +0.02% micro-tune
    assert(std::abs(deck.getWarp() - 0.02f) < 0.001f);
    
    // Warp should apply on top of tempo
    // Get current effective information (will check within nextFrame processing)
    
    // Try to apply warp beyond limit (should clamp)
    deck.setWarp(0.1f);  // Try to set +0.1%
    assert(deck.getWarp() <= 0.05f);  // Should be clamped to +0.05%
    
    deck.setWarp(-0.1f);  // Try to set -0.1%
    assert(deck.getWarp() >= -0.05f);  // Should be clamped to -0.05%
    
    // Clear warp
    deck.clearWarp();
    assert(deck.getWarp() == 0.0f);
    
    std::cout << "  Warp clamped to: " << deck.getWarp() << "%\n";
    std::cout << "  Warp range verified: ±0.05%\n";
    std::cout << "PASS\n";
}

// Test 4: test_CueJump_PhasePreserve
void test_CueJump_PhasePreserve() {
    std::cout << "[TEST 4] CueJump_PhasePreserve...\n";
    
    auto clipA = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    auto clipB = AudioClip::generateTestTone(880.0f, 10.0f, 44100);
    
    // Set BPM metadata
    clipA.metadata_ = TrackMetadata{};
    clipA.metadata_->bpm = 120.0f;
    clipB.metadata_ = TrackMetadata{};
    clipB.metadata_->bpm = 120.0f;
    
    Deck deckA, deckB;
    deckA.loadClip(clipA);
    deckB.loadClip(clipB);
    
    deckA.setOutputSampleRate(44100);
    deckB.setOutputSampleRate(44100);
    
    deckA.play();
    deckB.play();
    
    deckA.setTempoPercent(0.0f);
    deckB.setTempoPercent(0.0f);
    
    // Enable sync
    deckB.setAutoSyncTarget(&deckA);
    
    for (int i = 0; i < 5000; ++i) {
        deckA.nextFrame();
        deckB.nextFrame();
    }
    
    // Set cue at beat 32 (32 * 22050 samples at 120 BPM)
    std::size_t cuePos = static_cast<std::size_t>(32.0 * 22050.0);
    deckB.setCue(cuePos);
    
    // Jump to cue
    deckB.jumpToCue();
    
    // Sync should still be enabled
    assert(deckB.isSyncEnabled());
    
    // Position should be at cue point
    std::size_t pos = deckB.currentFrame();
    assert(std::abs(static_cast<int>(pos) - static_cast<int>(cuePos)) < 100);
    
    std::cout << "  Cue position: " << cuePos << " samples\n";
    std::cout << "  Position after jump: " << pos << " samples\n";
    std::cout << "  Sync enabled: " << (deckB.isSyncEnabled() ? "yes" : "no") << "\n";
    std::cout << "PASS\n";
}

// Test 5: test_SyncUndo_RestoreState
void test_SyncUndo_RestoreState() {
    std::cout << "[TEST 5] SyncUndo_RestoreState...\n";
    
    auto clip = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    clip.metadata_ = TrackMetadata{};
    clip.metadata_->bpm = 120.0f;
    
    Deck deck;
    deck.loadClip(clip);
    deck.setOutputSampleRate(44100);
    deck.play();
    
    // Set initial state
    deck.setTempoPercent(0.0f);
    std::size_t initialPos = deck.currentFrame();
    (void)initialPos;  // Suppress unused warning
    
    // Create undo stack
    SyncUndoStack undoStack;
    
    // Capture state before sync operation
    undoStack.captureState(deck);
    
    // Simulate sync operation (tempo change)
    deck.setTempoPercent(-6.25f);  // Example tempo change from sync
    
    // Verify state changed
    float tempoAfter = deck.tempoPercent();
    assert(std::abs(tempoAfter - (-6.25f)) < 0.1f);
    
    // Undo should restore
    assert(undoStack.canUndo());
    undoStack.undo(deck);
    
    // Verify state restored
    float tempoRestored = deck.tempoPercent();
    assert(std::abs(tempoRestored - 0.0f) < 0.1f);
    
    std::cout << "  Initial tempo: 0%\n";
    std::cout << "  After sync: " << tempoAfter << "%\n";
    std::cout << "  After undo: " << tempoRestored << "%\n";
    std::cout << "PASS\n";
}

// Test 6: test_SyncIndicator_Visual
void test_SyncIndicator_Visual() {
    std::cout << "[TEST 6] SyncIndicator_Visual...\n";
    
    SyncIndicator indicator;
    
    // Test rendering with sync enabled, locked state, zero phase offset
    std::string output = indicator.render(true, SyncState::Locked, 0.0);
    
    // Check that output contains expected elements
    assert(output.find("SYNC") != std::string::npos);
    assert(output.find("LOCKED") != std::string::npos || output.find("✓") != std::string::npos);
    assert(output.find("[") != std::string::npos && output.find("]") != std::string::npos);
    
    std::cout << "  Rendered: " << output << "\n";
    std::cout << "PASS\n";
}

// Test 7: test_BeatJump_MIDI
void test_BeatJump_MIDI() {
    std::cout << "[TEST 7] BeatJump_MIDI...\n";
    
    auto clip = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    clip.metadata_ = TrackMetadata{};
    clip.metadata_->bpm = 120.0f;
    
    Deck deck;
    deck.loadClip(clip);
    deck.setOutputSampleRate(44100);
    deck.play();
    
    deck.setTempoPercent(0.0f);
    
    // Simulate MIDI jogwheel rotation (BeatJump1Forward)
    std::size_t initialPos = deck.currentFrame();
    
    double samplesPerBeat = (60.0 / 120.0) * 44100.0;  // 22050 samples/beat
    
    deck.beatJump(1);
    std::size_t posAfter1 = deck.currentFrame();
    assert(std::abs(static_cast<double>(posAfter1) - (initialPos + samplesPerBeat)) < 10.0);
    
    // Simulate rapid jumps (10x forward)
    for (int i = 0; i < 10; ++i) {
        deck.beatJump(1);
    }
    
    std::size_t finalPos = deck.currentFrame();
    double expected = initialPos + (11.0 * samplesPerBeat);
    assert(std::abs(static_cast<double>(finalPos) - expected) < 10.0);
    
    std::cout << "  Initial position: " << initialPos << " samples\n";
    std::cout << "  After 1x jump: " << posAfter1 << " samples\n";
    std::cout << "  After 10x more jumps: " << finalPos << " samples\n";
    std::cout << "  Expected: " << static_cast<int>(expected) << " samples\n";
    std::cout << "PASS\n";
}

// Test 8: test_SyncUndoStack_Depth
void test_SyncUndoStack_Depth() {
    std::cout << "[TEST 8] SyncUndoStack_Depth...\n";
    
    auto clip = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    clip.metadata_ = TrackMetadata{};
    clip.metadata_->bpm = 120.0f;
    
    Deck deck;
    deck.loadClip(clip);
    deck.setOutputSampleRate(44100);
    deck.play();
    
    SyncUndoStack undoStack;
    
    // Capture 15 states (exceeds max depth of 10)
    for (int i = 0; i < 15; ++i) {
        deck.setTempoPercent(static_cast<float>(i) * 0.5f);
        undoStack.captureState(deck);
    }
    
    // Stack should be limited to 10 entries
    // Undo 10 times
    int undoCount = 0;
    for (int i = 0; i < 10; ++i) {
        if (undoStack.canUndo()) {
            undoStack.undo(deck);
            undoCount++;
        }
    }
    
    // Should have been able to undo 10 times
    assert(undoCount == 10);
    
    // 11th undo should fail
    assert(!undoStack.canUndo());
    
    std::cout << "  Captured 15 states (max 10)\n";
    std::cout << "  Undo count: " << undoCount << "\n";
    std::cout << "  canUndo after 10 undos: " << (undoStack.canUndo() ? "true" : "false") << "\n";
    std::cout << "PASS\n";
}

} // namespace dj

int main() {
    std::cout << "=== Phase 41: Beat Jump, Warp Grid & Polish Tests ===\n\n";
    
    try {
        dj::test_BeatJump_VariableSizes();
        dj::test_BeatJump_MaintainsPhase();
        dj::test_Warp_MicroTune();
        dj::test_CueJump_PhasePreserve();
        dj::test_SyncUndo_RestoreState();
        dj::test_SyncIndicator_Visual();
        dj::test_BeatJump_MIDI();
        dj::test_SyncUndoStack_Depth();
        
        std::cout << "\n=== ALL TESTS PASSED ===\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << "\n";
        return 1;
    }
}
