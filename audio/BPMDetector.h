#pragma once

#include <optional>

#include "audio/AudioClip.h"

namespace dj {

class BPMDetector {
public:
    static std::optional<float> estimate(const AudioClip& clip, float minBpm = 70.0f, float maxBpm = 180.0f);
};

} // namespace dj
