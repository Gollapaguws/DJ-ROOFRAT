#include "visuals/CamelotWheel.h"
#include "audio/CamelotAnalyzer.h"

#include <algorithm>
#include <map>
#include <sstream>

namespace dj {

// Mapping from Camelot code to note name
static const std::map<std::string, std::string> camelotNoteMap = {
    {"1A", "Ab"}, {"1B", "B"},   {"2A", "Eb"}, {"2B", "F#"}, {"3A", "Bb"}, {"3B", "Db"},
    {"4A", "F"},  {"4B", "Ab"},  {"5A", "C"},  {"5B", "Eb"}, {"6A", "G"},  {"6B", "Bb"},
    {"7A", "D"},  {"7B", "F"},   {"8A", "A"},  {"8B", "C"},  {"9A", "E"},  {"9B", "G"},
    {"10A", "B"}, {"10B", "D"},  {"11A", "F#"}, {"11B", "A"}, {"12A", "Db"}, {"12B", "E"}};

std::string CamelotWheel::render(const std::string& currentCamelotKey) const {
    std::ostringstream oss;

    // Create analyzer for compatibility checking
    CamelotAnalyzer analyzer;
    auto compatibleKeys = analyzer.getCompatibleKeys(currentCamelotKey);

    // Build the ASCII wheel with 12 positions
    oss << "\n";
    oss << "           12B [E]\n";
    oss << "     11B          1B\n";
    oss << "   [A]              [B]\n";
    oss << "                        \n";
    oss << " 10B                  2B\n";
    oss << "[D]       **" << currentCamelotKey << "**     [F#]\n";
    oss << "         [Cm]\n";
    oss << " 9B                   3B\n";
    oss << "[G]                 [Db]\n";
    oss << "   \n";
    oss << "     8B          4B\n";
    oss << "      [C]    [Ab]\n";
    oss << "           7B [F]\n";

    // Add legend
    oss << "\n";
    oss << "Legend:\n";
    oss << "  Inner ring: Minor (A) | Outer ring: Major (B)\n";
    oss << "  ** Current key | + Compatible keys\n";

    // Add compatible keys list
    oss << "\nCompatible with " << currentCamelotKey << ":\n";
    for (const auto& key : compatibleKeys) {
        if (key != currentCamelotKey) {
            auto it = camelotNoteMap.find(key);
            std::string noteName = (it != camelotNoteMap.end()) ? it->second : "?";
            oss << "  + " << key << " [" << noteName << "]\n";
        }
    }

    return oss.str();
}

std::vector<std::string> CamelotWheel::getCompatibleKeys(const std::string& camelotKey) const {
    CamelotAnalyzer analyzer;
    return analyzer.getCompatibleKeys(camelotKey);
}

std::string CamelotWheel::getNoteName(const std::string& camelotCode) const {
    auto it = camelotNoteMap.find(camelotCode);
    if (it != camelotNoteMap.end()) {
        return it->second;
    }
    return "?";
}

std::vector<std::string> CamelotWheel::getWheelPositions(char ringType) {
    std::vector<std::string> positions;
    for (int i = 1; i <= 12; ++i) {
        positions.push_back(std::to_string(i) + ringType);
    }
    return positions;
}

bool CamelotWheel::shouldHighlight(const std::string& wheelKey, const std::string& currentKey) {
    return wheelKey == currentKey;
}

bool CamelotWheel::isCompatibleWith(const std::string& wheelKey, const std::string& currentKey) {
    CamelotAnalyzer analyzer;
    return analyzer.isCompatible(currentKey, wheelKey, 0.7f);
}

}  // namespace dj
