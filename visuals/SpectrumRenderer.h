#pragma once

#include <array>
#include <cstddef>
#include <string>

namespace dj {

class SpectrumRenderer {
public:
    // Constructor: numBands (typically 20), height (row count for bar chart)
    explicit SpectrumRenderer(std::size_t numBands = 20, std::size_t height = 10);

    // Render spectrum bands as ASCII bar chart (terminal mode)
    std::string render(const std::array<float, 20>& bands) const;

    // Set display width (for terminal)
    void setWidth(std::size_t width) { width_ = width; }

    // Set display height (for terminal)
    void setHeight(std::size_t height) { height_ = height; }

    // Get current width
    std::size_t getWidth() const { return width_; }

    // Get current height
    std::size_t getHeight() const { return height_; }

private:
    std::size_t numBands_;
    std::size_t width_;
    std::size_t height_;
    mutable std::array<float, 20> peakHold_;    // Peak hold for each band
    mutable std::array<float, 20> peakDecay_;   // Decay timer for peaks

    // Internal methods
    std::string renderBars(const std::array<float, 20>& bands) const;
};

} // namespace dj
