#include "gameplay/Venue.h"
#include "gameplay/UnlockSystem.h"
#include "gameplay/AchievementSystem.h"
#include "gameplay/GameModes.h"

#include <cassert>
#include <iostream>
#include <filesystem>
#include <cmath>

namespace dj {

// Test 1: Venue Progression Hierarchy
void test_Venue_ProgressionHierarchy() {
    std::cout << "Running test_Venue_ProgressionHierarchy..." << std::endl;
    
    int venueCount = getVenueCount();
    assert(venueCount == 5);
    
    // Get all venues and verify tier/capacity/difficulty increase monotonically
    const Venue& tier0 = getVenueByTier(0);
    assert(tier0.tier == 0);
    assert(tier0.name == "Basement Bar");
    assert(tier0.capacity == 50);
    assert(tier0.difficulty == 0.5f);
    
    const Venue& tier1 = getVenueByTier(1);
    assert(tier1.tier == 1);
    assert(tier1.name == "City Club");
    assert(tier1.capacity == 200);
    assert(tier1.difficulty == 0.8f);
    
    const Venue& tier2 = getVenueByTier(2);
    assert(tier2.tier == 2);
    assert(tier2.name == "Rooftop Session");
    assert(tier2.capacity == 800);
    assert(tier2.difficulty == 1.0f);
    
    const Venue& tier3 = getVenueByTier(3);
    assert(tier3.tier == 3);
    assert(tier3.name == "Arena Showcase");
    assert(tier3.capacity == 5000);
    assert(tier3.difficulty == 1.5f);
    
    const Venue& tier4 = getVenueByTier(4);
    assert(tier4.tier == 4);
    assert(tier4.name == "Festival Main Stage");
    assert(tier4.capacity == 50000);
    assert(tier4.difficulty == 2.0f);
    
    // Verify monotonic increase
    assert(tier0.capacity < tier1.capacity);
    assert(tier1.capacity < tier2.capacity);
    assert(tier2.capacity < tier3.capacity);
    assert(tier3.capacity < tier4.capacity);
    
    assert(tier0.difficulty < tier1.difficulty);
    assert(tier1.difficulty < tier2.difficulty);
    assert(tier2.difficulty < tier3.difficulty);
    assert(tier3.difficulty < tier4.difficulty);
    
    std::cout << "✓ test_Venue_ProgressionHierarchy passed" << std::endl;
}

// Test 2: UnlockSystem Effect Gating
void test_UnlockSystem_EffectGating() {
    std::cout << "Running test_UnlockSystem_EffectGating..." << std::endl;
    
    UnlockSystem unlocks;
    
    // Tier 0: BasicEQ only
    {
        auto unlocked = unlocks.getUnlockedEffects(0);
        auto locked = unlocks.getLockedEffects(0);
        assert(unlocked.size() == 1);
        assert(unlocked[0] == EffectType::BasicEQ);
        assert(locked.size() == 6);
    }
    
    // Tier 1: BasicEQ + Filters
    {
        auto unlocked = unlocks.getUnlockedEffects(1);
        assert(unlocked.size() == 2);
        assert(unlocks.isEffectUnlocked(EffectType::BasicEQ, 1));
        assert(unlocks.isEffectUnlocked(EffectType::Filters, 1));
        assert(!unlocks.isEffectUnlocked(EffectType::Reverb, 1));
    }
    
    // Tier 2: BasicEQ + Filters + Reverb + Delay
    {
        auto unlocked = unlocks.getUnlockedEffects(2);
        assert(unlocked.size() == 4);
        assert(unlocks.isEffectUnlocked(EffectType::BasicEQ, 2));
        assert(unlocks.isEffectUnlocked(EffectType::Filters, 2));
        assert(unlocks.isEffectUnlocked(EffectType::Reverb, 2));
        assert(unlocks.isEffectUnlocked(EffectType::Delay, 2));
        assert(!unlocks.isEffectUnlocked(EffectType::Flanger, 2));
        assert(!unlocks.isEffectUnlocked(EffectType::BitCrusher, 2));
    }
    
    // Tier 3: Add Flanger + BitCrusher
    {
        auto unlocked = unlocks.getUnlockedEffects(3);
        assert(unlocked.size() == 6);
        assert(unlocks.isEffectUnlocked(EffectType::Flanger, 3));
        assert(unlocks.isEffectUnlocked(EffectType::BitCrusher, 3));
        assert(!unlocks.isEffectUnlocked(EffectType::VinylSimulator, 3));
    }
    
    // Tier 4: All effects
    {
        auto unlocked = unlocks.getUnlockedEffects(4);
        assert(unlocked.size() == 7);
        assert(unlocks.isEffectUnlocked(EffectType::VinylSimulator, 4));
        auto locked = unlocks.getLockedEffects(4);
        assert(locked.size() == 0);
    }
    
    // Test effect names
    assert(unlocks.getEffectName(EffectType::BasicEQ) == "Basic EQ");
    assert(unlocks.getEffectName(EffectType::Filters) == "Filters");
    assert(unlocks.getEffectName(EffectType::Reverb) == "Reverb");
    assert(unlocks.getEffectName(EffectType::Delay) == "Delay");
    assert(unlocks.getEffectName(EffectType::Flanger) == "Flanger");
    assert(unlocks.getEffectName(EffectType::BitCrusher) == "BitCrusher");
    assert(unlocks.getEffectName(EffectType::VinylSimulator) == "Vinyl Simulator");
    
    // Test tier requirements
    assert(unlocks.getEffectRequiredTier(EffectType::BasicEQ) == 0);
    assert(unlocks.getEffectRequiredTier(EffectType::Filters) == 1);
    assert(unlocks.getEffectRequiredTier(EffectType::Reverb) == 2);
    assert(unlocks.getEffectRequiredTier(EffectType::Delay) == 2);
    assert(unlocks.getEffectRequiredTier(EffectType::Flanger) == 3);
    assert(unlocks.getEffectRequiredTier(EffectType::BitCrusher) == 3);
    assert(unlocks.getEffectRequiredTier(EffectType::VinylSimulator) == 4);
    
    std::cout << "✓ test_UnlockSystem_EffectGating passed" << std::endl;
}

// Test 3: CareerProgression Reputation Gain
void test_CareerProgression_ReputationGain() {
    std::cout << "Running test_CareerProgression_ReputationGain..." << std::endl;
    
    CareerProgression career;
    career.reset();
    
    // Initial state
    assert(career.reputation() == 10.0f);
    assert(career.peakTier() == 0);
    assert(career.tier() == 0);
    assert(career.currentVenueName() == "Basement Bar");
    
    // Simulate crowd energy > 0.45 to increase reputation
    for (int i = 0; i < 10; ++i) {
        career.update(0.8f);  // Good crowd energy
    }
    
    // Reputation should increase
    float rep = career.reputation();
    assert(rep > 10.0f);
    
    // Keep updating to reach tier 1 (reputation >= 20)
    while (career.tier() < 1) {
        career.update(0.8f);
    }
    
    assert(career.tier() == 1);
    assert(career.currentVenueName() == "City Club");
    assert(career.isVenueUnlocked(0));
    assert(career.isVenueUnlocked(1));
    assert(!career.isVenueUnlocked(2));
    
    // Continue to reach tier 2 (reputation >= 45)
    while (career.tier() < 2) {
        career.update(0.8f);
    }
    
    assert(career.tier() == 2);
    assert(career.currentVenueName() == "Rooftop Session");
    assert(career.isVenueUnlocked(0));
    assert(career.isVenueUnlocked(1));
    assert(career.isVenueUnlocked(2));
    assert(!career.isVenueUnlocked(3));
    
    // Verify peakTier is monotonic
    auto peak = career.peakTier();
    // Even if we now reduce crowd energy
    for (int i = 0; i < 50; ++i) {
        career.update(0.1f);  // Poor crowd energy
    }
    // peakTier should not decrease
    assert(career.peakTier() >= peak);
    
    std::cout << "✓ test_CareerProgression_ReputationGain passed" << std::endl;
}

// Test 4: AchievementSystem Triggering
void test_AchievementSystem_Triggering() {
    std::cout << "Running test_AchievementSystem_Triggering..." << std::endl;
    
    AchievementSystem achievements;
    
    // Initially all unlocked should be false
    assert(!achievements.isUnlocked("first_gig"));
    assert(!achievements.isUnlocked("tier_2"));
    assert(!achievements.isUnlocked("tier_4"));
    assert(!achievements.isUnlocked("crowd_master"));
    assert(!achievements.isUnlocked("smooth_operator"));
    
    // Check and unlock achievements
    achievements.checkAndUnlock("first_gig");
    assert(achievements.isUnlocked("first_gig"));
    assert(achievements.getUnlockedCount() == 1);
    
    achievements.checkAndUnlock("tier_2");
    assert(achievements.isUnlocked("tier_2"));
    assert(achievements.getUnlockedCount() == 2);
    
    achievements.checkAndUnlock("tier_4");
    assert(achievements.isUnlocked("tier_4"));
    assert(achievements.getUnlockedCount() == 3);
    
    achievements.checkAndUnlock("crowd_master");
    assert(achievements.isUnlocked("crowd_master"));
    assert(achievements.getUnlockedCount() == 4);
    
    achievements.checkAndUnlock("smooth_operator");
    assert(achievements.isUnlocked("smooth_operator"));
    assert(achievements.getUnlockedCount() == 5);
    
    // Get all achievements
    auto all = achievements.getAllAchievements();
    assert(all.size() >= 5);
    
    // Verify at least the expected ones are present
    bool found_first_gig = false;
    bool found_tier_2 = false;
    for (const auto& ach : all) {
        if (ach.id == "first_gig" && ach.unlocked) {
            found_first_gig = true;
        }
        if (ach.id == "tier_2" && ach.unlocked) {
            found_tier_2 = true;
        }
    }
    assert(found_first_gig);
    assert(found_tier_2);
    
    std::cout << "✓ test_AchievementSystem_Triggering passed" << std::endl;
}

// Test 5: AchievementSystem Persistence
void test_AchievementSystem_Persistence() {
    std::cout << "Running test_AchievementSystem_Persistence..." << std::endl;
    
    std::string testFile = "test_achievements_phase23.json";
    
    // Create and save
    {
        AchievementSystem achievements;
        achievements.checkAndUnlock("first_gig");
        achievements.checkAndUnlock("tier_2");
        achievements.checkAndUnlock("tier_4");
        
        assert(achievements.saveToFile(testFile));
    }
    
    // Load and verify
    {
        AchievementSystem achievements;
        assert(achievements.loadFromFile(testFile));
        
        // These should be unlocked from the file
        assert(achievements.isUnlocked("first_gig"));
        assert(achievements.isUnlocked("tier_2"));
        assert(achievements.isUnlocked("tier_4"));
        assert(achievements.getUnlockedCount() >= 3);
    }
    
    // Cleanup
    if (std::filesystem::exists(testFile)) {
        std::filesystem::remove(testFile);
    }
    
    std::cout << "✓ test_AchievementSystem_Persistence passed" << std::endl;
}

} // namespace dj

int main() {
    std::cout << "=== Phase 23: Career Mode Expansion Tests ===" << std::endl;
    
    try {
        dj::test_Venue_ProgressionHierarchy();
        dj::test_UnlockSystem_EffectGating();
        dj::test_CareerProgression_ReputationGain();
        dj::test_AchievementSystem_Triggering();
        dj::test_AchievementSystem_Persistence();
        
        std::cout << "\n=== All tests passed! ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}
