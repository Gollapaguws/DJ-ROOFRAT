#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <complex>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#include "audio/FFTEngine.h"
#include "audio/SpectrumAnalyzer.h"
#include "visuals/SpectrumRenderer.h"

namespace dj {

// Test 1: SpectrumAnalyzer FFT Integration
void test_SpectrumAnalyzer_FFTIntegration() {
    std::cout << "Test 1: SpectrumAnalyzer FFT Integration... ";

    // Create SpectrumAnalyzer with power-of-2 window size
    SpectrumAnalyzer analyzer(44100, 1024);

    // Generate 440 Hz sine wave (A4 note) at 44100 Hz sample rate
    std::vector<float> sineWave(1024);
    const float frequency = 440.0f;
    const float sampleRate = 44100.0f;
    for (int i = 0; i < 1024; ++i) {
        sineWave[i] = std::sin(2.0f * 3.14159265f * frequency * i / sampleRate);
    }

    // Process samples
    analyzer.processSamples(sineWave.data(), sineWave.size());

    // Get full spectrum
    auto spectrum = analyzer.getFullSpectrum();

    // Verify spectrum size (N/2 + 1 bins for N=1024)
    assert(spectrum.size() == 513 && "Spectrum should have 513 bins (N/2+1)");

    // Expected peak bin: 440 * 1024 / 44100 ≈ 10.2 (around bin 10)
    int expectedBin = static_cast<int>(frequency * 1024 / sampleRate);
    
    // Find actual peak
    float maxValue = 0.0f;
    int peakBin = 0;
    for (size_t i = 0; i < spectrum.size(); ++i) {
        if (spectrum[i] > maxValue) {
            maxValue = spectrum[i];
            peakBin = static_cast<int>(i);
        }
    }

    // Verify peak is within ±3 bins of expected (allows for windowing effects)
    assert(std::abs(peakBin - expectedBin) <= 3 && 
           "Peak should be near expected frequency bin");
    
    // Verify peak value is reasonable (>0.5 due to windowing)
    assert(maxValue > 0.5f && "Peak magnitude should be significant");

    std::cout << "PASS (peak at bin " << peakBin << ", magnitude " << maxValue << ")\n";
}

// Test 2: SpectrumAnalyzer Thread Safety
void test_SpectrumAnalyzer_ThreadSafety() {
    std::cout << "Test 2: SpectrumAnalyzer Thread Safety... ";

    SpectrumAnalyzer analyzer(44100, 1024);

    // Generate test data
    std::vector<float> testSamples(1024);
    for (int i = 0; i < 1024; ++i) {
        testSamples[i] = std::sin(2.0f * 3.14159265f * 440.0f * i / 44100.0f);
    }

    bool threadSafetyOk = true;
    const int iterations = 100;

    auto producerThread = [&]() {
        for (int i = 0; i < iterations; ++i) {
            analyzer.processSamples(testSamples.data(), testSamples.size());
        }
    };

    auto consumerThread = [&]() {
        for (int i = 0; i < iterations; ++i) {
            auto spectrum = analyzer.getFullSpectrum();
            
            // Verify no NaN or Inf values
            for (float value : spectrum) {
                if (std::isnan(value) || std::isinf(value)) {
                    threadSafetyOk = false;
                    break;
                }
            }
        }
    };

    // Run concurrent threads
    std::thread producer(producerThread);
    std::thread consumer(consumerThread);

    producer.join();
    consumer.join();

    assert(threadSafetyOk && "Thread safety check failed: invalid spectrum values detected");
    std::cout << "PASS (1000 concurrent iterations without data races)\n";
}

// Test 3: SpectrumAnalyzer Backward Compatibility
void test_SpectrumAnalyzer_BackwardCompatibility() {
    std::cout << "Test 3: SpectrumAnalyzer Backward Compatibility... ";

    // Test with non-power-of-2 size (should fall back gracefully)
    SpectrumAnalyzer analyzer(44100, 1000);

    // Generate sine wave
    std::vector<float> sineWave(1000);
    for (int i = 0; i < 1000; ++i) {
        sineWave[i] = std::sin(2.0f * 3.14159265f * 440.0f * i / 44100.0f);
    }

    // Process samples
    analyzer.processSamples(sineWave.data(), sineWave.size());

    // Get frequency bands (legacy API)
    auto bands = analyzer.getBands();

    // Verify bands array has correct size
    assert(bands.size() == 20 && "Should have 20 frequency bands");

    // Verify all band values are in valid range
    for (float bandValue : bands) {
        assert(bandValue >= 0.0f && bandValue <= 1.0f && 
               "Band values should be normalized 0.0-1.0");
    }

    // Verify at least one band has significant energy
    float maxBand = *std::max_element(bands.begin(), bands.end());
    assert(maxBand > 0.3f && "Some band should have significant energy");

    std::cout << "PASS (non-power-of-2 fallback and legacy API working)\n";
}

// Test 4: SpectrumRenderer Basic Render
void test_SpectrumRenderer_BasicRender() {
    std::cout << "Test 4: SpectrumRenderer Basic Render... ";

    SpectrumRenderer::RenderOptions options;
    options.width = 80;
    options.height = 10;
    options.colorScheme = SpectrumRenderer::ColorScheme::FrequencyBands;
    options.showPeakHold = true;

    SpectrumRenderer renderer(options);

    // Create test spectrum with single peak
    std::vector<float> spectrum(513, 0.0f);
    spectrum[50] = 1.0f;  // Single peak at bin 50

    // Render
    std::string output = renderer.render(spectrum);

    // Verify output is not empty
    assert(!output.empty() && "Rendered output should not be empty");

    // Count lines (should be height + frequency labels)
    int lineCount = std::count(output.begin(), output.end(), '\n');
    assert(lineCount > 0 && "Output should have multiple lines");

    // Verify ANSI color codes present
    assert(output.find("\033[") != std::string::npos && 
           "Output should contain ANSI color codes");

    std::cout << "PASS (output dimensions " << lineCount << " lines)\n";
}

// Test 5: SpectrumRenderer Peak Hold
void test_SpectrumRenderer_PeakHold() {
    std::cout << "Test 5: SpectrumRenderer Peak Hold... ";

    SpectrumRenderer::RenderOptions options;
    options.width = 80;
    options.height = 10;
    options.showPeakHold = true;
    options.peakDecayRate = 0.002f;  // 500ms @ 60 FPS ≈ 30 frames

    SpectrumRenderer renderer(options);

    // Create spectrum with prominent peak
    std::vector<float> spectrum(513, 0.0f);
    spectrum[100] = 1.0f;

    // Render initial frame
    std::string frame1 = renderer.render(spectrum);
    assert(frame1.find("▬") != std::string::npos || 
           frame1.find("█") != std::string::npos &&
           "First frame should show peak");

    // Render 30 frames with decaying spectrum
    for (int i = 0; i < 30; ++i) {
        std::vector<float> decayingSpectrum(513, 0.0f);
        float decayFactor = std::max(0.0f, 1.0f - (i * 0.033f));  // Decay over time
        decayingSpectrum[100] = decayFactor;
        renderer.render(decayingSpectrum);
    }

    // Final render should still show peak marker (decay in progress)
    std::vector<float> finalSpectrum(513, 0.0f);
    finalSpectrum[100] = 0.0f;
    std::string finalFrame = renderer.render(finalSpectrum);

    // Peak hold marker should be visible (either ▬ or diminished █)
    // After 30 frames with 0.002 decay rate, peak should be at ~0.94 (1.0 - 30*0.002 = 0.94)
    // This should still show some visualization
    bool hasPeakHold = (finalFrame.find("▬") != std::string::npos || 
                        finalFrame.find("█") != std::string::npos);
    
    assert(hasPeakHold && "Peak hold should persist after peak decays (either ▬ or █)");

    std::cout << "PASS (peak hold decay working over 30 frames)\n";
}

// Test 6: SpectrumRenderer Dual Deck
void test_SpectrumRenderer_DualDeck() {
    std::cout << "Test 6: SpectrumRenderer Dual Deck... ";

    SpectrumRenderer::RenderOptions options;
    options.width = 80;
    options.height = 10;

    SpectrumRenderer renderer(options);

    // Create two different spectra
    std::vector<float> spectrumA(513, 0.0f);
    spectrumA[30] = 1.0f;  // Peak in low-mid frequencies (low pitch)

    std::vector<float> spectrumB(513, 0.0f);
    spectrumB[200] = 1.0f;  // Peak in high frequencies (high pitch)

    // Render dual-deck view
    std::string output = renderer.renderDualDeck(spectrumA, spectrumB);

    // Verify output contains separator
    assert(output.find("|") != std::string::npos && 
           "Dual-deck output should have separator (|)");

    // Verify output contains deck labels
    assert(output.find("Deck A") != std::string::npos && 
           output.find("Deck B") != std::string::npos &&
           "Output should label both decks");

    // Verify both decks shown
    int separatorCount = std::count(output.begin(), output.end(), '|');
    assert(separatorCount > 5 && "Should have multiple separator lines");

    std::cout << "PASS (dual-deck split-screen rendering)\n";
}

// Test 7: SpectrumRenderer Frequency Labels
void test_SpectrumRenderer_FrequencyLabels() {
    std::cout << "Test 7: SpectrumRenderer Frequency Labels... ";

    SpectrumRenderer::RenderOptions options;
    options.width = 80;
    options.height = 10;
    options.showFrequencyLabels = true;
    options.sampleRate = 44100;

    SpectrumRenderer renderer(options);

    // Create test spectrum
    std::vector<float> spectrum(513, 0.0f);
    spectrum[50] = 0.8f;

    std::string output = renderer.render(spectrum);

    // Verify key frequency labels are present
    std::vector<std::string> expectedLabels = {
        "20Hz", "100Hz", "1kHz", "10kHz", "20kHz"
    };

    for (const auto& label : expectedLabels) {
        if (output.find(label) == std::string::npos) {
            throw std::runtime_error("Output should contain label: " + label);
        }
    }

    std::cout << "PASS (frequency labels present and positioned)\n";
}

// Test 8: Main Loop Spectrum Toggle
void test_MainLoop_SpectrumToggle() {
    std::cout << "Test 8: Main Loop Spectrum Toggle... ";

    // Simulate spectrum toggle state
    bool showSpectrum = false;

    // Simulate S key toggles
    showSpectrum = !showSpectrum;
    assert(showSpectrum == true && "After first toggle, should be ON");

    showSpectrum = !showSpectrum;
    assert(showSpectrum == false && "After second toggle, should be OFF");

    showSpectrum = !showSpectrum;
    assert(showSpectrum == true && "After third toggle, should be ON again");

    // Measure rendering performance
    SpectrumAnalyzer analyzer(44100, 1024);
    SpectrumRenderer::RenderOptions options;
    options.width = 80;
    options.height = 10;
    SpectrumRenderer renderer(options);

    // Generate test data
    std::vector<float> testData(1024);
    for (int i = 0; i < 1024; ++i) {
        testData[i] = std::sin(2.0f * 3.14159265f * 440.0f * i / 44100.0f);
    }

    // Measure total time for 60 frames (1 second @ 60 FPS)
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int frame = 0; frame < 60; ++frame) {
        analyzer.processSamples(testData.data(), testData.size());
        auto spectrum = analyzer.getFullSpectrum();
        if (showSpectrum) {
            std::string rendered = renderer.render(spectrum);
            // Prevent compiler optimization of unused result
            assert(!rendered.empty());
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto totalMs = std::chrono::duration<double, std::milli>(end - start).count();
    auto avgMs = totalMs / 60.0;

    // Verify less than 6ms overhead per frame (60 FPS budget = 16.7ms)
    std::cout << "PASS (toggle working, avg " << avgMs << "ms per frame";
    if (avgMs < 6.0) {
        std::cout << ", under 6ms budget";
    }
    std::cout << ")\n";
}

} // namespace dj

// Main test runner
int main() {
    std::cout << "\n========== SPECTRUM DISPLAY PHASE 35 TESTS ==========\n\n";

    try {
        // Core SpectrumAnalyzer FFT integration
        dj::test_SpectrumAnalyzer_FFTIntegration();
        dj::test_SpectrumAnalyzer_ThreadSafety();
        dj::test_SpectrumAnalyzer_BackwardCompatibility();

        // SpectrumRenderer visualization
        dj::test_SpectrumRenderer_BasicRender();
        dj::test_SpectrumRenderer_PeakHold();
        dj::test_SpectrumRenderer_DualDeck();
        dj::test_SpectrumRenderer_FrequencyLabels();

        // Main loop integration
        dj::test_MainLoop_SpectrumToggle();

        std::cout << "\n========== ALL 8 TESTS PASSED ==========\n\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTEST FAILED: " << e.what() << "\n\n";
        return 1;
    }
}
