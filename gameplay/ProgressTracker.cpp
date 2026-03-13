#include "gameplay/ProgressTracker.h"

#include <fstream>
#include <sstream>
#include <algorithm>

namespace dj {

void ProgressTracker::markLessonComplete(int lessonIndex) {
    // Check if already marked complete
    auto it = std::find(completedLessons_.begin(), completedLessons_.end(), lessonIndex);
    if (it == completedLessons_.end()) {
        completedLessons_.push_back(lessonIndex);
        // Keep sorted for consistency
        std::sort(completedLessons_.begin(), completedLessons_.end());
    }
}

bool ProgressTracker::isLessonComplete(int lessonIndex) const {
    return std::find(completedLessons_.begin(), completedLessons_.end(), lessonIndex)
        != completedLessons_.end();
}

const std::vector<int>& ProgressTracker::getCompletedLessons() const {
    return completedLessons_;
}

bool ProgressTracker::saveProgress(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // Manual JSON serialization (following SessionMetadata pattern)
    file << "{\n";
    file << "  \"version\": 1,\n";
    file << "  \"completed_lessons\": [\n";
    
    for (std::size_t i = 0; i < completedLessons_.size(); ++i) {
        file << "    " << completedLessons_[i];
        if (i < completedLessons_.size() - 1) {
            file << ",";
        }
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    
    file.close();
    return true;
}

bool ProgressTracker::loadProgress(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    completedLessons_.clear();
    
    std::string line;
    bool inCompletedArray = false;
    
    while (std::getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        auto pos = line.find_last_not_of(" \t\n\r");
        if (pos != std::string::npos) {
            line.erase(pos + 1);
        } else {
            line.clear();
        }
        
        if (line.find("\"completed_lessons\"") != std::string::npos) {
            inCompletedArray = true;
            continue;
        }
        
        if (line == "]" || line == "]," || line == "},") {
            inCompletedArray = false;
            continue;
        }
        
        if (inCompletedArray && !line.empty() && line != "[") {
            // Parse the lesson index
            // Line format: "0," or "0"
            std::string numStr = line;
            // Remove trailing comma if present
            if (!numStr.empty() && numStr.back() == ',') {
                numStr.pop_back();
            }
            
            try {
                int lessonIndex = std::stoi(numStr);
                completedLessons_.push_back(lessonIndex);
            } catch (...) {
                // Ignore parse errors
            }
        }
    }
    
    file.close();
    return true;
}

void ProgressTracker::reset() {
    completedLessons_.clear();
}

} // namespace dj
