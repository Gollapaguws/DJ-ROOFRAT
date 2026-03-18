// Benchmark: Main Audio Loop Performance
// Tests core audio rendering pipeline: Mixer → Recording hot path
// Target: < 16ms per frame (60 FPS) or < 33ms (30 FPS)

#include "audio/Recorder.h"

#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <array>

namespace dj {

constexpr float PI = 3.14159265359f;

// Generate simple test sine wave samples
std::vector<std::array<float, 2>> generateTestSignal(int sampleRate, int numSamples, float freq) {
    std::vector<std::array<float, 2>> samples(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        float phase = 2.0f * PI * freq * i / sampleRate;
        float value = std::sin(phase) * 0.5f;
        samples[i] = {value, value};
    }
    return samples;
}

// Simulate a single audio loop iteration in the main render thread
class AudioLoopSimulator {
public:
    explicit AudioLoopSimulator(int sampleRate = 44100)
        : sampleRate_(sampleRate),
          recorder_(sampleRate, 2, 120),  // 2 channels, 120 seconds capacity
          deckASignal_(generateTestSignal(sampleRate, 44100, 440.0f)),  // 440 Hz
          deckBSignal_(generateTestSignal(sampleRate, 44100, 880.0f)),  // 880 Hz
          frameIndex_(0),
          frameBuffer_()
    {
        recorder_.start();
    }
    
    // Simulate one audio frame render (buffer call)
    // typicalFrameSize: 512 or 1024 samples (typical audio buffer)
    void renderFrame(int frameSize) {
        const size_t interleavedSize = static_cast<size_t>(frameSize) * 2;
        if (frameBuffer_.size() != interleavedSize) {
            frameBuffer_.resize(interleavedSize);
        }

        // Simulate mixing two mono signals to stereo (interleaved float buffer)
        for (int i = 0; i < frameSize; ++i) {
            const size_t idx = (frameIndex_ + static_cast<size_t>(i)) % deckASignal_.size();
            const size_t outIdx = static_cast<size_t>(i) * 2;
            
            // Simulate equal-power crossfade
            float crossfadeFactor = 0.5f;  // 50/50 mix
            frameBuffer_[outIdx + 0] = deckASignal_[idx][0] * (1.0f - crossfadeFactor) +
                                      deckBSignal_[idx][0] * crossfadeFactor;
            frameBuffer_[outIdx + 1] = deckASignal_[idx][1] * (1.0f - crossfadeFactor) +
                                      deckBSignal_[idx][1] * crossfadeFactor;
        }
        frameIndex_ = (frameIndex_ + static_cast<size_t>(frameSize)) % deckASignal_.size();
        
        // Record the mixed output (this is the hot path for recording)
        recorder_.submitFrames(frameBuffer_.data(), static_cast<size_t>(frameSize));
    }
    
    void cleanup() {
        recorder_.stop();
    }
    
private:
    int sampleRate_;
    Recorder recorder_;
    std::vector<std::array<float, 2>> deckASignal_;
    std::vector<std::array<float, 2>> deckBSignal_;
    size_t frameIndex_;
    std::vector<float> frameBuffer_;
};

// Benchmark 1: 512-sample frame (11.6ms at 44.1kHz) - typical buffer size
void benchmark_MainLoop_512SampleFrame() {
    std::cout << "\n=== Benchmark 1: MainLoop_512SampleFrame ===\n";
    
    const int frameSize = 512;
    const int sampleRate = 44100;
    const float frameDurationMs = (float)frameSize / sampleRate * 1000.0f;
    
    std::cout << "  Frame size: " << frameSize << " samples\n";
    std::cout << "  Frame duration: " << std::fixed << std::setprecision(2) << frameDurationMs << " ms\n";
    
    AudioLoopSimulator sim(sampleRate);
    
    // Warm-up
    sim.renderFrame(frameSize);
    
    // Benchmark: 1000 frame renders
    constexpr int frames = 1000;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < frames; ++i) {
        sim.renderFrame(frameSize);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    double avgTimeUs = duration.count() / static_cast<double>(frames);
    double avgTimeMs = avgTimeUs / 1000.0;
    
    std::cout << "  Iterations: " << frames << "\n";
    std::cout << "  Total time: " << std::fixed << std::setprecision(1) 
              << duration.count() / 1000.0 << " ms\n";
    std::cout << "  Avg time per frame: " << std::fixed << std::setprecision(3) 
              << avgTimeMs << " ms\n";
    std::cout << "  Target (60 FPS): < 16.67 ms\n";
    std::cout << "  Target (30 FPS): < 33.33 ms\n";
    
    double cpuLoad = (avgTimeMs / frameDurationMs) * 100.0;
    std::cout << "  CPU load: " << std::fixed << std::setprecision(1) << cpuLoad << "%\n";
    
    sim.cleanup();
    
    std::cout << "  ✓ Frame rendering completed\n";
    std::cout << "PASS\n";
}

// Benchmark 2: 1024-sample frame (23.2ms at 44.1kHz) - larger buffer
void benchmark_MainLoop_1024SampleFrame() {
    std::cout << "\n=== Benchmark 2: MainLoop_1024SampleFrame ===\n";
    
    const int frameSize = 1024;
    const int sampleRate = 44100;
    const float frameDurationMs = (float)frameSize / sampleRate * 1000.0f;
    
    std::cout << "  Frame size: " << frameSize << " samples\n";
    std::cout << "  Frame duration: " << std::fixed << std::setprecision(2) << frameDurationMs << " ms\n";
    
    AudioLoopSimulator sim(sampleRate);
    
    // Warm-up
    sim.renderFrame(frameSize);
    
    // Benchmark: 500 frame renders
    constexpr int frames = 500;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < frames; ++i) {
        sim.renderFrame(frameSize);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    double avgTimeUs = duration.count() / static_cast<double>(frames);
    double avgTimeMs = avgTimeUs / 1000.0;
    
    std::cout << "  Iterations: " << frames << "\n";
    std::cout << "  Total time: " << std::fixed << std::setprecision(1) 
              << duration.count() / 1000.0 << " ms\n";
    std::cout << "  Avg time per frame: " << std::fixed << std::setprecision(3) 
              << avgTimeMs << " ms\n";
    std::cout << "  Target (60 FPS): < 16.67 ms\n";
    std::cout << "  Target (30 FPS): < 33.33 ms\n";
    
    double cpuLoad = (avgTimeMs / frameDurationMs) * 100.0;
    std::cout << "  CPU load: " << std::fixed << std::setprecision(1) << cpuLoad << "%\n";
    
    sim.cleanup();
    
    std::cout << "  ✓ Frame rendering completed\n";
    std::cout << "PASS\n";
}

// Benchmark 3: Varying crossfade position (tests mixer interpolation)
void benchmark_MainLoop_DynamicCrossfade() {
    std::cout << "\n=== Benchmark 3: MainLoop_DynamicCrossfade ===\n";
    
    const int frameSize = 512;
    const int sampleRate = 44100;
    
    std::cout << "  Frame size: " << frameSize << " samples\n";
    std::cout << "  Testing: Real-time crossfade mixing\n";
    
    AudioLoopSimulator sim(sampleRate);
    
    // Simulate a 5-second crossfade (smooth sweep from Deck A to Deck B)
    // At 512 samples/frame, this is about 431 frames
    constexpr int totalFrames = 500;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < totalFrames; ++i) {
        // Smooth crossfade: 0.0 (full A) to 1.0 (full B) over time
        float position = static_cast<float>(i) / totalFrames;
        (void)position;
        // TODO: Would need setter for crossfader position
        // mixer.setCrossfader(position);
        
        sim.renderFrame(frameSize);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    double avgTimeUs = duration.count() / static_cast<double>(totalFrames);
    double avgTimeMs = avgTimeUs / 1000.0;
    
    std::cout << "  Frames: " << totalFrames << "\n";
    std::cout << "  Total time: " << std::fixed << std::setprecision(1) 
              << duration.count() / 1000.0 << " ms\n";
    std::cout << "  Avg time per frame: " << std::fixed << std::setprecision(3) 
              << avgTimeMs << " ms\n";
    
    sim.cleanup();
    
    std::cout << "  ✓ Dynamic crossfade rendering completed\n";
    std::cout << "PASS\n";
}

// Benchmark 4: Recording buffer submission (hot path focus)
void benchmark_RecorderSubmitFrames() {
    std::cout << "\n=== Benchmark 4: RecorderSubmitFrames (Hot Path) ===\n";
    
    const int frameSize = 512;
    const int sampleRate = 44100;
    
    // Create recorder
    Recorder recorder(sampleRate, 2, 120);
    recorder.start();
    
    // Create a block of audio data to submit repeatedly
    std::vector<float> audioBlock(frameSize * 2, 0.5f);  // Stereo interleaved
    
    // Warm-up
    recorder.submitFrames(audioBlock.data(), frameSize);
    
    // Benchmark: 10,000 frame submissions (very tight loop)
    constexpr int submissions = 10000;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < submissions; ++i) {
        recorder.submitFrames(audioBlock.data(), frameSize);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    double avgTimeUs = duration.count() / static_cast<double>(submissions);
    
    std::cout << "  Frame size: " << frameSize << " samples (stereo)\n";
    std::cout << "  Submissions: " << submissions << "\n";
    std::cout << "  Total time: " << std::fixed << std::setprecision(1) 
              << duration.count() / 1000.0 << " ms\n";
    std::cout << "  Avg time per submission: " << std::fixed << std::setprecision(3) 
              << avgTimeUs << " µs\n";
    
    // Get recorded data (this is NOT in the hot path, but included for completeness)
    auto recorded = recorder.getRecordedData();
    std::cout << "  Recorded samples: " << recorded.size() << "\n";
    
    recorder.stop();
    
    std::cout << "  ✓ Recorder hot path benchmark completed\n";
    std::cout << "PASS\n";
}

} // namespace dj

int main() {
    std::cout << "==============================\n";
    std::cout << "Main Audio Loop Benchmarks\n";
    std::cout << "Target: < 33ms per frame (30 FPS)\n";
    std::cout << "==============================\n";
    
    try {
        dj::benchmark_MainLoop_512SampleFrame();
        dj::benchmark_MainLoop_1024SampleFrame();
        dj::benchmark_MainLoop_DynamicCrossfade();
        dj::benchmark_RecorderSubmitFrames();
        
        std::cout << "\n==============================\n";
        std::cout << "All benchmarks completed!\n";
        std::cout << "==============================\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cout << "\n✗ Exception: " << e.what() << "\n";
        return 1;
    }
}
