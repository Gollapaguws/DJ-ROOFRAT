#pragma once

#include "audio/TrackMetadata.h"
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace dj {

/**
 * @brief A single detected beat in the grid.
 *
 * Represents a beat marker with timing, bar position, downbeat flag, and phrase start flag.
 */
struct BeatMarker {
    /// Time position from track start in seconds
    double timestamp;
    /// Beat position within bar (1–4 in 4/4 time)
    int barPosition;
    /// True if this is the first beat (downbeat) of its bar
    bool isDownbeat;
    /// True if this beat marks the start of a phrase
    bool isPhraseStart;
};

// BeatGridData is defined in TrackMetadata.h

/**
 * @brief Beat grid holding all beat markers for a track, with manual editing and phrase detection.
 *
 * Manages beat detection, manual editing (nudging), and phrase boundary identification
 * for precise DJ beat synchronization.
 */
class BeatGrid {
public:
    /**
     * @brief Construct beat grid.
     */
    BeatGrid();
    /**
     * @brief Destructor.
     */
    ~BeatGrid() = default;

    // Get all beat markers
    /**
     * @brief Return all beat markers in chronological order.
     * @return Vector of all BeatMarker objects.
     */
    std::vector<BeatMarker> getBeats() const;

    // Generate grid from BPM and track duration
    /**
     * @brief Generate a uniform beat grid from BPM.
     * @param bpm Tempo in beats per minute.
     * @param firstBeatOffset Time of first beat in seconds from track start.
     * @param trackDuration Total track duration in seconds.
     */
    void generateFromBPM(double bpm, double firstBeatOffset, double trackDuration);

    // Manual beat grid editing
    /**
     * @brief Shift a single beat marker.
     * @param beatIndex Index of beat (0-based).
     * @param offsetMs Time offset in milliseconds (positive=later, negative=earlier).
     */
    void nudgeBeat(std::size_t beatIndex, double offsetMs);  // ±offset in milliseconds
    /**
     * @brief Shift all beats uniformly.
     * @param offsetMs Millisecond offset applied to all beats (positive=later).
     */
    void nudgeAll(double offsetMs);                          // Shift all beats uniformly
    /**
     * @brief Snap a beat to the nearest onset.
     * @param beatIndex Index of beat to snap.
     * @param onsets Vector of onset timestamps in seconds.
     */
    void snapToOnset(std::size_t beatIndex, const std::vector<double>& onsets);

    // Phrase detection and clustering
    /**
     * @brief Detect and mark phrase boundaries.
     * @param barsPerPhrase Bars per phrase (8, 16, or 32, default 16).
     */
    void markPhrases(int barsPerPhrase = 16);  // 8, 16, or 32 bars

    // Get current BPM
    /**
     * @brief Get current BPM.
     * @return Beats per minute, or 0 if not set.
     */
    double getBPM() const;

    // Get first beat offset
    /**
     * @brief Get first beat offset.
     * @return Time of first beat in seconds.
     */
    double getFirstBeatOffset() const;

    // Convert to persistent data structure
    /**
     * @brief Serialize to persistent data structure.
     * @return BeatGridData representation.
     */
    BeatGridData toData() const;

    // Load from persistent data structure
    /**
     * @brief Deserialize from persistent data structure.
     * @param data BeatGridData to load.
     * @param trackDuration Track duration in seconds (for validation).
     */
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
