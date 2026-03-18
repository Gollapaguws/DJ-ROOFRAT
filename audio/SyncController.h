#pragma once

#include "audio/PhaseAligner.h"
#include <cstddef>

namespace dj {

class Deck;

enum class SyncState {
    Off,           // Manual control
    Initializing,  // Matching BPM + aligning phase
    Locked,        // Sync active, maintaining phase
    Drifting       // Phase drifting (needs correction)
};

/**
 * @brief Auto-tempo matching and phase lock system.
 * 
 * Synchronizes two decks by:
 * - Matching BPM smoothly over 2-5 seconds
 * - Maintaining phase lock (< 10ms drift over 5 minutes)
 * - Supporting beat jump controls while maintaining sync
 * - Integrating with keyboard and MIDI controller input
 * 
 * Usage:
 *   SyncController controller;
 *   deckB.setAutoSyncTarget(&deckA);
 *   // In main loop:
 *   controller.update(deckB, deckA, bpmA, bpmB);
 */
class SyncController {
public:
    SyncController() = default;
    ~SyncController() = default;

    /**
     * @brief Update sync state and adjust deck tempo/phase.
     * 
     * Called once per frame (or regularly) to maintain sync.
     * Updates the sync deck's tempo toward the target deck and checks phase alignment.
     * 
     * @param syncDeck Deck to synchronize (will be adjusted)
     * @param targetDeck Reference deck (will not be modified)
     * @param bpmA Current BPM of target deck
     * @param bpmB Current BPM of sync deck
     */
    void update(Deck& syncDeck, const Deck& targetDeck, double bpmA, double bpmB);

    /**
     * @brief Enable sync on a deck targeting another deck.
     * 
     * @param syncDeck Deck to synchronize
     * @param targetDeck Reference deck (can be null to disable)
     */
    void enableSync(Deck& syncDeck, Deck* targetDeck);

    /**
     * @brief Disable sync control.
     */
    void disableSync();

    /**
     * @brief Check if sync is currently locked and stable.
     * 
     * @return true if phase lock has been achieved and is stable
     */
    bool isLocked() const;

    /**
     * @brief Get current sync state.
     * 
     * @return Current SyncState (Off, Initializing, Locked, or Drifting)
     */
    SyncState getState() const;

private:
    SyncState state_ = SyncState::Off;
    Deck* targetDeck_ = nullptr;
    double lastPhaseCheck_ = 0.0;
    PhaseAligner aligner_;
    
    // Track how many frames since last adjustment
    int framesSinceAdjustment_ = 0;
    
    // Track sustained lock for stability detection
    int lockedFrameCount_ = 0;
    const int LOCK_STABILITY_THRESHOLD = 1000;  // Frames to consider "locked"
};

} // namespace dj
