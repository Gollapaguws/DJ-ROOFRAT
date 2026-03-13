#include "gameplay/Leaderboard.h"

#include <fstream>
#include <sstream>
#include <algorithm>

namespace dj {

Leaderboard::Leaderboard() {}

void Leaderboard::addScore(const std::string& playerName, float score, const std::string& missionName) {
    entries_.emplace_back(playerName, score, missionName);
    // Sort by score descending
    std::sort(entries_.begin(), entries_.end(), 
              [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
                  return a.score > b.score;
              });
}

std::vector<LeaderboardEntry> Leaderboard::getTopScores(const std::string& missionName, int count) const {
    std::vector<LeaderboardEntry> filtered;
    
    for (const auto& entry : entries_) {
        if (entry.missionName == missionName) {
            filtered.push_back(entry);
            if (static_cast<int>(filtered.size()) >= count) {
                break;
            }
        }
    }
    
    return filtered;
}

bool Leaderboard::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // Manual JSON serialization (following SessionMetadata pattern)
    file << "{\n";
    file << "  \"leaderboard\": [\n";
    
    for (std::size_t i = 0; i < entries_.size(); ++i) {
        file << "    {\n";
        file << "      \"player\": \"" << entries_[i].playerName << "\",\n";
        file << "      \"score\": " << entries_[i].score << ",\n";
        file << "      \"mission\": \"" << entries_[i].missionName << "\"\n";
        file << "    }";
        if (i < entries_.size() - 1) {
            file << ",";
        }
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    
    file.close();
    return true;
}

bool Leaderboard::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    entries_.clear();
    
    std::string line;
    std::string playerName;
    float score = 0.0f;
    std::string missionName;
    bool inEntry = false;
    
    while (std::getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        auto pos = line.find_last_not_of(" \t\n\r");
        if (pos != std::string::npos) {
            line.erase(pos + 1);
        } else {
            line.clear();
        }
        
        if (line.find("\"player\"") != std::string::npos) {
            // Extract player name: "player": "PlayerName",
            std::size_t colonPos = line.find(":");
            if (colonPos == std::string::npos) continue;
            std::size_t start = line.find("\"", colonPos + 1) + 1;
            std::size_t end = line.find("\"", start);
            if (start != std::string::npos && end != std::string::npos) {
                playerName = line.substr(start, end - start);
            }
        } else if (line.find("\"score\"") != std::string::npos) {
            // Extract score: "score": 12345.0,
            std::size_t colonPos = line.find(":");
            if (colonPos == std::string::npos) continue;
            std::size_t start = colonPos + 1;
            std::size_t end = line.find(",", start);
            if (end == std::string::npos) {
                end = line.length();
            }
            std::string scoreStr = line.substr(start);
            // Trim scoreStr
            scoreStr.erase(0, scoreStr.find_first_not_of(" \t\n\r"));
            scoreStr.erase(scoreStr.find_last_not_of(" \t,\n\r") + 1);
            try {
                score = std::stof(scoreStr);
            } catch (...) {
                score = 0.0f;
            }
        } else if (line.find("\"mission\"") != std::string::npos) {
            // Extract mission name: "mission": "MissionName"
            std::size_t colonPos = line.find(":");
            if (colonPos == std::string::npos) continue;
            std::size_t start = line.find("\"", colonPos + 1) + 1;
            std::size_t end = line.find("\"", start);
            if (start != std::string::npos && end != std::string::npos) {
                missionName = line.substr(start, end - start);
                // We have all three fields now
                entries_.emplace_back(playerName, score, missionName);
            }
        }
    }
    
    file.close();
    return true;
}

}  // namespace dj
