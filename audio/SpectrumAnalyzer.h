#pragma once

#include <array>
#include <complex>
#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>

namespace dj {

class FFTEngine;  // Forward declaration

class SpectrumAnalyzer {
public:
    static constexpr int NUM_BANDS = 20;

    // Constructor: sampleRate (44100), windowSize (512/1024/2048)
    explicit SpectrumAnalyzer(int sampleRate = 44100, int windowSize = 1024);

    // Destructor for unique_ptr cleanup
    ~SpectrumAnalyzer();

    // Feed audio samples to analyzer (legacy API still supported via setSamples)
    void processSamples(const float* samples, std::size_t count);
    void setSamples(const float* samples, std::size_t count);

    // Get raw FFT spectrum (normalized magnitude, 0.0-1.0)
    // Thread-safe access to full spectrum (N/2+1 bins)
    std::vector<float> getFullSpectrum() const;

    // Get raw FFT spectrum using legacy name
    std::vector<float> getSpectrum() const { return getFullSpectrum(); }

    // Get 20 logarithmic frequency bands (20Hz-20kHz, normalized 0.0-1.0)
    std::array<float, NUM_BANDS> getFrequencyBands() const;

    // Legacy API for backward compatibility
    std::array<float, 20> getBands() const { return getFrequencyBands(); }

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
    std::vector<float> spectrum_;         // Last computed spectrum (legacy)
    std::array<float, NUM_BANDS> frequencyBands_; // Last computed bands

    // FFT integration and thread safety
    std::unique_ptr<FFTEngine> fftEngine_;  // Non-null if window size is power of 2
    std::vector<float> fullSpectrum_;       // Thread-safe N/2+1 bins
    mutable std::mutex spectrumMutex_;      // Protects fullSpectrum_

    // Internal methods
    void initializeWindow();
    void computeSpectrum();
    void aggregateFrequencyBands();
    float hannWindow(float n, float N) const;
};

} // namespace dj
