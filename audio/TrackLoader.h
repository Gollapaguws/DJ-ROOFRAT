#pragma once

#include <optional>
#include <string>

#include "audio/AudioClip.h"

namespace dj {

/**
 * @brief Utility for loading audio files from disk.
 *
 * Supports WAV and MP3 formats. Returns loaded audio clips with metadata
 * or std::nullopt on error.
 */
class TrackLoader {
public:
    /**
     * @brief Load a WAV or MP3 file into an AudioClip.
     * @param path Absolute or relative file path to audio file.
     * @param errorOut Optional pointer to receive error description on failure.
     * @return Loaded AudioClip with samples and metadata, or std::nullopt on error.
     */
    static std::optional<AudioClip> loadFile(const std::string& path, std::string* errorOut = nullptr);
};

} // namespace dj
