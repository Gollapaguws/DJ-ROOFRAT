#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace dj {

/**
 * @brief Terminal waveform renderer outputting ASCII art to stdout.
 *
 * Renders interleaved stereo audio samples as a two-channel ASCII waveform
 * for terminal display during playback.
 */
class WaveformRenderer {
public:
    /**
     * @brief Construct renderer.
     * @param width Terminal width in columns (default 72).
     * @param height Display height in rows (default 13).
     */
    WaveformRenderer(std::size_t width = 72, std::size_t height = 13);
    /**
     * @brief Render interleaved stereo audio as ASCII waveform.
     * @param interleavedStereo Float samples, interleaved stereo (2*N samples for N frames).
     * @return Rendered string (ready for printing to terminal) containing ASCII art.
     */
    std::string render(const std::vector<float>& interleavedStereo) const;

private:
    std::size_t width_;
    std::size_t height_;
};

} // namespace dj
