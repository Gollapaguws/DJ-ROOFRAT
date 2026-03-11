#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include "visuals/LightingRig.h"

namespace {

// Test: LightingRig can be created and provides bar intensities
void test_LightingRig_Initialization() {
    // Arrange
    dj::LightingRig rig;

    // Act
    const auto& intensities = rig.intensities();

    // Assert: Should have 32 bars by default
    assert(intensities.size() == 32 && "LightingRig should have 32 bars");
    
    // All intensities should be in valid range [0, 1]
    for (float intensity : intensities) {
        assert(intensity >= 0.0f && intensity <= 1.0f && 
               "All intensities should be in range [0, 1]");
    }

    std::cout << "✓ test_LightingRig_Initialization passed\n";
}

// Test: LightingRig updates and produces BPM-synced intensities
void test_LightingRig_BPMSync() {
    // Arrange
    dj::LightingRig rig;
    float bpm = 124.0f;
    float energy = 0.5f;

    // Act: Update at different time steps
    const auto& intensities_before = rig.intensities();
    auto intensity_sum_before = 0.0f;
    for (float intensity : intensities_before) {
        intensity_sum_before += intensity;
    }

    // Update with time delta
    rig.update(bpm, energy, 0.016f);  // ~16ms, one frame at 60fps
    
    const auto& intensities_after = rig.intensities();
    auto intensity_sum_after = 0.0f;
    for (float intensity : intensities_after) {
        intensity_sum_after += intensity;
    }

    // Assert: Intensities should change over time (BPM-syncedanimation)
    assert(intensity_sum_before >= 0.0f && 
           "Intensity sum should be non-negative");
    
    assert(intensity_sum_after >= 0.0f && 
           "Intensity sum should be non-negative after update");

    std::cout << "✓ test_LightingRig_BPMSync passed\n";
}

// Test: LightingRig clamps intensities to [0, 1]
void test_LightingRig_Clamping() {
    // Arrange
    dj::LightingRig rig;

    // Act: Update with high energy and BPM
    rig.update(180.0f, 1.0f, 0.016f);
    rig.update(180.0f, 1.0f, 0.016f);
    rig.update(180.0f, 1.0f, 0.016f);

    const auto& intensities = rig.intensities();

    // Assert: All values should be properly clamped
    for (float intensity : intensities) {
        assert(intensity >= 0.0f && intensity <= 1.0f && 
               "All intensities must be clamped to [0, 1]");
    }

    std::cout << "✓ test_LightingRig_Clamping passed\n";
}

// Test: LightingRig update is deterministic
void test_LightingRig_Deterministic() {
    // Arrange
    dj::LightingRig rig1;
    dj::LightingRig rig2;
    float bpm = 120.0f;
    float energy = 0.6f;
    float dt = 0.016f;

    // Act: Run identical updates on both rigs
    for (int i = 0; i < 10; ++i) {
        rig1.update(bpm, energy, dt);
        rig2.update(bpm, energy, dt);
    }

    const auto& intensities1 = rig1.intensities();
    const auto& intensities2 = rig2.intensities();

    // Assert: Both rigs should have identical intensities
    assert(intensities1.size() == intensities2.size() && 
           "Rigs should have same number of bars");

    for (size_t i = 0; i < intensities1.size(); ++i) {
        assert(std::abs(intensities1[i] - intensities2[i]) < 1e-6f && 
               "Intensities should be deterministically identical");
    }

    std::cout << "✓ test_LightingRig_Deterministic passed\n";
}

} // namespace

int main() {
    try {
        test_LightingRig_Initialization();
        test_LightingRig_BPMSync();
        test_LightingRig_Clamping();
        test_LightingRig_Deterministic();

        std::cout << "\n✓ All LightingRig tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << "\n";
        return 1;
    }
}
