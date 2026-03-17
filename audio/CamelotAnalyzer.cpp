#include "audio/CamelotAnalyzer.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace dj {

// Key-to-Camelot mapping table
const std::map<std::string, std::string> CamelotAnalyzer::keyToCamelotMap = {
    // Minor keys (A)
    {"Ab minor", "1A"},
    {"G# minor", "1A"},
    {"Eb minor", "2A"},
    {"D# minor", "2A"},
    {"Bb minor", "3A"},
    {"A# minor", "3A"},
    {"F minor", "4A"},
    {"C minor", "5A"},
    {"G minor", "6A"},
    {"D minor", "7A"},
    {"A minor", "8A"},
    {"E minor", "9A"},
    {"B minor", "10A"},
    {"F# minor", "11A"},
    {"Gb minor", "11A"},
    {"Db minor", "12A"},
    {"C# minor", "12A"},

    // Major keys (B)
    {"B major", "1B"},
    {"F# major", "2B"},
    {"Gb major", "2B"},
    {"Db major", "3B"},
    {"C# major", "3B"},
    {"Ab major", "4B"},
    {"G# major", "4B"},
    {"Eb major", "5B"},
    {"D# major", "5B"},
    {"Bb major", "6B"},
    {"A# major", "6B"},
    {"F major", "7B"},
    {"C major", "8B"},
    {"G major", "9B"},
    {"D major", "10B"},
    {"A major", "11B"},
    {"E major", "12B"}};

std::string CamelotAnalyzer::keyToCamelot(const std::string& musicalKey) const {
    auto it = keyToCamelotMap.find(musicalKey);
    if (it != keyToCamelotMap.end()) {
        return it->second;
    }
    return "";  // Key not found
}

int CamelotAnalyzer::getWheelPosition(const std::string& camelotCode) {
    if (camelotCode.length() < 2) {
        return -1;
    }
    try {
        // Extract number part (e.g., "5" from "5A")
        return std::stoi(camelotCode.substr(0, camelotCode.length() - 1));
    } catch (...) {
        return -1;
    }
}

char CamelotAnalyzer::getRingType(const std::string& camelotCode) {
    if (camelotCode.length() >= 1) {
        char lastChar = camelotCode.back();
        if (lastChar == 'A' || lastChar == 'B') {
            return lastChar;
        }
    }
    return '\0';
}

int CamelotAnalyzer::circularDistance(int from, int to) {
    // Calculate minimum distance between two positions on a 12-position wheel
    if (from < 1 || from > 12 || to < 1 || to > 12) {
        return -1;
    }

    int clockwise = (to - from + 12) % 12;
    if (clockwise == 0 && from != to) {
        clockwise = 12;
    }
    int counterClockwise = (from - to + 12) % 12;
    if (counterClockwise == 0 && from != to) {
        counterClockwise = 12;
    }

    return std::min(clockwise, counterClockwise);
}

float CamelotAnalyzer::getCompatibilityScore(const std::string& fromCamelot,
                                              const std::string& toCamelot) const {
    // Validate codes
    int fromPos = getWheelPosition(fromCamelot);
    int toPos = getWheelPosition(toCamelot);
    char fromRing = getRingType(fromCamelot);
    char toRing = getRingType(toCamelot);

    if (fromPos < 1 || fromPos > 12 || toPos < 1 || toPos > 12 || fromRing == '\0' ||
        toRing == '\0') {
        return 0.0f;
    }

    // Perfect match (same key)
    if (fromCamelot == toCamelot) {
        return 1.0f;
    }

    // Relative minor/major (same position, different ring)
    if (fromPos == toPos && fromRing != toRing) {
        return 0.9f;
    }

    // If we're in different rings, not adjacent
    if (fromRing != toRing) {
        return 0.3f;  // Distant keys
    }

    // Calculate distance within the same ring
    int distance = circularDistance(fromPos, toPos);

    // Adjacent keys (±1 step)
    if (distance == 1) {
        return 0.8f;
    }

    // Distant keys (>2 steps)
    return 0.3f;
}

int CamelotAnalyzer::getEnergyDirection(const std::string& fromCamelot,
                                         const std::string& toCamelot) const {
    // Get positions and rings
    int fromPos = getWheelPosition(fromCamelot);
    int toPos = getWheelPosition(toCamelot);
    char fromRing = getRingType(fromCamelot);
    char toRing = getRingType(toCamelot);

    if (fromPos < 1 || fromPos > 12 || toPos < 1 || toPos > 12 || fromRing == '\0' ||
        toRing == '\0') {
        return 0;
    }

    // Same key = neutral
    if (fromCamelot == toCamelot) {
        return 0;
    }

    // A/B switch (relative keys) = neutral
    if (fromPos == toPos && fromRing != toRing) {
        return 0;
    }

    // Different rings but different positions = complex transition, treat as neutral
    if (fromRing != toRing) {
        return 0;
    }

    // Within same ring: calculate direction
    // Clockwise (1-2-3...-12-1): boost (+1)
    // Counter-clockwise (reverse): drop (-1)

    // For same ring, check if we're going clockwise or counter-clockwise
    int clockwiseDistance = (toPos - fromPos + 12) % 12;

    if (clockwiseDistance == 0) {
        return 0;  // Same key (shouldn't reach here)
    }

    if (clockwiseDistance <= 6) {
        return 1;  // Clockwise = boost
    } else {
        return -1;  // Counter-clockwise = drop
    }
}

bool CamelotAnalyzer::isCompatible(const std::string& fromCamelot, const std::string& toCamelot,
                                    float threshold) const {
    return getCompatibilityScore(fromCamelot, toCamelot) >= threshold;
}

std::vector<std::string> CamelotAnalyzer::getCompatibleKeys(const std::string& currentCamelot) const {
    std::vector<std::string> compatible;

    int currentPos = getWheelPosition(currentCamelot);
    char currentRing = getRingType(currentCamelot);

    if (currentPos < 1 || currentPos > 12 || currentRing == '\0') {
        return compatible;
    }

    // Add self (perfect match)
    compatible.push_back(currentCamelot);

    // Add adjacent keys (±1) with same ring
    for (int offset = -1; offset <= 1; offset += 2) {
        int adjPos = ((currentPos - 1 + offset + 12) % 12) + 1;
        compatible.push_back(std::to_string(adjPos) + currentRing);
    }

    // Add relative minor/major (same position, opposite ring)
    char oppositeRing = (currentRing == 'A') ? 'B' : 'A';
    compatible.push_back(std::to_string(currentPos) + oppositeRing);

    // Sort by compatibility score (highest first)
    std::sort(compatible.begin(), compatible.end(),
              [this, &currentCamelot](const std::string& a, const std::string& b) {
                  float scoreA = getCompatibilityScore(currentCamelot, a);
                  float scoreB = getCompatibilityScore(currentCamelot, b);
                  return scoreA > scoreB;
              });

    return compatible;
}

}  // namespace dj
