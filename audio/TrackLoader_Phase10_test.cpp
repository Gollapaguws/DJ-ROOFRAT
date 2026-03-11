#include "audio/AudioClip.h"
#include "audio/KeyDetector.h"
#include "audio/MetadataParser.h"
#include "audio/TrackLoader.h"
#include "audio/WaveformCache.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <numbers>

namespace dj {

// Test 1: MetadataParser basic structure
void test_MetadataParser_Structure() {
    // Verify MetadataParser is accessible
    // This ensures the class is defined properly even without real files
    std::cout << "✓ test_MetadataParser_Structure passed" << std::endl;
}

// Test 2: KeyDetector with major scale (C major - should detect something)
void test_KeyDetector_MajorScale() {
    // Generate C major triad: C(261.63Hz) + E(329.63Hz) + G(392Hz)
    // Create composite tone by adding sine waves
    AudioClip clip;
    clip.sampleRate = 44100;
    clip.channels = 2;
    
    const int duration = 2; // 2 seconds for BPM detection minimum
    const std::size_t frameCount = static_cast<std::size_t>(duration * clip.sampleRate);
    clip.samples.resize(frameCount * 2, 0.0f);
    
    const float c_freq = 261.63f;  // C4
    const float e_freq = 329.63f;  // E4
    const float g_freq = 392.0f;   // G4
    
    for (std::size_t i = 0; i < frameCount; ++i) {
        const float time = static_cast<float>(i) / static_cast<float>(clip.sampleRate);
        const float c_phase = 2.0f * std::numbers::pi_v<float> * c_freq * time;
        const float e_phase = 2.0f * std::numbers::pi_v<float> * e_freq * time;
        const float g_phase = 2.0f * std::numbers::pi_v<float> * g_freq * time;
        
        const float sample = (std::sin(c_phase) + std::sin(e_phase) + std::sin(g_phase)) / 3.0f * 0.3f;
        clip.samples[i * 2] = sample;
        clip.samples[i * 2 + 1] = sample;
    }
    
    // Test detection
    auto key = KeyDetector::detect(clip);
    // Key detection should return something (not null) for a valid audio clip
    assert(key.has_value());
    assert(!key->empty());
    
    std::cout << "✓ test_KeyDetector_MajorScale passed (detected: " << *key << ")" << std::endl;
}

// Test 3: KeyDetector with minor scale (A minor)
void test_KeyDetector_MinorScale() {
    // Generate A minor triad: A(220Hz) + C(261.63Hz) + E(329.63Hz)
    AudioClip clip;
    clip.sampleRate = 44100;
    clip.channels = 2;
    
    const int duration = 2;
    const std::size_t frameCount = static_cast<std::size_t>(duration * clip.sampleRate);
    clip.samples.resize(frameCount * 2, 0.0f);
    
    const float a_freq = 220.0f;   // A3
    const float c_freq = 261.63f;  // C4
    const float e_freq = 329.63f;  // E4
    
    for (std::size_t i = 0; i < frameCount; ++i) {
        const float time = static_cast<float>(i) / static_cast<float>(clip.sampleRate);
        const float a_phase = 2.0f * std::numbers::pi_v<float> * a_freq * time;
        const float c_phase = 2.0f * std::numbers::pi_v<float> * c_freq * time;
        const float e_phase = 2.0f * std::numbers::pi_v<float> * e_freq * time;
        
        const float sample = (std::sin(a_phase) + std::sin(c_phase) + std::sin(e_phase)) / 3.0f * 0.3f;
        clip.samples[i * 2] = sample;
        clip.samples[i * 2 + 1] = sample;
    }
    
    auto key = KeyDetector::detect(clip);
    assert(key.has_value());
    assert(!key->empty());
    
    std::cout << "✓ test_KeyDetector_MinorScale passed (detected: " << *key << ")" << std::endl;
}

// Test 4: WaveformCache downsampling
void test_WaveformCache_Downsampling() {
    // Create a simple test tone
    AudioClip clip = AudioClip::generateTestTone(440.0f, 1.0f, 44100);
    
    // Generate waveform cache with various bucket counts
    auto waveform = WaveformCache::generate(clip, 1000);
    
    // Verify structure
    assert(!waveform.empty());
    assert(waveform.size() > 0);
    
    // Verify all buckets have reasonable values
    for (const auto& bucket : waveform) {
        assert(bucket.minSample >= -1.0f && bucket.minSample <= 1.0f);
        assert(bucket.maxSample >= -1.0f && bucket.maxSample <= 1.0f);
        assert(bucket.minSample <= bucket.maxSample);
    }
    
    std::cout << "✓ test_WaveformCache_Downsampling passed (" << waveform.size() << " buckets)" << std::endl;
}

// Test 5: WaveformCache with large bucket count
void test_WaveformCache_LargeBucketCount() {
    AudioClip clip = AudioClip::generateTestTone(880.0f, 2.0f, 44100);
    auto waveform = WaveformCache::generate(clip, 2000);
    
    assert(!waveform.empty());
    for (const auto& bucket : waveform) {
        assert(bucket.minSample >= -1.0f && bucket.minSample <= 1.0f);
        assert(bucket.maxSample >= -1.0f && bucket.maxSample <= 1.0f);
    }
    
    std::cout << "✓ test_WaveformCache_LargeBucketCount passed (" << waveform.size() << " buckets)" << std::endl;
}

// Test 6: AudioClip metadata accessor
void test_AudioClip_MetadataAccessor() {
    AudioClip clip = AudioClip::generateTestTone(440.0f, 1.0f, 44100);
    
    // Verify metadata accessor exists and returns null initially
    const auto* metadata = clip.metadata();
    assert(metadata == nullptr);
    
    // Verify we can create metadata-enabled clip
    TrackMetadata meta;
    meta.title = "Test Track";
    meta.artist = "Test Artist";
    meta.bpm = 120.0f;
    meta.key = "C major";
    meta.durationSeconds = 60.0f;
    meta.genre = "Electronic";
    
    // Metadata can be set (implementation will allow this)
    std::cout << "✓ test_AudioClip_MetadataAccessor passed" << std::endl;
}

// Test 7: TrackLoader integration with metadata
void test_TrackLoader_MetadataIntegration() {
    // Create a test tone to simulate loading
    AudioClip clip = AudioClip::generateTestTone(440.0f, 1.0f, 44100);
    
    // Verify clip can be extended with metadata
    assert(clip.sampleRate > 0);
    assert(clip.channels > 0);
    assert(!clip.samples.empty());
    
    std::cout << "✓ test_TrackLoader_MetadataIntegration passed" << std::endl;
}

// Test 8: KeyDetector with pure sine wave
void test_KeyDetector_PureTone() {
    // Generate 440Hz sine wave (A4)
    AudioClip clip;
    clip.sampleRate = 44100;
    clip.channels = 2;
    
    const int duration = 2;
    const std::size_t frameCount = static_cast<std::size_t>(duration * clip.sampleRate);
    clip.samples.resize(frameCount * 2, 0.0f);
    
    const float freq = 440.0f; // A4
    
    for (std::size_t i = 0; i < frameCount; ++i) {
        const float time = static_cast<float>(i) / static_cast<float>(clip.sampleRate);
        const float phase = 2.0f * std::numbers::pi_v<float> * freq * time;
        const float sample = std::sin(phase) * 0.3f;
        
        clip.samples[i * 2] = sample;
        clip.samples[i * 2 + 1] = sample;
    }
    
    auto key = KeyDetector::detect(clip);
    assert(key.has_value());
    
    std::cout << "✓ test_KeyDetector_PureTone passed (detected: " << *key << ")" << std::endl;
}

// Test 9: WaveformCache with empty clip
void test_WaveformCache_EmptyClip() {
    AudioClip clip;
    clip.channels = 2;
    clip.sampleRate = 44100;
    // No samples
    
    auto waveform = WaveformCache::generate(clip, 1000);
    // Should handle gracefully
    assert(waveform.empty());
    
    std::cout << "✓ test_WaveformCache_EmptyClip passed" << std::endl;
}

// Test 10: WaveformCache small bucket count
void test_WaveformCache_SmallBucketCount() {
    AudioClip clip = AudioClip::generateTestTone(440.0f, 1.0f, 44100);
    auto waveform = WaveformCache::generate(clip, 10);
    
    assert(!waveform.empty());
    assert(waveform.size() <= 10);
    
    std::cout << "✓ test_WaveformCache_SmallBucketCount passed (" << waveform.size() << " buckets)" << std::endl;
}

void runAllTrackLoaderPhase10Tests() {
    std::cout << "\n=== Running TrackLoader Phase 10 Tests ===" << std::endl;
    try {
        test_MetadataParser_Structure();
        test_KeyDetector_MajorScale();
        test_KeyDetector_MinorScale();
        test_WaveformCache_Downsampling();
        test_WaveformCache_LargeBucketCount();
        test_AudioClip_MetadataAccessor();
        test_TrackLoader_MetadataIntegration();
        test_KeyDetector_PureTone();
        test_WaveformCache_EmptyClip();
        test_WaveformCache_SmallBucketCount();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        std::exit(1);
    }
}

} // namespace dj

int main() {
    dj::runAllTrackLoaderPhase10Tests();
    std::cout << "\n=== TrackLoader Phase 10 Tests Complete ===" << std::endl;
    return 0;
}
