#pragma once

#include <complex>
#include <vector>

namespace dj {

/**
 * @brief Cooley-Tukey radix-2 FFT engine with selectable windowing.
 *
 * Performs in-place Fast Fourier Transform on complex data with optional windowing
 * to reduce spectral leakage. Supports power-of-two sizes only.
 */
class FFTEngine {
public:
    /**
     * @brief Window function to apply before FFT to reduce spectral leakage.
     */
    enum class WindowFunction {
        None,      // Rectangular (no windowing)
        Hann,      // 0.5 * (1 - cos(2πn/(N-1)))
        Hamming,   // 0.54 - 0.46 * cos(2πn/(N-1))
        Blackman   // 0.42 - 0.5*cos(2πn/(N-1)) + 0.08*cos(4πn/(N-1))
    };

    // Constructor: size must be power of 2
    // Window defaults to Hann for spectral analysis
    /**
     * @brief Construct FFT engine.
     * @param size FFT size in samples (must be power of 2).
     * @param window Window function to apply (default Hann).
     */
    explicit FFTEngine(int size, WindowFunction window = WindowFunction::Hann);

    // Destructor
    /**
     * @brief Destructor.
     */
    ~FFTEngine() = default;

    // Perform in-place FFT on complex data
    // Input: vector of complex<double> samples (size must match FFT size)
    /**
     * @brief Perform in-place FFT on complex data.
     * @param data Complex input/output vector (size must equal FFT size).
     */
    void compute(std::vector<std::complex<double>>& data);

    // Helper: convert real samples to power spectrum
    // Input: array of real samples, count of samples
    // Output: power spectrum (0 to Nyquist, normalized magnitude)
    /**
     * @brief Compute normalized power spectrum from real samples.
     * @param samples Float input samples (real-valued time domain).
     * @param count Number of samples.
     * @return Power spectrum from 0 to Nyquist, normalized magnitudes [0.0–1.0].
     */
    std::vector<float> computePowerSpectrum(const float* samples, size_t count);

    // Get FFT size
    /**
     * @brief Get FFT size.
     * @return Size in samples (power of 2).
     */
    int getSize() const { return size_; }

    // Check if integer is power of 2
    /**
     * @brief Check if n is a power of two.
     * @param n Integer to check.
     * @return true if n is a power of two, false otherwise.
     */
    static bool isPowerOfTwo(int n);

private:
    int size_;
    WindowFunction window_;
    std::vector<double> windowCoefficients_;       // Window function values
    std::vector<int> bitReversalIndices_;          // Bit-reversal permutation
    std::vector<std::complex<double>> twiddleFactors_;  // Pre-computed twiddle factors

    // Initialization methods
    void initializeWindow();
    void initializeBitReversal();
    void initializeTwiddleFactors();

    // Core FFT operations
    void applyBitReversal(std::vector<std::complex<double>>& data);
    void applyWindow(std::vector<std::complex<double>>& data);
    void computeFFTInPlace(std::vector<std::complex<double>>& data);
};

} // namespace dj
