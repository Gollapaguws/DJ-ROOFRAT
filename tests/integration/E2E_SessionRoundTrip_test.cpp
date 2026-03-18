// Phase 1: Integration E2E Test - Session Round Trip
// Tests: Create session state → save to file → load from file → verify all fields match

#include "core/SessionState.h"

#include <cassert>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace dj {

// E2E Test 2: Session round-trip (save and load)
void test_E2E_SessionRoundTrip_SaveLoad() {
    std::cout << "[TEST 1] E2E_SessionRoundTrip_SaveLoad...\n";
    
    try {
        SessionManager sessionMgr;
        const std::string testSessionPath = "test_session_roundtrip.json";
        
        // Clean up any previous test file
        if (fs::exists(testSessionPath)) {
            fs::remove(testSessionPath);
        }
        
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
        bool saved = session.saveToFile(testSessionPath);
        assert(saved && "Failed to save session to file");
        assert(fs::exists(testSessionPath) && "Session file was not created");
        
        // Step 3: Load session from file
        SessionMetadata loadedSession;
        bool loaded = loadedSession.loadFromFile(testSessionPath);
        assert(loaded && "Failed to load session from file");
        
        // Step 4: Verify all fields match
        assert(loadedSession.deckAClip == session.deckAClip && "deckAClip mismatch");
        assert(loadedSession.deckBClip == session.deckBClip && "deckBClip mismatch");
        
        // Use approximate comparison for floats (within 0.001f)
        auto floatEqual = [](float a, float b, float tolerance = 0.001f) {
            return std::abs(a - b) < tolerance;
        };
        
        assert(floatEqual(loadedSession.bpmA, session.bpmA) && "bpmA mismatch");
        assert(floatEqual(loadedSession.bpmB, session.bpmB) && "bpmB mismatch");
        assert(floatEqual(loadedSession.crossfaderPos, session.crossfaderPos) && "crossfaderPos mismatch");
        assert(floatEqual(loadedSession.eqALow, session.eqALow) && "eqALow mismatch");
        assert(floatEqual(loadedSession.eqAMid, session.eqAMid) && "eqAMid mismatch");
        assert(floatEqual(loadedSession.eqAHigh, session.eqAHigh) && "eqAHigh mismatch");
        assert(floatEqual(loadedSession.eqBLow, session.eqBLow) && "eqBLow mismatch");
        assert(floatEqual(loadedSession.eqBMid, session.eqBMid) && "eqBMid mismatch");
        assert(floatEqual(loadedSession.eqBHigh, session.eqBHigh) && "eqBHigh mismatch");
        assert(floatEqual(loadedSession.masterGain, session.masterGain) && "masterGain mismatch");
        assert(loadedSession.sessionName == session.sessionName && "sessionName mismatch");
        
        // Clean up
        if (fs::exists(testSessionPath)) {
            fs::remove(testSessionPath);
        }
        
        std::cout << "  ✓ Session round-trip successful, all fields match\n";
        std::cout << "PASS\n";
    } catch (const std::exception& e) {
        std::cout << "  ✗ Exception during session round-trip: " << e.what() << "\n";
        // Clean up on failure
        if (fs::exists("test_session_roundtrip.json")) {
            fs::remove("test_session_roundtrip.json");
        }
        assert(false && "Session round-trip test failed");
    }
}

void runAllE2ESessionTests() {
    std::cout << "\n=== Running E2E Session Round-Trip Tests ===\n";
    try {
        test_E2E_SessionRoundTrip_SaveLoad();
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
