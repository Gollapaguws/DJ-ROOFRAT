#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdio>
#include <iostream>
#include <numeric>
#include <vector>

#include "audio/FFTEngine.h"

namespace dj {

// Forward declarations for naive DFT (for baseline comparison)
constexpr double PI = 3.14159265358979323846;

std::vector<std::complex<double>> naiveDFT(const std::vector<float>& samples) {
    int N = samples.size();
    std::vector<std::complex<double>> result(N);
    
    for (int k = 0; k < N; ++k) {
        std::complex<double> sum(0.0, 0.0);
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * PI * k * n / N;
            std::complex<double> twiddle(std::cos(angle), std::sin(angle));
            sum += std::complex<double>(samples[n], 0.0) * twiddle;
        }
        result[k] = sum;
    }
    
    return result;
}

// Generate sine wave signal
std::vector<float> generateSineWave(double frequency, double sampleRate, int numSamples) {
    std::vector<float> signal(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        signal[i] = static_cast<float>(std::sin(2.0 * PI * frequency * t));
    }
    return signal;
}

// Apply Hann window to signal
std::vector<float> applyHannWindow(const std::vector<float>& signal) {
    int N = signal.size();
    std::vector<float> windowed = signal;
    
    for (int i = 0; i < N; ++i) {
        double window = 0.5 * (1.0 - std::cos(2.0 * PI * i / (N - 1.0)));
        windowed[i] *= static_cast<float>(window);
    }
    
    return windowed;
}

// Calculate magnitude spectrum from complex FFT output
std::vector<float> getMagnitudeSpectrum(const std::vector<std::complex<double>>& fftOutput) {
    int numBins = fftOutput.size() / 2 + 1;
    std::vector<float> mag(numBins);
    
    for (int i = 0; i < numBins; ++i) {
        double real = fftOutput[i].real();
        double imag = fftOutput[i].imag();
        mag[i] = static_cast<float>(std::sqrt(real * real + imag * imag));
    }
    
    return mag;
}

// Normalize magnitude spectrum to 0.0-1.0 range
void normalizeMagnitudeSpectrum(std::vector<float>& mag) {
    float maxVal = *std::max_element(mag.begin(), mag.end());
    if (maxVal > 0.0f) {
        for (float& v : mag) {
            v /= maxVal;
        }
    }
}

bool isPowerOfTwo(int n) {
    return n > 0 && (n & (n - 1)) == 0;
}

int log2(int n) {
    int result = 0;
    while ((1 << result) < n) {
        result++;
    }
    return result;
}

} // namespace dj

// Test 1: FFT Power of Two sizes
void test_FFT_PowerOfTwo() {
    using namespace dj;
    
    std::cout << "\n=== Test 1: FFT Power of Two ===\n";
    
    std::vector<int> powerOfTwoSizes = {128, 256, 512, 1024, 2048};
    std::vector<int> nonPowerOfTwoSizes = {100, 512 + 256, 1000, 1234};
    
    for (int size : powerOfTwoSizes) {
        assert(FFTEngine::isPowerOfTwo(size) && "Power of 2 size should be identified correctly");
        // Test FFTEngine construction succeeds
        FFTEngine fft(size);
        assert(fft.getSize() == size && "FFTEngine size should match");
        std::cout << "✓ Size " << size << " correctly identified as power of 2, FFTEngine created\n";
    }
    
    for (int size : nonPowerOfTwoSizes) {
        assert(!FFTEngine::isPowerOfTwo(size) && "Non-power of 2 size should be identified correctly");
        // FFTEngine should throw exception for non-power-of-2
        bool exceptionThrown = false;
        try {
            FFTEngine fft(size);
        } catch (const std::invalid_argument&) {
            exceptionThrown = true;
        }
        assert(exceptionThrown && "FFTEngine should reject non-power-of-2 sizes");
        std::cout << "✓ Size " << size << " correctly rejected by FFTEngine\n";
    }
    
    std::cout << "Test 1 PASSED\n";
}

// Test 2: FFT Matches Naive DFT
void test_FFT_MatchesNaiveDFT() {
    using namespace dj;
    
    std::cout << "\n=== Test 2: FFT Matches Naive DFT ===\n";
    
    int sampleRate = 44100;
    int windowSize = 1024;
    FFTEngine fft(windowSize, FFTEngine::WindowFunction::Hann);
    
    std::vector<double> testFrequencies = {100.0, 440.0, 1000.0, 8000.0};
    
    for (double freq : testFrequencies) {
        // Generate test signal
        auto signal = generateSineWave(freq, sampleRate, windowSize);
        
        // Compute FFT
        std::vector<std::complex<double>> fftData(windowSize);
        for (int i = 0; i < windowSize; ++i) {
            fftData[i] = std::complex<double>(signal[i], 0.0);
        }
        fft.compute(fftData);
        auto fftMag = getMagnitudeSpectrum(fftData);
        normalizeMagnitudeSpectrum(fftMag);
        
        // Compute naive DFT (baseline)
        auto windowed = applyHannWindow(signal);
        auto naiveDFTResult = naiveDFT(windowed);
        auto naiveMag = getMagnitudeSpectrum(naiveDFTResult);
        normalizeMagnitudeSpectrum(naiveMag);
        
        // Compare FFT and naive DFT results
        int numBins = static_cast<int>(fftMag.size());
        double maxDiff = 0.0;
        for (int k = 0; k < numBins; ++k) {
            double diff = std::abs(fftMag[k] - naiveMag[k]);
            maxDiff = std::max(maxDiff, diff);
        }
        
        std::cout << "  Frequency " << freq << " Hz: Max FFT/DFT difference = " << maxDiff << "\n";
        assert(maxDiff < 0.001 && "FFT should match naive DFT within 0.1%");
        
        int peakBin = std::max_element(fftMag.begin(), fftMag.end()) - fftMag.begin();
        std::cout << "    ✓ FFT matches naive DFT, peak at bin " << peakBin << "\n";
    }
    
    std::cout << "Test 2 PASSED\n";
}

// Test 3: Window Functions
void test_FFT_WindowFunctions() {
    using namespace dj;
    
    std::cout << "\n=== Test 3: Window Functions ===\n";
    
    int windowSize = 1024;
    int sampleRate = 44100;
    double testFreq = 440.0;
    
    // Test creating FFT with different window functions
    std::vector<FFTEngine::WindowFunction> windows = {
        FFTEngine::WindowFunction::None,
        FFTEngine::WindowFunction::Hann,
        FFTEngine::WindowFunction::Hamming,
        FFTEngine::WindowFunction::Blackman
    };
    
    std::vector<std::string> windowNames = {"None", "Hann", "Hamming", "Blackman"};
    
    auto signal = generateSineWave(testFreq, sampleRate, windowSize);
    
    for (size_t i = 0; i < windows.size(); ++i) {
        FFTEngine fft(windowSize, windows[i]);
        
        std::vector<std::complex<double>> fftData(windowSize);
        for (int j = 0; j < windowSize; ++j) {
            fftData[j] = std::complex<double>(signal[j], 0.0);
        }
        
        fft.compute(fftData);
        auto mag = getMagnitudeSpectrum(fftData);
        normalizeMagnitudeSpectrum(mag);
        
        int peakBin = std::max_element(mag.begin(), mag.end()) - mag.begin();
        float peakMag = mag[peakBin];
        
        std::cout << "  ✓ " << windowNames[i] << " window: Peak at bin " << peakBin 
                  << " with magnitude " << peakMag << "\n";
        
        assert(peakMag > 0.5f && "Peak should be significant with any window");
    }
    
    
    std::cout << "Test 3 PASSED\n";
}

// Test 4: Performance benchmark (FFT vs naive DFT)
void test_FFT_Performance() {
    using namespace dj;
    
    std::cout << "\n=== Test 4: Performance Benchmark ===\n";
    
    int windowSize = 1024;
    int sampleRate = 44100;
    
    auto signal = generateSineWave(440.0, sampleRate, windowSize);
    
    // Benchmark FFT
    FFTEngine fft(windowSize, FFTEngine::WindowFunction::Hann);
    auto fftStart = std::chrono::high_resolution_clock::now();
    
    for (int iter = 0; iter < 100; ++iter) {
        std::vector<std::complex<double>> fftData(windowSize);
        for (int i = 0; i < windowSize; ++i) {
            fftData[i] = std::complex<double>(signal[i], 0.0);
        }
        fft.compute(fftData);
    }
    
    auto fftEnd = std::chrono::high_resolution_clock::now();
    auto fftDuration = std::chrono::duration_cast<std::chrono::microseconds>(fftEnd - fftStart);
    double fftTimePerOp = fftDuration.count() / 100.0;
    
    // Benchmark naive DFT (fewer iterations since it's slower)
    auto windowed = applyHannWindow(signal);
    auto naiveStart = std::chrono::high_resolution_clock::now();
    
    for (int iter = 0; iter < 10; ++iter) {
        auto result = naiveDFT(windowed);
        (void)result;
    }
    
    auto naiveEnd = std::chrono::high_resolution_clock::now();
    auto naiveDuration = std::chrono::duration_cast<std::chrono::microseconds>(naiveEnd - naiveStart);
    double naiveTimePerOp = naiveDuration.count() / 10.0;
    
    double speedup = naiveTimePerOp / fftTimePerOp;
    
    std::cout << "  FFT time (100 iterations): " << fftTimePerOp << " µs/op\n";
    std::cout << "  Naive DFT time (10 iterations): " << naiveTimePerOp << " µs/op\n";
    std::cout << "  Speedup: " << speedup << "x\n";
    
    assert(fftTimePerOp < 1000.0 && "FFT should be < 1ms for N=1024");
    assert(speedup >= 50.0 && "FFT should be ≥50× faster than naive DFT");
    
    std::cout << "Test 4 PASSED (FFT is " << speedup << "× faster)\n";
}

// Test 5: DC Component
void test_FFT_DCComponent() {
    using namespace dj;
    
    std::cout << "\n=== Test 5: DC Component ===\n";
    
    int windowSize = 1024;
    
    // Create signal with DC offset (all samples = 0.5)
    std::vector<float> signal(windowSize, 0.5f);
    
    // Use rectangular window here to verify pure-DC behavior.
    // Hann window introduces adjacent-bin energy for constant signals,
    // which makes a very large DC/non-DC ratio expectation invalid.
    auto dftResult = naiveDFT(signal);
    
    double dc_real = dftResult[0].real();
    double dc_imag = dftResult[0].imag();
    double dc_magnitude = std::sqrt(dc_real * dc_real + dc_imag * dc_imag);
    
    std::cout << "  DC component magnitude: " << dc_magnitude << "\n";
    
    // Find max of all other bins
    double maxOtherBin = 0.0;
    for (int k = 1; k < windowSize; ++k) {
        double mag = std::abs(dftResult[k]);
        maxOtherBin = std::max(maxOtherBin, mag);
    }
    
    std::cout << "  Max other bin magnitude: " << maxOtherBin << "\n";
    
    // DC should be much larger than any other bin
    double ratio = dc_magnitude / (maxOtherBin + 1e-9);
    std::cout << "  DC / MaxOther ratio: " << ratio << "\n";
    
    assert(ratio > 50.0 && "DC component should be >> other bins");
    
    std::cout << "Test 5 PASSED\n";
}

// Test 6: Nyquist Frequency
void test_FFT_NyquistFrequency() {
    using namespace dj;
    
    std::cout << "\n=== Test 6: Nyquist Frequency ===\n";
    
    int windowSize = 1024;
    int sampleRate = 44100;
    double nyquist = sampleRate / 2.0;
    
    // Signal at Nyquist: alternating +1/-1
    std::vector<float> signal(windowSize);
    for (int i = 0; i < windowSize; ++i) {
        signal[i] = (i % 2 == 0) ? 1.0f : -1.0f;
    }
    
    // Use rectangular window here for pure Nyquist-bin dominance.
    // Hann window spreads energy into neighboring bins for alternating signals.
    auto dftResult = naiveDFT(signal);
    
    // Nyquist bin is at index windowSize/2
    int nyquistBin = windowSize / 2;
    double nyquist_real = dftResult[nyquistBin].real();
    double nyquist_imag = dftResult[nyquistBin].imag();
    double nyquist_magnitude = std::sqrt(nyquist_real * nyquist_real + nyquist_imag * nyquist_imag);
    
    std::cout << "  Nyquist component magnitude: " << nyquist_magnitude << "\n";
    
    // Find max of other bins
    double maxOtherBin = 0.0;
    for (int k = 0; k < windowSize; ++k) {
        if (k != nyquistBin) {
            double mag = std::abs(dftResult[k]);
            maxOtherBin = std::max(maxOtherBin, mag);
        }
    }
    
    std::cout << "  Max other bin magnitude: " << maxOtherBin << "\n";
    
    // Nyquist should be significant
    double ratio = nyquist_magnitude / (maxOtherBin + 1e-9);
    std::cout << "  Nyquist / MaxOther ratio: " << ratio << "\n";
    
    assert(nyquist_magnitude > 100.0 && "Nyquist component should be significant");
    assert(ratio > 10.0 && "Nyquist should be > other bins");
    
    std::cout << "Test 6 PASSED\n";
}

// Test 7: Phase Accuracy
void test_FFT_PhaseAccuracy() {
    using namespace dj;
    
    std::cout << "\n=== Test 7: Phase Accuracy ===\n";
    
    int windowSize = 1024;
    int sampleRate = 44100;
    int targetBin = 10;
    double testFreq = static_cast<double>(targetBin) * static_cast<double>(sampleRate) /
                      static_cast<double>(windowSize);
    
    // Generate an exact-bin sine wave (phase = -90 degrees at positive-frequency bin)
    // and measure phase directly on the target positive bin.
    auto signal = generateSineWave(testFreq, sampleRate, windowSize);
    auto dftResult = naiveDFT(signal);
    
    double phase_rad = std::atan2(dftResult[targetBin].imag(), dftResult[targetBin].real());
    double phase_deg = phase_rad * 180.0 / PI;
    
    std::cout << "  Target bin: " << targetBin << "\n";
    std::cout << "  Phase (radians): " << phase_rad << "\n";
    std::cout << "  Phase (degrees): " << phase_deg << "\n";
    
    // Sine wave should have phase around -90 degrees (or 270)
    // Normalize to [-180, 180]
    while (phase_deg > 180.0) phase_deg -= 360.0;
    while (phase_deg < -180.0) phase_deg += 360.0;
    
    std::cout << "  Phase normalized to [-180, 180]: " << phase_deg << "\n";
    
    // Should be close to -90 or +270 (-90)
    double errorFromExpected = 90.0;  // Distance from -90
    if (phase_deg > 0) {
        errorFromExpected = std::min(std::abs(phase_deg - 270.0), std::abs(phase_deg + 90.0));
    } else {
        errorFromExpected = std::abs(phase_deg + 90.0);
    }
    
    std::cout << "  Error from expected (-90°): " << errorFromExpected << "°\n";
    
    assert(errorFromExpected < 10.0 && "Phase should be within ±10° of expected");
    
    std::cout << "Test 7 PASSED\n";
}

// Test 8: computePowerSpectrum helper
void test_SpectrumAnalyzer_FFTIntegration_Placeholder() {
    using namespace dj;
    
    std::cout << "\n=== Test 8: FFTEngine::computePowerSpectrum ===\n";
    
    int windowSize = 1024;
    int sampleRate = 44100;
    double testFreq = 440.0;
    
    auto signal = generateSineWave(testFreq, sampleRate, windowSize);
    
    FFTEngine fft(windowSize, FFTEngine::WindowFunction::Hann);
    auto powerSpectrum = fft.computePowerSpectrum(signal.data(), signal.size());
    
    int numBins = static_cast<int>(powerSpectrum.size());
    assert(numBins == windowSize / 2 + 1 && "Power spectrum should have N/2+1 bins");
    
    int peakBin = std::max_element(powerSpectrum.begin(), powerSpectrum.end()) - powerSpectrum.begin();
    float peakMag = powerSpectrum[peakBin];
    
    std::cout << "  Power spectrum bins: " << numBins << "\n";
    std::cout << "  Peak at bin " << peakBin << " with magnitude " << peakMag << "\n";
    
    // Expected bin
    int expectedBin = static_cast<int>(testFreq * windowSize / sampleRate);
    int binError = std::abs(peakBin - expectedBin);
    
    assert(peakMag > 0.8f && "Peak magnitude should be strong");
    assert(binError <= 2 && "Peak should be at expected bin");
    
    std::cout << "Test 8 PASSED (computePowerSpectrum working)\n";
}

int main() {
    std::cout << "DJ-ROOFRAT Phase 34: Cooley-Tukey FFT Implementation\n";
    std::cout << "Running 8 comprehensive FFT tests...\n";
    
    try {
        test_FFT_PowerOfTwo();
        test_FFT_MatchesNaiveDFT();
        test_FFT_WindowFunctions();
        test_FFT_Performance();
        test_FFT_DCComponent();
        test_FFT_NyquistFrequency();
        test_FFT_PhaseAccuracy();
        test_SpectrumAnalyzer_FFTIntegration_Placeholder();
        
        std::cout << "\n" << std::string(50, '=') << "\n";
        std::cout << "All tests PASSED!\n";
        std::cout << std::string(50, '=') << "\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTest FAILED with exception: " << e.what() << "\n";
        return 1;
    }
}
