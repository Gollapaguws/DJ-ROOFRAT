#pragma once

#include <optional>

#include "audio/AudioClip.h"

namespace dj {

/**
 * @brief Autocorrelation-based BPM estimator for loaded audio clips.
 *
 * Analyzes the autocorrelation function of audio to detect tempo peaks
 * within a specified BPM range.
 */
class BPMDetector {
public:
    /**
     * @brief Estimate tempo from an audio clip.
     * @param clip Source audio clip to analyze.
     * @param minBpm Minimum BPM to detect (default 70).
     * @param maxBpm Maximum BPM to detect (default 180).
     * @return Detected BPM, or std::nullopt if detection failed.
     */
    [[nodiscard]] static std::optional<float> estimate(const AudioClip& clip, float minBpm = 70.0f, float maxBpm = 180.0f);
};

} // namespace dj
