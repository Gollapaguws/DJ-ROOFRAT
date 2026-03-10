#include <cassert>
#include <iostream>
#include <sstream>

#include "crowdAI/CrowdStateMachine.h"

namespace {

// Test: CrowdStateMachine with Rave personality should have higher BPM weight
void test_CrowdStateMachine_RavePersonality() {
    // Arrange
    dj::CrowdStateMachine machine(dj::CrowdPersonality::Rave);

    // Act: Run multiple updates with high BPM to let energy stabilize
    for (int i = 0; i < 5; ++i) {
        machine.update(180.0f, 0.5f, 0.5f);
    }
    auto output_rave = machine.update(180.0f, 0.5f, 0.5f);

    // Arrange: Reference machine with Default personality for comparison
    dj::CrowdStateMachine machine_default(dj::CrowdPersonality::Default);
    for (int i = 0; i < 5; ++i) {
        machine_default.update(180.0f, 0.5f, 0.5f);
    }
    auto output_default = machine_default.update(180.0f, 0.5f, 0.5f);

    // Assert: Rave personality should respond more aggressively to high BPM
    assert(output_rave.energyMeter > output_default.energyMeter && 
           "Rave should reach higher energy than Default with high BPM");
    assert(output_rave.mood != dj::CrowdMood::Unimpressed && 
           "Rave should not be Unimpressed with high BPM");

    std::cout << "✓ test_CrowdStateMachine_RavePersonality passed\n";
}

// Test: CrowdStateMachine with Jazz personality should have lower sensitivity
void test_CrowdStateMachine_JazzPersonality() {
    // Arrange
    dj::CrowdStateMachine machine(dj::CrowdPersonality::Jazz);

    // Act: Update multiple times with low BPM, low smoothness, low energy
    for (int i = 0; i < 5; ++i) {
        machine.update(90.0f, 0.2f, 0.2f);
    }
    auto output_jazz = machine.update(90.0f, 0.2f, 0.2f);

    // Arrange: Reference machine with Default personality for comparison
    dj::CrowdStateMachine machine_default(dj::CrowdPersonality::Default);
    for (int i = 0; i < 5; ++i) {
        machine_default.update(90.0f, 0.2f, 0.2f);
    }
    auto output_default = machine_default.update(90.0f, 0.2f, 0.2f);

    // Assert: Jazz personality should be more forgiving and stay in higher mood
    assert(output_jazz.mood != dj::CrowdMood::Unimpressed || 
           output_default.mood == dj::CrowdMood::Unimpressed &&
           "Jazz should not get Unimpressed as easily as Default");

    std::cout << "✓ test_CrowdStateMachine_JazzPersonality passed\n";
}

// Test: CrowdStateMachine mood hysteresis prevents flicker
void test_CrowdStateMachine_MoodHysteresis() {
    // Arrange
    dj::CrowdStateMachine machine(dj::CrowdPersonality::Default);

    // Setup: Build up to Grooving mood with consistent high values (20 iterations to stabilize)
    for (int i = 0; i < 20; ++i) {
        machine.update(140.0f, 0.7f, 0.7f);
    }
    auto beforeDrop = machine.update(140.0f, 0.7f, 0.7f);
    assert(beforeDrop.mood == dj::CrowdMood::Grooving || beforeDrop.mood == dj::CrowdMood::Hyped &&
           "Should reach high mood after consistent high values");

    // Act: Introduce small, temporary drop
    auto after1Drop = machine.update(135.0f, 0.65f, 0.65f);
    auto after2Drops = machine.update(135.0f, 0.65f, 0.65f);
    auto after3Drops = machine.update(135.0f, 0.65f, 0.65f);

    // Assert: Small perturbations shouldn't cause mood flicker -
    // hysteresis should keep us in the same mood category
    const bool stayedStable = (beforeDrop.mood == after1Drop.mood && 
                               beforeDrop.mood == after2Drops.mood && 
                               beforeDrop.mood == after3Drops.mood) ||
                              (after1Drop.mood == after2Drops.mood &&
                               after2Drops.mood == after3Drops.mood);
    assert(stayedStable && 
           "Hysteresis should prevent mood flicker from small perturbations");

    // Act: Continue with stable high conditions to recover
    for (int i = 0; i < 5; ++i) {
        machine.update(140.0f, 0.7f, 0.7f);
    }
    auto afterRecovery = machine.update(140.0f, 0.7f, 0.7f);

    // Assert: Should recover to high mood
    assert((afterRecovery.mood == dj::CrowdMood::Grooving || 
            afterRecovery.mood == dj::CrowdMood::Hyped) &&
           "Should recover to high mood after perturbation subsides");

    std::cout << "✓ test_CrowdStateMachine_MoodHysteresis passed\n";
}

// Helper to demonstrate backward compatibility
void test_CrowdStateMachine_DefaultPersonality() {
    // Arrange
    dj::CrowdStateMachine machine(dj::CrowdPersonality::Default);

    // Act: Update with test values
    auto output = machine.update(120.0f, 0.5f, 0.5f);

    // Assert: Should produce valid output
    assert(output.energyMeter >= 0.0f && output.energyMeter <= 1.0f &&
           "Energy meter should be clamped [0, 1]");
    assert(!output.reaction.empty() && "Reaction should not be empty");

    std::cout << "✓ test_CrowdStateMachine_DefaultPersonality passed (backward compatibility)\n";
}

} // namespace

int main() {
    try {
        test_CrowdStateMachine_RavePersonality();
        test_CrowdStateMachine_JazzPersonality();
        test_CrowdStateMachine_MoodHysteresis();
        test_CrowdStateMachine_DefaultPersonality();

        std::cout << "\n✓✓✓ All Phase 5 tests passed! ✓✓✓\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test exception: " << e.what() << "\n";
        return 1;
    }
}
