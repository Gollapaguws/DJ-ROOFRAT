#pragma once

#include <optional>
#include <string>
#include <vector>

namespace dj {

// Persistent beat grid data (for serialization to metadata)
struct BeatGridData {
    double firstBeatOffset;              // Seconds to first beat
    double bpm;                          // Beats per minute
    std::vector<double> manualNudges;    // Per-beat adjustments in milliseconds
    int barsPerPhrase;                   // 8, 16, or 32 bars

    std::string serialize() const;
    static BeatGridData deserialize(const std::string& data);
};

struct TrackMetadata {
    std::string title;
    std::string artist;
    std::optional<float> bpm;
    std::optional<std::string> key; // e.g., "C major", "A minor"
    float durationSeconds = 0.0f;
    std::string genre;
    
    // Phase 32: Optional beat grid data
    std::optional<BeatGridData> beatGrid;
    
    // Phase 33: Energy rating (1-10 scale)
    std::optional<int> energyRating;
    std::optional<float> rmsEnergy;      // Optional: store RMS for reference
    std::optional<float> peakAmplitude;  // Optional: store peak amplitude
};

} // namespace dj
