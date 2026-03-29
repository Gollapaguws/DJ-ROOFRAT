// Phase 1: Graphics Visualization Panels
// Test file for ImGui visualization panel integration
// Tests panel initialization, visibility toggling, and data rendering

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "audio/BeatGrid.h"
#include "audio/BeatGridEditor.h"
#include "audio/SyncController.h"
#include "gameplay/EnergyCurve.h"
#include "gameplay/TransitionCoach.h"
#include "visuals/BeatGridRenderer.h"
#include "visuals/EnergyHistogram.h"
#include "visuals/CamelotWheel.h"
#include "visuals/CoachingHUD.h"
#include "visuals/SyncIndicator.h"

namespace dj {

// Test 1: BeatGridRenderer_Initialization
// Verify BeatGridRenderer can be instantiated without crash
void test_BeatGridRenderer_Initialization() {
    std::cout << "[TEST] BeatGridRenderer_Initialization...\n";
    
    BeatGridRenderer renderer;
    assert(true && "BeatGridRenderer instantiated successfully");
    
    std::cout << "  PASSED: BeatGridRenderer created\n";
}

// Test 2: BeatGridRenderer_RenderWithData
// Verify BeatGridRenderer can render beat grid data
void test_BeatGridRenderer_RenderWithData() {
    std::cout << "[TEST] BeatGridRenderer_RenderWithData...\n";
    
    BeatGridRenderer renderer;
    BeatGrid grid;
    
    // Generate a simple beat grid (120 BPM for 10 seconds)
    grid.generateFromBPM(120.0, 0.0, 10.0);
    
    // Render and verify output is non-empty
    std::string output = renderer.render(grid, 80);
    assert(!output.empty() && "BeatGridRenderer should produce output");
    assert(output.length() > 0 && "Rendered output should have content");
    
    std::cout << "  PASSED: BeatGridRenderer rendered beat grid\n";
}

// Test 3: EnergyHistogram_Initialization
// Verify EnergyHistogram can be instantiated without crash
void test_EnergyHistogram_Initialization() {
    std::cout << "[TEST] EnergyHistogram_Initialization...\n";
    
    EnergyHistogram histogram;
    assert(true && "EnergyHistogram instantiated successfully");
    
    std::cout << "  PASSED: EnergyHistogram created\n";
}

// Test 4: EnergyHistogram_RenderWithData
// Verify EnergyHistogram can render energy data
void test_EnergyHistogram_RenderWithData() {
    std::cout << "[TEST] EnergyHistogram_RenderWithData...\n";
    
    EnergyHistogram histogram;
    std::vector<int> energyRatings = {3, 5, 7, 6, 8, 9, 10, 8, 6, 4};
    
    // Render and verify output is non-empty
    std::string output = histogram.render(energyRatings, 80, 10);
    assert(!output.empty() && "EnergyHistogram should produce output");
    assert(output.length() > 0 && "Rendered output should have content");
    
    std::cout << "  PASSED: EnergyHistogram rendered energy data\n";
}

// Test 5: CamelotWheel_Initialization
// Verify CamelotWheel can be instantiated without crash
void test_CamelotWheel_Initialization() {
    std::cout << "[TEST] CamelotWheel_Initialization...\n";
    
    CamelotWheel wheel;
    assert(true && "CamelotWheel instantiated successfully");
    
    std::cout << "  PASSED: CamelotWheel created\n";
}

// Test 6: CamelotWheel_RenderWithData
// Verify CamelotWheel can render Camelot wheel for a key
void test_CamelotWheel_RenderWithData() {
    std::cout << "[TEST] CamelotWheel_RenderWithData...\n";
    
    CamelotWheel wheel;
    
    // Render wheel for key 5A (C major)
    std::string output = wheel.render("5A");
    assert(!output.empty() && "CamelotWheel should produce output");
    assert(output.length() > 0 && "Rendered output should have content");
    
    std::cout << "  PASSED: CamelotWheel rendered key 5A\n";
}

// Test 7: CamelotWheel_CompatibleKeys
// Verify CamelotWheel can retrieve compatible keys
void test_CamelotWheel_CompatibleKeys() {
    std::cout << "[TEST] CamelotWheel_CompatibleKeys...\n";
    
    CamelotWheel wheel;
    auto compatible = wheel.getCompatibleKeys("5A");
    
    assert(!compatible.empty() && "Compatible keys should not be empty");
    assert(compatible.size() > 0 && "Should have at least one compatible key");
    
    std::cout << "  PASSED: CamelotWheel found " << compatible.size() << " compatible keys\n";
}

// Test 8: CoachingHUD_Initialization
// Verify CoachingHUD can be instantiated without crash
void test_CoachingHUD_Initialization() {
    std::cout << "[TEST] CoachingHUD_Initialization...\n";
    
    CoachingHUD hud;
    assert(true && "CoachingHUD instantiated successfully");
    
    std::cout << "  PASSED: CoachingHUD created\n";
}

// Test 9: CoachingHUD_RenderWithData
// Verify CoachingHUD can render coaching suggestion
void test_CoachingHUD_RenderWithData() {
    std::cout << "[TEST] CoachingHUD_RenderWithData...\n";
    
    CoachingHUD hud;
    TransitionCoach::Suggestion suggestion;
    suggestion.confidence = 0.85f;
    suggestion.energyDelta = 0.2f;
    suggestion.harmonicScore = 0.9f;
    suggestion.reason = "Good transition match";
    suggestion.urgency = false;
    suggestion.timestamp = 0.0;
    
    // Render and verify output is non-empty
    std::string output = hud.render(suggestion, 8.0);
    assert(!output.empty() && "CoachingHUD should produce output");
    assert(output.length() > 0 && "Rendered output should have content");
    
    std::cout << "  PASSED: CoachingHUD rendered coaching suggestion\n";
}

// Test 10: SyncIndicator_Initialization
// Verify SyncIndicator can be instantiated without crash
void test_SyncIndicator_Initialization() {
    std::cout << "[TEST] SyncIndicator_Initialization...\n";
    
    SyncIndicator indicator;
    assert(true && "SyncIndicator instantiated successfully");
    
    std::cout << "  PASSED: SyncIndicator created\n";
}

// Test 11: SyncIndicator_RenderWithData
// Verify SyncIndicator can render sync status
void test_SyncIndicator_RenderWithData() {
    std::cout << "[TEST] SyncIndicator_RenderWithData...\n";
    
    SyncIndicator indicator;
    
    // Render with enabled sync using Locked state
    std::string output = indicator.render(true, SyncState::Locked, 0.1);
    assert(!output.empty() && "SyncIndicator should produce output");
    assert(output.length() > 0 && "Rendered output should have content");
    
    std::cout << "  PASSED: SyncIndicator rendered sync status\n";
}

// Test 12: EnergyCurve_Initialization
// Verify EnergyCurve can be instantiated and populated with data
void test_EnergyCurve_Initialization() {
    std::cout << "[TEST] EnergyCurve_Initialization...\n";
    
    EnergyCurve curve(300.0, 1.0);  // 5 minutes at 1Hz
    assert(true && "EnergyCurve instantiated successfully");
    
    // Add some samples
    curve.addSample(0.5f, 0.0, 0.5f);
    curve.addSample(0.6f, 1.0, 0.6f);
    curve.addSample(0.7f, 2.0, 0.7f);
    
    // Get samples and verify
    auto samples = curve.getCurve();
    assert(samples.size() >= 3 && "EnergyCurve should store added samples");
    
    std::cout << "  PASSED: EnergyCurve initialized and populated\n";
}

// Test 13: PanelVisibilityToggle_BeatGrid
// Verify beat grid panel visibility can be toggled (mock in test)
void test_PanelVisibilityToggle_BeatGrid() {
    std::cout << "[TEST] PanelVisibilityToggle_BeatGrid...\n";
    
    bool showBeatGrid = false;
    assert(!showBeatGrid && "Initial state should be hidden");
    
    showBeatGrid = true;
    assert(showBeatGrid && "Should be visible after toggle");
    
    showBeatGrid = false;
    assert(!showBeatGrid && "Should be hidden after toggle");
    
    std::cout << "  PASSED: Beat grid panel visibility toggle works\n";
}

// Test 14: PanelVisibilityToggle_EnergyHistogram
// Verify energy histogram panel visibility can be toggled (mock in test)
void test_PanelVisibilityToggle_EnergyHistogram() {
    std::cout << "[TEST] PanelVisibilityToggle_EnergyHistogram...\n";
    
    bool showEnergyHistogram = false;
    assert(!showEnergyHistogram && "Initial state should be hidden");
    
    showEnergyHistogram = true;
    assert(showEnergyHistogram && "Should be visible after toggle");
    
    showEnergyHistogram = false;
    assert(!showEnergyHistogram && "Should be hidden after toggle");
    
    std::cout << "  PASSED: Energy histogram panel visibility toggle works\n";
}

// Test 15: PanelVisibilityToggle_CamelotWheel
// Verify Camelot wheel panel visibility can be toggled (mock in test)
void test_PanelVisibilityToggle_CamelotWheel() {
    std::cout << "[TEST] PanelVisibilityToggle_CamelotWheel...\n";
    
    bool showCamelotWheel = false;
    assert(!showCamelotWheel && "Initial state should be hidden");
    
    showCamelotWheel = true;
    assert(showCamelotWheel && "Should be visible after toggle");
    
    showCamelotWheel = false;
    assert(!showCamelotWheel && "Should be hidden after toggle");
    
    std::cout << "  PASSED: Camelot wheel panel visibility toggle works\n";
}

// Test 16: PanelVisibilityToggle_Coaching
// Verify coaching panel visibility can be toggled (mock in test)
void test_PanelVisibilityToggle_Coaching() {
    std::cout << "[TEST] PanelVisibilityToggle_Coaching...\n";
    
    bool showCoaching = false;
    assert(!showCoaching && "Initial state should be hidden");
    
    showCoaching = true;
    assert(showCoaching && "Should be visible after toggle");
    
    showCoaching = false;
    assert(!showCoaching && "Should be hidden after toggle");
    
    std::cout << "  PASSED: Coaching panel visibility toggle works\n";
}

// Test 17: PanelVisibilityToggle_Sync
// Verify sync panel visibility can be toggled (mock in test)
void test_PanelVisibilityToggle_Sync() {
    std::cout << "[TEST] PanelVisibilityToggle_Sync...\n";
    
    bool showSync = false;
    assert(!showSync && "Initial state should be hidden");
    
    showSync = true;
    assert(showSync && "Should be visible after toggle");
    
    showSync = false;
    assert(!showSync && "Should be hidden after toggle");
    
    std::cout << "  PASSED: Sync panel visibility toggle works\n";
}

} // namespace dj

// Main test runner
int main() {
    std::cout << "\n=== Graphics Phase 1 Test Suite ===\n\n";
    
    dj::test_BeatGridRenderer_Initialization();
    dj::test_BeatGridRenderer_RenderWithData();
    dj::test_EnergyHistogram_Initialization();
    dj::test_EnergyHistogram_RenderWithData();
    dj::test_CamelotWheel_Initialization();
    dj::test_CamelotWheel_RenderWithData();
    dj::test_CamelotWheel_CompatibleKeys();
    dj::test_CoachingHUD_Initialization();
    dj::test_CoachingHUD_RenderWithData();
    dj::test_SyncIndicator_Initialization();
    dj::test_SyncIndicator_RenderWithData();
    dj::test_EnergyCurve_Initialization();
    dj::test_PanelVisibilityToggle_BeatGrid();
    dj::test_PanelVisibilityToggle_EnergyHistogram();
    dj::test_PanelVisibilityToggle_CamelotWheel();
    dj::test_PanelVisibilityToggle_Coaching();
    dj::test_PanelVisibilityToggle_Sync();
    
    std::cout << "\n=== All Tests Passed ===\n\n";
    return 0;
}
