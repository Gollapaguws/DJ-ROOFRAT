#pragma once
#include <string>

namespace dj {

struct Venue {
    std::string name;
    int tier;                      // Required tier to unlock (0-4)
    int capacity;                  // Crowd size (50-50000)
    float difficulty;              // Difficulty multiplier (0.5-2.0)
    
    Venue(const std::string& n, int t, int cap, float diff)
        : name(n), tier(t), capacity(cap), difficulty(diff) {}
};

// Global venue definitions (5 venues)
const Venue& getVenueByTier(int tier);
int getVenueCount();

} // namespace dj
