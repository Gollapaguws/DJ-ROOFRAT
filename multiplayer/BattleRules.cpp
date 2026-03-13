#include "multiplayer/BattleRules.h"

#include <algorithm>

namespace dj {

BattleRules::BattleRules(BattleFormat format, TurnMode mode)
    : format_(format), mode_(mode) {
}

int BattleRules::getRounds() const {
    switch (format_) {
        case BattleFormat::Quick:
            return 1;
        case BattleFormat::Standard:
            return 3;
        case BattleFormat::Tournament:
            return 5;
        default:
            return 1;
    }
}

int BattleRules::getTurnDurationBlocks() const {
    // At 44.1kHz:
    // 30 seconds = 1,323,000 blocks
    // 60 seconds = 2,646,000 blocks
    // 90 seconds = 3,969,000 blocks
    switch (format_) {
        case BattleFormat::Quick:
            return 1323000;  // 30 seconds
        case BattleFormat::Standard:
            return 2646000;  // 60 seconds
        case BattleFormat::Tournament:
            return 3969000;  // 90 seconds
        default:
            return 1323000;
    }
}

TurnMode BattleRules::getTurnMode() const {
    return mode_;
}

std::string BattleRules::getFormatName() const {
    switch (format_) {
        case BattleFormat::Quick:
            return "Quick";
        case BattleFormat::Standard:
            return "Standard";
        case BattleFormat::Tournament:
            return "Tournament";
        default:
            return "Unknown";
    }
}

float BattleRules::getBeatmatchWeight() const {
    return 0.25f;
}

float BattleRules::getTransitionWeight() const {
    return 0.20f;
}

float BattleRules::getEnergyWeight() const {
    return 0.20f;
}

float BattleRules::getCreativityWeight() const {
    return 0.15f;
}

float BattleRules::getCrowdWeight() const {
    return 0.20f;
}

} // namespace dj
