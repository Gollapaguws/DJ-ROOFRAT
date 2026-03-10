#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace dj {

class WaveformRenderer {
public:
    WaveformRenderer(std::size_t width = 72, std::size_t height = 13);
    std::string render(const std::vector<float>& interleavedStereo) const;

private:
    std::size_t width_;
    std::size_t height_;
};

} // namespace dj
