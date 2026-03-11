#include "library/TrackBrowser.h"

#include <algorithm>
#include <cmath>
#include <map>

namespace dj::library {

TrackBrowser::TrackBrowser(std::shared_ptr<TrackLibrary> library) : library(library) {}

std::vector<StoredTrack> TrackBrowser::filterByBpm(float minBpm, float maxBpm) const {
    if (!library) {
        return {};
    }
    return library->queryByBpm(minBpm, maxBpm);
}

std::vector<StoredTrack> TrackBrowser::filterByKey(const std::string& key) const {
    if (!library) {
        return {};
    }
    return library->queryByKey(key);
}

// Circle of Fifths mapping for key compatibility
// Each key maps to its compatible keys (itself and adjacent keys in circle)
static const std::map<std::string, std::vector<std::string>> COMPATIBLE_KEYS = {
    {"C major", {"C major", "G major", "F major", "A minor", "E minor", "D minor"}},
    {"G major", {"G major", "D major", "C major", "E minor", "B minor", "A minor"}},
    {"D major", {"D major", "A major", "G major", "B minor", "F# minor", "E minor"}},
    {"A major", {"A major", "E major", "D major", "F# minor", "C# minor", "B minor"}},
    {"E major", {"E major", "B major", "A major", "C# minor", "G# minor", "F# minor"}},
    {"B major", {"B major", "F# major", "E major", "G# minor", "D# minor", "C# minor"}},
    {"F# major", {"F# major", "C# major", "B major", "D# minor", "A# minor", "G# minor"}},
    {"C# major", {"C# major", "G# major", "F# major", "A# minor", "E# minor", "B# minor"}},
    {"F major", {"F major", "C major", "Bb major", "D minor", "A minor", "G minor"}},
    {"Bb major", {"Bb major", "F major", "Eb major", "G minor", "D minor", "C minor"}},
    {"Eb major", {"Eb major", "Bb major", "Ab major", "C minor", "G minor", "F minor"}},
    {"Ab major", {"Ab major", "Eb major", "Db major", "F minor", "C minor", "Bb minor"}},
    {"Db major", {"Db major", "Ab major", "Gb major", "Bb minor", "F minor", "Eb minor"}},
    {"Gb major", {"Gb major", "Db major", "Cb major", "Eb minor", "Bb minor", "Ab minor"}},
    {"Cb major", {"Cb major", "Gb major", "Bbb major", "Ab minor", "Eb minor", "Db minor"}},

    {"A minor", {"A minor", "E minor", "D minor", "C major", "G major", "F major"}},
    {"E minor", {"E minor", "B minor", "A minor", "G major", "D major", "C major"}},
    {"B minor", {"B minor", "F# minor", "E minor", "D major", "A major", "G major"}},
    {"F# minor", {"F# minor", "C# minor", "B minor", "A major", "E major", "D major"}},
    {"C# minor", {"C# minor", "G# minor", "F# minor", "E major", "B major", "A major"}},
    {"G# minor", {"G# minor", "D# minor", "C# minor", "B major", "F# major", "E major"}},
    {"D# minor", {"D# minor", "A# minor", "G# minor", "F# major", "C# major", "B major"}},
    {"A# minor", {"A# minor", "E# minor", "D# minor", "C# major", "G# major", "F# major"}},
    {"D minor", {"D minor", "A minor", "G minor", "F major", "C major", "Bb major"}},
    {"G minor", {"G minor", "D minor", "C minor", "Bb major", "F major", "Eb major"}},
    {"C minor", {"C minor", "G minor", "F minor", "Eb major", "Bb major", "Ab major"}},
    {"F minor", {"F minor", "C minor", "Bb minor", "Ab major", "Eb major", "Db major"}},
    {"Bb minor", {"Bb minor", "F minor", "Eb minor", "Db major", "Ab major", "Gb major"}},
    {"Eb minor", {"Eb minor", "Bb minor", "Ab minor", "Gb major", "Db major", "Cb major"}},
    {"Ab minor", {"Ab minor", "Eb minor", "Db minor", "Cb major", "Gb major", "Bbb major"}},
};

bool TrackBrowser::areKeysCompatible(const std::string& key1, const std::string& key2) {
    auto it = COMPATIBLE_KEYS.find(key1);
    if (it == COMPATIBLE_KEYS.end()) {
        // If key1 is not found, check the other direction or consider them compatible
        return true;  // More lenient: if key mapping unknown, consider compatible
    }

    const auto& compatibleList = it->second;
    return std::find(compatibleList.begin(), compatibleList.end(), key2) != compatibleList.end();
}

std::vector<std::string> TrackBrowser::getCompatibleKeys(const std::string& key) {
    auto it = COMPATIBLE_KEYS.find(key);
    if (it != COMPATIBLE_KEYS.end()) {
        return it->second;
    }
    return {key};  // Return the key itself if not found in map
}

std::vector<StoredTrack> TrackBrowser::getSmartPlaylist(const StoredTrack& referenceTrack, int maxTracks) const {
    if (!library) {
        return {};
    }

    std::vector<StoredTrack> allTracks = library->getAllTracks();
    std::vector<StoredTrack> recommendations;

    const float BPM_TOLERANCE = 10.0f;
    const float refBpm = referenceTrack.bpm.value_or(120.0f);
    const std::string& refKey = referenceTrack.key.value_or("C major");

    for (const auto& track : allTracks) {
        // Skip the reference track itself
        if (track.id == referenceTrack.id) {
            continue;
        }

        float trackBpm = track.bpm.value_or(120.0f);
        const std::string& trackKey = track.key.value_or("C major");

        // Check BPM compatibility (within ±10 BPM)
        if (std::abs(trackBpm - refBpm) > BPM_TOLERANCE) {
            continue;
        }

        // Check key compatibility
        if (!areKeysCompatible(refKey, trackKey)) {
            continue;
        }

        recommendations.push_back(track);

        if (static_cast<int>(recommendations.size()) >= maxTracks) {
            break;
        }
    }

    return recommendations;
}

std::vector<StoredTrack> TrackBrowser::sort(std::vector<StoredTrack> tracks,
                                             SortField field,
                                             SortOrder order) const {
    using CompareFunc = bool (*)(const StoredTrack&, const StoredTrack&);

    CompareFunc compareFunc = nullptr;

    switch (field) {
        case SortField::Title:
            compareFunc = [](const StoredTrack& a, const StoredTrack& b) { return a.title < b.title; };
            break;
        case SortField::Artist:
            compareFunc = [](const StoredTrack& a, const StoredTrack& b) { return a.artist < b.artist; };
            break;
        case SortField::BPM:
            compareFunc = [](const StoredTrack& a, const StoredTrack& b) {
                float aBpm = a.bpm.value_or(0.0f);
                float bBpm = b.bpm.value_or(0.0f);
                return aBpm < bBpm;
            };
            break;
        case SortField::Key:
            compareFunc = [](const StoredTrack& a, const StoredTrack& b) {
                return a.key.value_or("") < b.key.value_or("");
            };
            break;
        case SortField::Duration:
            compareFunc = [](const StoredTrack& a, const StoredTrack& b) {
                return a.durationSeconds < b.durationSeconds;
            };
            break;
        case SortField::Genre:
            compareFunc = [](const StoredTrack& a, const StoredTrack& b) { return a.genre < b.genre; };
            break;
    }

    if (compareFunc) {
        if (order == SortOrder::Ascending) {
            std::sort(tracks.begin(), tracks.end(), compareFunc);
        } else {
            std::sort(tracks.begin(), tracks.end(),
                      [compareFunc](const StoredTrack& a, const StoredTrack& b) { return compareFunc(b, a); });
        }
    }

    return tracks;
}

std::vector<StoredTrack> TrackBrowser::filter(float minBpm,
                                               float maxBpm,
                                               const std::string& key,
                                               const std::string& artist,
                                               const std::string& genre) const {
    if (!library) {
        return {};
    }

    // Start with all tracks
    auto results = library->getAllTracks();

    // Filter by BPM
    results.erase(
        std::remove_if(results.begin(), results.end(),
                       [minBpm, maxBpm](const StoredTrack& track) {
                           if (!track.bpm.has_value()) {
                               return true;
                           }
                           float bpm = track.bpm.value();
                           return bpm < minBpm || bpm > maxBpm;
                       }),
        results.end());

    // Filter by key
    if (!key.empty()) {
        results.erase(
            std::remove_if(results.begin(), results.end(),
                           [&key](const StoredTrack& track) {
                               return !track.key.has_value() || track.key.value() != key;
                           }),
            results.end());
    }

    // Filter by artist
    if (!artist.empty()) {
        results.erase(
            std::remove_if(results.begin(), results.end(),
                           [&artist](const StoredTrack& track) { return track.artist != artist; }),
            results.end());
    }

    // Filter by genre
    if (!genre.empty()) {
        results.erase(
            std::remove_if(results.begin(), results.end(),
                           [&genre](const StoredTrack& track) { return track.genre != genre; }),
            results.end());
    }

    return results;
}

} // namespace dj::library
