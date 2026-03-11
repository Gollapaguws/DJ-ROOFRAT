#pragma once

#include <optional>
#include <string>

namespace dj {

struct AudioClip;

class KeyDetector {
public:
    // Detect the musical key of an audio clip using Krumhansl-Schmuckler algorithm.
    // Returns key as string (e.g., "C major", "A minor") or empty optional if detection fails.
    static std::optional<std::string> detect(const AudioClip& clip, std::string* errorOut = nullptr);
};

} // namespace dj
