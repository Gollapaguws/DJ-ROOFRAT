#include "multiplayer/BattleMode.h"
#include "multiplayer/BattleRules.h"
#include "multiplayer/Judge.h"

#include <cassert>
#include <iostream>

namespace dj {

// Test 1: Turn-based battle mode switching
void test_BattleMode_TurnBasedSwitching() {
    BattleMode battle;
    auto rules = std::make_shared<BattleRules>(BattleFormat::Quick, TurnMode::TurnBased);
    
    battle.startBattle(rules);
    assert(battle.isBattleActive());
    assert(battle.getState() == BattleState::Player1Turn);
    assert(battle.getCurrentPlayer() == 1);
    
    battle.switchPlayer();
    assert(battle.getCurrentPlayer() == 2);
    assert(battle.getState() == BattleState::Player2Turn);
    
    battle.switchPlayer();
    assert(battle.getCurrentPlayer() == 1);
    
    std::cout << "✓ test_BattleMode_TurnBasedSwitching passed" << std::endl;
}

// Test 2: Simultaneous battle mode
void test_BattleMode_SimultaneousMode() {
    BattleMode battle;
    auto rules = std::make_shared<BattleRules>(BattleFormat::Standard, TurnMode::Simultaneous);
    
    battle.startBattle(rules);
    assert(battle.isBattleActive());
    assert(battle.getState() == BattleState::Simultaneous);
    
    // Submit performance for both players
    battle.submitPerformance(1, 0.5f, 0.9f, 0.85f, 1000);
    battle.submitPerformance(2, 0.3f, 0.95f, 0.90f, 1000);
    
    // Both players should have scores
    PlayerScore score1 = battle.getPlayerScore(1);
    PlayerScore score2 = battle.getPlayerScore(2);
    assert(score1.totalScore > 0.0f);
    assert(score2.totalScore > 0.0f);
    
    std::cout << "✓ test_BattleMode_SimultaneousMode passed" << std::endl;
}

// Test 3: Battle rules round counts
void test_BattleRules_RoundCounts() {
    BattleRules quick(BattleFormat::Quick, TurnMode::TurnBased);
    assert(quick.getRounds() == 1);
    
    BattleRules standard(BattleFormat::Standard, TurnMode::TurnBased);
    assert(standard.getRounds() == 3);
    
    BattleRules tournament(BattleFormat::Tournament, TurnMode::TurnBased);
    assert(tournament.getRounds() == 5);
    
    std::cout << "✓ test_BattleRules_RoundCounts passed" << std::endl;
}

// Test 4: Judge beatmatch scoring
void test_Judge_BeatmatchScoring() {
    Judge judge;
    
    // Perfect beatmatch (delta < 0.5)
    float score1 = judge.scoreBeatmatching(0.3f);
    assert(score1 == 100.0f);
    
    // Good beatmatch (delta < 2.0)
    float score2 = judge.scoreBeatmatching(1.5f);
    assert(score2 == 80.0f);
    
    // Okay beatmatch (delta < 5.0)
    float score3 = judge.scoreBeatmatching(4.0f);
    assert(score3 == 50.0f);
    
    // Poor beatmatch (delta >= 5.0)
    float score4 = judge.scoreBeatmatching(10.0f);
    assert(score4 == 0.0f);
    
    std::cout << "✓ test_Judge_BeatmatchScoring passed" << std::endl;
}

// Test 5: Judge transition scoring
void test_Judge_TransitionScoring() {
    Judge judge;
    
    float score1 = judge.scoreTransitions(0.0f);
    assert(score1 == 0.0f);
    
    float score2 = judge.scoreTransitions(0.5f);
    assert(score2 == 50.0f);
    
    float score3 = judge.scoreTransitions(1.0f);
    assert(score3 == 100.0f);
    
    std::cout << "✓ test_Judge_TransitionScoring passed" << std::endl;
}

// Test 6: Judge performance evaluation
void test_Judge_PerformanceEvaluation() {
    Judge judge;
    
    PerformanceMetrics metrics;
    metrics.beatmatchDelta = 0.4f;
    metrics.transitionSmoothness = 0.8f;
    metrics.crowdEnergy = 0.7f;
    metrics.blockNumber = 500;
    
    JudgmentScores scores = judge.evaluatePerformance(metrics);
    
    assert(scores.beatmatchScore == 100.0f);
    assert(scores.transitionScore == 80.0f);
    assert(scores.energyScore == 70.0f);
    assert(scores.crowdScore == 70.0f);
    assert(scores.creativityScore >= 0.0f);
    
    std::cout << "✓ test_Judge_PerformanceEvaluation passed" << std::endl;
}

// Test 7: Battle winner determination
void test_BattleMode_WinnerDetermination() {
    BattleMode battle;
    auto rules = std::make_shared<BattleRules>(BattleFormat::Quick, TurnMode::TurnBased);
    
    battle.startBattle(rules);
    
    // Player 1 excellent performance
    battle.submitPerformance(1, 0.3f, 0.95f, 0.9f, 1000);
    
    // Player 2 poor performance
    battle.submitPerformance(2, 8.0f, 0.4f, 0.3f, 1000);
    
    // Player 1 should win
    int winner = battle.getWinner();
    assert(winner == 1);
    
    std::cout << "✓ test_BattleMode_WinnerDetermination passed" << std::endl;
}

// Test 8: Multi-round progression and battle completion
void test_BattleMode_MultiRoundProgression() {
    BattleMode battle;
    auto rules = std::make_shared<BattleRules>(BattleFormat::Standard, TurnMode::TurnBased);
    
    battle.startBattle(rules);
    assert(battle.getCurrentRound() == 1);
    assert(battle.getTotalRounds() == 3);
    assert(battle.isBattleActive());
    
    // Round 1: Submit performances
    battle.submitPerformance(1, 0.5f, 0.9f, 0.85f, 1000);
    assert(battle.getCurrentRound() == 1);
    
    battle.submitPerformance(2, 0.3f, 0.95f, 0.90f, 1000);
    
    // Advance to round 2
    battle.advanceRound();
    assert(battle.getCurrentRound() == 2);
    assert(battle.getState() != BattleState::Complete);
    assert(battle.isBattleActive());
    
    // Round 2: Submit performances
    battle.submitPerformance(1, 0.4f, 0.88f, 0.88f, 2000);
    battle.submitPerformance(2, 0.6f, 0.92f, 0.92f, 2000);
    
    // Advance to round 3
    battle.advanceRound();
    assert(battle.getCurrentRound() == 3);
    assert(battle.getState() != BattleState::Complete);
    assert(battle.isBattleActive());
    
    // Round 3: Submit performances
    battle.submitPerformance(1, 0.2f, 0.98f, 0.95f, 3000);
    battle.submitPerformance(2, 0.5f, 0.90f, 0.88f, 3000);
    
    // Advance to complete
    battle.advanceRound();
    assert(battle.getCurrentRound() == 4);  // Exceeds total rounds
    assert(battle.getState() == BattleState::Complete);
    assert(!battle.isBattleActive());
    
    std::cout << "✓ test_BattleMode_MultiRoundProgression passed" << std::endl;
}

void runAllBattleSystemTests() {
    std::cout << "\n=== Running BattleSystem Phase 24 Tests ===" << std::endl;
    try {
        test_BattleMode_TurnBasedSwitching();
        test_BattleMode_SimultaneousMode();
        test_BattleRules_RoundCounts();
        test_Judge_BeatmatchScoring();
        test_Judge_TransitionScoring();
        test_Judge_PerformanceEvaluation();
        test_BattleMode_WinnerDetermination();
        test_BattleMode_MultiRoundProgression();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
    }
}

} // namespace dj

// Entry point for running battle system tests
int main() {
    dj::runAllBattleSystemTests();
    std::cout << "\n=== BattleSystem Phase 24 Tests Complete ===" << std::endl;
    return 0;
}
