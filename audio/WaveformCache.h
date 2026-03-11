#pragma once

#include <vector>

namespace dj {

class AudioClip;

struct WaveformBucket {
    float minSample = 0.0f;
    float maxSample = 0.0f;
};

class WaveformCache {
public:
    // Generate downsampled waveform for visualization.
    // bucketCount: target number of buckets (e.g., 1000-2000 points)
    // Returns vector of min/max pairs for efficient rendering.
    static std::vector<WaveformBucket> generate(const AudioClip& clip, int bucketCount = 2000);
};

} // namespace dj
