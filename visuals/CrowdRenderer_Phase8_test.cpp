#include <cassert>
#include <iostream>

#include "visuals/CrowdRenderer.h"

namespace {

// Test: CrowdRenderer can be created and initialized
void test_CrowdRenderer_Initialization() {
    // Arrange
    dj::CrowdRenderer renderer;

    // Act & Assert: Check default state is valid
    assert(renderer.motionAmplitude() >= 0.0f && renderer.motionAmplitude() <= 1.0f &&
           "Motion amplitude should be in [0, 1]");
    assert(renderer.density() >= 0.0f && renderer.density() <= 1.0f &&
           "Density should be in [0, 1]");
    assert(renderer.visibleSilhouettes() >= 0 && renderer.visibleSilhouettes() <= 1000 &&
           "Visible silhouettes should be non-negative and reasonable");

    std::cout << "✓ test_CrowdRenderer_Initialization passed\n";
}

// Test: CrowdRenderer updates based on mood
void test_CrowdRenderer_MoodAnimation() {
    // Arrange
    dj::CrowdRenderer renderer;

    // Act: Update with Unimpressed mood
    renderer.update(0, 0.3f, 0.016f);  // Unimpressed = 0
    float amplitude_low = renderer.motionAmplitude();
    int silhouettes_low = renderer.visibleSilhouettes();

    // Update with Hyped mood (assume 3 = Hyped, matching CrowdMood enum)
    renderer.update(3, 0.9f, 0.016f);  // Hyped = 3
    float amplitude_high = renderer.motionAmplitude();
    int silhouettes_high = renderer.visibleSilhouettes();

    // Assert: Hyped mood should produce higher animation
    assert(amplitude_high > amplitude_low && "Hyped mood should have higher animation amplitude");
    assert(silhouettes_high > silhouettes_low && "Hyped mood should show more silhouettes");

    std::cout << "✓ test_CrowdRenderer_MoodAnimation passed\n";
}

// Test: CrowdRenderer density scales with energy
void test_CrowdRenderer_DensityScaling() {
    // Arrange
    dj::CrowdRenderer renderer;

    // Act: Update with low energy
    renderer.update(2, 0.2f, 0.016f);  // Low energy
    float density_low = renderer.density();

    // Update with high energy
    renderer.update(2, 0.9f, 0.016f);  // High energy
    float density_high = renderer.density();

    // Assert: Higher energy should produce higher density
    assert(density_low >= 0.0f && density_low <= 1.0f && "Low energy density should be in [0, 1]");
    assert(density_high >= 0.0f && density_high <= 1.0f && "High energy density should be in [0, 1]");
    assert(density_high > density_low && "Higher energy should produce higher density");

    std::cout << "✓ test_CrowdRenderer_DensityScaling passed\n";
}

// Test: CrowdRenderer invalid mood is handled safely
void test_CrowdRenderer_SafeMoodHandling() {
    // Arrange
    dj::CrowdRenderer renderer;

    // Act: Update with invalid mood value (should clamp/handle gracefully)
    renderer.update(-100, 0.5f, 0.016f);
    float amplitude1 = renderer.motionAmplitude();
    int silhouettes1 = renderer.visibleSilhouettes();

    renderer.update(999, 0.5f, 0.016f);
    float amplitude2 = renderer.motionAmplitude();
    int silhouettes2 = renderer.visibleSilhouettes();

    // Assert: Should not crash and outputs should be valid
    assert(amplitude1 >= 0.0f && amplitude1 <= 1.0f && "Amplitude should be safe even with bad mood");
    assert(silhouettes1 >= 0 && silhouettes1 <= 1000 && "Silhouettes should be safe even with bad mood");
    assert(amplitude2 >= 0.0f && amplitude2 <= 1.0f && "Amplitude should be safe even with bad mood");
    assert(silhouettes2 >= 0 && silhouettes2 <= 1000 && "Silhouettes should be safe even with bad mood");

    std::cout << "✓ test_CrowdRenderer_SafeMoodHandling passed\n";
}

// Test: CrowdRenderer clamps energy safely
void test_CrowdRenderer_EnergyClamp() {
    // Arrange
    dj::CrowdRenderer renderer;

    // Act: Update with out-of-range energy
    renderer.update(2, -1.0f, 0.016f);  // Negative energy
    float density1 = renderer.density();

    renderer.update(2, 2.5f, 0.016f);  // Energy > 1.0
    float density2 = renderer.density();

    // Assert: Should clamp safely
    assert(density1 >= 0.0f && density1 <= 1.0f && "Density should clamp negative energy");
    assert(density2 >= 0.0f && density2 <= 1.0f && "Density should clamp excess energy");

    std::cout << "✓ test_CrowdRenderer_EnergyClamp passed\n";
}

// Test: CrowdRenderer is deterministic
void test_CrowdRenderer_Deterministic() {
    // Arrange & Act: Run same sequence twice
    dj::CrowdRenderer renderer1;
    renderer1.update(2, 0.5f, 0.016f);
    renderer1.update(2, 0.5f, 0.016f);
    renderer1.update(2, 0.5f, 0.016f);
    float a1 = renderer1.motionAmplitude();
    float d1 = renderer1.density();
    int s1 = renderer1.visibleSilhouettes();

    dj::CrowdRenderer renderer2;
    renderer2.update(2, 0.5f, 0.016f);
    renderer2.update(2, 0.5f, 0.016f);
    renderer2.update(2, 0.5f, 0.016f);
    float a2 = renderer2.motionAmplitude();
    float d2 = renderer2.density();
    int s2 = renderer2.visibleSilhouettes();

    // Assert: Same inputs should produce same outputs
    assert(a1 == a2 && "Motion amplitudes should be deterministic");
    assert(d1 == d2 && "Densities should be deterministic");
    assert(s1 == s2 && "Visible silhouettes should be deterministic");

    std::cout << "✓ test_CrowdRenderer_Deterministic passed\n";
}

// Test: CrowdRenderer never returns negative silhouettes
void test_CrowdRenderer_NonNegativeSilhouettes() {
    // Arrange
    dj::CrowdRenderer renderer;

    // Act: Update with various invalid inputs
    renderer.update(-5, -0.5f, 0.016f);
    int silhouettes1 = renderer.visibleSilhouettes();

    renderer.update(100, 0.0f, 0.016f);
    int silhouettes2 = renderer.visibleSilhouettes();

    renderer.update(0, 0.0f, 0.016f);
    int silhouettes3 = renderer.visibleSilhouettes();

    // Assert: Silhouettes should never be negative
    assert(silhouettes1 >= 0 && "Silhouettes should not be negative");
    assert(silhouettes2 >= 0 && "Silhouettes should not be negative");
    assert(silhouettes3 >= 0 && "Silhouettes should not be negative");

    std::cout << "✓ test_CrowdRenderer_NonNegativeSilhouettes passed\n";
}

} // namespace

int main() {
    std::cout << "Running CrowdRenderer Phase 8 Tests...\n\n";

    test_CrowdRenderer_Initialization();
    test_CrowdRenderer_MoodAnimation();
    test_CrowdRenderer_DensityScaling();
    test_CrowdRenderer_SafeMoodHandling();
    test_CrowdRenderer_EnergyClamp();
    test_CrowdRenderer_Deterministic();
    test_CrowdRenderer_NonNegativeSilhouettes();

    std::cout << "\n✓ All CrowdRenderer Phase 8 tests passed!\n";
    return 0;
}
