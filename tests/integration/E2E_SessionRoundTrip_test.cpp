// Phase 1: Integration E2E Test - Session Round Trip
// Tests: Create session state → save to file → load from file → verify all fields match

#include "core/SessionState.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace dj {

namespace {

fs::path getWritableTestPath(const std::string& filename) {
    std::error_code ec;
    fs::path tempRoot = fs::temp_directory_path(ec);
    if (ec) {
        return fs::path(filename);
    }

    fs::path testDir = tempRoot / "dj-roofrat-tests";
    fs::create_directories(testDir, ec);
    if (ec) {
        return fs::path(filename);
    }

    return testDir / filename;
}

void removeIfExists(const fs::path& path) {
    std::error_code ec;
    fs::remove(path, ec);
}

} // namespace

// E2E Test 2: Session round-trip (save and load)
void test_E2E_SessionRoundTrip_SaveLoad() {
    std::cout << "[TEST 1] E2E_SessionRoundTrip_SaveLoad...\n";

    try {
        SessionManager sessionMgr;
        const fs::path testSessionPath = getWritableTestPath("test_session_roundtrip.json");

        // Clean up any previous test file
        removeIfExists(testSessionPath);

        // Step 1: Create session state
        SessionState session;
        session.deckA.trackPath = "test_track_A.wav";
        session.deckA.playbackPosition = 45.5;
        session.deckA.tempoBend = 0.02f;
        session.deckA.isPlaying = true;
        session.deckA.lowGain = 0.8f;
        session.deckA.midGain = 1.0f;
        session.deckA.highGain = 0.9f;
        
        session.deckB.trackPath = "test_track_B.wav";
        session.deckB.playbackPosition = 102.3;
        session.deckB.tempoBend = -0.01f;
        session.deckB.isPlaying = false;
        session.deckB.lowGain = 1.1f;
        session.deckB.midGain = 0.95f;
        session.deckB.highGain = 1.05f;
        
        session.crossfader = 0.3f;
        session.currentCareerTier = 5;
        session.crowdEnergy = 0.75f;
        session.venueId = "underground_club";

        // Step 2: Save session to file
        bool saved = sessionMgr.saveSession(testSessionPath.string(), session);
        assert(saved && "Failed to save session to file");
        assert(fs::exists(testSessionPath) && "Session file was not created");

        // Step 3: Load session from file
        auto loadedSessionOpt = sessionMgr.loadSession(testSessionPath.string());
        assert(loadedSessionOpt.has_value() && "Failed to load session from file");
        SessionState loadedSession = loadedSessionOpt.value();

        // Step 4: Verify all fields match
        std::cout << "  Verifying deck A fields...\n";
        assert(loadedSession.deckA.trackPath == session.deckA.trackPath && "deckA.trackPath mismatch");
        assert(std::abs(loadedSession.deckA.playbackPosition - session.deckA.playbackPosition) < 0.001 &&
               "deckA.playbackPosition mismatch");
        assert(std::abs(loadedSession.deckA.tempoBend - session.deckA.tempoBend) < 0.001f &&
               "deckA.tempoBend mismatch");
        assert(loadedSession.deckA.isPlaying == session.deckA.isPlaying && "deckA.isPlaying mismatch");
        assert(std::abs(loadedSession.deckA.lowGain - session.deckA.lowGain) < 0.001f &&
               "deckA.lowGain mismatch");
        assert(std::abs(loadedSession.deckA.midGain - session.deckA.midGain) < 0.001f &&
               "deckA.midGain mismatch");
        assert(std::abs(loadedSession.deckA.highGain - session.deckA.highGain) < 0.001f &&
               "deckA.highGain mismatch");

        std::cout << "  Verifying deck B fields...\n";
        assert(loadedSession.deckB.trackPath == session.deckB.trackPath && "deckB.trackPath mismatch");
        assert(std::abs(loadedSession.deckB.playbackPosition - session.deckB.playbackPosition) < 0.001 &&
               "deckB.playbackPosition mismatch");
        assert(std::abs(loadedSession.deckB.tempoBend - session.deckB.tempoBend) < 0.001f &&
               "deckB.tempoBend mismatch");
        assert(loadedSession.deckB.isPlaying == session.deckB.isPlaying && "deckB.isPlaying mismatch");
        assert(std::abs(loadedSession.deckB.lowGain - session.deckB.lowGain) < 0.001f &&
               "deckB.lowGain mismatch");
        assert(std::abs(loadedSession.deckB.midGain - session.deckB.midGain) < 0.001f &&
               "deckB.midGain mismatch");
        assert(std::abs(loadedSession.deckB.highGain - session.deckB.highGain) < 0.001f &&
               "deckB.highGain mismatch");

        std::cout << "  Verifying global session fields...\n";
        assert(std::abs(loadedSession.crossfader - session.crossfader) < 0.001f &&
               "crossfader mismatch");
        assert(loadedSession.currentCareerTier == session.currentCareerTier &&
               "currentCareerTier mismatch");
        assert(std::abs(loadedSession.crowdEnergy - session.crowdEnergy) < 0.001f &&
               "crowdEnergy mismatch");
        assert(loadedSession.venueId == session.venueId && "venueId mismatch");

        // Clean up
        removeIfExists(testSessionPath);

        std::cout << "  ✓ Session round-trip successful, all fields match\n";
        std::cout << "PASS\n";
    } catch (const fs::filesystem_error& e) {
        if (e.code() == std::errc::permission_denied) {
            std::cout << "  ⊘ SKIPPED (permission denied while accessing test files)\n";
            return;
        }
        removeIfExists(getWritableTestPath("test_session_roundtrip.json"));
        std::cout << "  ✗ Filesystem error during session round-trip: " << e.what() << "\n";
        assert(false && "Session round-trip test failed");
    } catch (const std::exception& e) {
        std::cout << "  ✗ Exception during session round-trip: " << e.what() << "\n";
        // Clean up on failure
        removeIfExists(getWritableTestPath("test_session_roundtrip.json"));
        assert(false && "Session round-trip test failed");
    }
}

// E2E Test 2: JSON import/export round-trip with escaped strings
void test_E2E_SessionRoundTrip_JSONEscapedStrings() {
    std::cout << "[TEST 2] E2E_SessionRoundTrip_JSONEscapedStrings...\n";

    try {
        SessionManager sessionMgr;
        const fs::path testSessionPath = getWritableTestPath("test_session_escaped.json");
        removeIfExists(testSessionPath);

        SessionState session;
        session.deckA.trackPath = "C:\\Music\\set_\"alpha\"\\deckA.wav";
        session.deckA.playbackPosition = 12.25;
        session.deckA.tempoBend = 0.01f;
        session.deckA.isPlaying = true;
        session.deckA.lowGain = 0.9f;
        session.deckA.midGain = 1.0f;
        session.deckA.highGain = 1.1f;

        session.deckB.trackPath = "D:\\Audio\\set_\"beta\"\\deckB.wav";
        session.deckB.playbackPosition = 18.75;
        session.deckB.tempoBend = -0.02f;
        session.deckB.isPlaying = false;
        session.deckB.lowGain = 1.05f;
        session.deckB.midGain = 0.95f;
        session.deckB.highGain = 1.0f;

        session.crossfader = -0.1f;
        session.currentCareerTier = 3;
        session.crowdEnergy = 0.62f;
        session.venueId = "warehouse_\"room_B\"";

        assert(sessionMgr.saveSession(testSessionPath.string(), session) && "Failed to save escaped JSON session");

        std::ifstream jsonFile(testSessionPath);
        assert(jsonFile.is_open() && "Failed to open saved JSON for inspection");
        std::stringstream buffer;
        buffer << jsonFile.rdbuf();
        const std::string jsonContent = buffer.str();
        assert(jsonContent.find("\\\\") != std::string::npos && "Expected escaped backslashes in JSON");
        assert(jsonContent.find("\\\"") != std::string::npos && "Expected escaped quotes in JSON");

        auto loadedSessionOpt = sessionMgr.loadSession(testSessionPath.string());
        assert(loadedSessionOpt.has_value() && "Failed to load escaped JSON session");
        SessionState loadedSession = loadedSessionOpt.value();

        assert(loadedSession.deckA.trackPath == session.deckA.trackPath && "Escaped deckA.trackPath mismatch");
        assert(loadedSession.deckB.trackPath == session.deckB.trackPath && "Escaped deckB.trackPath mismatch");
        assert(loadedSession.venueId == session.venueId && "Escaped venueId mismatch");

        removeIfExists(testSessionPath);
        std::cout << "  ✓ Escaped JSON round-trip successful\n";
        std::cout << "PASS\n";
    } catch (const fs::filesystem_error& e) {
        if (e.code() == std::errc::permission_denied) {
            std::cout << "  ⊘ SKIPPED (permission denied while accessing test files)\n";
            return;
        }
        removeIfExists(getWritableTestPath("test_session_escaped.json"));
        std::cout << "  ✗ Filesystem error during escaped JSON round-trip: " << e.what() << "\n";
        assert(false && "Escaped JSON session round-trip test failed");
    } catch (const std::exception& e) {
        removeIfExists(getWritableTestPath("test_session_escaped.json"));
        std::cout << "  ✗ Exception during escaped JSON round-trip: " << e.what() << "\n";
        assert(false && "Escaped JSON session round-trip test failed");
    }
}

void runAllE2ESessionTests() {
    std::cout << "\n=== Running E2E Session Round-Trip Tests ===\n";
    try {
        test_E2E_SessionRoundTrip_SaveLoad();
        test_E2E_SessionRoundTrip_JSONEscapedStrings();
    } catch (const std::exception& e) {
        std::cerr << "Session test failed with exception: " << e.what() << "\n";
    }
}

} // namespace dj

// Entry point
int main() {
    dj::runAllE2ESessionTests();
    std::cout << "\n=== E2E Session Tests Complete ===\n";
    return 0;
}
