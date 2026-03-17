#pragma once

#include "audio/BeatGrid.h"
#include <deque>
#include <memory>
#include <mutex>

namespace dj {

class BeatGridEditor {
public:
    BeatGridEditor(const BeatGrid& initialGrid, int sampleRate);
    ~BeatGridEditor() = default;

    // Get current beat grid
    const BeatGrid& getBeatGrid() const;

    // Nudge all beats by deltaMs (can be positive or negative)
    void nudgeBeats(double deltaMs);

    // Adjust first beat offset, recalculate all subsequent beats
    void setFirstBeatOffset(double deltaMs);

    // Undo/redo operations
    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;

    // Get track duration for validation
    double getTrackDuration() const;
    void setTrackDuration(double durationSeconds);

private:
    void pushState();

    BeatGrid currentGrid_;
    std::deque<BeatGrid> undoStack_;
    std::deque<BeatGrid> redoStack_;
    int sampleRate_;
    double trackDuration_;
    
    static constexpr size_t MAX_UNDO_STACK = 50;

    mutable std::mutex mutex_;  // For thread safety
};

} // namespace dj
