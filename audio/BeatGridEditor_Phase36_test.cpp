#include "audio/BeatGridEditor.h"
#include "audio/BeatGrid.h"
#include "audio/TrackMetadata.h"
#include "visuals/BeatMarkerOverlay.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

namespace dj {

// Test 1: BeatGridEditor Basic Nudge
void test_BeatGridEditor_BasicNudge() {
    std::cout << "[TEST 1] BeatGridEditor_BasicNudge..." << std::endl;
    
    // Create BeatGrid with 4 beats at specific sample positions
    BeatGrid grid;
    double trackDuration = 5.0;  // 5 seconds
    double bpm = 120.0;
    double firstBeatOffset = 1000.0 / 44100.0;  // 1000 samples @ 44.1kHz = ~0.0227 sec
    
    grid.generateFromBPM(bpm, firstBeatOffset, trackDuration);
    
    // Create editor
    BeatGridEditor editor(grid, 44100);
    
    // Get initial beat positions
    std::vector<BeatMarker> initialBeats = editor.getBeatGrid().getBeats();
    assert(initialBeats.size() >= 4);
    
    // Record first 4 beat timestamps (in seconds)
    std::vector<double> initialTimestamps;
    for (size_t i = 0; i < 4 && i < initialBeats.size(); ++i) {
        initialTimestamps.push_back(initialBeats[i].timestamp);
    }
    
    // Nudge all beats by +100ms
    editor.nudgeBeats(100.0);
    
    // Get updated beat positions
    std::vector<BeatMarker> updatedBeats = editor.getBeatGrid().getBeats();
    assert(updatedBeats.size() == initialBeats.size());
    
    // Verify all beat positions increased by 100ms (0.1 seconds)
    for (size_t i = 0; i < 4 && i < updatedBeats.size(); ++i) {
        double expectedTimestamp = initialTimestamps[i] + 0.1;
        double actualTimestamp = updatedBeats[i].timestamp;
        assert(std::abs(actualTimestamp - expectedTimestamp) < 0.001);  // 1ms tolerance
    }
    
    // Verify undo stack has 1 entry, redo stack empty
    assert(editor.canUndo());
    assert(!editor.canRedo());
    
    std::cout << "✓ test_BeatGridEditor_BasicNudge passed" << std::endl;
}

// Test 2: BeatGridEditor First Beat Offset
void test_BeatGridEditor_FirstBeatOffset() {
    std::cout << "[TEST 2] BeatGridEditor_FirstBeatOffset..." << std::endl;
    
    // Create BeatGrid with first beat at 1000 samples, BPM 120
    BeatGrid grid;
    double trackDuration = 5.0;
    double bpm = 120.0;
    double firstBeatOffset = 1000.0 / 44100.0;  // ~0.0227 sec
    
    grid.generateFromBPM(bpm, firstBeatOffset, trackDuration);
    
    // Create editor
    BeatGridEditor editor(grid, 44100);
    
    std::vector<BeatMarker> initialBeats = editor.getBeatGrid().getBeats();
    double initialFirstBeatTime = initialBeats[0].timestamp;
    
    // Adjust first beat offset by +500ms
    editor.setFirstBeatOffset(500.0);
    
    // Get updated beats
    std::vector<BeatMarker> updatedBeats = editor.getBeatGrid().getBeats();
    
    // Verify first beat shifted by 500ms
    double expectedFirstBeatTime = initialFirstBeatTime + 0.5;
    assert(std::abs(updatedBeats[0].timestamp - expectedFirstBeatTime) < 0.001);
    
    // Verify subsequent beats recalculated from new first beat
    // At BPM 120, beats are 0.5 seconds apart (60/120 = 0.5)
    double secondsPerBeat = 60.0 / bpm;
    for (size_t i = 1; i < std::min(size_t(4), updatedBeats.size()); ++i) {
        double expectedTimestamp = updatedBeats[0].timestamp + (i * secondsPerBeat);
        assert(std::abs(updatedBeats[i].timestamp - expectedTimestamp) < 0.001);
    }
    
    // Verify undo stack has 1 entry
    assert(editor.canUndo());
    assert(!editor.canRedo());
    
    std::cout << "✓ test_BeatGridEditor_FirstBeatOffset passed" << std::endl;
}

// Test 3: BeatGridEditor Undo/Redo
void test_BeatGridEditor_UndoRedo() {
    std::cout << "[TEST 3] BeatGridEditor_UndoRedo..." << std::endl;
    
    BeatGrid grid;
    grid.generateFromBPM(120.0, 0.0, 5.0);
    
    BeatGridEditor editor(grid, 44100);
    
    // Get original timestamp
    double originalFirstBeat = editor.getBeatGrid().getBeats()[0].timestamp;
    
    // Perform 3 nudge operations: +10ms, -5ms, +20ms
    editor.nudgeBeats(10.0);
    double afterFirst = editor.getBeatGrid().getBeats()[0].timestamp;
    
    editor.nudgeBeats(-5.0);
    double afterSecond = editor.getBeatGrid().getBeats()[0].timestamp;
    
    editor.nudgeBeats(20.0);
    double afterThird = editor.getBeatGrid().getBeats()[0].timestamp;
    
    // Verify final state
    assert(std::abs(afterThird - (originalFirstBeat + 0.010 - 0.005 + 0.020)) < 0.001);
    
    // Undo 3 times - should return to original
    assert(editor.canUndo());
    editor.undo();
    assert(std::abs(editor.getBeatGrid().getBeats()[0].timestamp - afterSecond) < 0.001);
    
    assert(editor.canUndo());
    editor.undo();
    assert(std::abs(editor.getBeatGrid().getBeats()[0].timestamp - afterFirst) < 0.001);
    
    assert(editor.canUndo());
    editor.undo();
    assert(std::abs(editor.getBeatGrid().getBeats()[0].timestamp - originalFirstBeat) < 0.001);
    
    assert(!editor.canUndo());  // No more undo
    
    // Redo 2 times
    assert(editor.canRedo());
    editor.redo();
    assert(std::abs(editor.getBeatGrid().getBeats()[0].timestamp - afterFirst) < 0.001);
    
    assert(editor.canRedo());
    editor.redo();
    assert(std::abs(editor.getBeatGrid().getBeats()[0].timestamp - afterSecond) < 0.001);
    
    // Still have one more redo available
    assert(editor.canRedo());
    
    std::cout << "✓ test_BeatGridEditor_UndoRedo passed" << std::endl;
}

// Test 4: BeatGridEditor Undo Stack Limit
void test_BeatGridEditor_UndoStackLimit() {
    std::cout << "[TEST 4] BeatGridEditor_UndoStackLimit..." << std::endl;
    
    BeatGrid grid;
    grid.generateFromBPM(120.0, 0.0, 5.0);
    
    BeatGridEditor editor(grid, 44100);
    
    // Get timestamp after 10 operations (these will be lost when we exceed 50)
    for (int i = 0; i < 10; ++i) {
        editor.nudgeBeats(1.0);
    }
    double timestampAfter10 = editor.getBeatGrid().getBeats()[0].timestamp;
    
    // Perform 50 more nudge operations (total 60, exceeds 50 limit)
    for (int i = 0; i < 50; ++i) {
        editor.nudgeBeats(1.0);
    }
    
    // Undo 50 times (max stack size)
    int undoCount = 0;
    while (editor.canUndo() && undoCount < 51) {
        editor.undo();
        undoCount++;
    }
    
    // Should have undone exactly 50 operations
    assert(undoCount == 50);
    assert(!editor.canUndo());
    
    // Should be at state after first 10 operations (not original)
    double currentTimestamp = editor.getBeatGrid().getBeats()[0].timestamp;
    assert(std::abs(currentTimestamp - timestampAfter10) < 0.001);
    
    std::cout << "✓ test_BeatGridEditor_UndoStackLimit passed" << std::endl;
}

// Test 5: BeatMarkerOverlay Basic Render
void test_BeatMarkerOverlay_BasicRender() {
    std::cout << "[TEST 5] BeatMarkerOverlay_BasicRender..." << std::endl;
    
    // Create a simple waveform output (80 chars wide, 10 lines)
    std::string waveform;
    for (int i = 0; i < 10; ++i) {
        waveform += std::string(80, '-') + "\n";
    }
    
    // Create beat positions at samples: 1000, 2000, 3000
    // Assume window shows samples 0-4000
    std::vector<size_t> beatPositions = {1000, 2000, 3000};
    size_t windowStart = 0;
    size_t windowEnd = 4000;
    size_t currentPos = 0;
    
    // Create overlay and render
    BeatMarkerOverlay overlay;
    std::string result = overlay.render(waveform, beatPositions, currentPos, windowStart, windowEnd);
    
    // Verify result contains marker characters
    assert(result.find('|') != std::string::npos);
    
    // Verify result is longer than original (added marker line)
    assert(result.length() > waveform.length());
    
    std::cout << "✓ test_BeatMarkerOverlay_BasicRender passed" << std::endl;
}

// Test 6: BeatMarkerOverlay Current Beat Highlight
void test_BeatMarkerOverlay_CurrentBeatHighlight() {
    std::cout << "[TEST 6] BeatMarkerOverlay_CurrentBeatHighlight..." << std::endl;
    
    // Create simple waveform
    std::string waveform;
    for (int i = 0; i < 10; ++i) {
        waveform += std::string(80, '-') + "\n";
    }
    
    // Beat positions: 1000, 2000, 3000
    std::vector<size_t> beatPositions = {1000, 2000, 3000};
    size_t windowStart = 0;
    size_t windowEnd = 4000;
    
    // Set current playback position near beat 2 (sample 1950, closest to beat 2)
    size_t currentPos = 1950;
    
    BeatMarkerOverlay overlay;
    std::string result = overlay.render(waveform, beatPositions, currentPos, windowStart, windowEnd);
    
    // Verify both marker types are present
    assert(result.find('|') != std::string::npos);
    assert(result.find('v') != std::string::npos);  // Highlight character
    
    // Change current position to 2800 (closest to beat 3)
    currentPos = 2800;
    std::string result2 = overlay.render(waveform, beatPositions, currentPos, windowStart, windowEnd);
    
    // Verify highlight character still present
    assert(result2.find('v') != std::string::npos);
    
    std::cout << "✓ test_BeatMarkerOverlay_CurrentBeatHighlight passed" << std::endl;
}

// Test 7: BeatMarkerOverlay Edge Cases
void test_BeatMarkerOverlay_EdgeCases() {
    std::cout << "[TEST 7] BeatMarkerOverlay_EdgeCases..." << std::endl;
    
    // Create simple waveform
    std::string waveform;
    for (int i = 0; i < 10; ++i) {
        waveform += std::string(80, '-') + "\n";
    }
    
    // Beat position outside visible window
    std::vector<size_t> beatPositions = {100000};  // Way beyond window
    size_t windowStart = 0;
    size_t windowEnd = 4000;
    size_t currentPos = 0;
    
    BeatMarkerOverlay overlay;
    std::string result = overlay.render(waveform, beatPositions, currentPos, windowStart, windowEnd);
    
    // Should not crash, result should be valid (no markers visible)
    assert(!result.empty());
    
    // Test with empty beat positions
    std::vector<size_t> emptyBeats;
    std::string result2 = overlay.render(waveform, emptyBeats, currentPos, windowStart, windowEnd);
    
    // Should return waveform unchanged
    assert(result2 == waveform);
    
    // Test with empty waveform
    std::string emptyWaveform = "";
    std::string result3 = overlay.render(emptyWaveform, beatPositions, currentPos, windowStart, windowEnd);
    
    // Should return empty string without crashing
    assert(result3.empty());
    
    std::cout << "✓ test_BeatMarkerOverlay_EdgeCases passed" << std::endl;
}

// Test 8: Main Loop Integration
void test_MainLoopIntegration() {
    std::cout << "[TEST 8] MainLoopIntegration..." << std::endl;
    
    // Placeholder - this will be verified through manual testing in main.cpp
    
    std::cout << "✓ test_MainLoopIntegration passed (placeholder)" << std::endl;
}

} // namespace dj

int main() {
    std::cout << "=== Phase 36: Beat Grid Nudge Editor Tests ===" << std::endl;
    
    // Run tests 1-4 for BeatGridEditor
    dj::test_BeatGridEditor_BasicNudge();
    dj::test_BeatGridEditor_FirstBeatOffset();
    dj::test_BeatGridEditor_UndoRedo();
    dj::test_BeatGridEditor_UndoStackLimit();
    
    // Run tests 5-7 for BeatMarkerOverlay (placeholders for now)
    dj::test_BeatMarkerOverlay_BasicRender();
    dj::test_BeatMarkerOverlay_CurrentBeatHighlight();
    dj::test_BeatMarkerOverlay_EdgeCases();
    
    // Test 8: Integration test (placeholder)
    dj::test_MainLoopIntegration();
    
    std::cout << "\n=== All Phase 36 tests passed! ===" << std::endl;
    return 0;
}
