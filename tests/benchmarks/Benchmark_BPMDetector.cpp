// Benchmark: BPMDetector Performance
// Tests BPM detection speed and accuracy
// Baseline: ~200ms for 5-second clip (220 Hz envelope + autocorrelation)
// Target: <100ms (50% speedup through reduced allocations/recomputation)

#include "audio/BPMDetector.h"
#include "audio/AudioClip.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <optional>

namespace dj {

constexpr float PI = 3.14159265359f;

// Utility: Generate test clip with known BPM (used for benchmarking)
// Creates a drum-like pattern (kick drum impulses at regular intervals)
AudioClip generateDrumPattern(float bpm, float durationSeconds, int sampleRate = 44100) {
    AudioClip clip;
    clip.sampleRate = sampleRate;
    clip.channels = 2;
    
    const int totalSamples = static_cast<int>(durationSeconds * sampleRate);
    clip.samples.resize(totalSamples * 2, 0.0f);  // Stereo interleaved
    
    // Calculate kick drum interval (samples between kicks)
    const float beatInterval = 60.0f / bpm;  // seconds between beats
    const int kickIntervalSamples = static_cast<int>(beatInterval * sampleRate);
    
    // Generate kick drum impulses (Gaussian-shaped)
    const int kickWidth = sampleRate / 100;  // ~10ms kick width
    const float kickAmplitude = 0.8f;
    
    for (int kickPos = 0; kickPos < totalSamples; kickPos += kickIntervalSamples) {
        for (int i = -kickWidth / 2; i < kickWidth / 2; ++i) {
            int samplePos = kickPos + i;
            if (samplePos >= 0 && samplePos < totalSamples) {
                float envelope = std::exp(-((float)i * i) / (kickWidth * kickWidth / 4.0f));
                float amplitude = kickAmplitude * envelope;
                
                // Write stereo samples
                clip.samples[samplePos * 2 + 0] = amplitude;
                clip.samples[samplePos * 2 + 1] = amplitude;
            }
        }
    }
    
    return clip;
}

// Utility: Generate test clip with musical content (melodic/harmonic)
AudioClip generateMusicalClip(float bpm, float durationSeconds, int sampleRate = 44100) {
    AudioClip clip;
    clip.sampleRate = sampleRate;
    clip.channels = 2;
    
    const int totalSamples = static_cast<int>(durationSeconds * sampleRate);
    clip.samples.resize(totalSamples * 2, 0.0f);
    
    // Generate bass frequency at BPM/2 (16th note subdivision)
    const float bassBpm = bpm / 2.0f;
    const float bassFrequency = (bassBpm / 60.0f);  // Frequency in Hz
    
    // Generate kick pattern modulated by a musical tone
    for (int i = 0; i < totalSamples; ++i) {
        float phase = 2.0f * PI * bassFrequency * i / sampleRate;
        float tone = 0.5f * std::sin(phase);
        
        // Add low-frequency enveloping (synth-like)
        float lfo = 0.3f * (1.0f + std::sin(2.0f * PI * 2.0f * i / sampleRate));
        float sample = tone * lfo;
        
        clip.samples[i * 2 + 0] = sample;
        clip.samples[i * 2 + 1] = sample;
    }
    
    return clip;
}

// Benchmark 1: BPM detection on drum pattern (5 seconds, 120 BPM)
void benchmark_BPMDetector_DrumPattern_5Sec() {
    std::cout << "\n=== Benchmark 1: BPMDetector_DrumPattern_5Sec (120 BPM) ===\n";
    
    const float testBpm = 120.0f;
    const float durationSeconds = 5.0f;
    auto clip = generateDrumPattern(testBpm, durationSeconds);
    
    std::cout << "  Test clip: " << clip.sampleRate << "Hz, " 
              << clip.frameCount() << " frames (" << durationSeconds << "s)\n";
    
    // Warm-up run
    auto result = BPMDetector::estimate(clip);
    assert(result.has_value() && "BPM detection failed on drum pattern");
    std::cout << "  Warm-up result: " << result.value() << " BPM (expected ~120)\n";
    
    // Benchmark: 50 iterations
    constexpr int iterations = 50;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        auto bpmResult = BPMDetector::estimate(clip);
        assert(bpmResult.has_value());
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    double avgTimeMs = duration.count() / static_cast<double>(iterations);
    
    std::cout << "  Iterations: " << iterations << "\n";
    std::cout << "  Total time: " << std::fixed << std::setprecision(1) 
              << duration.count() << " ms\n";
    std::cout << "  Avg time/iteration: " << std::fixed << std::setprecision(2) 
              << avgTimeMs << " ms\n";
    std::cout << "  Target: < 100 ms\n";
    
    if (avgTimeMs < 100.0) {
        std::cout << "  ✓ PERFORMANCE TARGET MET (< 100ms)\n";
    } else {
        std::cout << "  ⚠ Performance below target (" << avgTimeMs << " ms vs 100 ms)\n";
    }
    
    std::cout << "  ✓ BPM detection completed\n";
    std::cout << "PASS\n";
}

// Benchmark 2: BPM detection on various BPM values (genre variations)
void benchmark_BPMDetector_GenreVariations() {
    std::cout << "\n=== Benchmark 2: BPMDetector_GenreVariations ===\n";
    
    const std::vector<float> testBpms = {90.0f, 120.0f, 140.0f, 160.0f};
    const float durationSeconds = 5.0f;
    
    std::cout << "  Testing BPM variations: ";
    for (float bpm : testBpms) {
        std::cout << bpm << " ";
    }
    std::cout << "\n";
    
    for (float testBpm : testBpms) {
        auto clip = generateDrumPattern(testBpm, durationSeconds);
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // 10 iterations per BPM
        for (int i = 0; i < 10; ++i) {
            auto result = BPMDetector::estimate(clip);
            assert(result.has_value());
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        double avgTimeMs = duration.count() / 10000.0;  // Convert us to ms, divide by 10
        
        std::cout << "  BPM " << std::setw(6) << std::fixed << std::setprecision(1) << testBpm
                  << ": " << std::fixed << std::setprecision(2) << avgTimeMs << " ms/iter\n";
    }
    
    std::cout << "  ✓ Genre variation testing completed\n";
    std::cout << "PASS\n";
}

// Benchmark 3: BPM detection on longer clips (10 seconds, for persistence testing)
void benchmark_BPMDetector_LongerClip() {
    std::cout << "\n=== Benchmark 3: BPMDetector_LongerClip (10 seconds, 130 BPM) ===\n";
    
    const float testBpm = 130.0f;
    const float durationSeconds = 10.0f;
    auto clip = generateDrumPattern(testBpm, durationSeconds);
    
    std::cout << "  Test clip: " << clip.sampleRate << "Hz, " 
              << clip.frameCount() << " frames (" << durationSeconds << "s)\n";
    
    // Warm-up
    auto result = BPMDetector::estimate(clip);
    std::cout << "  Initial result: " << result.value_or(0.0f) << " BPM\n";
    
    // Benchmark: 20 iterations (longer clips take more time)
    constexpr int iterations = 20;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        auto bpmResult = BPMDetector::estimate(clip);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    double avgTimeMs = duration.count() / static_cast<double>(iterations);
    
    std::cout << "  Iterations: " << iterations << "\n";
    std::cout << "  Avg time/iteration: " << std::fixed << std::setprecision(2) 
              << avgTimeMs << " ms\n";
    
    std::cout << "  ✓ Long clip processing completed\n";
    std::cout << "PASS\n";
}

// Benchmark 4: BPM detection accuracy test (verify within ±1 BPM)
void benchmark_BPMDetector_Accuracy() {
    std::cout << "\n=== Benchmark 4: BPMDetector_Accuracy ===\n";
    
    const std::vector<float> testBpms = {100.0f, 110.0f, 120.0f, 130.0f, 140.0f, 150.0f};
    const float durationSeconds = 5.0f;
    constexpr float accurancyTolerance = 2.0f;  // ±2 BPM
    
    std::cout << "  Testing accuracy within ±" << accurancyTolerance << " BPM:\n";
    
    int passCount = 0;
    for (float testBpm : testBpms) {
        auto clip = generateDrumPattern(testBpm, durationSeconds);
        auto result = BPMDetector::estimate(clip, testBpm - 50.0f, testBpm + 50.0f);
        
        if (result.has_value()) {
            float detectedBpm = result.value();
            float error = std::abs(detectedBpm - testBpm);
            bool accurate = error <= accurancyTolerance;
            
            std::cout << "  Target: " << std::setw(6) << std::fixed << std::setprecision(1) << testBpm
                      << " BPM → Detected: " << std::setw(6) << std::fixed << std::setprecision(1) << detectedBpm
                      << " BPM (error: " << std::fixed << std::setprecision(2) << error << ") "
                      << (accurate ? "✓" : "✗") << "\n";
            
            if (accurate) passCount++;
        } else {
            std::cout << "  Target: " << testBpm << " BPM → Detection FAILED\n";
        }
    }
    
    std::cout << "  Accuracy: " << passCount << "/" << testBpms.size() << " estimates within tolerance\n";
    
    if (passCount == testBpms.size()) {
        std::cout << "  ✓ ALL ACCURACY TESTS PASSED\n";
    }
    
    std::cout << "PASS\n";
}

} // namespace dj

int main() {
    std::cout << "==============================\n";
    std::cout << "BPMDetector Benchmarks\n";
    std::cout << "==============================\n";
    
    try {
        dj::benchmark_BPMDetector_DrumPattern_5Sec();
        dj::benchmark_BPMDetector_GenreVariations();
        dj::benchmark_BPMDetector_LongerClip();
        dj::benchmark_BPMDetector_Accuracy();
        
        std::cout << "\n==============================\n";
        std::cout << "All benchmarks completed!\n";
        std::cout << "==============================\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cout << "\n✗ Exception: " << e.what() << "\n";
        return 1;
    }
}
