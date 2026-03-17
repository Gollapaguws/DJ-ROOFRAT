#pragma once

#include <optional>
#include <string>
#include <vector>
#include <memory>

#include "library/TrackLibrary.h"

namespace dj::library {

enum class SortField {
    Title,
    Artist,
    BPM,
    Key,
    Duration,
    Genre
};

enum class SortOrder {
    Ascending,
    Descending
};

class TrackBrowser {
public:
    explicit TrackBrowser(std::shared_ptr<TrackLibrary> library);

    // Filter tracks by BPM range
    std::vector<StoredTrack> filterByBpm(float minBpm, float maxBpm) const;

    // Filter tracks by key
    std::vector<StoredTrack> filterByKey(const std::string& key) const;

    // Get smart playlist recommendations (compatible keys and similar BPM)
    std::vector<StoredTrack> getSmartPlaylist(const StoredTrack& referenceTrack, int maxTracks = 20) const;

    // Sort results
    std::vector<StoredTrack> sort(std::vector<StoredTrack> tracks, 
                                   SortField field, 
                                   SortOrder order = SortOrder::Ascending) const;

    // Combined filtering
    std::vector<StoredTrack> filter(float minBpm = 0.0f, 
                                     float maxBpm = 999.0f,
                                     const std::string& key = "",
                                     const std::string& artist = "",
                                     const std::string& genre = "") const;

    // Get harmonically compatible tracks using Camelot Wheel system
    // Returns tracks with compatible musical keys sorted by compatibility score (highest first)
    // Minimum compatibility threshold: 0.7
    std::vector<StoredTrack> getHarmonicMatches(const std::string& camelotKey, int maxTracks = 20) const;

    // Filter tracks by energy range (1-10 scale)
    // Returns tracks with energy ratings within minEnergy to maxEnergy (inclusive)
    std::vector<StoredTrack> filterByEnergyRange(int minEnergy, int maxEnergy, int maxTracks = 999) const;

private:
    std::shared_ptr<TrackLibrary> library;

    // Helper: Check if two keys are compatible (within circle of fifths)
    static bool areKeysCompatible(const std::string& key1, const std::string& key2);

    // Get all compatible keys for a given key
    static std::vector<std::string> getCompatibleKeys(const std::string& key);
};

} // namespace dj::library
