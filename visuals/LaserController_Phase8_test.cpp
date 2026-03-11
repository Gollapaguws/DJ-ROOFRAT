#include <cassert>
#include <cmath>
#include <iostream>

#include "visuals/LaserController.h"

namespace {

// Test: LaserController can be created and initialized with sensible defaults
void test_LaserController_Initialization() {
    // Arrange
    dj::LaserController controller;

    // Act & Assert: Check default state is valid
    assert(controller.primaryAngleDegrees() >= -180.0f && controller.primaryAngleDegrees() <= 180.0f &&
           "Primary angle should be within valid range");
    assert(controller.secondaryAngleDegrees() >= -180.0f && controller.secondaryAngleDegrees() <= 180.0f &&
           "Secondary angle should be within valid range");
    assert(controller.intensity() >= 0.0f && controller.intensity() <= 1.0f &&
           "Intensity should be in [0, 1]");

    std::cout << "✓ test_LaserController_Initialization passed\n";
}

// Test: LaserController primary angle follows crossfader position
void test_LaserController_CrossfaderSync() {
    // Arrange
    dj::LaserController controller;

    // Act: Update with crossfader at left (-1.0)
    controller.update(124.0f, -1.0f, 0.016f);
    float angle_left = controller.primaryAngleDegrees();

    // Update with crossfader at center (0.0)
    controller.update(124.0f, 0.0f, 0.016f);
    float angle_center = controller.primaryAngleDegrees();

    // Update with crossfader at right (+1.0)
    controller.update(124.0f, 1.0f, 0.016f);
    float angle_right = controller.primaryAngleDegrees();

    // Assert: Angles should vary with crossfader and stay within [-45, +45]
    assert(angle_left < angle_center && "Left angle should be less than center");
    assert(angle_center < angle_right && "Center angle should be less than right");
    assert(angle_left >= -45.0f && angle_left <= 45.0f && "Left angle should be in [-45, +45]");
    assert(angle_center >= -45.0f && angle_center <= 45.0f && "Center angle should be in [-45, +45]");
    assert(angle_right >= -45.0f && angle_right <= 45.0f && "Right angle should be in [-45, +45]");

    std::cout << "✓ test_LaserController_CrossfaderSync passed\n";
}

// Test: LaserController secondary angle oscillates with BPM
void test_LaserController_BPMSweep() {
    // Arrange
    dj::LaserController controller;

    // Act: Update multiple times at high BPM to see oscillation
    controller.update(200.0f, 0.0f, 0.016f);
    float angle1 = controller.secondaryAngleDegrees();

    controller.update(200.0f, 0.0f, 0.016f);
    float angle2 = controller.secondaryAngleDegrees();

    controller.update(200.0f, 0.0f, 0.016f);
    float angle3 = controller.secondaryAngleDegrees();

    controller.update(200.0f, 0.0f, 0.016f);
    float angle4 = controller.secondaryAngleDegrees();

    // Assert: Secondary angle should be different over time (oscillating)
    // and all values should be in valid range
    bool changed = (angle1 != angle2) || (angle2 != angle3) || (angle3 != angle4);
    assert(changed && "Secondary angle should change over time with BPM");

    assert(angle1 >= -180.0f && angle1 <= 180.0f && "Angle1 should be in valid range");
    assert(angle2 >= -180.0f && angle2 <= 180.0f && "Angle2 should be in valid range");
    assert(angle3 >= -180.0f && angle3 <= 180.0f && "Angle3 should be in valid range");
    assert(angle4 >= -180.0f && angle4 <= 180.0f && "Angle4 should be in valid range");

    std::cout << "✓ test_LaserController_BPMSweep passed\n";
}

// Test: LaserController intensity increases with BPM
void test_LaserController_IntensitySyncBPM() {
    // Arrange
    dj::LaserController controller;

    // Act: Update with low BPM
    controller.update(60.0f, 0.0f, 0.016f);
    float intensity_low = controller.intensity();

    // Update with high BPM
    for (int i = 0; i < 10; ++i) {
        controller.update(180.0f, 0.0f, 0.016f);
    }
    float intensity_high = controller.intensity();

    // Assert: Higher BPM should yield higher intensity
    assert(intensity_low >= 0.0f && intensity_low <= 1.0f && "Low BPM intensity should be in [0, 1]");
    assert(intensity_high >= 0.0f && intensity_high <= 1.0f && "High BPM intensity should be in [0, 1]");
    assert(intensity_high > intensity_low && "Higher BPM should produce higher intensity");

    std::cout << "✓ test_LaserController_IntensitySyncBPM passed\n";
}

// Test: LaserController clamps all outputs safely
void test_LaserController_Clamp() {
    // Arrange
    dj::LaserController controller;

    // Act: Update with extreme values
    controller.update(999.0f, -2.0f, 1.0f);

    // Assert: All outputs should be clamped to safe ranges
    float primary = controller.primaryAngleDegrees();
    float secondary = controller.secondaryAngleDegrees();
    float intensity = controller.intensity();

    assert(primary >= -45.0f && primary <= 45.0f && "Primary angle should be clamped to [-45, +45]");
    assert(secondary >= -180.0f && secondary <= 180.0f && "Secondary angle should be clamped to [-180, +180]");
    assert(intensity >= 0.0f && intensity <= 1.0f && "Intensity should be clamped to [0, 1]");

    std::cout << "✓ test_LaserController_Clamp passed\n";
}

// Test: LaserController is deterministic
void test_LaserController_Deterministic() {
    // Arrange & Act: Run same sequence twice
    dj::LaserController controller1;
    controller1.update(124.0f, 0.5f, 0.016f);
    controller1.update(124.0f, 0.5f, 0.016f);
    controller1.update(124.0f, 0.5f, 0.016f);
    float p1_final = controller1.primaryAngleDegrees();
    float s1_final = controller1.secondaryAngleDegrees();
    float i1_final = controller1.intensity();

    dj::LaserController controller2;
    controller2.update(124.0f, 0.5f, 0.016f);
    controller2.update(124.0f, 0.5f, 0.016f);
    controller2.update(124.0f, 0.5f, 0.016f);
    float p2_final = controller2.primaryAngleDegrees();
    float s2_final = controller2.secondaryAngleDegrees();
    float i2_final = controller2.intensity();

    // Assert: Same inputs should produce same outputs
    assert(p1_final == p2_final && "Primary angles should be deterministic");
    assert(s1_final == s2_final && "Secondary angles should be deterministic");
    assert(i1_final == i2_final && "Intensities should be deterministic");

    std::cout << "✓ test_LaserController_Deterministic passed\n";
}

} // namespace

int main() {
    std::cout << "Running LaserController Phase 8 Tests...\n\n";

    test_LaserController_Initialization();
    test_LaserController_CrossfaderSync();
    test_LaserController_BPMSweep();
    test_LaserController_IntensitySyncBPM();
    test_LaserController_Clamp();
    test_LaserController_Deterministic();

    std::cout << "\n✓ All LaserController Phase 8 tests passed!\n";
    return 0;
}
