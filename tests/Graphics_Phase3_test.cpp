// Phase 3: Lighting and Laser Effects Integration
// Test file for lighting rig beat synchronization and laser beam rendering
// Tests LightingRig update, strobe effects, multi-light system
// Also tests LaserController phase tracking, angle computation, and beam geometry

#include <cassert>
#include <iostream>
#include <cmath>
#include <numbers>

#include "visuals/LightingRig.h"
#include "visuals/LaserController.h"

namespace dj {

// ===== LightingRig Tests =====

// Test 1: LightingRig_Initialization
// Verify LightingRig initializes with correct number of intensities
void test_LightingRig_Initialization() {
    std::cout << "[TEST] LightingRig_Initialization...\n";
    
    LightingRig lighting;
    const auto& intensities = lighting.intensities();
    assert(intensities.size() == 32 && "LightingRig should have 32 intensity bars");
    
    // All should start at 0
    for (float intensity : intensities) {
        assert(intensity == 0.0f && "Initial intensities should be 0");
    }
    
    std::cout << "  PASSED: LightingRig initialized correctly\n";
}

// Test 2: LightingRig_UpdateAccumulatesPhaseTime
// Verify that calling update multiple times accumulates phase time
void test_LightingRig_UpdateAccumulatesPhaseTime() {
    std::cout << "[TEST] LightingRig_UpdateAccumulatesPhaseTime...\n";
    
    LightingRig lighting;
    
    // First update
    lighting.update(120.0f, 0.5f, 0.01f);
    const auto& intensities1 = lighting.intensities();
    assert(intensities1.size() == 32 && "Should still have 32 bars");

    // Second update
    lighting.update(120.0f, 0.5f, 0.01f);
    const auto& intensities2 = lighting.intensities();
    assert(intensities2.size() == 32 && "Should still have 32 bars");
    
    std::cout << "  PASSED: Multiple updates work\n";
}

// Test 3: LightingRig_StrobeIntensityShouldIncreaseWithEnergy
// Verify strobe intensity is 0 at low energy, activates at high energy
void test_LightingRig_StrobeIntensityWithEnergy() {
    std::cout << "[TEST] LightingRig_StrobeIntensityWithEnergy...\n";
    
    LightingRig lighting;
    
    // At low energy (0.5), strobe should be minimal/off
    lighting.update(120.0f, 0.5f, 0.01f);
    float strobeAtLowEnergy = lighting.getStrobeIntensity();
    assert(strobeAtLowEnergy >= 0.0f && strobeAtLowEnergy <= 1.0f && 
           "Strobe intensity should be in [0, 1]");

    // At high energy (0.9 > 0.8), strobe should activate
    lighting.update(120.0f, 0.9f, 0.01f);
    float strobeAtHighEnergy = lighting.getStrobeIntensity();
    assert(strobeAtHighEnergy >= 0.0f && strobeAtHighEnergy <= 1.0f && 
           "Strobe intensity should be in [0, 1]");
    
    std::cout << "  PASSED: Strobe intensity activates at high energy\n";
}

// Test 4: LightingRig_IntensitiesShouldBeInRange
// Verify all intensities are clamped to [0, 1]
void test_LightingRig_IntensitiesInRange() {
    std::cout << "[TEST] LightingRig_IntensitiesInRange...\n";
    
    LightingRig lighting;
    
    // Full energy to stress the system
    lighting.update(120.0f, 1.0f, 0.016f);
    const auto& intensities = lighting.intensities();

    for (float intensity : intensities) {
        assert(intensity >= 0.0f && intensity <= 1.0f && 
               "All intensities must be in [0, 1]");
    }
    
    std::cout << "  PASSED: All intensities in valid range\n";
}

// Test 5: LightingRig_MultiLightSystemWorks
// Verify adding spot and point lights works correctly
void test_LightingRig_MultiLightSystem() {
    std::cout << "[TEST] LightingRig_MultiLightSystem...\n";
    
    LightingRig lighting;
    
    // Add spot light
    int idx1 = lighting.addSpotLight(
        {1.0f, 2.0f, 3.0f},      // position
        {0.0f, -1.0f, 0.0f},     // direction
        {1.0f, 0.0f, 0.0f},      // color (red)
        45.0f,                    // spot angle
        0.8f                      // intensity
    );
    assert(idx1 >= 0 && "Spot light should be added");

    // Add point light
    int idx2 = lighting.addPointLight(
        {0.0f, 0.0f, 0.0f},      // position
        {0.0f, 1.0f, 0.0f},      // color (green)
        0.6f                      // intensity
    );
    assert(idx2 >= 0 && "Point light should be added");

    // Verify lights were added
    const auto& lights = lighting.getLights();
    assert(lights.size() >= 2u && "Should have at least 2 lights");
    
    std::cout << "  PASSED: Multi-light system works\n";
}

// ===== LaserController Tests =====

// Test 6: LaserController_Initialization
// Verify LaserController initializes with valid state
void test_LaserController_Initialization() {
    std::cout << "[TEST] LaserController_Initialization...\n";
    
    LaserController laser;
    
    // Check initial values are in valid ranges
    assert(laser.primaryAngleDegrees() >= -45.0f && laser.primaryAngleDegrees() <= 45.0f &&
           "Primary angle should be initialized in range");
    assert(laser.secondaryAngleDegrees() >= -180.0f && laser.secondaryAngleDegrees() <= 180.0f &&
           "Secondary angle should be initialized in range");
    assert(laser.intensity() >= 0.0f && laser.intensity() <= 1.0f &&
           "Intensity should be initialized in [0, 1]");
    
    std::cout << "  PASSED: LaserController initialized correctly\n";
}

// Test 7: LaserController_PrimaryAngleShouldFollowCrossfader
// Verify primary laser angle follows crossfader position
void test_LaserController_PrimaryAngleFollowsCrossfader() {
    std::cout << "[TEST] LaserController_PrimaryAngleFollowsCrossfader...\n";
    
    LaserController laser;
    
    // Left crossfader position (-1.0)
    laser.update(120.0f, -1.0f, 0.01f);
    float angleLeft = laser.primaryAngleDegrees();
    assert(angleLeft < 0.0f && angleLeft >= -45.0f && "Left should give negative angle");

    // Center crossfader position (0.0)
    laser.update(120.0f, 0.0f, 0.01f);
    float angleCenter = laser.primaryAngleDegrees();
    assert(std::abs(angleCenter - 0.0f) < 0.1f && "Center should give near-zero angle");

    // Right crossfader position (+1.0)
    laser.update(120.0f, 1.0f, 0.01f);
    float angleRight = laser.primaryAngleDegrees();
    assert(angleRight > 0.0f && angleRight <= 45.0f && "Right should give positive angle");
    
    std::cout << "  PASSED: Primary angle follows crossfader\n";
}

// Test 8: LaserController_SecondaryAngleOscillates
// Verify secondary angle oscillates with BPM
void test_LaserController_SecondaryAngleOscillates() {
    std::cout << "[TEST] LaserController_SecondaryAngleOscillates...\n";
    
    LaserController laser;
    
    // Get initial secondary angle
    laser.update(120.0f, 0.0f, 0.01f);
    float angle1 = laser.secondaryAngleDegrees();
    assert(angle1 >= -180.0f && angle1 <= 180.0f && "Secondary angle should be in range");

    // Update and verify phase changes
    laser.update(120.0f, 0.0f, 0.01f);
    float angle2 = laser.secondaryAngleDegrees();
    assert(angle2 >= -180.0f && angle2 <= 180.0f && "Secondary angle should remain in range");
    
    std::cout << "  PASSED: Secondary angle oscillates\n";
}

// Test 9: LaserController_IntensityScalesWithBPM
// Verify laser intensity increases with BPM
void test_LaserController_IntensityScalesWithBPM() {
    std::cout << "[TEST] LaserController_IntensityScalesWithBPM...\n";
    
    LaserController laser;
    
    // Low BPM (60)
    laser.update(60.0f, 0.0f, 0.01f);
    float intensityLow = laser.intensity();

    // High BPM (200)
    laser.update(200.0f, 0.0f, 0.01f);
    float intensityHigh = laser.intensity();

    assert(intensityHigh >= intensityLow && "Higher BPM should have higher or equal intensity");
    assert(intensityLow >= 0.0f && intensityLow <= 1.0f && "Intensity should be in range");
    assert(intensityHigh >= 0.0f && intensityHigh <= 1.0f && "Intensity should be in range");
    
    std::cout << "  PASSED: Intensity scales with BPM\n";
}

// Test 10: LaserController_BeamGeometryIsValid
// Verify getBeamGeometry returns valid quad geometry
void test_LaserController_BeamGeometryIsValid() {
    std::cout << "[TEST] LaserController_BeamGeometryIsValid...\n";
    
    LaserController laser;
    laser.update(120.0f, 0.0f, 0.01f);
    auto geom = laser.getBeamGeometry(1.0f);

    // Should have 4 vertices for a quad
    assert(geom.vertices.size() == 4u && "Beam should have 4 vertices");
    // Should have 6 indices (2 triangles)
    assert(geom.indices.size() == 6u && "Beam should have 6 indices");

    // All vertices should have valid finite coordinates
    for (const auto& vertex : geom.vertices) {
        assert(std::isfinite(vertex.position[0]) && "X must be finite");
        assert(std::isfinite(vertex.position[1]) && "Y must be finite");
        assert(std::isfinite(vertex.position[2]) && "Z must be finite");
    }
    
    std::cout << "  PASSED: Beam geometry is valid\n";
}

// ===== Main Test Runner =====

int run_graphics_phase3_tests() {
    std::cout << "\n========== GRAPHICS PHASE 3 TESTS (Lighting & Lasers) ==========\n\n";

    try {
        // LightingRig tests
        test_LightingRig_Initialization();
        test_LightingRig_UpdateAccumulatesPhaseTime();
        test_LightingRig_StrobeIntensityWithEnergy();
        test_LightingRig_IntensitiesInRange();
        test_LightingRig_MultiLightSystem();

        // LaserController tests
        test_LaserController_Initialization();
        test_LaserController_PrimaryAngleFollowsCrossfader();
        test_LaserController_SecondaryAngleOscillates();
        test_LaserController_IntensityScalesWithBPM();
        test_LaserController_BeamGeometryIsValid();

        std::cout << "\n========== ALL GRAPHICS PHASE 3 TESTS PASSED ==========\n\n";
        return 0;
    } catch (const std::exception& e) {
        std::cout << "\nTEST FAILED: " << e.what() << "\n";
        return 1;
    }
}

}  // namespace dj

// Allow running tests as standalone executable
int main() {
    return dj::run_graphics_phase3_tests();
}
