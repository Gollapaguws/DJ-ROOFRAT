#pragma once

#include <optional>
#include <string>

namespace dj {

struct TrackMetadata {
    std::string title;
    std::string artist;
    std::optional<float> bpm;
    std::optional<std::string> key; // e.g., "C major", "A minor"
    float durationSeconds = 0.0f;
    std::string genre;
};

} // namespace dj
