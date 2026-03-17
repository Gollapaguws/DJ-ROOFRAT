#pragma once

#include <complex>
#include <vector>

namespace dj {

class FFTEngine {
public:
    enum class WindowFunction {
        None,      // Rectangular (no windowing)
        Hann,      // 0.5 * (1 - cos(2πn/(N-1)))
        Hamming,   // 0.54 - 0.46 * cos(2πn/(N-1))
        Blackman   // 0.42 - 0.5*cos(2πn/(N-1)) + 0.08*cos(4πn/(N-1))
    };

    // Constructor: size must be power of 2
    // Window defaults to Hann for spectral analysis
    explicit FFTEngine(int size, WindowFunction window = WindowFunction::Hann);

    // Destructor
    ~FFTEngine() = default;

    // Perform in-place FFT on complex data
    // Input: vector of complex<double> samples (size must match FFT size)
    void compute(std::vector<std::complex<double>>& data);

    // Helper: convert real samples to power spectrum
    // Input: array of real samples, count of samples
    // Output: power spectrum (0 to Nyquist, normalized magnitude)
    std::vector<float> computePowerSpectrum(const float* samples, size_t count);

    // Get FFT size
    int getSize() const { return size_; }

    // Check if integer is power of 2
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
