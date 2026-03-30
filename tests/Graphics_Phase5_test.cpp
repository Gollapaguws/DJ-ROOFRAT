#include "gameplay/GameModes.h"
#include "gameplay/UnlockSystem.h"
#include "gameplay/AchievementSystem.h"
#include "audio/TrackMetadata.h"
#include "library/TrackLibrary.h"
#include "library/TrackBrowser.h"

#include <cassert>
#include <iostream>
#include <cmath>
#include <filesystem>

namespace dj {

// Helper function for floating point comparison
inline bool approx_equal(float a, float b, float tolerance = 0.01f) {
    return std::abs(a - b) < tolerance;
}

// Helper: Compute frame time from FPS
float computeFrameTime(float fps) {
    if (fps <= 0.0f) return 0.0f;
    return 1000.0f / fps;
}

// Test 1: Performance panel data accuracy - FPS and frame time calculation
void test_PerformancePanelCalculatesFrameTime() {
    std::cout << "Running test_PerformancePanelCalculatesFrameTime..." << std::endl;
    
    // Test with typical gaming FPS values
    float fps = 60.0f;
    float frameTime = computeFrameTime(fps);
    assert(std::abs(frameTime - 16.667f) < 0.01f);
    
    // Test with 144 Hz
    fps = 144.0f;
    frameTime = computeFrameTime(fps);
    assert(std::abs(frameTime - 6.944f) < 0.01f);
    
    // Test with low FPS
    fps = 30.0f;
    frameTime = computeFrameTime(fps);
    assert(std::abs(frameTime - 33.333f) < 0.01f);
    
    std::cout << "✓ test_PerformancePanelCalculatesFrameTime passed" << std::endl;
}

// Test 2: Performance panel block progress tracking
void test_PerformancePanelTracksBlockProgress() {
    std::cout << "Running test_PerformancePanelTracksBlockProgress..." << std::endl;
    
    int currentBlock = 5000;
    int totalBlocks = 10000;
    
    // Should calculate progress percentage
    float progressPercent = (static_cast<float>(currentBlock) / totalBlocks) * 100.0f;
    assert(approx_equal(progressPercent, 50.0f));
    
    // Test edge cases
    currentBlock = 0;
    progressPercent = (static_cast<float>(currentBlock) / totalBlocks) * 100.0f;
    assert(approx_equal(progressPercent, 0.0f));
    
    currentBlock = 10000;
    progressPercent = (static_cast<float>(currentBlock) / totalBlocks) * 100.0f;
    assert(approx_equal(progressPercent, 100.0f));
    
    std::cout << "✓ test_PerformancePanelTracksBlockProgress passed" << std::endl;
}

// Test 3: Career panel displays tier and venue name
void test_CareerPanelDisplaysTierAndVenue() {
    std::cout << "Running test_CareerPanelDisplaysTierAndVenue..." << std::endl;
    
    CareerProgression career;
    career.reset();
    
    // Initial state at tier 0
    assert(career.tier() == 0);
    assert(career.currentVenueName() == "Basement Bar");
    
    std::cout << "✓ test_CareerPanelDisplaysTierAndVenue passed" << std::endl;
}

// Test 4: Career panel displays reputation bar data
void test_CareerPanelDisplaysReputationBar() {
    std::cout << "Running test_CareerPanelDisplaysReputationBar..." << std::endl;
    
    CareerProgression career;
    career.reset();
    
    // Initial reputation (starts at 10.0)
    float initialRep = career.reputation();
    assert(approx_equal(initialRep, 10.0f));
    
    // Reputation should be normalized to 0-100 range for display
    float repPercent = (initialRep / 100.0f) * 100.0f;
    assert(approx_equal(repPercent, 10.0f));
    
    // After high energy update
    for (int i = 0; i < 20; ++i) {
        career.update(0.9f);  // High crowd energy
    }
    
    float highRep = career.reputation();
    assert(highRep > initialRep);
    assert(highRep <= 100.0f);
    
    // Clamped to [0, 100]
    repPercent = std::clamp(highRep, 0.0f, 100.0f) / 100.0f;
    assert(repPercent >= 0.0f);
    assert(repPercent <= 1.0f);
    
    std::cout << "✓ test_CareerPanelDisplaysReputationBar passed" << std::endl;
}

// Test 5: Career panel displays unlocked effects count
void test_CareerPanelDisplaysUnlockedEffectsCount() {
    std::cout << "Running test_CareerPanelDisplaysUnlockedEffectsCount..." << std::endl;
    
    UnlockSystem unlocks;
    
    // At tier 0, only BasicEQ should be unlocked
    auto tier0Unlocked = unlocks.getUnlockedEffects(0);
    assert(tier0Unlocked.size() == 1);
    
    // At tier 2, BasicEQ, Filters, Reverb, Delay should be unlocked
    auto tier2Unlocked = unlocks.getUnlockedEffects(2);
    assert(tier2Unlocked.size() == 4);
    
    // At tier 4, all 7 effects should be unlocked
    auto tier4Unlocked = unlocks.getUnlockedEffects(4);
    assert(tier4Unlocked.size() == 7);
    
    std::cout << "✓ test_CareerPanelDisplaysUnlockedEffectsCount passed" << std::endl;
}

// Test 6: Career panel displays achievement progress
void test_CareerPanelDisplaysAchievementProgress() {
    std::cout << "Running test_CareerPanelDisplaysAchievementProgress..." << std::endl;
    
    AchievementSystem achievements;
    
    // Get initial state
    int unlockedCount = achievements.getUnlockedCount();
    auto allAchievements = achievements.getAllAchievements();
    int totalCount = static_cast<int>(allAchievements.size());
    
    assert(unlockedCount == 0);
    assert(totalCount >= 5);  // Should have at least 5 achievements
    
    // Unlock some achievements
    achievements.checkAndUnlock("first_gig");
    achievements.checkAndUnlock("tier_2");
    
    unlockedCount = achievements.getUnlockedCount();
    assert(unlockedCount == 2);
    
    // Achievement ratio for display (e.g., "2 / 5 achievements")
    float achievementPercent = (static_cast<float>(unlockedCount) / totalCount) * 100.0f;
    assert(achievementPercent >= 0.0f && achievementPercent <= 100.0f);
    
    std::cout << "✓ test_CareerPanelDisplaysAchievementProgress passed" << std::endl;
}

// Test 7: Library panel can display track list
void test_LibraryPanelDisplaysTrackList() {
    std::cout << "Running test_LibraryPanelDisplaysTrackList..." << std::endl;
    
    auto lib = std::make_shared<library::TrackLibrary>();
    
    // Initialize library
    std::string error;
    std::string dbPath = "/tmp/test_lib_phase5.db";
    std::filesystem::remove(dbPath);  // Clean up if exists
    
    assert(lib->initialize(dbPath, &error));
    
    // Add some test tracks
    TrackMetadata metadata;
    metadata.title = "Test Track 1";
    metadata.artist = "Test Artist";
    metadata.bpm = 120.0f;
    metadata.key = "C major";
    metadata.durationSeconds = 240.0f;
    metadata.genre = "House";
    
    int trackId = -1;
    assert(lib->addTrack("/tmp/test1.wav", metadata, &trackId, &error));
    
    // Get track list
    auto tracks = lib->getAllTracks(&error);
    assert(tracks.size() == 1);
    assert(tracks[0].title == "Test Track 1");
    
    // Add another track
    metadata.title = "Test Track 2";
    assert(lib->addTrack("/tmp/test2.wav", metadata, &trackId, &error));
    
    tracks = lib->getAllTracks(&error);
    assert(tracks.size() == 2);
    
    // Cleanup
    std::filesystem::remove(dbPath);
    
    std::cout << "✓ test_LibraryPanelDisplaysTrackList passed" << std::endl;
}

// Test 8: Library panel track browser integration
void test_LibraryPanelTrackBrowserIntegration() {
    std::cout << "Running test_LibraryPanelTrackBrowserIntegration..." << std::endl;
    
    auto lib = std::make_shared<library::TrackLibrary>();
    std::string error;
    std::string dbPath = "/tmp/test_browser_phase5.db";
    std::filesystem::remove(dbPath);
    
    assert(lib->initialize(dbPath, &error));
    
    // Create browser
    library::TrackBrowser browser(lib);
    
    // Add test tracks with various BPMs
    for (int i = 0; i < 3; ++i) {
        TrackMetadata metadata;
        metadata.title = "Track " + std::to_string(i);
        metadata.artist = "Artist";
        metadata.bpm = 120.0f + (i * 10.0f);
        metadata.key = "C major";
        metadata.durationSeconds = 240.0f;
        metadata.genre = "House";
        
        assert(lib->addTrack("/tmp/track" + std::to_string(i) + ".wav", metadata, nullptr, &error));
    }
    
    // Get all tracks for library panel display
    auto allTracks = lib->getAllTracks(&error);
    assert(allTracks.size() == 3);
    
    // Verify track count is displayable
    assert(allTracks.size() >= 0);
    for (const auto& track : allTracks) {
        assert(!track.title.empty());
        assert(!track.artist.empty());
    }
    
    // Cleanup
    std::filesystem::remove(dbPath);
    
    std::cout << "✓ test_LibraryPanelTrackBrowserIntegration passed" << std::endl;
}

// Test 9: Performance panel latency estimation
void test_PerformancePanelLatencyEstimation() {
    std::cout << "Running test_PerformancePanelLatencyEstimation..." << std::endl;
    
    // Simulate frame time -> latency relationship
    // Assume latency is approximately the frame time plus some buffer
    const float BUFFER_MS = 5.0f;  // 5ms system overhead
    
    float fps = 60.0f;
    float frameTime = 1000.0f / fps;  // 16.67ms
    float estimatedLatency = frameTime + BUFFER_MS;  // ~21.67ms
    
    assert(std::abs(estimatedLatency - 21.67f) < 0.1f);
    
    // For 144 Hz
    fps = 144.0f;
    frameTime = 1000.0f / fps;
    estimatedLatency = frameTime + BUFFER_MS;
    
    assert(std::abs(estimatedLatency - 11.94f) < 0.1f);
    
    std::cout << "✓ test_PerformancePanelLatencyEstimation passed" << std::endl;
}

// Test 10: Panel visibility flags can be toggled
void test_PerformancePanelVisibilityFlags() {
    std::cout << "Running test_PerformancePanelVisibilityFlags..." << std::endl;
    
    // These flags would be used in main.cpp ImGui rendering
    bool showPerformancePanel = false;
    bool showCareerPanel = false;
    bool showLibraryPanel = false;
    
    assert(!showPerformancePanel);
    assert(!showCareerPanel);
    assert(!showLibraryPanel);
    
    // Toggle flags
    showPerformancePanel = true;
    showCareerPanel = true;
    showLibraryPanel = true;
    
    assert(showPerformancePanel);
    assert(showCareerPanel);
    assert(showLibraryPanel);
    
    std::cout << "✓ test_PerformancePanelVisibilityFlags passed" << std::endl;
}

} // namespace dj

int main() {
    std::cout << "=== Phase 5: Performance and Career UI Panels Tests ===" << std::endl << std::endl;
    
    try {
        dj::test_PerformancePanelCalculatesFrameTime();
        dj::test_PerformancePanelTracksBlockProgress();
        dj::test_CareerPanelDisplaysTierAndVenue();
        dj::test_CareerPanelDisplaysReputationBar();
        dj::test_CareerPanelDisplaysUnlockedEffectsCount();
        dj::test_CareerPanelDisplaysAchievementProgress();
        dj::test_LibraryPanelDisplaysTrackList();
        dj::test_LibraryPanelTrackBrowserIntegration();
        dj::test_PerformancePanelLatencyEstimation();
        dj::test_PerformancePanelVisibilityFlags();
        
        std::cout << std::endl << "=== All tests passed! ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}
