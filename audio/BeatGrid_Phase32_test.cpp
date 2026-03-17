#include "audio/OnsetDetector.h"
#include "audio/BeatGrid.h"
#include "audio/TrackMetadata.h"
#include "visuals/BeatGridRenderer.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>

namespace dj {

// Helper: Generate audio with clear transients (drum hits)
std::vector<float> generateDrumHits(int sampleRate, int numHits, float spacingSeconds) {
    std::vector<float> audio;
    int samplesPerHit = static_cast<int>(spacingSeconds * sampleRate);
    
    for (int hit = 0; hit < numHits; ++hit) {
        // Generate impulse (sharp transient) - exponential decay
        for (int i = 0; i < 100; ++i) {  // 100 samples = ~2.3ms @ 44100 Hz
            float envelope = std::exp(-i / 10.0f);
            audio.push_back(envelope * (i % 2 == 0 ? 1.0f : -1.0f));
        }
        
        // Silence between hits
        for (int i = 100; i < samplesPerHit; ++i) {
            audio.push_back(0.0f);
        }
    }
    
    return audio;
}

// Helper: Generate white noise
std::vector<float> generateWhiteNoise(int sampleRate, float durationSeconds) {
    std::vector<float> audio;
    int numSamples = static_cast<int>(sampleRate * durationSeconds);
    
    // Simple LCG random number generator
    unsigned long seed = 12345;
    for (int i = 0; i < numSamples; ++i) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        float random = (static_cast<float>(seed) / 0x7fffffff) * 2.0f - 1.0f;
        audio.push_back(random * 0.1f);  // Scale down to prevent clipping
    }
    
    return audio;
}

// Test 1: OnsetDetector - Spectral Flux with clear transients
void test_OnsetDetector_SpectralFlux() {
    std::cout << "[TEST] OnsetDetector_SpectralFlux..." << std::endl;
    
    const int sampleRate = 44100;
    const int numHits = 4;
    const float spacingSeconds = 0.5f;  // 4 hits 0.5s apart = 2 seconds total
    
    // Generate drum hits
    std::vector<float> audio = generateDrumHits(sampleRate, numHits, spacingSeconds);
    
    // Create detector and process audio
    OnsetDetector detector(sampleRate, 512, 256);
    detector.processSamples(audio.data(), audio.size());
    
    // Get detected onsets
    std::vector<double> onsets = detector.getOnsets();
    
    // Should detect approximately 4 onsets
    assert(onsets.size() >= 3 && onsets.size() <= 5);
    
    // Verify spacing between onsets is roughly 0.5 seconds
    for (size_t i = 1; i < onsets.size(); ++i) {
        double spacing = onsets[i] - onsets[i-1];
        assert(spacing > 0.4 && spacing < 0.6);  // ±100ms tolerance
    }
    
    // Get spectral flux curve - should have peaks at onset locations
    std::vector<float> flux = detector.getSpectralFlux();
    assert(flux.size() > 0);
    
    std::cout << "✓ test_OnsetDetector_SpectralFlux passed (detected " << onsets.size() << " onsets)" << std::endl;
}

// Test 2: OnsetDetector - Noise rejection (false positives)
void test_OnsetDetector_NoiseRejection() {
    std::cout << "[TEST] OnsetDetector_NoiseRejection..." << std::endl;
    
    const int sampleRate = 44100;
    const float durationSeconds = 5.0f;
    
    // Generate white noise (no clear onsets)
    std::vector<float> audio = generateWhiteNoise(sampleRate, durationSeconds);
    
    // Create detector and process audio
    OnsetDetector detector(sampleRate, 512, 256);
    detector.processSamples(audio.data(), audio.size());
    
    // Get detected onsets
    std::vector<double> onsets = detector.getOnsets();
    
    // Should reject most noise - expect minimal false positives in 5 seconds
    // Note: With adaptive threshold (mean + 2.5*stddev), white noise can still trigger peaks
    // Acceptable range: <= 8 false positives in 5 seconds (vs hundreds without filtering)
    if (onsets.size() > 2) {
        std::cout << "  Note: Detected " << onsets.size() << " onsets in white noise" << std::endl;
    }
    assert(onsets.size() <= 8);
    
    std::cout << "✓ test_OnsetDetector_NoiseRejection passed (detected " << onsets.size() << " onsets in noise)" << std::endl;
}

// Test 3: BeatGrid - Generate from BPM
void test_BeatGrid_GenerateFromBPM() {
    std::cout << "[TEST] BeatGrid_GenerateFromBPM..." << std::endl;
    
    const double bpm = 120.0;
    const double trackDuration = 60.0;  // 60 seconds
    const double firstBeatOffset = 0.0;
    
    BeatGrid grid;
    grid.generateFromBPM(bpm, firstBeatOffset, trackDuration);
    
    std::vector<BeatMarker> beats = grid.getBeats();
    
    // At 120 BPM, we should have 2 beats per second
    // In 60 seconds = 120 beats
    assert(beats.size() == 120);
    
    // Check beat spacing: 60/120 = 0.5 seconds between beats
    for (size_t i = 1; i < beats.size(); ++i) {
        double spacing = beats[i].timestamp - beats[i-1].timestamp;
        // Allow small floating point error
        assert(spacing > 0.499 && spacing < 0.501);
    }
    
    // Verify downbeat markers every 4 beats
    for (size_t i = 0; i < beats.size(); ++i) {
        bool shouldBeDownbeat = (i % 4 == 0);
        assert(beats[i].isDownbeat == shouldBeDownbeat);
    }
    
    std::cout << "✓ test_BeatGrid_GenerateFromBPM passed (generated " << beats.size() << " beats)" << std::endl;
}

// Test 4: BeatGrid - Manual nudge of single beat
void test_BeatGrid_ManualNudge() {
    std::cout << "[TEST] BeatGrid_ManualNudge..." << std::endl;
    
    const double bpm = 120.0;
    const double trackDuration = 60.0;
    const double firstBeatOffset = 0.0;
    
    BeatGrid grid;
    grid.generateFromBPM(bpm, firstBeatOffset, trackDuration);
    
    std::vector<BeatMarker> beforeNudge = grid.getBeats();
    double originalTimestamp = beforeNudge[10].timestamp;
    
    // Nudge beat #10 by +10ms
    grid.nudgeBeat(10, 10.0);
    
    std::vector<BeatMarker> afterNudge = grid.getBeats();
    
    // Beat #10 should be shifted by 10ms (0.01 seconds)
    double newTimestamp = afterNudge[10].timestamp;
    assert(std::abs(newTimestamp - (originalTimestamp + 0.01)) < 0.0001);
    
    // Other beats should remain unchanged
    for (size_t i = 0; i < afterNudge.size(); ++i) {
        if (i != 10) {
            assert(std::abs(afterNudge[i].timestamp - beforeNudge[i].timestamp) < 0.0001);
        }
    }
    
    // Test negative nudge
    grid.nudgeBeat(20, -10.0);
    std::vector<BeatMarker> afterNegativeNudge = grid.getBeats();
    assert(std::abs(afterNegativeNudge[20].timestamp - (beforeNudge[20].timestamp - 0.01)) < 0.0001);
    
    std::cout << "✓ test_BeatGrid_ManualNudge passed" << std::endl;
}

// Test 5: BeatGrid - Nudge all beats uniformly
void test_BeatGrid_NudgeAll() {
    std::cout << "[TEST] BeatGrid_NudgeAll..." << std::endl;
    
    const double bpm = 120.0;
    const double trackDuration = 60.0;
    const double firstBeatOffset = 0.0;
    
    BeatGrid grid;
    grid.generateFromBPM(bpm, firstBeatOffset, trackDuration);
    
    std::vector<BeatMarker> beforeNudge = grid.getBeats();
    
    // Nudge all beats by +20ms
    grid.nudgeAll(20.0);
    
    std::vector<BeatMarker> afterNudge = grid.getBeats();
    
    // All beats should be shifted uniformly by 20ms (0.02 seconds)
    for (size_t i = 0; i < afterNudge.size(); ++i) {
        double expectedTimestamp = beforeNudge[i].timestamp + 0.02;
        assert(std::abs(afterNudge[i].timestamp - expectedTimestamp) < 0.0001);
    }
    
    // Relative spacing should be preserved
    for (size_t i = 1; i < afterNudge.size(); ++i) {
        double beforeSpacing = beforeNudge[i].timestamp - beforeNudge[i-1].timestamp;
        double afterSpacing = afterNudge[i].timestamp - afterNudge[i-1].timestamp;
        assert(std::abs(beforeSpacing - afterSpacing) < 0.0001);
    }
    
    std::cout << "✓ test_BeatGrid_NudgeAll passed" << std::endl;
}

// Test 6: BeatGrid - Phrase detection and clustering
void test_BeatGrid_PhraseDetection() {
    std::cout << "[TEST] BeatGrid_PhraseDetection..." << std::endl;
    
    const double bpm = 128.0;
    const double trackDuration = 120.0;  // 2 minutes
    const double firstBeatOffset = 0.0;
    
    BeatGrid grid;
    grid.generateFromBPM(bpm, firstBeatOffset, trackDuration);
    grid.markPhrases(16);  // 16-bar phrases
    
    std::vector<BeatMarker> beats = grid.getBeats();
    
    // At 128 BPM, in 120 seconds we have: 128/60 * 120 = 256 beats
    // 256 beats / 4 beats per bar = 64 bars
    // 64 bars / 16 bars per phrase = 4 phrases
    
    int phraseCount = 0;
    for (const auto& beat : beats) {
        if (beat.isPhraseStart) {
            phraseCount++;
        }
    }
    
    // Should have 4 phrase starts (including the initial one)
    assert(phraseCount >= 4 && phraseCount <= 5);
    
    // Verify all beats have valid bar positions (1-4)
    for (const auto& beat : beats) {
        assert(beat.barPosition >= 1 && beat.barPosition <= 4);
    }
    
    // Verify isDownbeat matches bar position 1
    for (const auto& beat : beats) {
        assert((beat.barPosition == 1) == beat.isDownbeat);
    }
    
    std::cout << "✓ test_BeatGrid_PhraseDetection passed (detected " << phraseCount << " phrases)" << std::endl;
}

// Test 7: BeatGridRenderer - ASCII visualization
void test_BeatGridRenderer_Visualization() {
    std::cout << "[TEST] BeatGridRenderer_Visualization..." << std::endl;
    
    BeatGrid grid;
    grid.generateFromBPM(120.0, 0.0, 10.0);
    
    BeatGridRenderer renderer;
    std::string output = renderer.render(grid, 80);  // 80 character width
    
    // Debug: print the actual output
    std::cout << "  Rendered output:\n" << output << std::endl;
    
    // Output should contain beat markers
    assert(output.find("|") != std::string::npos);
    
    // Output should contain bar position numbers (1-4)
    assert(output.find("1") != std::string::npos);
    assert(output.find("2") != std::string::npos);
    assert(output.find("3") != std::string::npos);
    assert(output.find("4") != std::string::npos);
    
    // Output should be non-empty and contain multiple lines
    assert(output.size() > 10);
    assert(output.find('\n') != std::string::npos);
    
    std::cout << "✓ test_BeatGridRenderer_Visualization passed" << std::endl;
}

// Test 8: TrackMetadata - Beat grid persistence (serialize/deserialize)
void test_TrackMetadata_BeatGridPersistence() {
    std::cout << "[TEST] TrackMetadata_BeatGridPersistence..." << std::endl;
    
    // Create BeatGridData
    BeatGridData gridData;
    gridData.firstBeatOffset = 0.5;
    gridData.bpm = 120.0;
    gridData.barsPerPhrase = 16;
    gridData.manualNudges = {0.0, 5.0, -3.0, 10.0};
    
    // Serialize
    std::string serialized = gridData.serialize();
    assert(serialized.size() > 0);
    
    // Check serialized data contains key information
    assert(serialized.find("120") != std::string::npos);  // BPM
    assert(serialized.find("0.5") != std::string::npos);  // first beat offset
    
    // Deserialize
    BeatGridData deserialized = BeatGridData::deserialize(serialized);
    
    // Verify all fields match
    assert(std::abs(deserialized.firstBeatOffset - 0.5) < 0.0001);
    assert(std::abs(deserialized.bpm - 120.0) < 0.0001);
    assert(deserialized.barsPerPhrase == 16);
    assert(deserialized.manualNudges.size() == 4);
    assert(std::abs(deserialized.manualNudges[0] - 0.0) < 0.0001);
    assert(std::abs(deserialized.manualNudges[1] - 5.0) < 0.0001);
    assert(std::abs(deserialized.manualNudges[2] - (-3.0)) < 0.0001);
    assert(std::abs(deserialized.manualNudges[3] - 10.0) < 0.0001);
    
    std::cout << "✓ test_TrackMetadata_BeatGridPersistence passed" << std::endl;
}

} // namespace dj

// Main test runner
int main() {
    std::cout << "========== Phase 32: Phrase Detection & Beat Grid Editor Tests ==========\n" << std::endl;
    
    try {
        dj::test_OnsetDetector_SpectralFlux();
        dj::test_OnsetDetector_NoiseRejection();
        dj::test_BeatGrid_GenerateFromBPM();
        dj::test_BeatGrid_ManualNudge();
        dj::test_BeatGrid_NudgeAll();
        dj::test_BeatGrid_PhraseDetection();
        dj::test_BeatGridRenderer_Visualization();
        dj::test_TrackMetadata_BeatGridPersistence();
        
        std::cout << "\n========== All 8 tests PASSED ==========\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n!!! Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
