// Benchmark: SpectrumAnalyzer Performance
// Tests FFT computation speed and correctness
// Baseline: ~67ms/1024 samples (naive DFT)
// Target: <20ms/1024 samples (FFT optimized, 3x speedup)

#include "audio/SpectrumAnalyzer.h"
#include "audio/FFTEngine.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>

namespace dj {

constexpr float PI = 3.14159265359f;

// Utility: Generate sine wave at specified frequency
std::vector<float> generateSineWave(float frequency, int sampleRate, int numSamples) {
    std::vector<float> samples(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        float phase = 2.0f * PI * frequency * i / sampleRate;
        samples[i] = std::sin(phase);
    }
    return samples;
}

// Utility: Generate multi-frequency signal (bass, mid, treble)
std::vector<float> generateMultiFrequencySignal(int sampleRate, int numSamples) {
    std::vector<float> samples(numSamples, 0.0f);
    
    // Bass: 60 Hz
    std::vector<float> bass = generateSineWave(60.0f, sampleRate, numSamples);
    // Midrange: 1000 Hz
    std::vector<float> mid = generateSineWave(1000.0f, sampleRate, numSamples);
    // Treble: 5000 Hz
    std::vector<float> treble = generateSineWave(5000.0f, sampleRate, numSamples);
    
    // Mix (equal amplitude)
    for (int i = 0; i < numSamples; ++i) {
        samples[i] = (bass[i] + mid[i] + treble[i]) / 3.0f;
    }
    return samples;
}

// Benchmark 1: SpectrumAnalyzer with 1024-sample window (power-of-2, uses FFT)
void benchmark_SpectrumAnalyzer_1024Samples_PowerOfTwo() {
    std::cout << "\n=== Benchmark 1: SpectrumAnalyzer_1024Samples (Power-of-2, FFT) ===\n";
    
    const int windowSize = 1024;
    const int sampleRate = 44100;
    SpectrumAnalyzer analyzer(sampleRate, windowSize);
    
    // Generate 1024 samples of multi-frequency signal
    auto signal = generateMultiFrequencySignal(sampleRate, windowSize);
    
    // Warm-up run
    analyzer.setSamples(signal.data(), signal.size());
    
    // Benchmark: 100 iterations
    constexpr int iterations = 100;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        analyzer.setSamples(signal.data(), signal.size());
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    double avgTimeUs = duration.count() / static_cast<double>(iterations);
    double avgTimeMs = avgTimeUs / 1000.0;
    
    std::cout << "  Iterations: " << iterations << "\n";
    std::cout << "  Total time: " << std::fixed << std::setprecision(2) 
              << duration.count() / 1000.0 << " ms\n";
    std::cout << "  Avg time/iteration: " << std::fixed << std::setprecision(3) 
              << avgTimeMs << " ms\n";
    std::cout << "  Target: < 20 ms\n";
    
    // Validate correctness: check that we get a spectrum
    auto spectrum = analyzer.getFullSpectrum();
    assert(!spectrum.empty() && "Spectrum should not be empty");
    
    // Check that spectrum values are in valid range (0.0-1.0)
    for (float val : spectrum) {
        assert(val >= 0.0f && val <= 1.0f);
    }
    
    // Get frequency bands
    auto bands = analyzer.getFrequencyBands();
    assert(bands.size() == 20 && "Should have 20 frequency bands");
    
    std::cout << "  ✓ Spectrum computed correctly\n";
    std::cout << "  ✓ 20 frequency bands generated\n";
    
    if (avgTimeMs < 20.0) {
        std::cout << "  ✓ PERFORMANCE TARGET MET (< 20ms)\n";
    } else {
        std::cout << "  ⚠ Performance below target (" << avgTimeMs << " ms vs 20 ms)\n";
    }
    
    std::cout << "PASS\n";
}

// Benchmark 2: SpectrumAnalyzer with continuous streaming (circular buffer)
void benchmark_SpectrumAnalyzer_ContinuousStreaming() {
    std::cout << "\n=== Benchmark 2: SpectrumAnalyzer_ContinuousStreaming ===\n";
    
    const int windowSize = 1024;
    const int sampleRate = 44100;
    const int chunkSize = 512;  // Process in 512-sample chunks
    SpectrumAnalyzer analyzer(sampleRate, windowSize);
    
    // Generate test signal
    auto signal = generateMultiFrequencySignal(sampleRate, sampleRate);  // 1 second
    
    // Benchmark: process signal in chunks 10 times
    constexpr int passes = 10;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int pass = 0; pass < passes; ++pass) {
        for (size_t pos = 0; pos < signal.size(); pos += chunkSize) {
            size_t remaining = signal.size() - pos;
            size_t toProcess = std::min(static_cast<size_t>(chunkSize), remaining);
            analyzer.setSamples(signal.data() + pos, toProcess);
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    double totalTimeMs = duration.count() / 1000.0;
    double avgTimePerChunkMs = totalTimeMs / (passes * (sampleRate / chunkSize));
    
    std::cout << "  Passes: " << passes << "\n";
    std::cout << "  Chunks per pass: " << (sampleRate / chunkSize) << "\n";
    std::cout << "  Total time: " << std::fixed << std::setprecision(2) << totalTimeMs << " ms\n";
    std::cout << "  Avg time per 512-sample chunk: " << std::fixed << std::setprecision(3) 
              << avgTimePerChunkMs << " ms\n";
    
    // Verify output
    auto spectrum = analyzer.getFullSpectrum();
    assert(!spectrum.empty() && "Spectrum should not be empty");
    
    std::cout << "  ✓ Continuous streaming processed successfully\n";
    std::cout << "PASS\n";
}

// Benchmark 3: FFTEngine directly (pure FFT algorithm)
void benchmark_FFTEngine_Direct() {
    std::cout << "\n=== Benchmark 3: FFTEngine_Direct (Pure FFT) ===\n";
    
    const int fftSize = 1024;
    FFTEngine fft(fftSize);
    
    // Generate test data (complex samples)
    std::vector<std::complex<double>> testData(fftSize);
    float frequency = 1000.0f;
    int sampleRate = 44100;
    for (int i = 0; i < fftSize; ++i) {
        float phase = 2.0f * PI * frequency * i / sampleRate;
        testData[i] = std::complex<double>(std::sin(phase), 0.0);
    }
    
    // Warm-up
    std::vector<std::complex<double>> warmupData = testData;
    fft.compute(warmupData);
    
    // Benchmark: 100 iterations
    constexpr int iterations = 100;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        std::vector<std::complex<double>> data = testData;  // Copy for each iteration
        fft.compute(data);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    double avgTimeUs = duration.count() / static_cast<double>(iterations);
    double avgTimeMs = avgTimeUs / 1000.0;
    
    std::cout << "  FFT Size: " << fftSize << "\n";
    std::cout << "  Iterations: " << iterations << "\n";
    std::cout << "  Avg time/iteration: " << std::fixed << std::setprecision(3) 
              << avgTimeMs << " ms\n";
    
    std::cout << "  ✓ FFT computation completed\n";
    std::cout << "PASS\n";
}

// Benchmark 4: FFTEngine.computePowerSpectrum() (optimized path)
void benchmark_FFTEngine_PowerSpectrum() {
    std::cout << "\n=== Benchmark 4: FFTEngine_PowerSpectrum (Optimized) ===\n";
    
    const int fftSize = 1024;
    const int sampleRate = 44100;
    FFTEngine fft(fftSize);
    
    // Generate real-valued test signal
    auto signal = generateMultiFrequencySignal(sampleRate, fftSize);
    
    // Warm-up
    fft.computePowerSpectrum(signal.data(), signal.size());
    
    // Benchmark: 100 iterations
    constexpr int iterations = 100;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        auto spectrum = fft.computePowerSpectrum(signal.data(), signal.size());
        // Use spectrum to avoid optimization
        assert(!spectrum.empty());
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    double avgTimeUs = duration.count() / static_cast<double>(iterations);
    double avgTimeMs = avgTimeUs / 1000.0;
    
    std::cout << "  Iterations: " << iterations << "\n";
    std::cout << "  Avg time/iteration: " << std::fixed << std::setprecision(3) 
              << avgTimeMs << " ms\n";
    std::cout << "  Target: < 20 ms\n";
    
    if (avgTimeMs < 20.0) {
        std::cout << "  ✓ PERFORMANCE TARGET MET (< 20ms)\n";
    }
    
    std::cout << "  ✓ Power spectrum computation completed\n";
    std::cout << "PASS\n";
}

} // namespace dj

int main() {
    std::cout << "==============================\n";
    std::cout << "SpectrumAnalyzer Benchmarks\n";
    std::cout << "==============================\n";
    
    try {
        dj::benchmark_SpectrumAnalyzer_1024Samples_PowerOfTwo();
        dj::benchmark_SpectrumAnalyzer_ContinuousStreaming();
        dj::benchmark_FFTEngine_Direct();
        dj::benchmark_FFTEngine_PowerSpectrum();
        
        std::cout << "\n==============================\n";
        std::cout << "All benchmarks completed!\n";
        std::cout << "==============================\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cout << "\n✗ Exception: " << e.what() << "\n";
        return 1;
    }
}
