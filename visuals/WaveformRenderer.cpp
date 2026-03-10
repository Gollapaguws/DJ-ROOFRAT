#include "visuals/WaveformRenderer.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

namespace dj {

WaveformRenderer::WaveformRenderer(std::size_t width, std::size_t height)
    : width_(std::max<std::size_t>(16, width)),
      height_(std::max<std::size_t>(7, height | 1U)) {
}

std::string WaveformRenderer::render(const std::vector<float>& interleavedStereo) const {
    const std::size_t frameCount = interleavedStereo.size() / 2U;
    if (frameCount == 0) {
        return "(no waveform)";
    }

    const std::size_t samplesPerColumn = std::max<std::size_t>(1, frameCount / width_);
    std::vector<float> columnValues(width_, 0.0f);

    for (std::size_t x = 0; x < width_; ++x) {
        const std::size_t start = x * samplesPerColumn;
        const std::size_t end = std::min(frameCount, start + samplesPerColumn);

        if (start >= end) {
            continue;
        }

        float sum = 0.0f;
        for (std::size_t i = start; i < end; ++i) {
            const float mono = 0.5f * (interleavedStereo[(i * 2U)] + interleavedStereo[(i * 2U) + 1U]);
            sum += mono;
        }

        columnValues[x] = sum / static_cast<float>(end - start);
    }

    float maxAbs = 0.001f;
    for (const float value : columnValues) {
        maxAbs = std::max(maxAbs, std::abs(value));
    }

    std::vector<std::string> canvas(height_, std::string(width_, ' '));
    const std::size_t center = height_ / 2U;
    for (std::size_t x = 0; x < width_; ++x) {
        canvas[center][x] = '-';
    }

    const int extent = static_cast<int>(center);
    for (std::size_t x = 0; x < width_; ++x) {
        const float normalized = std::clamp(columnValues[x] / maxAbs, -1.0f, 1.0f);
        const int y = static_cast<int>(center) - static_cast<int>(std::round(normalized * static_cast<float>(extent - 1)));
        const int yClamped = std::clamp(y, 0, static_cast<int>(height_) - 1);

        const int low = std::min(static_cast<int>(center), yClamped);
        const int high = std::max(static_cast<int>(center), yClamped);
        for (int row = low; row <= high; ++row) {
            canvas[static_cast<std::size_t>(row)][x] = '|';
        }
        canvas[static_cast<std::size_t>(yClamped)][x] = '*';
    }

    std::ostringstream output;
    for (std::size_t row = 0; row < height_; ++row) {
        output << canvas[row];
        if (row + 1U < height_) {
            output << '\n';
        }
    }

    return output.str();
}

} // namespace dj
