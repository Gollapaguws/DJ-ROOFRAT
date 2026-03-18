// Phase 1: Integration E2E Test - Auto-Sync
// Tests: Enable sync on deck → verify SyncController state → verify SyncIndicator renders correctly

#include "audio/Deck.h"
#include "audio/AudioClip.h"
#include "visuals/SyncIndicator.h"

#include <cassert>
#include <iostream>
#include <string>

namespace dj {

// E2E Test 3: Auto-sync state management and visual indicators
void test_E2E_AutoSync_VisualIndicators() {
    std::cout << "[TEST 1] E2E_AutoSync_VisualIndicators...\n";
    
    try {
        const int sampleRate = 44100;
        
        // Step 1: Create two test clips
        auto clipA = AudioClip::generateTestTone(440.0f, 2.0f, sampleRate);  // A4: 440 Hz
        auto clipB = AudioClip::generateTestTone(880.0f, 2.0f, sampleRate);  // A5: 880 Hz
        
        // Set BPM metadata
        clipA.metadata_ = TrackMetadata{};
        clipA.metadata_->bpm = 120.0f;
        clipB.metadata_ = TrackMetadata{};
        clipB.metadata_->bpm = 125.0f;
        
        // Step 2: Initialize two decks
        Deck deckA, deckB;
        deckA.setOutputSampleRate(sampleRate);
        deckB.setOutputSampleRate(sampleRate);
        
        bool loadedA = deckA.loadClip(clipA);
        bool loadedB = deckB.loadClip(clipB);
        assert(loadedA && loadedB && "Failed to load clips");
        
        // Step 3: Start both decks
        deckA.play();
        deckB.play();
        
        // Step 4: Enable sync on DeckB targeting DeckA
        deckB.setAutoSyncTarget(&deckA);
        assert(deckB.isSyncEnabled() && "Sync not enabled on DeckB");
        
        // Step 5: Verify sync state
        SyncState syncState = deckB.getSyncState();
        assert(syncState != SyncState::Off && "Sync state should not be Off");
        
        // Step 6: Render sync indicator with initial state
        SyncIndicator indicator;
        double phaseOffset = deckB.getPhaseOffset();
        std::string syncDisplay = indicator.render(deckB.isSyncEnabled(), syncState, phaseOffset);
        assert(!syncDisplay.empty() && "Sync indicator should render non-empty string");
        assert(syncDisplay.find("[SYNC") != std::string::npos && "Sync indicator should contain '[SYNC'");
        
        std::cout << "  ✓ Sync enabled on DeckB\n";
        std::cout << "  ✓ Sync state: " << (syncState == SyncState::Initializing ? "Initializing" :
                                             syncState == SyncState::Locked ? "Locked" :
                                             syncState == SyncState::Drifting ? "Drifting" : "Unknown") << "\n";
        std::cout << "  ✓ Indicator rendered: " << syncDisplay << "\n";
        
        // Step 7: Disable sync and verify
        deckB.disableAutoSync();
        assert(!deckB.isSyncEnabled() && "Sync should be disabled");
        assert(deckB.getSyncState() == SyncState::Off && "Sync state should be Off");
        
        // Render with disabled sync
        std::string syncDisplayOff = indicator.render(deckB.isSyncEnabled(), deckB.getSyncState(), phaseOffset);
        // Should be empty or minimal when disabled
        
        std::cout << "  ✓ Sync disabled on DeckB\n";
        std::cout << "PASS\n";
    } catch (const std::exception& e) {
        std::cout << "  ✗ Exception during auto-sync test: " << e.what() << "\n";
        assert(false && "Auto-sync test failed");
    }
}

void runAllE2EAutoSyncTests() {
    std::cout << "\n=== Running E2E Auto-Sync Tests ===\n";
    try {
        test_E2E_AutoSync_VisualIndicators();
    } catch (const std::exception& e) {
        std::cerr << "Auto-sync test failed with exception: " << e.what() << "\n";
    }
}

} // namespace dj

// Entry point
int main() {
    dj::runAllE2EAutoSyncTests();
    std::cout << "\n=== E2E Auto-Sync Tests Complete ===\n";
    return 0;
}
