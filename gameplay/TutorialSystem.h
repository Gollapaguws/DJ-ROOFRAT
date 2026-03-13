#pragma once

#include <memory>
#include <vector>

namespace dj {

class Lesson;

class TutorialSystem {
public:
    TutorialSystem();
    ~TutorialSystem() = default;
    
    // Get current lesson index
    int getCurrentLessonIndex() const;
    
    // Get current lesson (nullptr if complete)
    std::shared_ptr<Lesson> getCurrentLesson() const;
    
    // Move to next lesson
    void nextLesson();
    
    // Check if all lessons are complete
    bool isComplete() const;
    
    // Get total number of lessons
    int getTotalLessons() const;
    
private:
    std::vector<std::shared_ptr<Lesson>> lessons_;
    int currentLessonIndex_ = 0;
};

} // namespace dj
