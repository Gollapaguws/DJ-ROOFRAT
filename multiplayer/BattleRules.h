#pragma once

#include <string>

namespace dj {

enum class BattleFormat {
    Quick,       // 1 round, 30 seconds each
    Standard,    // 3 rounds, 60 seconds each
    Tournament   // 5 rounds, 90 seconds each
};

enum class TurnMode {
    TurnBased,    // Alternating turns
    Simultaneous  // Both players at once
};

class BattleRules {
public:
    BattleRules(BattleFormat format, TurnMode mode);
    
    // Rule queries
    int getRounds() const;
    int getTurnDurationBlocks() const;  // Duration per turn in audio blocks
    TurnMode getTurnMode() const;
    std::string getFormatName() const;
    
    // Judging weights
    float getBeatmatchWeight() const;
    float getTransitionWeight() const;
    float getEnergyWeight() const;
    float getCreativityWeight() const;
    float getCrowdWeight() const;
    
private:
    BattleFormat format_;
    TurnMode mode_;
};

} // namespace dj
