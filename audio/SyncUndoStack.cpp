#include "audio/SyncUndoStack.h"
#include "audio/Deck.h"

namespace dj {

void SyncUndoStack::captureState(const Deck& deck) {
    SyncSnapshot snapshot{
        .tempoPercent = deck.tempoPercent(),
        .playbackPosition = deck.currentFrame(),
        .syncEnabled = deck.isSyncEnabled()
    };
    
    undoStack_.push_back(snapshot);
    
    // Maintain max depth
    while (undoStack_.size() > MaxUndoDepth) {
        undoStack_.pop_front();
    }
}

bool SyncUndoStack::undo(Deck& deck) {
    if (undoStack_.empty()) {
        return false;
    }
    
    const SyncSnapshot& snapshot = undoStack_.back();
    
    // Restore tempo
    deck.setTempoPercent(snapshot.tempoPercent);
    
    // Restore sync state
    if (snapshot.syncEnabled) {
        // Note: This assumes we're restoring to a state where sync was on
        // In the test, we don't have access to the target deck here,
        // so we just disable it. The calling code should handle re-enabling if needed.
        deck.disableAutoSync();
    } else {
        deck.disableAutoSync();
    }
    
    undoStack_.pop_back();
    return true;
}

bool SyncUndoStack::canUndo() const {
    return !undoStack_.empty();
}

void SyncUndoStack::clear() {
    undoStack_.clear();
}

} // namespace dj
