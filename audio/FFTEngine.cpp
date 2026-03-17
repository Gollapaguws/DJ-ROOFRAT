#include "audio/FFTEngine.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace dj {

constexpr double PI = 3.14159265358979323846;

FFTEngine::FFTEngine(int size, WindowFunction window)
    : size_(size), window_(window) {
    // Validate that size is power of 2
    if (!isPowerOfTwo(size)) {
        throw std::invalid_argument("FFT size must be a power of 2");
    }

    // Initialize internal data structures
    initializeWindow();
    initializeBitReversal();
    initializeTwiddleFactors();
}

bool FFTEngine::isPowerOfTwo(int n) {
    return n > 0 && (n & (n - 1)) == 0;
}

void FFTEngine::initializeWindow() {
    windowCoefficients_.resize(size_);

    for (int i = 0; i < size_; ++i) {
        double n = static_cast<double>(i);
        double N = static_cast<double>(size_ - 1);

        switch (window_) {
            case WindowFunction::None:
                windowCoefficients_[i] = 1.0;
                break;

            case WindowFunction::Hann:
                windowCoefficients_[i] =
                    0.5 * (1.0 - std::cos(2.0 * PI * n / N));
                break;

            case WindowFunction::Hamming:
                windowCoefficients_[i] =
                    0.54 - 0.46 * std::cos(2.0 * PI * n / N);
                break;

            case WindowFunction::Blackman:
                windowCoefficients_[i] = 0.42 -
                    0.5 * std::cos(2.0 * PI * n / N) +
                    0.08 * std::cos(4.0 * PI * n / N);
                break;
        }
    }
}

void FFTEngine::initializeBitReversal() {
    bitReversalIndices_.resize(size_);
    int numBits = static_cast<int>(std::log2(size_));

    for (int i = 0; i < size_; ++i) {
        int reversed = 0;
        for (int bit = 0; bit < numBits; ++bit) {
            if (i & (1 << bit)) {
                reversed |= (1 << (numBits - 1 - bit));
            }
        }
        bitReversalIndices_[i] = reversed;
    }
}

void FFTEngine::initializeTwiddleFactors() {
    // Pre-compute twiddle factors for all stages
    // We compute them on-the-fly during the algorithm instead
    // to save memory, but cache them for one-time computation
    // This is optional optimization - for now we compute during FFT
}

void FFTEngine::applyBitReversal(std::vector<std::complex<double>>& data) {
    for (int i = 0; i < size_; ++i) {
        int j = bitReversalIndices_[i];
        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }
}

void FFTEngine::applyWindow(std::vector<std::complex<double>>& data) {
    for (int i = 0; i < size_; ++i) {
        data[i] *= windowCoefficients_[i];
    }
}

void FFTEngine::computeFFTInPlace(std::vector<std::complex<double>>& data) {
    int numStages = static_cast<int>(std::log2(size_));

    // Process each stage of the FFT
    for (int stage = 0; stage < numStages; ++stage) {
        int m = 1 << (stage + 1);  // Butterfly span: 2^(stage+1)
        int halfM = m >> 1;        // Half span: 2^stage

        // Twiddle factor for this stage: e^(-i*2π/m)
        std::complex<double> wm =
            std::exp(std::complex<double>(0.0, -2.0 * PI / m));

        // Process all butterflies at this stage
        for (int k = 0; k < size_; k += m) {
            std::complex<double> w(1.0, 0.0);

            for (int j = 0; j < halfM; ++j) {
                // Read operands
                std::complex<double> u = data[k + j];
                std::complex<double> t = w * data[k + j + halfM];

                // Butterfly: add and subtract
                data[k + j] = u + t;
                data[k + j + halfM] = u - t;

                // Update twiddle factor
                w *= wm;
            }
        }
    }
}

void FFTEngine::compute(std::vector<std::complex<double>>& data) {
    // Validate input
    if (static_cast<int>(data.size()) != size_) {
        throw std::invalid_argument(
            "Input data size does not match FFT size");
    }

    // Step 1: Apply window function to input BEFORE bit-reversal
    if (window_ != WindowFunction::None) {
        applyWindow(data);
    }

    // Step 2: Bit-reversal permutation
    applyBitReversal(data);

    // Step 3: Perform iterative FFT with butterfly operations
    computeFFTInPlace(data);
}

std::vector<float> FFTEngine::computePowerSpectrum(const float* samples,
                                                    size_t count) {
    if (!samples || count == 0) {
        return std::vector<float>();
    }

    // Prepare input: convert to complex, apply window
    std::vector<std::complex<double>> fftInput(size_);

    for (int i = 0; i < size_; ++i) {
        if (i < static_cast<int>(count)) {
            fftInput[i] = std::complex<double>(samples[i], 0.0);
        } else {
            fftInput[i] = std::complex<double>(0.0, 0.0);
        }
    }

    // Perform FFT
    compute(fftInput);

    // Extract power spectrum (magnitude)
    // Only compute positive frequencies (0 to Nyquist)
    int numBins = size_ / 2 + 1;
    std::vector<float> powerSpectrum(numBins);

    double maxMagnitude = 0.0;

    for (int i = 0; i < numBins; ++i) {
        double real = fftInput[i].real();
        double imag = fftInput[i].imag();
        double magnitude = std::sqrt(real * real + imag * imag);

        // Normalize by window sum to account for energy loss from windowing
        // For Hann window, divide by size_/2 (approximately)
        magnitude /= (size_ / 2.0);

        powerSpectrum[i] = static_cast<float>(magnitude);
        maxMagnitude = std::max(maxMagnitude, static_cast<double>(powerSpectrum[i]));
    }

    // Normalize to 0.0-1.0 range
    if (maxMagnitude > 0.0) {
        for (float& value : powerSpectrum) {
            value /= static_cast<float>(maxMagnitude);
        }
    }

    return powerSpectrum;
}

} // namespace dj
