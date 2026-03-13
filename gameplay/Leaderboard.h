#pragma once

#include <string>
#include <vector>

namespace dj {

struct LeaderboardEntry {
    std::string playerName;
    float score;
    std::string missionName;
    
    // Constructor
    LeaderboardEntry(const std::string& name, float s, const std::string& mission)
        : playerName(name), score(s), missionName(mission) {}
};

class Leaderboard {
public:
    Leaderboard();
    
    // Score management
    void addScore(const std::string& playerName, float score, const std::string& missionName);
    std::vector<LeaderboardEntry> getTopScores(const std::string& missionName, int count) const;
    
    // Persistence
    bool saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);
    
private:
    std::vector<LeaderboardEntry> entries_;
};

}  // namespace dj
