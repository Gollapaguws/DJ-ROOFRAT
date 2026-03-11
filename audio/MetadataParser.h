#pragma once

#include <optional>
#include <string>

#include "audio/TrackMetadata.h"

namespace dj {

class MetadataParser {
public:
    // Parse metadata from file path. Returns optional metadata on success.
    static std::optional<TrackMetadata> parseFile(const std::string& path, std::string* errorOut = nullptr);
};

} // namespace dj
