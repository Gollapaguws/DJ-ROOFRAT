#include <cassert>
#include <iostream>
#include <set>

#include "crowdAI/CrowdStateMachine.h"

namespace {

// Test: Multiple reactions per mood demonstrate variety (non-repetition strategy)
void test_CrowdStateMachine_ReactionVariety() {
    // Arrange
    dj::CrowdStateMachine machine(dj::CrowdPersonality::Default);

    // Warm up to Grooving mood
    for (int i = 0; i < 15; ++i) {
        machine.update(140.0f, 0.7f, 0.7f, 0.5f);
    }

    // Act: Collect reactions from multiple updates in same mood context
    std::set<std::string> reactionsCollected;
    for (int i = 0; i < 20; ++i) {
        auto output = machine.update(140.0f, 0.7f, 0.7f, 0.5f);
        reactionsCollected.insert(output.reaction);
    }

    // Assert: Should see at least 2 distinct reactions for Grooving mood
    // (3+ reactions per mood in library, but round-robin may show 2-3 in short sequence)
    assert(reactionsCollected.size() >= 2 &&
           "Should see multiple distinct reactions for same mood over time");

    std::cout << "✓ test_CrowdStateMachine_ReactionVariety passed (saw "
              << reactionsCollected.size() << " distinct reactions)\n";
}

// Test: Beatmatch bonus - tight beatmatch (delta < 1.0) boosts energy vs baseline
void test_CrowdStateMachine_BeatmatchBonus() {
    // Arrange: Establish baseline energy without beatmatch bonus
    dj::CrowdStateMachine machineNoBeat(dj::CrowdPersonality::Default);
    for (int i = 0; i < 10; ++i) {
        machineNoBeat.update(120.0f, 0.5f, 0.5f, 2.0f); // Large delta, no bonus
    }
    auto baselineOutput = machineNoBeat.update(120.0f, 0.5f, 0.5f, 2.0f);
    const float baselineEnergy = baselineOutput.energyMeter;

    // Arrange: Machine with tight beatmatch
    dj::CrowdStateMachine machineWithBeat(dj::CrowdPersonality::Default);
    for (int i = 0; i < 10; ++i) {
        machineWithBeat.update(120.0f, 0.5f, 0.5f, 0.5f); // Tight delta < 1.0, bonus applies
    }
    auto boostedOutput = machineWithBeat.update(120.0f, 0.5f, 0.5f, 0.5f);
    const float boostedEnergy = boostedOutput.energyMeter;

    // Assert: Tight beatmatch should result in higher or equal energy
    assert(boostedEnergy >= baselineEnergy &&
           "Tight beatmatch (delta < 1.0) should boost or maintain energy vs large delta");

    std::cout << "✓ test_CrowdStateMachine_BeatmatchBonus passed (baseline: "
              << baselineEnergy << ", boosted: " << boostedEnergy << ")\n";
}

// Test: Beatmatch penalty - poor beatmatch (delta > 3.0) reduces energy vs baseline
void test_CrowdStateMachine_BeatmatchPenalty() {
    // Arrange: Establish baseline energy with good beatmatch
    dj::CrowdStateMachine machineGoodBeat(dj::CrowdPersonality::Default);
    for (int i = 0; i < 15; ++i) {
        machineGoodBeat.update(140.0f, 0.7f, 0.7f, 1.5f); // Good beatmatch
    }
    auto goodOutput = machineGoodBeat.update(140.0f, 0.7f, 0.7f, 1.5f);
    const float goodEnergy = goodOutput.energyMeter;

    // Arrange: Machine with poor beatmatch (delta > 3.0)
    dj::CrowdStateMachine machinePoorBeat(dj::CrowdPersonality::Default);
    for (int i = 0; i < 15; ++i) {
        machinePoorBeat.update(140.0f, 0.7f, 0.7f, 3.5f); // Poor beatmatch delta > 3.0
    }
    auto poorOutput = machinePoorBeat.update(140.0f, 0.7f, 0.7f, 3.5f);
    const float poorEnergy = poorOutput.energyMeter;

    // Assert: Poor beatmatch should result in lower energy
    assert(poorEnergy < goodEnergy &&
           "Poor beatmatch (delta > 3.0) should reduce energy vs good beatmatch");

    std::cout << "✓ test_CrowdStateMachine_BeatmatchPenalty passed (good: "
              << goodEnergy << ", poor: " << poorEnergy << ")\n";
}

// Test: Backward compatibility - old signature (3 args) still works
void test_CrowdStateMachine_BackwardCompatibility() {
    // Arrange
    dj::CrowdStateMachine machine(dj::CrowdPersonality::Default);

    // Act: Call update with 3 arguments (old signature via default parameter)
    auto output = machine.update(120.0f, 0.5f, 0.5f);

    // Assert: Should successfully return valid output
    assert(output.energyMeter >= 0.0f && output.energyMeter <= 1.0f &&
           "Energy meter should be clamped [0, 1]");
    assert(!output.reaction.empty() &&
           "Reaction should not be empty");

    std::cout << "✓ test_CrowdStateMachine_BackwardCompatibility passed\n";
}

// Test: Energy stays clamped to [0, 1] even with extreme beatmatch deltas
void test_CrowdStateMachine_EnergyClamp() {
    // Arrange
    dj::CrowdStateMachine machine(dj::CrowdPersonality::Default);

    // Act: Apply extreme beatmatch values and high inputs
    for (int i = 0; i < 20; ++i) {
        machine.update(200.0f, 1.0f, 1.0f, 10.0f); // Extreme delta
    }
    auto output1 = machine.update(200.0f, 1.0f, 1.0f, 10.0f);

    // Act: Apply extreme low beatmatch values
    for (int i = 0; i < 20; ++i) {
        machine.update(60.0f, 0.0f, 0.0f, 10.0f);
    }
    auto output2 = machine.update(60.0f, 0.0f, 0.0f, 10.0f);

    // Assert: Energy should always be in [0, 1]
    assert(output1.energyMeter >= 0.0f && output1.energyMeter <= 1.0f &&
           "Energy should be clamped even with high inputs");
    assert(output2.energyMeter >= 0.0f && output2.energyMeter <= 1.0f &&
           "Energy should be clamped even with low inputs");

    std::cout << "✓ test_CrowdStateMachine_EnergyClamp passed\n";
}

// Test: Beatmatch bonus effect is favorable vs penalty
void test_CrowdStateMachine_BonusVsPenalty() {
    // Arrange: Create baseline with tight beatmatch (bonus)
    dj::CrowdStateMachine machineBonus(dj::CrowdPersonality::Default);
    for (int i = 0; i < 20; ++i) {
        machineBonus.update(130.0f, 0.6f, 0.6f, 0.3f); // Very tight, strong bonus
    }
    auto bonusOutput = machineBonus.update(130.0f, 0.6f, 0.6f, 0.3f);

    // Arrange: Create machine with poor beatmatch (penalty)
    dj::CrowdStateMachine machinePenalty(dj::CrowdPersonality::Default);
    for (int i = 0; i < 20; ++i) {
        machinePenalty.update(130.0f, 0.6f, 0.6f, 5.0f); // Poor, strong penalty
    }
    auto penaltyOutput = machinePenalty.update(130.0f, 0.6f, 0.6f, 5.0f);

    // Assert: Bonus energy should be noticeably higher than penalty
    assert(bonusOutput.energyMeter > penaltyOutput.energyMeter &&
           "Bonus beatmatch should produce significantly higher energy than penalty");

    std::cout << "✓ test_CrowdStateMachine_BonusVsPenalty passed (bonus: "
              << bonusOutput.energyMeter << ", penalty: " << penaltyOutput.energyMeter << ")\n";
}

} // namespace

int main() {
    try {
        test_CrowdStateMachine_ReactionVariety();
        test_CrowdStateMachine_BeatmatchBonus();
        test_CrowdStateMachine_BeatmatchPenalty();
        test_CrowdStateMachine_BackwardCompatibility();
        test_CrowdStateMachine_EnergyClamp();
        test_CrowdStateMachine_BonusVsPenalty();

        std::cout << "\n✓✓✓ All Phase 6 tests passed! ✓✓✓\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test exception: " << e.what() << "\n";
        return 1;
    }
}
