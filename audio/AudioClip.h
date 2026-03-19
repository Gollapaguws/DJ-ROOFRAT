#pragma once

#include "audio/TrackMetadata.h"

#include <array>
#include <cstddef>
#include <optional>
#include <vector>

namespace dj {

/**
 * @brief Container for decoded audio samples with metadata.
 *
 * Holds decoded stereo audio data, sample rate, and optional track metadata
 * for playback and analysis operations.
 */
struct AudioClip {
    int sampleRate = 44100;
    int channels = 2;
    std::vector<float> samples;
    std::optional<TrackMetadata> metadata_;

    /**
     * @brief Returns true if the clip has no samples.
     * @return true if samples vector is empty, false otherwise.
     */
    bool empty() const;

    /**
     * @brief Returns number of stereo frames.
     * @return Number of frames (samples / channels).
     */
    std::size_t frameCount() const;

    /**
     * @brief Interpolated stereo frame at fractional index.
     * @param frameIndex Fractional frame position (may be between integer frames).
     * @return Stereo sample pair [L, R] interpolated at index, or [0, 0] if out of range.
     */
    std::array<float, 2> frameAt(double frameIndex) const;
    
    /**
     * @brief Returns pointer to metadata, or nullptr if absent.
     * @return Const pointer to TrackMetadata if present, nullptr otherwise.
     */
    const TrackMetadata* metadata() const;

    /**
     * @brief Factory: generate a sine-wave test tone.
     * @param frequencyHz Tone frequency in Hertz (e.g., 440 for A4).
     * @param seconds Duration in seconds.
     * @param sampleRate Sample rate in Hz (default 44100).
     * @return New AudioClip with generated samples at specified frequency and duration.
     */
    static AudioClip generateTestTone(float frequencyHz, float seconds, int sampleRate = 44100);
};

} // namespace dj
