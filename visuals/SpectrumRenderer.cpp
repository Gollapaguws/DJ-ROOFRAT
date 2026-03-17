#include "visuals/SpectrumRenderer.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace dj {

SpectrumRenderer::SpectrumRenderer(const RenderOptions& options)
    : options_(options), numBands_(20) {
    peakHold_.resize(options_.width, 0.0f);
    legacyPeakHold_.fill(0.0f);
    legacyPeakDecay_.fill(0.0f);
}

SpectrumRenderer::SpectrumRenderer(std::size_t numBands, std::size_t height)
    : options_{static_cast<int>(numBands * 4), static_cast<int>(height),
               ColorScheme::FrequencyBands, true, 0.95f, true, 44100},
      numBands_(numBands) {
    peakHold_.resize(options_.width, 0.0f);
    legacyPeakHold_.fill(0.0f);
    legacyPeakDecay_.fill(0.0f);
}

void SpectrumRenderer::resetPeaks() {
    std::fill(peakHold_.begin(), peakHold_.end(), 0.0f);
    legacyPeakHold_.fill(0.0f);
    legacyPeakDecay_.fill(0.0f);
}

std::vector<float> SpectrumRenderer::binsToBars(
    const std::vector<float>& spectrum, 
    int numBars
) {
    if (spectrum.empty() || numBars <= 0) {
        return std::vector<float>(numBars, 0.0f);
    }

    std::vector<float> bars(numBars, 0.0f);

    // Log-scale mapping: distribute bins across bars logarithmically
    // More bars for low frequencies, fewer for high (matches human perception)

    double minFreq = 20.0;
    double maxFreq = options_.sampleRate / 2.0;  // Nyquist
    double logMin = std::log10(minFreq);
    double logMax = std::log10(maxFreq);

    for (int bar = 0; bar < numBars; ++bar) {
        double logFreq = logMin + (bar / static_cast<double>(numBars)) * (logMax - logMin);
        double freq = std::pow(10.0, logFreq);
        
        // Map frequency to bin index
        // spec size = N/2+1, so freq_per_bin = sampleRate/2 / (N/2+1)
        int bin = static_cast<int>(freq * spectrum.size() * 2 / options_.sampleRate);

        if (bin >= 0 && bin < static_cast<int>(spectrum.size())) {
            bars[bar] = spectrum[bin];
        }
    }

    return bars;
}

std::string SpectrumRenderer::getColorCode(int barIndex, int totalBars) const {
    if (options_.colorScheme == ColorScheme::Monochrome) {
        return "\033[0m";  // Reset/default
    }

    // Frequency-based coloring
    // Bass (0-25%): Red (30Hz-250Hz, kick/bass)
    // Mid (25-75%): Green (250Hz-4kHz, vocals, snare)
    // Treble (75-100%): Blue (4kHz-20kHz, hi-hats, cymbals)

    float position = static_cast<float>(barIndex) / totalBars;

    if (position < 0.25f) {
        return "\033[31m";  // Red
    } else if (position < 0.75f) {
        return "\033[32m";  // Green
    } else {
        return "\033[34m";  // Blue
    }
}

std::string SpectrumRenderer::renderBar(
    float value, 
    int width, 
    const std::string& colorCode
) const {
    int barLength = static_cast<int>(value * width);
    barLength = std::clamp(barLength, 0, width);

    std::string bar = colorCode;
    for (int i = 0; i < barLength; ++i) {
        bar += "█";  // Full block (UTF-8 string literal)
    }
    bar += "\033[0m";  // Reset color

    return bar;
}

std::string SpectrumRenderer::renderFrequencyLabels(int width) const {
    std::ostringstream oss;

    // Labels at key positions: 20Hz, 100Hz, 1kHz, 10kHz, 20kHz
    std::vector<std::pair<float, std::string>> labels = {
        {20.0f, "20Hz"},
        {100.0f, "100Hz"},
        {1000.0f, "1kHz"},
        {10000.0f, "10kHz"},
        {20000.0f, "20kHz"}
    };

    double minFreq = 20.0;
    double maxFreq = options_.sampleRate / 2.0;
    double logMin = std::log10(minFreq);
    double logMax = std::log10(maxFreq);

    std::string labelLine(width, ' ');

    for (const auto& [freq, label] : labels) {
        if (freq > maxFreq) continue;

        double logFreq = std::log10(freq);
        int pos = static_cast<int>(
            (logFreq - logMin) / (logMax - logMin) * width
        );

        // Clamp position to ensure label fits within width
        int maxPos = width - static_cast<int>(label.size());
        pos = std::clamp(pos, 0, maxPos);

        if (pos >= 0 && pos + static_cast<int>(label.size()) <= width) {
            labelLine.replace(pos, label.size(), label);
        }
    }

    oss << labelLine << "\n";
    return oss.str();
}

std::string SpectrumRenderer::render(const std::vector<float>& spectrum) {
    std::ostringstream oss;

    // Convert spectrum bins to display bars
    auto bars = binsToBars(spectrum, options_.width);

    // Update peak hold
    if (options_.showPeakHold) {
        for (size_t i = 0; i < bars.size() && i < peakHold_.size(); ++i) {
            if (bars[i] > peakHold_[i]) {
                peakHold_[i] = bars[i];
            } else {
                peakHold_[i] -= options_.peakDecayRate;
                peakHold_[i] = std::max(0.0f, peakHold_[i]);
            }
        }
    }

    // Render from top to bottom (reversed for visual effect)
    for (int row = options_.height - 1; row >= 0; --row) {
        float threshold = static_cast<float>(row) / options_.height;

        for (size_t col = 0; col < bars.size(); ++col) {
            std::string colorCode = getColorCode(col, bars.size());

            if (bars[col] >= threshold) {
                oss << colorCode << "█\033[0m";
            } else if (options_.showPeakHold && 
                       std::abs(peakHold_[col] - threshold) < 0.05f) {
                oss << colorCode << "▬\033[0m";  // Peak hold marker
            } else {
                oss << " ";
            }
        }
        oss << "\n";
    }

    // Frequency labels
    if (options_.showFrequencyLabels) {
        oss << renderFrequencyLabels(options_.width);
    }

    return oss.str();
}

std::string SpectrumRenderer::renderDualDeck(
    const std::vector<float>& deckASpectrum,
    const std::vector<float>& deckBSpectrum
) {
    std::ostringstream oss;

    int halfWidth = options_.width / 2 - 2;

    auto barsA = binsToBars(deckASpectrum, halfWidth);
    auto barsB = binsToBars(deckBSpectrum, halfWidth);

    // Header line
    oss << "Deck A" << std::string(std::max(0, halfWidth - 6), ' ') 
        << " | " 
        << "Deck B" << "\n";
    
    // Separator line - use character repetition
    for (int i = 0; i < halfWidth; ++i) oss << "─";
    oss << " | ";
    for (int i = 0; i < halfWidth; ++i) oss << "─";
    oss << "\n";

    // Render side-by-side
    for (int row = options_.height - 1; row >= 0; --row) {
        float threshold = static_cast<float>(row) / options_.height;

        // Deck A
        for (size_t col = 0; col < barsA.size(); ++col) {
            if (barsA[col] >= threshold) {
                oss << getColorCode(static_cast<int>(col), static_cast<int>(barsA.size())) << "█\033[0m";
            } else {
                oss << " ";
            }
        }

        oss << " | ";

        // Deck B
        for (size_t col = 0; col < barsB.size(); ++col) {
            if (barsB[col] >= threshold) {
                oss << getColorCode(static_cast<int>(col), static_cast<int>(barsB.size())) << "█\033[0m";
            } else {
                oss << " ";
            }
        }

        oss << "\n";
    }

    return oss.str();
}

// Legacy 20-band rendering for backward compatibility
std::string SpectrumRenderer::render(const std::array<float, 20>& bands) const {
    return renderBars(bands);
}

std::string SpectrumRenderer::renderBars(const std::array<float, 20>& bands) const {
    std::ostringstream oss;

    // Update peak hold with decay
    constexpr float PEAK_DECAY_RATE = 0.95f;  // Decay rate per frame
    for (std::size_t i = 0; i < 20; ++i) {
        // Update peak hold: take current or previous peak
        legacyPeakHold_[i] = std::max(bands[i], legacyPeakHold_[i] * PEAK_DECAY_RATE);
        legacyPeakDecay_[i]--;
    }

    // Render bar chart from top to bottom
    for (std::size_t row = options_.height; row > 0; --row) {
        float rowThreshold = static_cast<float>(row) / static_cast<float>(options_.height);

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
            if (legacyPeakHold_[band] >= rowThreshold && 
                legacyPeakHold_[band] < rowThreshold + 0.05f) {
                // Output peak marker instead (simplified, no character replacement)
                oss << "●";
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
