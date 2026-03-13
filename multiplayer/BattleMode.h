#pragma once

#include <memory>
#include <vector>
#include <string>

namespace dj {

class Judge;
class BattleRules;

enum class BattleState {
    Idle,
    Player1Turn,
    Player2Turn,
    Simultaneous,
    Complete
};

struct PlayerScore {
    float beatmatchScore = 0.0f;
    float transitionScore = 0.0f;
    float energyScore = 0.0f;
    float creativityScore = 0.0f;
    float crowdScore = 0.0f;
    float totalScore = 0.0f;
};

class BattleMode {
public:
    BattleMode();
    
    // Battle lifecycle
    void startBattle(std::shared_ptr<BattleRules> rules);
    void updateBattle(int blockNumber);
    bool isBattleActive() const;
    BattleState getState() const;
    
    // Round management
    int getCurrentRound() const;
    int getTotalRounds() const;
    void advanceRound();
    
    // Player management
    int getCurrentPlayer() const;  // 1 or 2
    void switchPlayer();
    
    // Scoring
    void submitPerformance(int player, float beatmatchDelta, float transitionSmoothness, float crowdEnergy, int blockNumber);
    PlayerScore getPlayerScore(int player) const;
    int getWinner() const;  // Returns 1, 2, or 0 (tie)
    
private:
    std::shared_ptr<BattleRules> rules_;
    std::shared_ptr<Judge> judge_;
    BattleState state_;
    int currentRound_;
    int currentPlayer_;
    PlayerScore player1Score_;
    PlayerScore player2Score_;
    int performanceBlockCount_[2];  // Track blocks per player
};

} // namespace dj
