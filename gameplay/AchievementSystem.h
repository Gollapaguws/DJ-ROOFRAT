#pragma once
#include <string>
#include <vector>

namespace dj {

struct Achievement {
    std::string id;
    std::string name;
    std::string description;
    bool unlocked;
    
    Achievement(const std::string& i, const std::string& n, const std::string& desc)
        : id(i), name(n), description(desc), unlocked(false) {}
};

class AchievementSystem {
public:
    AchievementSystem();
    
    // Achievement management
    void checkAndUnlock(const std::string& achievementId);
    bool isUnlocked(const std::string& achievementId) const;
    std::vector<Achievement> getAllAchievements() const;
    int getUnlockedCount() const;
    
    // Persistence
    bool saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);
    
private:
    std::vector<Achievement> achievements_;
};

} // namespace dj
