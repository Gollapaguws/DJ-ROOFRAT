#pragma once

#include "audio/TrackMetadata.h"

#include <array>
#include <cstddef>
#include <optional>
#include <vector>

namespace dj {

struct AudioClip {
    int sampleRate = 44100;
    int channels = 2;
    std::vector<float> samples;
    std::optional<TrackMetadata> metadata_;

    bool empty() const;
    std::size_t frameCount() const;
    std::array<float, 2> frameAt(double frameIndex) const;
    
    // Metadata accessor: returns pointer to metadata if present, nullptr otherwise
    const TrackMetadata* metadata() const;

    static AudioClip generateTestTone(float frequencyHz, float seconds, int sampleRate = 44100);
};

} // namespace dj
