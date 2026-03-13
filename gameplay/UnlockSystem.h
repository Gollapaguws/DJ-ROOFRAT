#pragma once
#include <string>
#include <vector>

namespace dj {

enum class EffectType {
    BasicEQ,         // Tier 0 (always unlocked)
    Filters,         // Tier 1
    Reverb,          // Tier 2
    Delay,           // Tier 2
    Flanger,         // Tier 3
    BitCrusher,      // Tier 3
    VinylSimulator   // Tier 4
};

class UnlockSystem {
public:
    UnlockSystem();
    
    // Check unlock status
    bool isEffectUnlocked(EffectType effect, int currentTier) const;
    std::vector<EffectType> getUnlockedEffects(int currentTier) const;
    std::vector<EffectType> getLockedEffects(int currentTier) const;
    
    // Get metadata
    std::string getEffectName(EffectType effect) const;
    int getEffectRequiredTier(EffectType effect) const;
    
private:
    // Effect tier requirements (defined in .cpp)
};

} // namespace dj
