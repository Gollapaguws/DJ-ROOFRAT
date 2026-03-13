#include "gameplay/TutorialSystem.h"
#include "gameplay/Lesson.h"
#include "gameplay/lessons/BeatmatchingLesson.h"
#include "gameplay/lessons/EQMixingLesson.h"

namespace dj {

TutorialSystem::TutorialSystem()
    : currentLessonIndex_(0)
{
    // Initialize tutorial lessons in sequence
    lessons_.push_back(std::make_shared<BeatmatchingLesson>());
    lessons_.push_back(std::make_shared<EQMixingLesson>());
}

int TutorialSystem::getCurrentLessonIndex() const {
    return currentLessonIndex_;
}

std::shared_ptr<Lesson> TutorialSystem::getCurrentLesson() const {
    if (isComplete()) {
        return nullptr;
    }
    return lessons_[currentLessonIndex_];
}

void TutorialSystem::nextLesson() {
    if (!isComplete()) {
        ++currentLessonIndex_;
    }
}

bool TutorialSystem::isComplete() const {
    return currentLessonIndex_ >= static_cast<int>(lessons_.size());
}

int TutorialSystem::getTotalLessons() const {
    return static_cast<int>(lessons_.size());
}

} // namespace dj
