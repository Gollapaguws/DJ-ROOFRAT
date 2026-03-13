#include "gameplay/UnlockSystem.h"
#include <algorithm>

namespace dj {

UnlockSystem::UnlockSystem() {}

int UnlockSystem::getEffectRequiredTier(EffectType effect) const {
    switch (effect) {
    case EffectType::BasicEQ:
        return 0;
    case EffectType::Filters:
        return 1;
    case EffectType::Reverb:
        return 2;
    case EffectType::Delay:
        return 2;
    case EffectType::Flanger:
        return 3;
    case EffectType::BitCrusher:
        return 3;
    case EffectType::VinylSimulator:
        return 4;
    default:
        return 5;
    }
}

bool UnlockSystem::isEffectUnlocked(EffectType effect, int currentTier) const {
    return currentTier >= getEffectRequiredTier(effect);
}

std::vector<EffectType> UnlockSystem::getUnlockedEffects(int currentTier) const {
    std::vector<EffectType> unlocked;
    
    if (currentTier >= 0) {
        unlocked.push_back(EffectType::BasicEQ);
    }
    if (currentTier >= 1) {
        unlocked.push_back(EffectType::Filters);
    }
    if (currentTier >= 2) {
        unlocked.push_back(EffectType::Reverb);
        unlocked.push_back(EffectType::Delay);
    }
    if (currentTier >= 3) {
        unlocked.push_back(EffectType::Flanger);
        unlocked.push_back(EffectType::BitCrusher);
    }
    if (currentTier >= 4) {
        unlocked.push_back(EffectType::VinylSimulator);
    }
    
    return unlocked;
}

std::vector<EffectType> UnlockSystem::getLockedEffects(int currentTier) const {
    std::vector<EffectType> locked;
    
    if (currentTier < 1) {
        locked.push_back(EffectType::Filters);
    }
    if (currentTier < 2) {
        locked.push_back(EffectType::Reverb);
        locked.push_back(EffectType::Delay);
    }
    if (currentTier < 3) {
        locked.push_back(EffectType::Flanger);
        locked.push_back(EffectType::BitCrusher);
    }
    if (currentTier < 4) {
        locked.push_back(EffectType::VinylSimulator);
    }
    
    return locked;
}

std::string UnlockSystem::getEffectName(EffectType effect) const {
    switch (effect) {
    case EffectType::BasicEQ:
        return "Basic EQ";
    case EffectType::Filters:
        return "Filters";
    case EffectType::Reverb:
        return "Reverb";
    case EffectType::Delay:
        return "Delay";
    case EffectType::Flanger:
        return "Flanger";
    case EffectType::BitCrusher:
        return "BitCrusher";
    case EffectType::VinylSimulator:
        return "Vinyl Simulator";
    default:
        return "Unknown Effect";
    }
}

} // namespace dj
