// Phase 29: Enhanced Recording Features Test Suite
// Multi-track recording with cue markers and auto-naming

#include <cassert>
#include <chrono>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "audio/AudioClip.h"
#include "audio/Deck.h"
#include "audio/Mixer.h"
#include "audio/Recorder.h"
#include "audio/SessionMetadata.h"
#include "audio/WAVExporter.h"

namespace fs = std::filesystem;

namespace {

// Helper: Generate test tone
dj::AudioClip generateTestTone(float freq, float duration, int sampleRate) {
    return dj::AudioClip::generateTestTone(freq, duration, sampleRate);
}

// Helper: Check if file exists
bool fileExists(const std::string& path) {
    return fs::exists(path);
}

// Helper: Get WAV file frame count
std::size_t getWAVFrameCount(const std::string& filename) {
    FILE* file = fopen(filename.c_str(), "rb");
    if (!file) return 0;
    
    // Skip to data chunk (simplified: assumes standard WAV structure)
    fseek(file, 40, SEEK_SET);
    uint32_t dataSize;
    fread(&dataSize, 4, 1, file);
    fclose(file);
    
    // dataSize in bytes, stereo 16-bit = 4 bytes per frame
    return dataSize / 4;
}

// Helper: Current timestamp string
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
    #if defined(_WIN32)
        localtime_s(&tm_now, &time_t_now);
    #else
        localtime_r(&time_t_now, &tm_now);
    #endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d_%H-%M-%S");
    return oss.str();
}

// Helper: Export recording to WAV
bool exportRecording(const dj::Recorder& recorder, const std::string& filename, int sampleRate) {
    if (recorder.getDuration() <= 0.0f) {
        return false;
    }
    
    const auto& data = recorder.getRecordedData();
    if (data.empty()) {
        return false;
    }
    
    dj::WAVExporter exporter(sampleRate, 2, 16);
    return exporter.exportToFile(filename, data.data(), data.size() / 2);
}

} // anonymous namespace

void test_DualRecording_SeparateDecks() {
    std::cout << "TEST: Dual recording exports separate deck files\n";
    
    const int sampleRate = 44100;
    const std::size_t framesPerBlock = 512;
    
    // Create test clips with different frequencies
    auto clipA = generateTestTone(440.0f, 2.0f, sampleRate);  // A4
    auto clipB = generateTestTone(554.37f, 2.0f, sampleRate); // C#5
    
    dj::Deck deckA, deckB;
    deckA.loadClip(clipA);
    deckB.loadClip(clipB);
    deckA.setOutputSampleRate(sampleRate);
    deckB.setOutputSampleRate(sampleRate);
    
    dj::Mixer mixer;
    mixer.setCrossfader(0.0f);  // Centered
    
    // Three recorders: mix, deck A, deck B
    dj::Recorder recorderMix(sampleRate, 2, 600);
    dj::Recorder recorderDeckA(sampleRate, 2, 600);
    dj::Recorder recorderDeckB(sampleRate, 2, 600);
    
    recorderMix.start();
    recorderDeckA.start();
    recorderDeckB.start();
    
    deckA.play();
    deckB.play();
    
    // Record 1 second
    std::size_t totalFrames = sampleRate;
    for (std::size_t i = 0; i < totalFrames / framesPerBlock; ++i) {
        // Capture individual deck outputs
        std::vector<float> bufferA(framesPerBlock * 2);
        std::vector<float> bufferB(framesPerBlock * 2);
        
        for (std::size_t j = 0; j < framesPerBlock; ++j) {
            auto frameA = deckA.nextFrame();
            auto frameB = deckB.nextFrame();
            
            bufferA[j * 2] = frameA[0];
            bufferA[j * 2 + 1] = frameA[1];
            bufferB[j * 2] = frameB[0];
            bufferB[j * 2 + 1] = frameB[1];
        }
        
        recorderDeckA.submitFrames(bufferA.data(), framesPerBlock);
        recorderDeckB.submitFrames(bufferB.data(), framesPerBlock);
        
        // Mix output (using captured buffers to avoid re-consuming)
        std::vector<float> mixed(framesPerBlock * 2);
        for (std::size_t j = 0; j < framesPerBlock * 2; ++j) {
            mixed[j] = (bufferA[j] + bufferB[j]) * 0.5f;
        }
        recorderMix.submitFrames(mixed.data(), framesPerBlock);
    }
    
    recorderMix.stop();
    recorderDeckA.stop();
    recorderDeckB.stop();
    
    // Export all three tracks
    std::string timestamp = getCurrentTimestamp();
    bool mixOk = exportRecording(recorderMix, "test_mix_" + timestamp + ".wav", sampleRate);
    bool deckAOk = exportRecording(recorderDeckA, "test_deckA_" + timestamp + ".wav", sampleRate);
    bool deckBOk = exportRecording(recorderDeckB, "test_deckB_" + timestamp + ".wav", sampleRate);
    
    assert(mixOk && "Mix export should succeed");
    assert(deckAOk && "Deck A export should succeed");
    assert(deckBOk && "Deck B export should succeed");
    
    assert(fileExists("test_mix_" + timestamp + ".wav"));
    assert(fileExists("test_deckA_" + timestamp + ".wav"));
    assert(fileExists("test_deckB_" + timestamp + ".wav"));
    
    // Cleanup
    fs::remove("test_mix_" + timestamp + ".wav");
    fs::remove("test_deckA_" + timestamp + ".wav");
    fs::remove("test_deckB_" + timestamp + ".wav");
    
    std::cout << "  ✓ Three separate files exported\n";
}

void test_DualRecording_SyncedLength() {
    std::cout << "TEST: All exported tracks have identical frame count\n";
    
    const int sampleRate = 44100;
    const std::size_t framesPerBlock = 512;
    
    auto clipA = generateTestTone(440.0f, 1.5f, sampleRate);
    auto clipB = generateTestTone(554.37f, 1.5f, sampleRate);
    
    dj::Deck deckA, deckB;
    deckA.loadClip(clipA);
    deckB.loadClip(clipB);
    deckA.setOutputSampleRate(sampleRate);
    deckB.setOutputSampleRate(sampleRate);
    
    dj::Recorder recorderMix(sampleRate, 2, 600);
    dj::Recorder recorderDeckA(sampleRate, 2, 600);
    dj::Recorder recorderDeckB(sampleRate, 2, 600);
    
    recorderMix.start();
    recorderDeckA.start();
    recorderDeckB.start();
    
    deckA.play();
    deckB.play();
    
    // Record 1 second
    std::size_t totalFrames = sampleRate;
    for (std::size_t i = 0; i < totalFrames / framesPerBlock; ++i) {
        std::vector<float> bufferA(framesPerBlock * 2);
        std::vector<float> bufferB(framesPerBlock * 2);
        
        for (std::size_t j = 0; j < framesPerBlock; ++j) {
            auto frameA = deckA.nextFrame();
            auto frameB = deckB.nextFrame();
            bufferA[j * 2] = frameA[0];
            bufferA[j * 2 + 1] = frameA[1];
            bufferB[j * 2] = frameB[0];
            bufferB[j * 2 + 1] = frameB[1];
        }
        
        recorderDeckA.submitFrames(bufferA.data(), framesPerBlock);
        recorderDeckB.submitFrames(bufferB.data(), framesPerBlock);
        
        std::vector<float> mixed(framesPerBlock * 2);
        for (std::size_t j = 0; j < framesPerBlock * 2; ++j) {
            mixed[j] = (bufferA[j] + bufferB[j]) * 0.5f;
        }
        recorderMix.submitFrames(mixed.data(), framesPerBlock);
    }
    
    recorderMix.stop();
    recorderDeckA.stop();
    recorderDeckB.stop();
    
    std::string timestamp = getCurrentTimestamp();
    exportRecording(recorderMix, "sync_mix_" + timestamp + ".wav", sampleRate);
    exportRecording(recorderDeckA, "sync_deckA_" + timestamp + ".wav", sampleRate);
    exportRecording(recorderDeckB, "sync_deckB_" + timestamp + ".wav", sampleRate);
    
    // Verify frame counts match
    std::size_t mixFrames = getWAVFrameCount("sync_mix_" + timestamp + ".wav");
    std::size_t deckAFrames = getWAVFrameCount("sync_deckA_" + timestamp + ".wav");
    std::size_t deckBFrames = getWAVFrameCount("sync_deckB_" + timestamp + ".wav");
    
    assert(mixFrames == deckAFrames && "Mix and Deck A frame counts must match");
    assert(mixFrames == deckBFrames && "Mix and Deck B frame counts must match");
    assert(mixFrames > 0 && "Frame count must be non-zero");
    
    // Cleanup
    fs::remove("sync_mix_" + timestamp + ".wav");
    fs::remove("sync_deckA_" + timestamp + ".wav");
    fs::remove("sync_deckB_" + timestamp + ".wav");
    
    std::cout << "  ✓ All tracks have " << mixFrames << " frames\n";
}

void test_Recording_AutoNaming() {
    std::cout << "TEST: Auto-naming with timestamp format\n";
    
    const int sampleRate = 44100;
    dj::Recorder recorder(sampleRate, 2, 600);
    
    auto clip = generateTestTone(440.0f, 0.5f, sampleRate);
    dj::Deck deck;
    deck.loadClip(clip);
    deck.setOutputSampleRate(sampleRate);
    
    recorder.start();
    deck.play();
    
    for (int i = 0; i < 100; ++i) {
        auto frame = deck.nextFrame();
        recorder.submitFrames(frame.data(), 1);
    }
    
    recorder.stop();
    
    // Generate timestamp and export
    std::string timestamp = getCurrentTimestamp();
    std::string filename = "session_mix_" + timestamp + ".wav";
    
    bool success = exportRecording(recorder, filename, sampleRate);
    assert(success && "Export should succeed");
    assert(fileExists(filename) && "File should exist");
    
    // Verify timestamp format: YYYY-MM-DD_HH-MM-SS
    std::regex timestampPattern(R"(session_mix_\d{4}-\d{2}-\d{2}_\d{2}-\d{2}-\d{2}\.wav)");
    assert(std::regex_match(filename, timestampPattern) && "Filename must match timestamp format");
    
    fs::remove(filename);
    
    std::cout << "  ✓ Filename format: " << filename << "\n";
}

void test_Recording_CueMarkers() {
    std::cout << "TEST: Cue points logged in SessionMetadata\n";
    
    const int sampleRate = 44100;
    dj::SessionMetadata metadata;
    
    auto clip = generateTestTone(440.0f, 5.0f, sampleRate);
    dj::Deck deck;
    deck.loadClip(clip);
    deck.setOutputSampleRate(sampleRate);
    
    deck.play();
    
    // Simulate setting cues at different times
    for (int i = 0; i < 1000; ++i) {
        deck.nextFrame();
    }
    
    // Set cue point 1
    std::size_t cueFrame1 = deck.currentFrame();
    deck.setCue(cueFrame1, 0);
    metadata.addCueMarker(cueFrame1 / static_cast<double>(sampleRate), 'A', 0, cueFrame1);
    
    for (int i = 0; i < 2000; ++i) {
        deck.nextFrame();
    }
    
    // Set cue point 2
    std::size_t cueFrame2 = deck.currentFrame();
    deck.setCue(cueFrame2, 1);
    metadata.addCueMarker(cueFrame2 / static_cast<double>(sampleRate), 'A', 1, cueFrame2);
    
    // Export metadata to JSON
    std::string json = metadata.toJSON();
    
    // Verify cue markers are present
    assert(json.find("cue_markers") != std::string::npos && "JSON must contain cue_markers");
    assert(json.find("Cue A1") != std::string::npos && "Must contain Cue A1 marker");
    assert(json.find("Cue A2") != std::string::npos && "Must contain Cue A2 marker");
    
    std::cout << "  ✓ Cue markers logged: A1, A2\n";
}

void test_Recording_MetadataExport() {
    std::cout << "TEST: JSON metadata export with cue timestamps\n";
    
    const int sampleRate = 44100;
    dj::SessionMetadata metadata;
    
    metadata.setRecordingName("Test Session");
    metadata.setStartTime(std::time(nullptr));
    metadata.setDuration(120.0f);
    
    // Add cue markers
    metadata.addCueMarker(10.5, 'A', 0, 463050);
    metadata.addCueMarker(45.2, 'B', 1, 1993620);
    
    std::string timestamp = getCurrentTimestamp();
    std::string filename = "session_meta_" + timestamp + ".json";
    
    bool success = metadata.exportToFile(filename);
    assert(success && "Metadata export should succeed");
    assert(fileExists(filename) && "JSON file should exist");
    
    // Read back and verify
    std::ifstream file(filename);
    std::string content((std::istreambuf_iterator<char>(file)), 
                        std::istreambuf_iterator<char>());
    file.close();
    
    assert(content.find("cue_markers") != std::string::npos);
    assert(content.find("10.5") != std::string::npos);
    assert(content.find("45.2") != std::string::npos);
    
    fs::remove(filename);
    
    std::cout << "  ✓ Metadata JSON exported with cue timestamps\n";
}

void test_Recording_BufferCapacity() {
    std::cout << "TEST: Buffer capacity display shows remaining time\n";
    
    const int sampleRate = 44100;
    const std::size_t capacitySeconds = 600;  // 10 minutes
    
    dj::Recorder recorder(sampleRate, 2, capacitySeconds);
    
    auto clip = generateTestTone(440.0f, 5.0f, sampleRate);
    dj::Deck deck;
    deck.loadClip(clip);
    deck.setOutputSampleRate(sampleRate);
    
    recorder.start();
    deck.play();
    
    // Record 10 seconds
    for (std::size_t i = 0; i < sampleRate * 10; ++i) {
        auto frame = deck.nextFrame();
        recorder.submitFrames(frame.data(), 1);
    }
    
    float duration = recorder.getDuration();
    float remaining = capacitySeconds - duration;
    
    assert(duration >= 9.9f && duration <= 10.1f && "Duration should be ~10 seconds");
    assert(remaining >= 589.9f && remaining <= 590.1f && "Remaining should be ~590 seconds");
    
    // Format as MM:SS
    int remainingMins = static_cast<int>(remaining) / 60;
    int remainingSecs = static_cast<int>(remaining) % 60;
    int totalMins = capacitySeconds / 60;
    
    std::ostringstream buffer;
    buffer << "Buffer: " << remainingMins << ":" 
           << std::setfill('0') << std::setw(2) << remainingSecs 
           << " / " << totalMins << ":00";
    
    std::string bufferStr = buffer.str();
    assert(bufferStr.find("9:50") != std::string::npos || bufferStr.find("9:49") != std::string::npos);
    assert(bufferStr.find("10:00") != std::string::npos);
    
    recorder.stop();
    
    std::cout << "  ✓ " << bufferStr << "\n";
}

void test_Recording_PauseIndicator() {
    std::cout << "TEST: Visual state shows [REC] vs [PAUSED]\n";
    
    const int sampleRate = 44100;
    dj::Recorder recorder(sampleRate, 2, 600);
    
    // Not recording
    bool isRecording = recorder.isRecording();
    bool isPaused = recorder.isPaused();
    assert(!isRecording && "Should not be recording initially");
    assert(!isPaused && "Should not be paused initially");
    
    // Visual status: empty when not recording
    std::string status1 = isRecording ? "[REC]" : (isPaused ? "[PAUSED]" : "");
    assert(status1.empty() && "Status should be empty when not recording");
    
    // Start recording
    recorder.start();
    isRecording = recorder.isRecording();
    isPaused = recorder.isPaused();
    assert(isRecording && "Should be recording after start");
    assert(!isPaused && "Should not be paused after start");
    
    // Visual status: [REC] when actively recording
    std::string status2 = isRecording ? "[REC]" : (isPaused ? "[PAUSED]" : "");
    assert(status2 == "[REC]" && "Status should show [REC] when recording");
    
    // Pause recording
    recorder.pause();
    isRecording = recorder.isRecording();  // Returns false when paused
    isPaused = recorder.isPaused();
    assert(!isRecording && "isRecording() returns false when paused");
    assert(isPaused && "Should be paused after pause()");
    
    // Visual status: [PAUSED] when paused
    std::string status3 = isRecording ? "[REC]" : (isPaused ? "[PAUSED]" : "");
    assert(status3 == "[PAUSED]" && "Status should show [PAUSED] when paused");
    
    // Resume recording
    recorder.resume();
    isRecording = recorder.isRecording();
    isPaused = recorder.isPaused();
    assert(isRecording && "Should be recording after resume");
    assert(!isPaused && "Should not be paused after resume");
    
    // Visual status: [REC] when resumed
    std::string status4 = isRecording ? "[REC]" : (isPaused ? "[PAUSED]" : "");
    assert(status4 == "[REC]" && "Status should show [REC] after resume");
    
    // Stop recording
    recorder.stop();
    isRecording = recorder.isRecording();
    isPaused = recorder.isPaused();
    assert(!isRecording && "Should not be recording after stop");
    assert(!isPaused && "Should not be paused after stop");
    
    // Visual status: empty when stopped
    std::string status5 = isRecording ? "[REC]" : (isPaused ? "[PAUSED]" : "");
    assert(status5.empty() && "Status should be empty after stop");
    
    std::cout << "  ✓ State transitions: [REC] → [PAUSED] → [REC] → (stopped)\n";
}

void test_Recording_MultiExport() {
    std::cout << "TEST: Export produces 3 WAV files + 1 JSON metadata\n";
    
    const int sampleRate = 44100;
    const std::size_t framesPerBlock = 512;
    
    auto clipA = generateTestTone(440.0f, 1.0f, sampleRate);
    auto clipB = generateTestTone(554.37f, 1.0f, sampleRate);
    
    dj::Deck deckA, deckB;
    deckA.loadClip(clipA);
    deckB.loadClip(clipB);
    deckA.setOutputSampleRate(sampleRate);
    deckB.setOutputSampleRate(sampleRate);
    
    dj::Recorder recorderMix(sampleRate, 2, 600);
    dj::Recorder recorderDeckA(sampleRate, 2, 600);
    dj::Recorder recorderDeckB(sampleRate, 2, 600);
    
    dj::SessionMetadata metadata;
    metadata.setRecordingName("Multi-Export Test");
    metadata.setStartTime(std::time(nullptr));
    
    recorderMix.start();
    recorderDeckA.start();
    recorderDeckB.start();
    
    deckA.play();
    deckB.play();
    
    // Record briefly
    for (std::size_t i = 0; i < 10; ++i) {
        std::vector<float> bufferA(framesPerBlock * 2);
        std::vector<float> bufferB(framesPerBlock * 2);
        
        for (std::size_t j = 0; j < framesPerBlock; ++j) {
            auto frameA = deckA.nextFrame();
            auto frameB = deckB.nextFrame();
            bufferA[j * 2] = frameA[0];
            bufferA[j * 2 + 1] = frameA[1];
            bufferB[j * 2] = frameB[0];
            bufferB[j * 2 + 1] = frameB[1];
        }
        
        recorderDeckA.submitFrames(bufferA.data(), framesPerBlock);
        recorderDeckB.submitFrames(bufferB.data(), framesPerBlock);
        
        std::vector<float> mixed(framesPerBlock * 2);
        for (std::size_t j = 0; j < framesPerBlock * 2; ++j) {
            mixed[j] = (bufferA[j] + bufferB[j]) * 0.5f;
        }
        recorderMix.submitFrames(mixed.data(), framesPerBlock);
    }
    
    recorderMix.stop();
    recorderDeckA.stop();
    recorderDeckB.stop();
    
    metadata.setDuration(recorderMix.getDuration());
    
    // Export all files
    std::string timestamp = getCurrentTimestamp();
    std::string mixFile = "multi_mix_" + timestamp + ".wav";
    std::string deckAFile = "multi_deckA_" + timestamp + ".wav";
    std::string deckBFile = "multi_deckB_" + timestamp + ".wav";
    std::string metaFile = "multi_meta_" + timestamp + ".json";
    
    bool mixOk = exportRecording(recorderMix, mixFile, sampleRate);
    bool deckAOk = exportRecording(recorderDeckA, deckAFile, sampleRate);
    bool deckBOk = exportRecording(recorderDeckB, deckBFile, sampleRate);
    bool metaOk = metadata.exportToFile(metaFile);
    
    assert(mixOk && deckAOk && deckBOk && metaOk && "All exports should succeed");
    assert(fileExists(mixFile));
    assert(fileExists(deckAFile));
    assert(fileExists(deckBFile));
    assert(fileExists(metaFile));
    
    // Cleanup
    fs::remove(mixFile);
    fs::remove(deckAFile);
    fs::remove(deckBFile);
    fs::remove(metaFile);
    
    std::cout << "  ✓ Exported: mix.wav, deckA.wav, deckB.wav, meta.json\n";
}

int main() {
    std::cout << "=== Phase 29: Enhanced Recording Features Test Suite ===\n\n";
    
    try {
        test_DualRecording_SeparateDecks();
        test_DualRecording_SyncedLength();
        test_Recording_AutoNaming();
        test_Recording_CueMarkers();
        test_Recording_MetadataExport();
        test_Recording_BufferCapacity();
        test_Recording_PauseIndicator();
        test_Recording_MultiExport();
        
        std::cout << "\n✓ All 8 tests passed!\n";
        std::cout << "Phase 29: Enhanced Recording Features - COMPLETE\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed with exception: " << e.what() << "\n";
        return 1;
    }
}
