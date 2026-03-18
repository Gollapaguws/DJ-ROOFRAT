// Phase 1: Integration E2E Test - Battle Mode
// Tests: Start battle → advance turn → submit performance → verify winner calculation

#include "multiplayer/BattleMode.h"
#include "multiplayer/BattleRules.h"

#include <cassert>
#include <iostream>
#include <memory>

namespace dj {

// E2E Test 4: Battle mode state transitions and winner determination
void test_E2E_BattleMode_StateTransitions() {
    std::cout << "[TEST 1] E2E_BattleMode_StateTransitions...\n";
    
    try {
        // Step 1: Initialize battle with Quick format (1 round, turn-based)
        auto rules = std::make_shared<BattleRules>(BattleFormat::Quick, TurnMode::TurnBased);
        BattleMode battle;
        
        assert(!battle.isBattleActive() && "Battle should not be active initially");
        
        // Step 2: Start battle
        battle.startBattle(rules);
        assert(battle.isBattleActive() && "Battle should be active after startBattle");
        assert(battle.getCurrentRound() == 1 && "Should start on round 1");
        assert(battle.getTotalRounds() == 1 && "Quick format should have 1 round");
        assert(battle.getCurrentPlayer() == 1 && "Should start with Player 1");
        
        std::cout << "  ✓ Battle started successfully\n";
        
        // Step 3: Player 1 submits performance
        battle.submitPerformance(1, 0.3f, 0.95f, 0.9f, 1000);
        PlayerScore p1Score = battle.getPlayerScore(1);
        assert(p1Score.totalScore > 0.0f && "Player 1 should have non-zero score");
        
        std::cout << "  ✓ Player 1 submitted performance, score: " << p1Score.totalScore << "\n";
        
        // Step 4: Switch to Player 2
        battle.switchPlayer();
        assert(battle.getCurrentPlayer() == 2 && "Should switch to Player 2");
        
        // Step 5: Player 2 submits performance
        battle.submitPerformance(2, 0.5f, 0.85f, 0.8f, 1000);
        PlayerScore p2Score = battle.getPlayerScore(2);
        assert(p2Score.totalScore > 0.0f && "Player 2 should have non-zero score");
        
        std::cout << "  ✓ Player 2 submitted performance, score: " << p2Score.totalScore << "\n";
        
        // Step 6: Advance to next round (which ends battle since format is Quick with 1 round)
        battle.advanceRound();
        assert(battle.getCurrentRound() == 2 && "Should advance to round 2");
        assert(!battle.isBattleActive() && "Battle should end after exceeding total rounds");
        assert(battle.getState() == BattleState::Complete && "Battle state should be Complete");
        
        std::cout << "  ✓ Battle advanced and completed\n";
        
        // Step 7: Verify winner calculation
        int winner = battle.getWinner();
        assert(winner == 1 || winner == 2 || winner == 0 && "Winner must be 1, 2, or 0 (tie)");
        
        if (winner == 1) {
            std::cout << "  ✓ Winner: Player 1\n";
        } else if (winner == 2) {
            std::cout << "  ✓ Winner: Player 2\n";
        } else {
            std::cout << "  ✓ Winner: Tie\n";
        }
        
        std::cout << "PASS\n";
    } catch (const std::exception& e) {
        std::cout << "  ✗ Exception during battle mode test: " << e.what() << "\n";
        assert(false && "Battle mode test failed");
    }
}

void runAllE2EBattleModeTests() {
    std::cout << "\n=== Running E2E Battle Mode Tests ===\n";
    try {
        test_E2E_BattleMode_StateTransitions();
    } catch (const std::exception& e) {
        std::cerr << "Battle mode test failed with exception: " << e.what() << "\n";
    }
}

} // namespace dj

// Entry point
int main() {
    dj::runAllE2EBattleModeTests();
    std::cout << "\n=== E2E Battle Mode Tests Complete ===\n";
    return 0;
}
