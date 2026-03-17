#pragma once

#include <array>
#include <complex>
#include <cstddef>
#include <vector>

namespace dj {

class SpectrumAnalyzer {
public:
    // Constructor: sampleRate (44100), windowSize (512/1024/2048)
    explicit SpectrumAnalyzer(int sampleRate = 44100, int windowSize = 1024);

    // Feed audio samples to analyzer
    void setSamples(const float* samples, std::size_t count);

    // Get raw FFT spectrum (normalized magnitude, 0.0-1.0)
    std::vector<float> getSpectrum() const;

    // Get 20 logarithmic frequency bands (20Hz-20kHz, normalized 0.0-1.0)
    std::array<float, 20> getFrequencyBands() const;

    // Get window size
    int getWindowSize() const { return windowSize_; }

    // Get sample rate
    int getSampleRate() const { return sampleRate_; }

private:
    int sampleRate_;
    int windowSize_;
    std::vector<float> window_;           // Hann window coefficients
    std::vector<float> buffer_;           // Circular buffer for samples
    std::size_t bufferPosition_;          // Current write position
    std::vector<float> spectrum_;         // Last computed spectrum
    std::array<float, 20> frequencyBands_; // Last computed bands

    // Internal methods
    void initializeWindow();
    void computeFFT();
    void aggregateFrequencyBands();
    float hannWindow(float n, float N) const;
};

} // namespace dj
