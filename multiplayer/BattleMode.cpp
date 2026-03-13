#include "multiplayer/BattleMode.h"
#include "multiplayer/BattleRules.h"
#include "multiplayer/Judge.h"

#include <algorithm>

namespace dj {

BattleMode::BattleMode()
    : state_(BattleState::Idle), currentRound_(0), currentPlayer_(1) {
    performanceBlockCount_[0] = 0;
    performanceBlockCount_[1] = 0;
}

void BattleMode::startBattle(std::shared_ptr<BattleRules> rules) {
    rules_ = rules;
    judge_ = std::make_shared<Judge>();
    currentRound_ = 1;
    currentPlayer_ = 1;
    
    // Determine initial state based on turn mode
    if (rules_->getTurnMode() == TurnMode::Simultaneous) {
        state_ = BattleState::Simultaneous;
    } else {
        state_ = BattleState::Player1Turn;
    }
    
    // Reset scores
    player1Score_ = PlayerScore();
    player2Score_ = PlayerScore();
    performanceBlockCount_[0] = 0;
    performanceBlockCount_[1] = 0;
}

void BattleMode::updateBattle(int blockNumber) {
    // Placeholder for future block-based timing logic
    // This would track timing for round transitions
    (void)blockNumber;  // Silence unused parameter warning
}

bool BattleMode::isBattleActive() const {
    return state_ != BattleState::Idle && state_ != BattleState::Complete;
}

BattleState BattleMode::getState() const {
    return state_;
}

int BattleMode::getCurrentRound() const {
    return currentRound_;
}

int BattleMode::getTotalRounds() const {
    if (!rules_) return 0;
    return rules_->getRounds();
}

void BattleMode::advanceRound() {
    currentRound_++;
    if (currentRound_ > getTotalRounds()) {
        state_ = BattleState::Complete;
    }
}

int BattleMode::getCurrentPlayer() const {
    return currentPlayer_;
}

void BattleMode::switchPlayer() {
    if (currentPlayer_ == 1) {
        currentPlayer_ = 2;
        state_ = BattleState::Player2Turn;
    } else {
        currentPlayer_ = 1;
        state_ = BattleState::Player1Turn;
    }
}

void BattleMode::submitPerformance(int player, float beatmatchDelta, float transitionSmoothness, float crowdEnergy, int blockNumber) {
    if (player < 1 || player > 2) return;
    if (!judge_) return;
    
    PerformanceMetrics metrics;
    metrics.beatmatchDelta = beatmatchDelta;
    metrics.transitionSmoothness = std::clamp(transitionSmoothness, 0.0f, 1.0f);
    metrics.crowdEnergy = std::clamp(crowdEnergy, 0.0f, 1.0f);
    metrics.blockNumber = blockNumber;
    
    JudgmentScores scores = judge_->evaluatePerformance(metrics);
    
    PlayerScore* targetScore = (player == 1) ? &player1Score_ : &player2Score_;
    
    targetScore->beatmatchScore += scores.beatmatchScore;
    targetScore->transitionScore += scores.transitionScore;
    targetScore->energyScore += scores.energyScore;
    targetScore->creativityScore += scores.creativityScore;
    targetScore->crowdScore += scores.crowdScore;
    
    // Recalculate total score using judging weights
    if (rules_) {
        targetScore->totalScore = 
            targetScore->beatmatchScore * rules_->getBeatmatchWeight() +
            targetScore->transitionScore * rules_->getTransitionWeight() +
            targetScore->energyScore * rules_->getEnergyWeight() +
            targetScore->creativityScore * rules_->getCreativityWeight() +
            targetScore->crowdScore * rules_->getCrowdWeight();
    }
    
    performanceBlockCount_[player - 1]++;
}

PlayerScore BattleMode::getPlayerScore(int player) const {
    if (player < 1 || player > 2) return PlayerScore{};
    return (player == 1) ? player1Score_ : player2Score_;
}

int BattleMode::getWinner() const {
    if (player1Score_.totalScore > player2Score_.totalScore) {
        return 1;
    } else if (player2Score_.totalScore > player1Score_.totalScore) {
        return 2;
    } else {
        return 0;  // Tie
    }
}

} // namespace dj
