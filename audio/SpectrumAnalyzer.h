#pragma once

#include <array>
#include <complex>
#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>

namespace dj {

class FFTEngine;  // Forward declaration

/**
 * @brief Real-time FFT-based spectrum analyzer with 20 logarithmic bands.
 *
 * Processes incoming audio samples and computes normalized frequency spectrum
 * and logarithmic band energies for visualization and analysis.
 */
class SpectrumAnalyzer {
public:
    static constexpr int NUM_BANDS = 20;

    // Constructor: sampleRate (44100), windowSize (512/1024/2048)
    /**
     * @brief Construct analyzer.
     * @param sampleRate Audio sample rate in Hz (default 44100).
     * @param windowSize FFT window size in samples (512/1024/2048, default 1024).
     */
    explicit SpectrumAnalyzer(int sampleRate = 44100, int windowSize = 1024);

    // Destructor for unique_ptr cleanup
    /**
     * @brief Destructor for unique_ptr cleanup.
     */
    ~SpectrumAnalyzer();

    // Feed audio samples to analyzer (legacy API still supported via setSamples)
    /**
     * @brief Feed audio samples and update spectrum.
     * @param samples Pointer to float samples [-1.0, 1.0].
     * @param count Number of samples to process.
     */
    void processSamples(const float* samples, std::size_t count);
    /**
     * @brief Set audio samples (legacy API name).
     * @param samples Pointer to float samples.
     * @param count Number of samples.
     */
    void setSamples(const float* samples, std::size_t count);

    // Get raw FFT spectrum (normalized magnitude, 0.0-1.0)
    // Thread-safe access to full spectrum (N/2+1 bins)
    /**
     * @brief Get N/2+1 FFT magnitude bins.
     * @return Normalized magnitudes [0.0–1.0], thread-safe, covering 0 to Nyquist frequency.
     */
    std::vector<float> getFullSpectrum() const;

    // Get raw FFT spectrum using legacy name
    /**
     * @brief Get raw FFT spectrum using legacy name.
     * @return Full spectrum (N/2+1 bins).
     */
    std::vector<float> getSpectrum() const { return getFullSpectrum(); }

    // Get 20 logarithmic frequency bands (20Hz-20kHz, normalized 0.0-1.0)
    /**
     * @brief Get 20 log-spaced frequency bands (20Hz–20kHz).
     * @return Array of 20 band energies [0.0–1.0], thread-safe.
     */
    std::array<float, NUM_BANDS> getFrequencyBands() const;

    // Legacy API for backward compatibility
    /**
     * @brief Get 20 logarithmic frequency bands using legacy name.
     * @return Array of 20 band energies.
     */
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
