#pragma once

#include <string>

namespace dj {

class BeatGrid;

class BeatGridRenderer {
public:
    BeatGridRenderer() = default;
    ~BeatGridRenderer() = default;

    // Render beat grid as ASCII visualization
    // width: terminal width in characters
    std::string render(const BeatGrid& grid, int width = 80) const;

private:
    std::string renderBeatMarkers(const BeatGrid& grid, int width) const;
    std::string renderBarNumbers(const BeatGrid& grid, int width) const;
    std::string renderPhraseMarkers(const BeatGrid& grid, int width) const;
};

} // namespace dj
