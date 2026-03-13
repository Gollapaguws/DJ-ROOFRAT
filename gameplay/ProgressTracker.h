#pragma once

#include <string>
#include <vector>

namespace dj {

class ProgressTracker {
public:
    ProgressTracker() = default;
    ~ProgressTracker() = default;
    
    // Mark a lesson as complete
    void markLessonComplete(int lessonIndex);
    
    // Check if a lesson is complete
    bool isLessonComplete(int lessonIndex) const;
    
    // Get all completed lessons
    const std::vector<int>& getCompletedLessons() const;
    
    // Save progress to JSON file (manual serialization)
    bool saveProgress(const std::string& filename) const;
    
    // Load progress from JSON file
    bool loadProgress(const std::string& filename);
    
    // Clear all progress
    void reset();
    
private:
    std::vector<int> completedLessons_;
};

} // namespace dj
