#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace dj {

class SpectrumRenderer {
public:
    enum class ColorScheme {
        Monochrome,         // Single color (white/default)
        FrequencyBands,     // Bass=red, Mid=green, Treble=blue
        Gradient            // Smooth color gradient across spectrum
    };

    struct RenderOptions {
        int width = 80;
        int height = 10;
        ColorScheme colorScheme = ColorScheme::FrequencyBands;
        bool showPeakHold = true;
        float peakDecayRate = 0.002f;  // Decay per frame (500ms @ 60 FPS)
        bool showFrequencyLabels = true;
        int sampleRate = 44100;
    };

    // Constructor with options or legacy band-based initialization
    explicit SpectrumRenderer(const RenderOptions& options = RenderOptions{});
    explicit SpectrumRenderer(std::size_t numBands, std::size_t height = 10);

    // Render full spectrum (N/2+1 bins) to ASCII art
    std::string render(const std::vector<float>& spectrum);

    // Legacy: Render 20-band spectrum (backward compatible)
    std::string render(const std::array<float, 20>& bands) const;

    // Render dual-deck comparison (split-screen)
    std::string renderDualDeck(
        const std::vector<float>& deckASpectrum,
        const std::vector<float>& deckBSpectrum
    );

    // Set display width (for terminal)
    void setWidth(std::size_t width) { options_.width = static_cast<int>(width); }

    // Set display height (for terminal)
    void setHeight(std::size_t height) { options_.height = static_cast<int>(height); }

    // Get current width
    std::size_t getWidth() const { return options_.width; }

    // Get current height
    std::size_t getHeight() const { return options_.height; }

    // Reset peak hold values
    void resetPeaks();

private:
    RenderOptions options_;
    std::vector<float> peakHold_;  // Peak hold values for display bars
    
    // Legacy support
    std::size_t numBands_;
    mutable std::array<float, 20> legacyPeakHold_;
    mutable std::array<float, 20> legacyPeakDecay_;

    // Helper: map spectrum bins to display bars
    std::vector<float> binsToBars(const std::vector<float>& spectrum, int numBars);

    // Helper: get ANSI color code for frequency band
    std::string getColorCode(int barIndex, int totalBars) const;

    // Helper: render single horizontal bar with color
    std::string renderBar(float value, int width, const std::string& colorCode) const;

    // Helper: generate frequency labels (20Hz, 100Hz, 1kHz, 10kHz, 20kHz)
    std::string renderFrequencyLabels(int width) const;
    
    // Legacy helper: render bars from 20-band spectrum
    std::string renderBars(const std::array<float, 20>& bands) const;
};

} // namespace dj
