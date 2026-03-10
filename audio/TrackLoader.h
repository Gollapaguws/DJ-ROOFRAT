#pragma once

#include <optional>
#include <string>

#include "audio/AudioClip.h"

namespace dj {

class TrackLoader {
public:
    static std::optional<AudioClip> loadFile(const std::string& path, std::string* errorOut = nullptr);
};

} // namespace dj
