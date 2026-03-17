#pragma once

#include <map>
#include <string>
#include <vector>

namespace dj {

class CamelotAnalyzer {
public:
    // Convert musical key string (e.g., "C minor", "F# major") to Camelot code (e.g., "5A", "2B")
    // Returns the Camelot code or empty string if key is not recognized
    std::string keyToCamelot(const std::string& musicalKey) const;

    // Get compatibility score between two Camelot codes (0.0 - 1.0)
    // 1.0 = perfect match (same key)
    // 0.9 = relative (A ↔ B switch)
    // 0.8 = adjacent (±1 step on wheel)
    // 0.3 = distant (>2 steps away)
    float getCompatibilityScore(const std::string& fromCamelot, const std::string& toCamelot) const;

    // Get energy direction for transition between two Camelot codes
    // +1 = boost energy (clockwise on wheel)
    // -1 = drop energy (counter-clockwise)
    // 0 = neutral (same code or A ↔ B switch)
    int getEnergyDirection(const std::string& fromCamelot, const std::string& toCamelot) const;

    // Check if two keys are compatible based on threshold (default 0.7)
    // Returns true if compatibility score >= threshold
    bool isCompatible(const std::string& fromCamelot, const std::string& toCamelot, 
                     float threshold = 0.7f) const;

    // Get all compatible Camelot codes for a given key
    // Returns vector of keys sorted by compatibility (highest first)
    // Includes: self (1.0), adjacent keys (0.8), relative minor/major (0.9)
    std::vector<std::string> getCompatibleKeys(const std::string& currentCamelot) const;

private:
    // Lookup table: musical key string -> Camelot code
    static const std::map<std::string, std::string> keyToCamelotMap;

    // Helper: Extract position number (1-12) from Camelot code
    static int getWheelPosition(const std::string& camelotCode);

    // Helper: Extract ring type (A=minor, B=major) from Camelot code
    static char getRingType(const std::string& camelotCode);

    // Helper: Calculate circular distance between two positions (accounts for wrapping)
    static int circularDistance(int from, int to);
};

}  // namespace dj
