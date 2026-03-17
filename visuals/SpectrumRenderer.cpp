#include "visuals/SpectrumRenderer.h"

#include <algorithm>
#include <sstream>
#include <iomanip>

namespace dj {

SpectrumRenderer::SpectrumRenderer(std::size_t numBands, std::size_t height)
    : numBands_(numBands), width_(numBands * 4), height_(height) {
    peakHold_.fill(0.0f);
    peakDecay_.fill(0.0f);
}

std::string SpectrumRenderer::render(const std::array<float, 20>& bands) const {
    return renderBars(bands);
}

std::string SpectrumRenderer::renderBars(const std::array<float, 20>& bands) const {
    std::ostringstream oss;

    // Update peak hold with decay
    constexpr float PEAK_DECAY_RATE = 0.95f;  // Decay rate per frame
    for (std::size_t i = 0; i < 20; ++i) {
        // Update peak hold: take current or previous peak
        peakHold_[i] = std::max(bands[i], peakHold_[i] * PEAK_DECAY_RATE);
        peakDecay_[i]--;
    }

    // Render bar chart from top to bottom
    for (std::size_t row = height_; row > 0; --row) {
        float rowThreshold = static_cast<float>(row) / static_cast<float>(height_);

        for (std::size_t band = 0; band < 20; ++band) {
            // Determine bar character based on band level
            if (bands[band] >= rowThreshold) {
                // Full block
                oss << "█";
            } else if (bands[band] >= rowThreshold - 0.25f) {
                // 3/4 block
                oss << "▓";
            } else if (bands[band] >= rowThreshold - 0.5f) {
                // 1/2 block
                oss << "▒";
            } else if (bands[band] >= rowThreshold - 0.75f) {
                // 1/4 block
                oss << "░";
            } else {
                // Empty space
                oss << " ";
            }

            // Add peak marker if peak hold is above this row
            if (peakHold_[band] >= rowThreshold && peakHold_[band] < rowThreshold + 0.05f) {
                // Replace last character with peak marker
                std::string str = oss.str();
                if (!str.empty()) {
                    str.back() = '*';  // Peak marker
                    oss.str("");
                    oss.clear();
                    oss << str;
                }
            }
            
            oss << " ";  // Space between bands
        }

        oss << "\n";
    }

    // Add frequency labels at bottom
    oss << "20Hz";
    oss << std::string(20, ' ');
    oss << "250Hz";
    oss << std::string(15, ' ');
    oss << "4kHz";
    oss << std::string(15, ' ');
    oss << "20kHz\n";

    return oss.str();
}

} // namespace dj
