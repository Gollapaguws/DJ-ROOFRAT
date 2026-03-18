#pragma once

#include <deque>
#include <cstddef>

namespace dj {

class Deck;

/**
 * @brief Undo stack for sync operations.
 * 
 * Captures deck state before sync operations to allow
 * Ctrl+Z style undo. Max 10 operations.
 */
class SyncUndoStack {
public:
    SyncUndoStack() = default;
    ~SyncUndoStack() = default;

    /**
     * @brief Capture current deck state.
     * 
     * @param deck Deck to snapshot
     */
    void captureState(const Deck& deck);

    /**
     * @brief Restore last captured state.
     * 
     * @param deck Deck to restore
     * @return true if undo successful, false if stack empty
     */
    bool undo(Deck& deck);

    /**
     * @brief Check if undo is available.
     * 
     * @return true if undo stack has snapshots
     */
    bool canUndo() const;

    /**
     * @brief Clear undo stack.
     */
    void clear();

private:
    struct SyncSnapshot {
        float tempoPercent;
        std::size_t playbackPosition;
        bool syncEnabled;
    };

    static constexpr std::size_t MaxUndoDepth = 10;
    std::deque<SyncSnapshot> undoStack_;
};

} // namespace dj
