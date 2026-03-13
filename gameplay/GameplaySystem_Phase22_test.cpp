#include "gameplay/MissionSystem.h"
#include "gameplay/Mission.h"
#include "gameplay/missions/SustainedBeatmatchMission.h"
#include "gameplay/missions/EnergySurvivalMission.h"
#include "gameplay/missions/TransitionMission.h"
#include "gameplay/Leaderboard.h"

#include <cassert>
#include <iostream>
#include <filesystem>
#include <cmath>

namespace dj {

// Test 1: MissionSystem Lifecycle
void test_MissionSystem_Lifecycle() {
    std::cout << "Running test_MissionSystem_Lifecycle..." << std::endl;
    
    MissionSystem system;
    
    // Initially should be idle
    assert(system.getMissionState() == MissionState::Idle);
    assert(!system.isMissionActive());
    assert(system.getCurrentMission() == nullptr);
    
    // Create a mock mission
    auto mission = std::make_shared<SustainedBeatmatchMission>();
    
    // Start mission
    system.startMission(mission);
    assert(system.getMissionState() == MissionState::Running);
    assert(system.isMissionActive());
    assert(system.getCurrentMission() == mission);
    
    // Simulate completion
    auto beatmatchMission = std::static_pointer_cast<SustainedBeatmatchMission>(mission);
    beatmatchMission->validateBeatmatch(120.0f, 120.5f, 0);
    for (int i = 0; i < 1323000; ++i) {
        beatmatchMission->validateBeatmatch(120.0f, 120.5f, i);
    }
    
    // Update system
    system.updateMission(0);
    
    // Should transition to Complete
    assert(system.getMissionState() == MissionState::Complete);
    assert(!system.isMissionActive());
    
    std::cout << "✓ test_MissionSystem_Lifecycle passed" << std::endl;
}

// Test 2: SustainedBeatmatchMission Timer
void test_SustainedBeatmatchMission_Timer() {
    std::cout << "Running test_SustainedBeatmatchMission_Timer..." << std::endl;
    
    auto mission = std::make_shared<SustainedBeatmatchMission>();
    mission->setup();
    
    // Validate beatmatch for 30 seconds (1323000 blocks)
    float bpmA = 120.0f;
    float bpmB = 120.8f;  // Within 2% tolerance (120.8/120 = 1.0067, delta = 0.67%)
    
    for (int i = 0; i < 1323000; ++i) {
        mission->validateBeatmatch(bpmA, bpmB, i);
    }
    
    assert(mission->isComplete());
    assert(!mission->isFailed());
    assert(mission->getProgress() >= 0.99f);  // Should be nearly 1.0
    
    std::cout << "✓ test_SustainedBeatmatchMission_Timer passed" << std::endl;
}

// Test 3: SustainedBeatmatchMission Failure (Timer Reset)
void test_SustainedBeatmatchMission_Failure() {
    std::cout << "Running test_SustainedBeatmatchMission_Failure..." << std::endl;
    
    auto mission = std::make_shared<SustainedBeatmatchMission>();
    mission->setup();
    
    // Validate beatmatch for 10 seconds
    for (int i = 0; i < 441000; ++i) {  // 10 seconds at 44.1kHz
        mission->validateBeatmatch(120.0f, 120.8f, i);
    }
    
    // Should still be in progress, not complete
    assert(!mission->isComplete());
    
    // Now break the match (delta > 2%)
    mission->validateBeatmatch(120.0f, 122.5f, 441000);  // Delta = 2.08%
    
    // Timer should reset
    assert(!mission->isComplete());
    assert(mission->getProgress() == 0.0f);
    
    std::cout << "✓ test_SustainedBeatmatchMission_Failure passed" << std::endl;
}

// Test 4: EnergySurvivalMission Threshold Detection
void test_EnergySurvivalMission_ThresholdDetection() {
    std::cout << "Running test_EnergySurvivalMission_ThresholdDetection..." << std::endl;
    
    auto mission = std::make_shared<EnergySurvivalMission>();
    mission->setup();
    
    // Validate energy > 0.5 for 60 seconds (2646000 blocks)
    for (int i = 0; i < 2646000; ++i) {
        mission->validateEnergy(0.7f, i);  // Energy above threshold
    }
    
    assert(mission->isComplete());
    assert(!mission->isFailed());
    
    // Test instant failure when energy drops below threshold
    auto mission2 = std::make_shared<EnergySurvivalMission>();
    mission2->setup();
    
    // Validate for some blocks above threshold
    for (int i = 0; i < 100000; ++i) {
        mission2->validateEnergy(0.6f, i);
    }
    
    // Drop energy below threshold
    mission2->validateEnergy(0.4f, 100000);
    
    assert(mission2->isFailed());
    
    std::cout << "✓ test_EnergySurvivalMission_ThresholdDetection passed" << std::endl;
}

// Test 5: TransitionMission Smoothness Boundary
void test_TransitionMission_SmoothnessBoundary() {
    std::cout << "Running test_TransitionMission_SmoothnessBoundary..." << std::endl;
    
    auto mission = std::make_shared<TransitionMission>();
    mission->setup();
    
    // Test below threshold (0.6)
    mission->validateTransition(0.6f, 0);
    assert(!mission->isComplete());
    assert(!mission->isFailed());  // No instant failure
    
    // Reset and test above threshold (0.75)
    auto mission2 = std::make_shared<TransitionMission>();
    mission2->setup();
    mission2->validateTransition(0.75f, 0);
    assert(mission2->isComplete());
    
    std::cout << "✓ test_TransitionMission_SmoothnessBoundary passed" << std::endl;
}

// Test 6: Leaderboard Persistence and Ranking
void test_Leaderboard_PersistenceAndRanking() {
    std::cout << "Running test_Leaderboard_PersistenceAndRanking..." << std::endl;
    
    const std::string testFile = "leaderboard_test_phase22.json";
    
    {
        Leaderboard leaderboard;
        
        // Add 3 scores in random order
        leaderboard.addScore("Player1", 1000.0f, "SustainedBeatmatch");
        leaderboard.addScore("Player2", 2500.0f, "SustainedBeatmatch");
        leaderboard.addScore("Player3", 1500.0f, "SustainedBeatmatch");
        
        // Save to file
        bool saved = leaderboard.saveToFile(testFile);
        assert(saved);
    }
    
    // Load from file
    {
        Leaderboard loaded;
        bool loadedSuccess = loaded.loadFromFile(testFile);
        assert(loadedSuccess);
        
        // Get top scores
        auto topScores = loaded.getTopScores("SustainedBeatmatch", 3);
        assert(topScores.size() == 3);
        
        // Verify sorted descending
        assert(topScores[0].playerName == "Player2");
        assert(topScores[0].score == 2500.0f);
        
        assert(topScores[1].playerName == "Player3");
        assert(topScores[1].score == 1500.0f);
        
        assert(topScores[2].playerName == "Player1");
        assert(topScores[2].score == 1000.0f);
    }
    
    // Cleanup
    std::filesystem::remove(testFile);
    
    std::cout << "✓ test_Leaderboard_PersistenceAndRanking passed" << std::endl;
}

// Test 7: Leaderboard Multi-Mission Tracking
void test_Leaderboard_MultiMissionTracking() {
    std::cout << "Running test_Leaderboard_MultiMissionTracking..." << std::endl;
    
    Leaderboard leaderboard;
    
    // Add scores for 2 different missions
    leaderboard.addScore("Alice", 5000.0f, "SustainedBeatmatch");
    leaderboard.addScore("Bob", 4000.0f, "SustainedBeatmatch");
    leaderboard.addScore("Charlie", 3500.0f, "EnergySurvival");
    leaderboard.addScore("Diana", 4500.0f, "EnergySurvival");
    
    // Test filtering by mission
    auto beatmatchTopScores = leaderboard.getTopScores("SustainedBeatmatch", 10);
    assert(beatmatchTopScores.size() == 2);
    assert(beatmatchTopScores[0].playerName == "Alice");
    assert(beatmatchTopScores[0].missionName == "SustainedBeatmatch");
    assert(beatmatchTopScores[1].playerName == "Bob");
    
    auto energyTopScores = leaderboard.getTopScores("EnergySurvival", 10);
    assert(energyTopScores.size() == 2);
    assert(energyTopScores[0].playerName == "Diana");
    assert(energyTopScores[0].missionName == "EnergySurvival");
    assert(energyTopScores[1].playerName == "Charlie");
    
    std::cout << "✓ test_Leaderboard_MultiMissionTracking passed" << std::endl;
}

// Run all tests
void runAllMissionTests() {
    std::cout << "\n=== Running GameplaySystem Phase 22 Tests ===" << std::endl;
    try {
        test_MissionSystem_Lifecycle();
        test_SustainedBeatmatchMission_Timer();
        test_SustainedBeatmatchMission_Failure();
        test_EnergySurvivalMission_ThresholdDetection();
        test_TransitionMission_SmoothnessBoundary();
        test_Leaderboard_PersistenceAndRanking();
        test_Leaderboard_MultiMissionTracking();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
    }
}

}  // namespace dj

// Entry point for running tests
int main() {
    dj::runAllMissionTests();
    std::cout << "\n=== GameplaySystem Phase 22 Tests Complete ===" << std::endl;
    return 0;
}
