#pragma once

#include "audio/TrackMetadata.h"
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace dj {

// Individual beat marker in the grid
struct BeatMarker {
    double timestamp;      // Seconds from track start
    int barPosition;       // 1-4 (beat within 4/4 bar)
    bool isDownbeat;       // First beat of bar (bar position == 1)
    bool isPhraseStart;    // First beat of phrase
};

// BeatGridData is defined in TrackMetadata.h

// Beat grid for manual editing and phrase detection
class BeatGrid {
public:
    BeatGrid();
    ~BeatGrid() = default;

    // Get all beat markers
    std::vector<BeatMarker> getBeats() const;

    // Generate grid from BPM and track duration
    void generateFromBPM(double bpm, double firstBeatOffset, double trackDuration);

    // Manual beat grid editing
    void nudgeBeat(std::size_t beatIndex, double offsetMs);  // ±offset in milliseconds
    void nudgeAll(double offsetMs);                          // Shift all beats uniformly
    void snapToOnset(std::size_t beatIndex, const std::vector<double>& onsets);

    // Phrase detection and clustering
    void markPhrases(int barsPerPhrase = 16);  // 8, 16, or 32 bars

    // Get current BPM
    double getBPM() const;

    // Get first beat offset
    double getFirstBeatOffset() const;

    // Convert to persistent data structure
    BeatGridData toData() const;

    // Load from persistent data structure
    void fromData(const BeatGridData& data, double trackDuration);

private:
    void updateBeatPositions(double trackDuration);

    std::vector<BeatMarker> beats_;
    double firstBeatOffset_;  // Seconds
    double bpm_;              // Beats per minute
    int barsPerPhrase_;       // For phrase detection
    std::vector<double> manualNudges_;  // Per-beat nudge adjustments in milliseconds
};

} // namespace dj
