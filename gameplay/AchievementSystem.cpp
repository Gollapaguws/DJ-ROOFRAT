#include "gameplay/AchievementSystem.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>

namespace dj {

AchievementSystem::AchievementSystem() {
    // Initialize achievements
    achievements_.emplace_back("first_gig", "First Gig", "Complete your first session");
    achievements_.emplace_back("tier_2", "Rising Star", "Reach Tier 2");
    achievements_.emplace_back("tier_4", "Headliner", "Reach Tier 4 (Festival Main Stage)");
    achievements_.emplace_back("crowd_master", "Crowd Master", "Maintain crowd energy > 0.8 for 60 seconds");
    achievements_.emplace_back("smooth_operator", "Smooth Operator", "Execute 10 smooth transitions (smoothness > 0.7)");
}

void AchievementSystem::checkAndUnlock(const std::string& achievementId) {
    auto it = std::find_if(achievements_.begin(), achievements_.end(),
        [&achievementId](const Achievement& a) { return a.id == achievementId; });
    
    if (it != achievements_.end()) {
        it->unlocked = true;
    }
}

bool AchievementSystem::isUnlocked(const std::string& achievementId) const {
    auto it = std::find_if(achievements_.begin(), achievements_.end(),
        [&achievementId](const Achievement& a) { return a.id == achievementId; });
    
    if (it != achievements_.end()) {
        return it->unlocked;
    }
    return false;
}

std::vector<Achievement> AchievementSystem::getAllAchievements() const {
    return achievements_;
}

int AchievementSystem::getUnlockedCount() const {
    int count = 0;
    for (const auto& ach : achievements_) {
        if (ach.unlocked) {
            count++;
        }
    }
    return count;
}

bool AchievementSystem::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // Manual JSON serialization
    file << "{\n";
    file << "  \"achievements\": [\n";
    
    for (std::size_t i = 0; i < achievements_.size(); ++i) {
        file << "    {\n";
        file << "      \"id\": \"" << achievements_[i].id << "\",\n";
        file << "      \"name\": \"" << achievements_[i].name << "\",\n";
        file << "      \"description\": \"" << achievements_[i].description << "\",\n";
        file << "      \"unlocked\": " << (achievements_[i].unlocked ? "true" : "false") << "\n";
        file << "    }";
        if (i < achievements_.size() - 1) {
            file << ",";
        }
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    
    file.close();
    return true;
}

bool AchievementSystem::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    std::map<std::string, bool> loadedStates;
    
    while (std::getline(file, line)) {
        // Look for "id": "..." lines
        if (line.find("\"id\"") != std::string::npos) {
            std::size_t start = line.find("\"id\"");
            start = line.find("\"", start + 5);  // Find opening quote after "id":
            std::size_t end = line.find("\"", start + 1);
            if (start != std::string::npos && end != std::string::npos) {
                std::string id = line.substr(start + 1, end - start - 1);
                
                // Read next lines to find "unlocked": ...
                std::string currentBlock = line;
                while (std::getline(file, line)) {
                    currentBlock += line;
                    if (line.find("\"unlocked\"") != std::string::npos) {
                        bool unlocked = (line.find("true") != std::string::npos);
                        loadedStates[id] = unlocked;
                        break;
                    }
                }
            }
        }
    }
    
    file.close();
    
    // Apply loaded states to achievements
    for (auto& ach : achievements_) {
        auto it = loadedStates.find(ach.id);
        if (it != loadedStates.end()) {
            ach.unlocked = it->second;
        }
    }
    
    return true;
}

} // namespace dj
