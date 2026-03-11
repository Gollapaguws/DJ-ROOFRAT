#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <optional>

#include "library/TrackLibrary.h"
#include "library/LibraryScanner.h"
#include "library/TrackBrowser.h"
#include "audio/TrackMetadata.h"

namespace fs = std::filesystem;
using namespace dj::library;

// ============================================================================
// Test Utilities
// ============================================================================

struct TestContext {
    fs::path dbPath;
    fs::path testDataDir;
    
    TestContext() {
        // Create temporary test directories
        dbPath = fs::temp_directory_path() / "dj_roofrat_test.db";
        testDataDir = fs::temp_directory_path() / "dj_roofrat_test_audio";
        
        if (fs::exists(dbPath)) {
            fs::remove(dbPath);
        }
        if (fs::exists(testDataDir)) {
            fs::remove_all(testDataDir);
        }
        fs::create_directories(testDataDir);
    }
    
    ~TestContext() {
        if (fs::exists(dbPath)) {
            fs::remove(dbPath);
        }
        if (fs::exists(testDataDir)) {
            fs::remove_all(testDataDir);
        }
    }
};

// Create a dummy audio file for testing
void createDummyAudioFile(const fs::path& path) {
    std::ofstream file(path, std::ios::binary);
    if (file) {
        // Write a simple marker for testing purposes
        std::string marker = "DUMMYAUDIO";
        file.write(marker.c_str(), marker.size());
    }
}

// ============================================================================
// Test 1: TrackLibrary Initialization
// ============================================================================

bool test_TrackLibrary_Initialize() {
    std::cout << "[TEST] TrackLibrary_Initialize..." << std::endl;
    TestContext ctx;
    
    TrackLibrary lib;
    std::string error;
    bool result = lib.initialize(ctx.dbPath.string(), &error);
    assert(result && "Library initialization should succeed");
    assert(lib.getTrackCount() == 0 && "Empty library should have 0 tracks");
    
    std::cout << "  ✓ PASSED" << std::endl;
    return true;
}

// ============================================================================
// Test 2: TrackLibrary Add Track
// ============================================================================

bool test_TrackLibrary_AddTrack() {
    std::cout << "[TEST] TrackLibrary_AddTrack..." << std::endl;
    TestContext ctx;
    
    TrackLibrary lib;
    std::string error;
    assert(lib.initialize(ctx.dbPath.string(), &error) && "Library init failed");
    
    dj::TrackMetadata metadata;
    metadata.title = "Test Track 1";
    metadata.artist = "Test Artist";
    metadata.bpm = 120.0f;
    metadata.key = "C major";
    metadata.durationSeconds = 180.0f;
    metadata.genre = "House";
    
    int trackId = -1;
    bool added = lib.addTrack("/tmp/test1.wav", metadata, &trackId, &error);
    assert(added && "Adding track should succeed");
    assert(trackId > 0 && "Track ID should be positive");
    assert(lib.getTrackCount() == 1 && "Should have 1 track");
    
    // Retrieve and verify
    auto retrieved = lib.getTrack(trackId, &error);
    assert(retrieved.has_value() && "Should retrieve added track");
    assert(retrieved->title == "Test Track 1" && "Title should match");
    assert(retrieved->artist == "Test Artist" && "Artist should match");
    assert(retrieved->bpm.value_or(0) == 120.0f && "BPM should match");
    
    std::cout << "  ✓ PASSED" << std::endl;
    return true;
}

// ============================================================================
// Test 3: TrackLibrary Query by BPM
// ============================================================================

bool test_TrackLibrary_QueryByBPM() {
    std::cout << "[TEST] TrackLibrary_QueryByBPM..." << std::endl;
    TestContext ctx;
    
    TrackLibrary lib;
    std::string error;
    assert(lib.initialize(ctx.dbPath.string(), &error) && "Library init failed");
    
    // Add tracks with different BPMs
    std::vector<float> bpms = {100.0f, 115.0f, 125.0f, 130.0f, 145.0f};
    for (size_t i = 0; i < bpms.size(); ++i) {
        dj::TrackMetadata metadata;
        metadata.title = "Track " + std::to_string(i);
        metadata.artist = "Artist";
        metadata.bpm = bpms[i];
        metadata.key = "C major";
        metadata.durationSeconds = 180.0f;
        metadata.genre = "House";
        
        lib.addTrack("/tmp/track" + std::to_string(i) + ".wav", metadata);
    }
    
    // Query for 120-130 BPM range
    auto result = lib.queryByBpm(120.0f, 130.0f, &error);
    assert(result.size() == 2 && "Should find 2 tracks in 120-130 range");
    assert(result[0].bpm.value_or(0) >= 120.0f && "Results should be in range");
    assert(result[0].bpm.value_or(0) <= 130.0f && "Results should be in range");
    
    std::cout << "  ✓ PASSED" << std::endl;
    return true;
}

// ============================================================================
// Test 4: TrackLibrary Query by Key
// ============================================================================

bool test_TrackLibrary_QueryByKey() {
    std::cout << "[TEST] TrackLibrary_QueryByKey..." << std::endl;
    TestContext ctx;
    
    TrackLibrary lib;
    std::string error;
    assert(lib.initialize(ctx.dbPath.string(), &error) && "Library init failed");
    
    // Add tracks with different keys
    std::vector<std::string> keys = {"C major", "G major", "C major", "D minor", "C major"};
    for (size_t i = 0; i < keys.size(); ++i) {
        dj::TrackMetadata metadata;
        metadata.title = "Track " + std::to_string(i);
        metadata.artist = "Artist";
        metadata.bpm = 120.0f;
        metadata.key = keys[i];
        metadata.durationSeconds = 180.0f;
        metadata.genre = "House";
        
        lib.addTrack("/tmp/track" + std::to_string(i) + ".wav", metadata);
    }
    
    // Query for C major
    auto result = lib.queryByKey("C major", &error);
    assert(result.size() == 3 && "Should find 3 C major tracks");
    for (const auto& track : result) {
        assert(track.key.value_or("") == "C major" && "All results should be C major");
    }
    
    std::cout << "  ✓ PASSED" << std::endl;
    return true;
}

// ============================================================================
// Test 5: LibraryScanner Recursive Scan
// ============================================================================

bool test_LibraryScanner_RecursiveScan() {
    std::cout << "[TEST] LibraryScanner_RecursiveScan..." << std::endl;
    TestContext ctx;
    
    // Create test directory structure
    fs::create_directories(ctx.testDataDir / "subdir1");
    fs::create_directories(ctx.testDataDir / "subdir2");
    
    createDummyAudioFile(ctx.testDataDir / "track1.wav");
    createDummyAudioFile(ctx.testDataDir / "track2.mp3");
    createDummyAudioFile(ctx.testDataDir / "subdir1" / "track3.flac");
    createDummyAudioFile(ctx.testDataDir / "subdir1" / "track4.wav");
    createDummyAudioFile(ctx.testDataDir / "subdir2" / "track5.mp3");
    createDummyAudioFile(ctx.testDataDir / "ignore.txt");
    
    LibraryScanner scanner;
    std::string error;
    ScanResult result;
    int callCount = 0;
    
    bool scanSucceeded = scanner.scanDirectory(ctx.testDataDir, 
        [&result, &callCount](const ScanResult& progress, const std::string& /*path*/) {
            result = progress;
            callCount++;
        }, 
        &error);
    
    assert(scanSucceeded && "Scan should succeed");
    assert(result.scannedCount == 5 && "Should scan 5 audio files");
    assert(result.skippedCount == 1 && "Should skip 1 non-audio file");
    assert(callCount > 0 && "Progress callback should be called");
    
    std::cout << "  ✓ PASSED" << std::endl;
    return true;
}

// ============================================================================
// Test 6: TrackBrowser Filtering by BPM
// ============================================================================

bool test_TrackBrowser_FilterByBPM() {
    std::cout << "[TEST] TrackBrowser_FilterByBPM..." << std::endl;
    TestContext ctx;
    
    auto lib = std::make_shared<TrackLibrary>();
    std::string error;
    assert(lib->initialize(ctx.dbPath.string(), &error) && "Library init failed");
    
    // Add tracks
    std::vector<float> bpms = {95.0f, 115.0f, 128.0f, 135.0f, 150.0f};
    for (size_t i = 0; i < bpms.size(); ++i) {
        dj::TrackMetadata metadata;
        metadata.title = "Track " + std::to_string(i);
        metadata.artist = "Artist";
        metadata.bpm = bpms[i];
        metadata.key = "C major";
        metadata.durationSeconds = 180.0f;
        metadata.genre = "House";
        lib->addTrack("/tmp/track" + std::to_string(i) + ".wav", metadata);
    }
    
    TrackBrowser browser(lib);
    auto filtered = browser.filterByBpm(120.0f, 140.0f);
    
    assert(filtered.size() == 2 && "Should filter to 2 tracks");
    for (const auto& track : filtered) {
        assert(track.bpm >= 120.0f && track.bpm <= 140.0f && "All tracks should be in range");
    }
    
    std::cout << "  ✓ PASSED" << std::endl;
    return true;
}

// ============================================================================
// Test 7: TrackBrowser Smart Playlist
// ============================================================================

bool test_TrackBrowser_SmartPlaylist() {
    std::cout << "[TEST] TrackBrowser_SmartPlaylist..." << std::endl;
    TestContext ctx;
    
    auto lib = std::make_shared<TrackLibrary>();
    std::string error;
    assert(lib->initialize(ctx.dbPath.string(), &error) && "Library init failed");
    
    // Add reference track and compatible tracks
    dj::TrackMetadata refMetadata;
    refMetadata.title = "Reference";
    refMetadata.artist = "Artist";
    refMetadata.bpm = 128.0f;
    refMetadata.key = "C major";
    refMetadata.durationSeconds = 180.0f;
    refMetadata.genre = "House";
    
    int refId = -1;
    lib->addTrack("/tmp/reference.wav", refMetadata, &refId);
    
    // Add compatible tracks (±10 BPM, compatible keys)
    dj::TrackMetadata track1;
    track1.title = "Compatible 1";
    track1.artist = "Artist";
    track1.bpm = 127.0f;
    track1.key = "G major";  // 5th - compatible
    track1.durationSeconds = 180.0f;
    track1.genre = "House";
    lib->addTrack("/tmp/compatible1.wav", track1);
    
    dj::TrackMetadata track2;
    track2.title = "Far Away";
    track2.artist = "Artist";
    track2.bpm = 90.0f;  // Out of range
    track2.key = "A minor";
    track2.durationSeconds = 180.0f;
    track2.genre = "Techno";
    lib->addTrack("/tmp/faraway.wav", track2);
    
    TrackBrowser browser(lib);
    auto refTrack = lib->getTrack(refId);
    assert(refTrack.has_value() && "Reference track should exist");
    
    auto playlist = browser.getSmartPlaylist(refTrack.value(), 10);
    assert(playlist.size() > 0 && "Should recommend at least one track");
    // The playlist should contain tracks compatible with the reference
    
    std::cout << "  ✓ PASSED" << std::endl;
    return true;
}

// ============================================================================
// Test 8: TrackBrowser Sorting
// ============================================================================

bool test_TrackBrowser_Sorting() {
    std::cout << "[TEST] TrackBrowser_Sorting..." << std::endl;
    TestContext ctx;
    
    auto lib = std::make_shared<TrackLibrary>();
    std::string error;
    assert(lib->initialize(ctx.dbPath.string(), &error) && "Library init failed");
    
    // Add tracks with different properties
    std::vector<std::pair<std::string, float>> tracksData = {
        {"Charlie", 100.0f},
        {"Alice", 140.0f},
        {"Bob", 120.0f}
    };
    
    for (const auto& [title, bpm] : tracksData) {
        dj::TrackMetadata metadata;
        metadata.title = title;
        metadata.artist = "Artist";
        metadata.bpm = bpm;
        metadata.key = "C major";
        metadata.durationSeconds = 180.0f;
        metadata.genre = "House";
        lib->addTrack("/tmp/" + title + ".wav", metadata);
    }
    
    auto tracks = lib->getAllTracks();
    TrackBrowser browser(lib);
    
    // Sort by title ascending
    auto sortedByTitle = browser.sort(tracks, SortField::Title, SortOrder::Ascending);
    assert(sortedByTitle[0].title == "Alice" && "First should be Alice");
    assert(sortedByTitle[2].title == "Charlie" && "Last should be Charlie");
    
    // Sort by BPM descending
    auto sortedByBpm = browser.sort(tracks, SortField::BPM, SortOrder::Descending);
    assert(sortedByBpm[0].bpm >= sortedByBpm[1].bpm && "Should be descending");
    
    std::cout << "  ✓ PASSED" << std::endl;
    return true;
}

// ============================================================================
// Test 9: TrackLibrary Remove Track
// ============================================================================

bool test_TrackLibrary_RemoveTrack() {
    std::cout << "[TEST] TrackLibrary_RemoveTrack..." << std::endl;
    TestContext ctx;
    
    TrackLibrary lib;
    std::string error;
    assert(lib.initialize(ctx.dbPath.string(), &error) && "Library init failed");
    
    dj::TrackMetadata metadata;
    metadata.title = "To Remove";
    metadata.artist = "Artist";
    metadata.bpm = 120.0f;
    metadata.key = "C major";
    metadata.durationSeconds = 180.0f;
    metadata.genre = "House";
    
    int trackId = -1;
    lib.addTrack("/tmp/toremove.wav", metadata, &trackId);
    assert(lib.getTrackCount() == 1 && "Should have 1 track");
    
    bool removed = lib.removeTrack(trackId, &error);
    assert(removed && "Remove should succeed");
    assert(lib.getTrackCount() == 0 && "Should have 0 tracks after removal");
    
    std::cout << "  ✓ PASSED" << std::endl;
    return true;
}

// ============================================================================
// Test 10: TrackLibrary Clear Database
// ============================================================================

bool test_TrackLibrary_ClearDatabase() {
    std::cout << "[TEST] TrackLibrary_ClearDatabase..." << std::endl;
    TestContext ctx;
    
    TrackLibrary lib;
    std::string error;
    assert(lib.initialize(ctx.dbPath.string(), &error) && "Library init failed");
    
    // Add multiple tracks
    for (int i = 0; i < 5; ++i) {
        dj::TrackMetadata metadata;
        metadata.title = "Track " + std::to_string(i);
        metadata.artist = "Artist";
        metadata.bpm = 120.0f + i;
        metadata.key = "C major";
        metadata.durationSeconds = 180.0f;
        metadata.genre = "House";
        lib.addTrack("/tmp/track" + std::to_string(i) + ".wav", metadata);
    }
    
    assert(lib.getTrackCount() == 5 && "Should have 5 tracks");
    
    bool cleared = lib.clearDatabase(&error);
    assert(cleared && "Clear should succeed");
    assert(lib.getTrackCount() == 0 && "Should have 0 tracks after clear");
    
    std::cout << "  ✓ PASSED" << std::endl;
    return true;
}

// ============================================================================
// Test 11: Empty Database Handling
// ============================================================================

bool test_EmptyDatabase_Handling() {
    std::cout << "[TEST] EmptyDatabase_Handling..." << std::endl;
    TestContext ctx;
    
    TrackLibrary lib;
    std::string error;
    assert(lib.initialize(ctx.dbPath.string(), &error) && "Library init failed");
    
    // Query on empty database should return empty results
    auto byBpm = lib.queryByBpm(120.0f, 130.0f);
    assert(byBpm.size() == 0 && "Empty query should return 0 results");
    
    auto byKey = lib.queryByKey("C major");
    assert(byKey.size() == 0 && "Empty query should return 0 results");
    
    auto all = lib.getAllTracks();
    assert(all.size() == 0 && "getAllTracks on empty db should return empty");
    
    auto track = lib.getTrack(999);
    assert(!track.has_value() && "Non-existent track should return nullopt");
    
    std::cout << "  ✓ PASSED" << std::endl;
    return true;
}

// ============================================================================
// Test 12: TrackBrowser Combined Filtering
// ============================================================================

bool test_TrackBrowser_CombinedFiltering() {
    std::cout << "[TEST] TrackBrowser_CombinedFiltering..." << std::endl;
    TestContext ctx;
    
    auto lib = std::make_shared<TrackLibrary>();
    std::string error;
    assert(lib->initialize(ctx.dbPath.string(), &error) && "Library init failed");
    
    // Add diverse tracks
    std::vector<std::tuple<std::string, std::string, float, std::string>> data = {
        {"Track 1", "Artist A", 120.0f, "C major"},
        {"Track 2", "Artist B", 125.0f, "C major"},
        {"Track 3", "Artist A", 140.0f, "G major"},
        {"Track 4", "Artist B", 115.0f, "D minor"},
    };
    
    for (const auto& [title, artist, bpm, key] : data) {
        dj::TrackMetadata metadata;
        metadata.title = title;
        metadata.artist = artist;
        metadata.bpm = bpm;
        metadata.key = key;
        metadata.durationSeconds = 180.0f;
        metadata.genre = "House";
        lib->addTrack("/tmp/" + title + ".wav", metadata);
    }
    
    TrackBrowser browser(lib);
    auto results = browser.filter(120.0f, 130.0f, "C major", "Artist A");
    
    assert(results.size() >= 1 && "Should find at least 1 track");
    for (const auto& track : results) {
        assert(track.bpm >= 120.0f && track.bpm <= 130.0f && "BPM should be in range");
        assert(track.key == "C major" && "Key should match filter");
        assert(track.artist == "Artist A" && "Artist should match filter");
    }
    
    std::cout << "  ✓ PASSED" << std::endl;
    return true;
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Phase 11: Track Library & Browser Tests" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    
    int passedTests = 0;
    int totalTests = 0;
    
    try {
        // Run all tests
        std::vector<std::pair<const char*, bool(*)()>> tests = {
            {"test_TrackLibrary_Initialize", test_TrackLibrary_Initialize},
            {"test_TrackLibrary_AddTrack", test_TrackLibrary_AddTrack},
            {"test_TrackLibrary_QueryByBPM", test_TrackLibrary_QueryByBPM},
            {"test_TrackLibrary_QueryByKey", test_TrackLibrary_QueryByKey},
            {"test_LibraryScanner_RecursiveScan", test_LibraryScanner_RecursiveScan},
            {"test_TrackBrowser_FilterByBPM", test_TrackBrowser_FilterByBPM},
            {"test_TrackBrowser_SmartPlaylist", test_TrackBrowser_SmartPlaylist},
            {"test_TrackBrowser_Sorting", test_TrackBrowser_Sorting},
            {"test_TrackLibrary_RemoveTrack", test_TrackLibrary_RemoveTrack},
            {"test_TrackLibrary_ClearDatabase", test_TrackLibrary_ClearDatabase},
            {"test_EmptyDatabase_Handling", test_EmptyDatabase_Handling},
            {"test_TrackBrowser_CombinedFiltering", test_TrackBrowser_CombinedFiltering},
        };
        
        for (const auto& [name, testFunc] : tests) {
            totalTests++;
            try {
                if (testFunc()) {
                    passedTests++;
                }
            } catch (const std::exception& e) {
                std::cerr << "  ✗ FAILED: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "  ✗ FAILED: Unknown exception" << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << std::endl << "========================================" << std::endl;
    std::cout << "Results: " << passedTests << "/" << totalTests << " tests passed" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return passedTests == totalTests ? 0 : 1;
}
