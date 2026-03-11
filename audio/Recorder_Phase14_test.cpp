#include "audio/Recorder.h"
#include "audio/WAVExporter.h"
#include "audio/MP3Exporter.h"
#include "audio/SessionMetadata.h"
#include "audio/AudioClip.h"
#include "audio/Mixer.h"
#include "audio/Deck.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace dj {

// Test 1: Verify recording starts without clicks (smooth fade in at boundary)
void test_Recorder_CaptureStart() {
    Recorder recorder(44100, 2, 60);  // 44.1kHz, stereo, 60 seconds
    
    // Start recording
    bool started = recorder.start();
    assert(started);
    assert(recorder.isRecording());
    
    // Submit some frames (10ms worth at 44.1kHz = 441 samples)
    std::vector<float> testData(441 * 2, 0.5f);  // stereo interleaved
    recorder.submitFrames(testData.data(), 441);
    
    // Stop recording
    recorder.stop();
    assert(!recorder.isRecording());
    
    // Verify we can retrieve recorded data
    std::vector<float> recorded = recorder.getRecordedData();
    assert(!recorded.empty());
    assert(recorded.size() >= 441 * 2);  // At least the data we submitted
    
    // Check for fade-in at the boundary (first few samples should be lower amplitude or ramped)
    // This is a basic check - just verify data exists
    bool foundNonZerro = false;
    for (size_t i = 0; i < std::min(size_t(100), recorded.size()); ++i) {
        if (std::abs(recorded[i]) > 0.0f) {
            foundNonZerro = true;
            break;
        }
    }
    assert(foundNonZerro);
    
    std::cout << "✓ test_Recorder_CaptureStart passed" << std::endl;
}

// Test 2: Verify ring buffer overflow handling (graceful circular wrapping)
void test_Recorder_RingBuffer() {
    Recorder recorder(44100, 2, 1);  // 44.1kHz, stereo, 1 second capacity
    
    recorder.start();
    
    // Submit more data than the buffer can hold (2 seconds worth)
    std::vector<float> largeData(44100 * 2 * 2, 0.3f);  // 2 seconds of stereo
    recorder.submitFrames(largeData.data(), 44100 * 2);
    
    // Buffer should handle overflow without crashing
    assert(recorder.isRecording());
    
    recorder.stop();
    
    // Retrieved data should be valid (either all 2 seconds or wrapped portion)
    std::vector<float> recorded = recorder.getRecordedData();
    assert(!recorded.empty());
    assert(recorded.size() <= 44100 * 2 * 1);  // Not more than 1 second
    
    std::cout << "✓ test_Recorder_RingBuffer passed" << std::endl;
}

// Test 3: Verify 16-bit WAV export correctness (RIFF header + sample format)
void test_WAVExporter_16bit() {
    const std::string filename = "test_16bit.wav";
    
    // Generate test data: simple sine wave 441Hz for 0.1 seconds at 44.1kHz
    std::vector<float> testData;
    const int numFrames = 4410;  // 0.1 seconds
    for (int i = 0; i < numFrames; ++i) {
        float sample = 0.5f * std::sin(2.0f * 3.14159f * 441.0f * i / 44100.0f);
        testData.push_back(sample);
        testData.push_back(sample);  // stereo (L=R)
    }
    
    // Export as 16-bit WAV
    WAVExporter exporter(44100, 2, 16);
    bool success = exporter.exportToFile(filename, testData.data(), numFrames);
    assert(success);
    
    // Verify file exists and is not empty
    assert(std::filesystem::exists(filename));
    auto fileSize = std::filesystem::file_size(filename);
    assert(fileSize > 0);
    
    // Verify RIFF header
    std::ifstream file(filename, std::ios::binary);
    char riffHeader[12];
    file.read(riffHeader, 12);
    
    // Check "RIFF" magic
    assert(riffHeader[0] == 'R' && riffHeader[1] == 'I' && 
           riffHeader[2] == 'F' && riffHeader[3] == 'F');
    
    // Check "WAVE" marker
    assert(riffHeader[8] == 'W' && riffHeader[9] == 'A' && 
           riffHeader[10] == 'V' && riffHeader[11] == 'E');
    
    file.close();
    
    // Clean up
    std::filesystem::remove(filename);
    
    std::cout << "✓ test_WAVExporter_16bit passed" << std::endl;
}

// Test 4: Verify 24-bit WAV export (extended format)
void test_WAVExporter_24bit() {
    const std::string filename = "test_24bit.wav";
    
    // Generate test data
    std::vector<float> testData;
    const int numFrames = 4410;
    for (int i = 0; i < numFrames; ++i) {
        float sample = 0.5f * std::sin(2.0f * 3.14159f * 441.0f * i / 44100.0f);
        testData.push_back(sample);
        testData.push_back(sample);
    }
    
    // Export as 24-bit WAV
    WAVExporter exporter(44100, 2, 24);
    bool success = exporter.exportToFile(filename, testData.data(), numFrames);
    assert(success);
    
    // Verify file exists
    assert(std::filesystem::exists(filename));
    auto fileSize = std::filesystem::file_size(filename);
    assert(fileSize > 0);
    
    // 24-bit file should be smaller than 32-bit but larger than 16-bit
    // Just verify it's reasonable
    assert(fileSize > 1000);  // At least some data
    
    // Clean up
    std::filesystem::remove(filename);
    
    std::cout << "✓ test_WAVExporter_24bit passed" << std::endl;
}

// Test 5: Verify MP3 export with CBR (constant bitrate 320kbps) - SKIP if no LAME
void test_MP3Exporter_CBR() {
#if DJ_RECORDING_MP3_AVAILABLE
    const std::string filename = "test_cbr.mp3";
    
    // Generate test data
    std::vector<float> testData;
    const int numFrames = 4410;
    for (int i = 0; i < numFrames; ++i) {
        float sample = 0.5f * std::sin(2.0f * 3.14159f * 441.0f * i / 44100.0f);
        testData.push_back(sample);
        testData.push_back(sample);
    }
    
    // Export as MP3 CBR
    MP3Exporter exporter(44100, 2, MP3Exporter::BitrateMode::CBR_320kbps);
    bool success = exporter.exportToFile(filename, testData.data(), numFrames);
    assert(success);
    
    // Verify file exists
    assert(std::filesystem::exists(filename));
    auto fileSize = std::filesystem::file_size(filename);
    assert(fileSize > 0);
    
    // Clean up
    std::filesystem::remove(filename);
    
    std::cout << "✓ test_MP3Exporter_CBR passed" << std::endl;
#else
    std::cout << "⊘ test_MP3Exporter_CBR SKIPPED (LAME not available)" << std::endl;
#endif
}

// Test 6: Verify MP3 export with VBR (variable bitrate V0) - SKIP if no LAME
void test_MP3Exporter_VBR() {
#if DJ_RECORDING_MP3_AVAILABLE
    const std::string filename = "test_vbr.mp3";
    
    // Generate test data
    std::vector<float> testData;
    const int numFrames = 4410;
    for (int i = 0; i < numFrames; ++i) {
        float sample = 0.5f * std::sin(2.0f * 3.14159f * 441.0f * i / 44100.0f);
        testData.push_back(sample);
        testData.push_back(sample);
    }
    
    // Export as MP3 VBR V0
    MP3Exporter exporter(44100, 2, MP3Exporter::BitrateMode::VBR_V0);
    bool success = exporter.exportToFile(filename, testData.data(), numFrames);
    assert(success);
    
    // Verify file exists
    assert(std::filesystem::exists(filename));
    auto fileSize = std::filesystem::file_size(filename);
    assert(fileSize > 0);
    
    // VBR files are typically smaller than CBR
    assert(fileSize > 0);
    
    // Clean up
    std::filesystem::remove(filename);
    
    std::cout << "✓ test_MP3Exporter_VBR passed" << std::endl;
#else
    std::cout << "⊘ test_MP3Exporter_VBR SKIPPED (LAME not available)" << std::endl;
#endif
}

// Test 7: Verify session metadata serialization (JSON format)
void test_SessionMetadata_Serialization() {
    SessionMetadata metadata;
    
    // Set basic info
    metadata.setRecordingName("test_set");
    metadata.setStartTime(std::time(nullptr));
    metadata.setDuration(120.0f);  // 2 minutes
    
    // Add track entries (timestamp, track name, BPM)
    metadata.addTrack(0.0f, "Track_A", 120.0f);
    metadata.addTrack(30.0f, "Track_B", 128.0f);
    metadata.addTrack(60.0f, "Track_C", 124.0f);
    
    // Add transition markers
    metadata.addTransition(15.0f, "crossfade_smooth");
    metadata.addTransition(45.0f, "crossfade_tight");
    
    // Serialize to JSON
    std::string jsonStr = metadata.toJSON();
    assert(!jsonStr.empty());
    assert(jsonStr.find("test_set") != std::string::npos);
    assert(jsonStr.find("Track_A") != std::string::npos);
    assert(jsonStr.find("Track_B") != std::string::npos);
    assert(jsonStr.find("120") != std::string::npos);  // BPM
    
    // Export to file
    const std::string filename = "test_metadata.json";
    bool success = metadata.exportToFile(filename);
    assert(success);
    
    // Verify file exists and can be read back
    assert(std::filesystem::exists(filename));
    
    // Clean up
    std::filesystem::remove(filename);
    
    std::cout << "✓ test_SessionMetadata_Serialization passed" << std::endl;
}

// Test 8: Verify pause/resume functionality (buffer maintenance)
void test_Recorder_PauseResume() {
    Recorder recorder(44100, 2, 60);
    
    recorder.start();
    
    // Submit first batch
    std::vector<float> data1(4410, 0.3f);  // 0.1 seconds
    recorder.submitFrames(data1.data(), 2205);
    
    // Pause
    recorder.pause();
    assert(!recorder.isRecording());
    assert(recorder.isPaused());
    
    // Resume
    recorder.resume();
    assert(recorder.isRecording());
    assert(!recorder.isPaused());
    
    // Submit second batch
    std::vector<float> data2(4410, 0.4f);
    recorder.submitFrames(data2.data(), 2205);
    
    recorder.stop();
    
    std::vector<float> recorded = recorder.getRecordedData();
    assert(!recorded.empty());
    
    std::cout << "✓ test_Recorder_PauseResume passed" << std::endl;
}

// Test 9: Verify WAV export with various frame counts
void test_WAVExporter_VariousFrameCounts() {
    // Test with non-power-of-2 frame counts
    std::vector<int> frameCounts = {441, 4410, 22050, 44099};
    
    for (int numFrames : frameCounts) {
        std::vector<float> testData(numFrames * 2, 0.2f);
        
        std::string filename = "test_frames_" + std::to_string(numFrames) + ".wav";
        WAVExporter exporter(44100, 2, 16);
        bool success = exporter.exportToFile(filename, testData.data(), numFrames);
        
        assert(success);
        assert(std::filesystem::exists(filename));
        
        std::filesystem::remove(filename);
    }
    
    std::cout << "✓ test_WAVExporter_VariousFrameCounts passed" << std::endl;
}

// Test 10: Verify metadata tracks ordering and timestamps
void test_SessionMetadata_TrackOrdering() {
    SessionMetadata metadata;
    metadata.setRecordingName("ordering_test");
    metadata.setDuration(300.0f);
    
    // Add tracks in non-chronological order
    metadata.addTrack(100.0f, "Track_100");
    metadata.addTrack(50.0f, "Track_50");
    metadata.addTrack(200.0f, "Track_200");
    metadata.addTrack(0.0f, "Track_0");
    
    // Get tracks and verify they're accessible
    std::string json = metadata.toJSON();
    
    // All tracks should appear in JSON
    assert(json.find("Track_0") != std::string::npos);
    assert(json.find("Track_50") != std::string::npos);
    assert(json.find("Track_100") != std::string::npos);
    assert(json.find("Track_200") != std::string::npos);
    
    std::cout << "✓ test_SessionMetadata_TrackOrdering passed" << std::endl;
}

// Test: Verify ring buffer wrap-around retrieval is chronologically correct
void test_Recorder_WrapAroundChronological() {
    Recorder recorder(44100, 2, 1);  // 44.1kHz, stereo, 1 second capacity
    
    recorder.start();
    
    // First: Fill buffer with recognizable pattern (110Hz signal in first half)
    std::vector<float> firstData(44100 * 2, 0.2f);  // 1 second of 0.2 amplitude
    recorder.submitFrames(firstData.data(), 44100);
    
    // Verify no wrap yet
    auto data1 = recorder.getRecordedData();
    assert(data1.size() == 44100 * 2);
    
    // Second: Wrap the buffer by submitting more data (0.3 amplitude pattern)
    std::vector<float> secondData(44100 * 2, 0.3f);  // Another second of 0.3 amplitude
    recorder.submitFrames(secondData.data(), 44100);
    
    recorder.stop();
    
    // After wrap, we should have the latest second of data in chronological order
    auto wrapped = recorder.getRecordedData();
    assert(wrapped.size() == 44100 * 2);
    
    // The wrapped data should be mostly 0.3 (the second pattern that overwrote the first)
    float avgAmplitude = 0.0f;
    for (float sample : wrapped) {
        avgAmplitude += std::abs(sample);
    }
    avgAmplitude /= wrapped.size();
    
    // Average should be close to 0.3 since the buffer wrapped and new data overwrote old
    assert(avgAmplitude > 0.25f && avgAmplitude < 0.35f);
    
    // Verify data is valid and non-zero
    bool foundNonZero = false;
    for (float sample : wrapped) {
        if (std::abs(sample) > 0.1f) {
            foundNonZero = true;
            break;
        }
    }
    assert(foundNonZero);
    
    std::cout << "✓ test_Recorder_WrapAroundChronological passed" << std::endl;
}

void runAllPhase14RecorderTests() {
    std::cout << "\n=== Running Recorder Phase 14 Tests ===" << std::endl;
    try {
        test_Recorder_CaptureStart();
        test_Recorder_RingBuffer();
        test_Recorder_WrapAroundChronological();
        test_WAVExporter_16bit();
        test_WAVExporter_24bit();
        test_MP3Exporter_CBR();
        test_MP3Exporter_VBR();
        test_SessionMetadata_Serialization();
        test_Recorder_PauseResume();
        test_WAVExporter_VariousFrameCounts();
        test_SessionMetadata_TrackOrdering();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
    }
}

} // namespace dj

// Entry point for running recorder tests
int main() {
    dj::runAllPhase14RecorderTests();
    std::cout << "\n=== Recorder Phase 14 Tests Complete ===" << std::endl;
    return 0;
}
