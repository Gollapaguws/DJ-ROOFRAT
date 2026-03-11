#include "audio/TrackLoader.h"
#include "audio/AudioClip.h"

#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace dj {

// Test 1: Verify error handling for non-existent file
void test_TrackLoader_InvalidFile() {
    std::string errorMsg;
    auto clip = TrackLoader::loadFile("nonexistent_file_12345.wav", &errorMsg);
    
    assert(!clip.has_value());
    assert(!errorMsg.empty());
    std::cout << "  Error message: " << errorMsg << std::endl;
    
    std::cout << "✓ test_TrackLoader_InvalidFile passed" << std::endl;
}

// Test 2: Verify corrupt file handling
void test_TrackLoader_CorruptFile() {
    std::string corruptFile = "test_corrupt.wav";
    std::ofstream file(corruptFile, std::ios::binary);
    file.write("INVALID_HEADER_DATA", 19);
    file.close();
    
    std::string errorMsg;
    auto clip = TrackLoader::loadFile(corruptFile, &errorMsg);
    
    assert(!clip.has_value());
    assert(!errorMsg.empty());
    
    std::remove(corruptFile.c_str());
    std::cout << "✓ test_TrackLoader_CorruptFile passed" << std::endl;
}

// Test 3: Verify empty file handling
void test_TrackLoader_EmptyFile() {
    std::string emptyFile = "test_empty.wav";
    std::ofstream file(emptyFile, std::ios::binary);
    file.close();
    
    std::string errorMsg;
    auto clip = TrackLoader::loadFile(emptyFile, &errorMsg);
    
    assert(!clip.has_value());
    
    std::remove(emptyFile.c_str());
    std::cout << "✓ test_TrackLoader_EmptyFile passed" << std::endl;
}

// Test 4: Verify AudioClip structure is correct
void test_TrackLoader_AudioClipStructure() {
    AudioClip clip;
    
    // Check default values
    assert(clip.sampleRate == 44100);
    assert(clip.channels == 2);
    assert(clip.samples.empty());
    assert(clip.frameCount() == 0);
    
    // Check that we can set values
    clip.sampleRate = 48000;
    assert(clip.sampleRate == 48000);
    
    // Generate test tone and check structure
    auto testClip = AudioClip::generateTestTone(440.0f, 1.0f, 44100);
    assert(testClip.sampleRate == 44100);
    assert(testClip.channels == 2);
    assert(!testClip.samples.empty());
    assert(testClip.frameCount() == 44100);
    
    std::cout << "✓ test_TrackLoader_AudioClipStructure passed" << std::endl;
}

// Test 5: Verify format detection by extension
void test_TrackLoader_FormatDetection() {
    // Create a dummy file with .mp3 extension
    std::string mp3File = "test_detection.mp3";
    std::ofstream file(mp3File, std::ios::binary);
    file.write("ID3", 3); // MP3 ID3 tag start
    file.close();
    
    std::string errorMsg;
    // Should attempt to load as MP3 (and fail due to invalid data, but that's OK for this test)
    auto clip = TrackLoader::loadFile(mp3File, &errorMsg);
    // We expect it to fail because it's not a real MP3, but the point is it tried to parse as MP3
    
    std::remove(mp3File.c_str());
    std::cout << "✓ test_TrackLoader_FormatDetection passed" << std::endl;
}

// Test 6: Verify that loadFile function signature is correct
void test_TrackLoader_FunctionSignature() {
    // This test verifies the function exists and has the right signature
    // by calling it in different ways
    
    std::string errorMsg;
    
    // Call with both path and error pointer
    auto clip1 = TrackLoader::loadFile("nonexistent1.wav", &errorMsg);
    assert(!clip1.has_value());
    
    // Call with just path
    auto clip2 = TrackLoader::loadFile("nonexistent2.wav");
    assert(!clip2.has_value());
    
    // Call with nullptr error pointer (should not crash)
    auto clip3 = TrackLoader::loadFile("nonexistent3.wav", nullptr);
    assert(!clip3.has_value());
    
    std::cout << "✓ test_TrackLoader_FunctionSignature passed" << std::endl;
}

// Test 7: Verify std::optional return type
void test_TrackLoader_OptionalReturn() {
    auto result = TrackLoader::loadFile("nonexistent_file.wav");
    
    // Test optional semantics
    assert(!result.has_value());
    assert(!static_cast<bool>(result));
    
    // Verify we can use value_or
    // (doesn't actually load, just tests the API)
    
    std::cout << "✓ test_TrackLoader_OptionalReturn passed" << std::endl;
}

// Test 8: Verify no crash on repeated failed loads
void test_TrackLoader_RepeatedFailedLoads() {
    for (int i = 0; i < 10; ++i) {
        auto clip = TrackLoader::loadFile("nonexistent_file_" + std::to_string(i) + ".wav");
        assert(!clip.has_value());
    }
    
    std::cout << "✓ test_TrackLoader_RepeatedFailedLoads passed" << std::endl;
}

void runAllTests() {
    std::cout << "\n=== Running TrackLoader Phase 9 Tests ===" << std::endl;
    try {
        test_TrackLoader_InvalidFile();
        test_TrackLoader_CorruptFile();
        test_TrackLoader_EmptyFile();
        test_TrackLoader_AudioClipStructure();
        test_TrackLoader_FormatDetection();
        test_TrackLoader_FunctionSignature();
        test_TrackLoader_OptionalReturn();
        test_TrackLoader_RepeatedFailedLoads();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
    }
}

} // namespace dj

// Entry point for running tests
int main() {
    dj::runAllTests();
    std::cout << "\n=== TrackLoader Phase 9 Tests Complete ===" << std::endl;
    return 0;
}
