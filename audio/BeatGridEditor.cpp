#include "audio/BeatGridEditor.h"
#include <algorithm>

namespace dj {

BeatGridEditor::BeatGridEditor(const BeatGrid& initialGrid, int sampleRate)
    : currentGrid_(initialGrid)
    , sampleRate_(sampleRate)
    , trackDuration_(10.0)  // Default 10 seconds
{
}

const BeatGrid& BeatGridEditor::getBeatGrid() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return currentGrid_;
}

void BeatGridEditor::nudgeBeats(double deltaMs) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Push current state to undo stack before modification
    pushState();
    
    // Nudge all beats
    currentGrid_.nudgeAll(deltaMs);
    // Note: BeatGrid::nudgeAll() maintains beat time invariants internally
}

void BeatGridEditor::setFirstBeatOffset(double deltaMs) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Push current state to undo stack before modification
    pushState();
    
    // Get current grid parameters
    double currentBpm = currentGrid_.getBPM();
    double currentFirstBeat = currentGrid_.getFirstBeatOffset();
    
    // Calculate new first beat offset (convert deltaMs to seconds and add to current)
    double deltaSeconds = deltaMs / 1000.0;
    double newFirstBeatOffset = currentFirstBeat + deltaSeconds;
    
    // Ensure first beat offset is non-negative
    newFirstBeatOffset = std::max(0.0, newFirstBeatOffset);
    
    // Regenerate the beat grid with new first beat offset
    currentGrid_.generateFromBPM(currentBpm, newFirstBeatOffset, trackDuration_);
    // Note: BeatGrid::generateFromBPM() maintains beat time invariants internally
}

void BeatGridEditor::undo() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (undoStack_.empty()) {
        return;
    }
    
    // Push current state to redo stack
    redoStack_.push_back(currentGrid_);
    
    // Restore state from undo stack
    currentGrid_ = undoStack_.back();
    undoStack_.pop_back();
}

void BeatGridEditor::redo() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (redoStack_.empty()) {
        return;
    }
    
    // Push current state to undo stack
    undoStack_.push_back(currentGrid_);
    
    // Enforce undo stack size limit
    if (undoStack_.size() > MAX_UNDO_STACK) {
        undoStack_.pop_front();
    }
    
    // Restore state from redo stack
    currentGrid_ = redoStack_.back();
    redoStack_.pop_back();
}

bool BeatGridEditor::canUndo() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return !undoStack_.empty();
}

bool BeatGridEditor::canRedo() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return !redoStack_.empty();
}

double BeatGridEditor::getTrackDuration() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return trackDuration_;
}

void BeatGridEditor::setTrackDuration(double durationSeconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    trackDuration_ = durationSeconds;
}

void BeatGridEditor::pushState() {
    // Clear redo stack when making a new change
    redoStack_.clear();
    
    // Push current state to undo stack
    undoStack_.push_back(currentGrid_);
    
    // Enforce undo stack size limit
    if (undoStack_.size() > MAX_UNDO_STACK) {
        undoStack_.pop_front();
    }
}

} // namespace dj
