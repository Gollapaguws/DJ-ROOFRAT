#include "gameplay/TutorialSystem.h"
#include "gameplay/Lesson.h"
#include "gameplay/lessons/BeatmatchingLesson.h"
#include "gameplay/lessons/EQMixingLesson.h"
#include "gameplay/ProgressTracker.h"
#include "audio/Deck.h"
#include "audio/Mixer.h"

#include <cassert>
#include <iostream>
#include <filesystem>
#include <cmath>

namespace dj {

// Test 1: Verify lessons unlock in order
void test_TutorialSystem_SequenceProgression() {
    TutorialSystem tutorial;
    
    // Should start at lesson 0 (Beatmatching)
    assert(tutorial.getCurrentLessonIndex() == 0);
    assert(!tutorial.isComplete());
    
    // Move to next lesson
    tutorial.nextLesson();
    assert(tutorial.getCurrentLessonIndex() == 1);
    
    // Move to next lesson
    tutorial.nextLesson();
    assert(tutorial.getCurrentLessonIndex() == 2);
    
    // Should be complete after all lessons
    assert(tutorial.isComplete());
    
    std::cout << "✓ test_TutorialSystem_SequenceProgression passed" << std::endl;
}

// Test 2: Detect when deckB tempo is within ±2% of deckA
void test_BeatmatchingLesson_Validation() {
    BeatmatchingLesson lesson;
    
    Deck deckA;
    Deck deckB;
    
    // Setup with generated tones
    deckA.loadClip(AudioClip::generateTestTone(440.0f, 1.0f, 44100));
    deckB.loadClip(AudioClip::generateTestTone(440.0f, 1.0f, 44100));
    
    float baselineBpmA = 120.0f;
    float baselineBpmB = 120.0f;
    
    lesson.setup(baselineBpmA, baselineBpmB);
    
    // Test 1: BPM delta too high (5%)
    float bpmA = 120.0f;
    float bpmB = 126.0f;  // 5% delta = too high
    bool valid = lesson.validate(bpmA, bpmB, 0);
    assert(!valid);  // Should not validate yet
    
    // Test 2: BPM delta within 2% - should need sustained time
    bpmB = 122.0f;  // 1.67% delta = within 2%, but first block
    valid = lesson.validate(bpmA, bpmB, 1);
    assert(!valid);  // First call should not complete
    
    // Test 3: Sustained match for 5 seconds (220 blocks at 44.1kHz)
    // Simulate 5+ seconds of matching
    for (int i = 2; i <= 250; ++i) {
        valid = lesson.validate(bpmA, bpmB, i);
        if (i <= 220) {
            assert(!valid);  // Should not complete before 5 seconds (221 blocks total)
        }
    }
    assert(valid);  // Should now be valid after sustained time
    
    lesson.teardown();
    
    std::cout << "✓ test_BeatmatchingLesson_Validation passed" << std::endl;
}

// Test 3: Verify lesson completion when EQ moves from preset-A to preset-B
void test_EQMixingLesson_Transition() {
    EQMixingLesson lesson;
    
    Deck deckA;
    Deck deckB;
    Mixer mixer;
    
    // Setup with generated tones
    deckA.loadClip(AudioClip::generateTestTone(220.0f, 1.0f, 44100));
    deckB.loadClip(AudioClip::generateTestTone(330.0f, 1.0f, 44100));
    
    lesson.setup(deckA, deckB, mixer);
    
    // Test 1: Not valid at start (EQ should be at preset A for deckA)
    bool valid = lesson.validate(deckA, deckB, mixer, 0);
    assert(!valid);
    
    // Simulate crossfader transition from deckA (-1.0) to deckB (+1.0)
    // and EQ transition during that time
    
    // Mid-transition: EQ adjusting but crossfader still favoring A
    mixer.setCrossfader(-0.5f);
    valid = lesson.validate(deckA, deckB, mixer, 60);
    assert(!valid);  // Still not at preset B
    
    // Near complete: Crossfader mostly B, EQ transitioning
    mixer.setCrossfader(0.8f);
    valid = lesson.validate(deckA, deckB, mixer, 120);
    assert(!valid);  // Not fully transitioned yet
    
    // Fully transitioned: EQ at preset B, crossfader at B
    deckB.setEQ(1.0f, 1.0f, 1.0f);  // Preset B EQ (boosted)
    mixer.setCrossfader(1.0f);
    valid = lesson.validate(deckA, deckB, mixer, 180);
    assert(valid);  // Should be valid when transition is complete
    
    lesson.teardown();
    
    std::cout << "✓ test_EQMixingLesson_Transition passed" << std::endl;
}

// Test 4: JSON save/load of completion state
void test_ProgressTracker_SaveLoad() {
    ProgressTracker tracker;
    
    // Mark some lessons as complete
    tracker.markLessonComplete(0);  // Beatmatching
    tracker.markLessonComplete(1);  // EQ Mixing
    
    // Save to file
    const std::string testFile = "tutorial_progress_test.json";
    bool saved = tracker.saveProgress(testFile);
    assert(saved);
    
    // Load from file
    ProgressTracker loaded;
    bool loadSuccess = loaded.loadProgress(testFile);
    assert(loadSuccess);
    
    // Verify completion state
    assert(loaded.isLessonComplete(0));
    assert(loaded.isLessonComplete(1));
    assert(!loaded.isLessonComplete(2));  // Not marked complete
    
    // Cleanup
    std::filesystem::remove(testFile);
    
    std::cout << "✓ test_ProgressTracker_SaveLoad passed" << std::endl;
}

// Test 5: Verify hints emitted at correct checkpoint
void test_TutorialSystem_HintGeneration() {
    BeatmatchingLesson lesson;
    
    Deck deckA;
    Deck deckB;
    
    deckA.loadClip(AudioClip::generateTestTone(440.0f, 1.0f, 44100));
    deckB.loadClip(AudioClip::generateTestTone(440.0f, 1.0f, 44100));
    
    lesson.setup(120.0f, 125.0f);
    
    // Get hint text
    std::string hint = lesson.getHint();
    assert(!hint.empty());
    assert(hint.find("Match") != std::string::npos || 
           hint.find("Tempo") != std::string::npos ||
           hint.find("Deck") != std::string::npos);
    
    // Simulate some blocks with matching BPM
    float bpmA = 120.0f;
    float bpmB = 120.5f;
    
    // Hint should be retrievable every checkpoint (e.g., every 60 blocks)
    for (int i = 0; i < 300; ++i) {
        lesson.validate(bpmA, bpmB, i);
        
        if (i % 60 == 0 && i > 0) {
            std::string checkpointHint = lesson.getHint();
            assert(!checkpointHint.empty());
        }
    }
    
    lesson.teardown();
    
    std::cout << "✓ test_TutorialSystem_HintGeneration passed" << std::endl;
}

void runAllTutorialTests() {
    std::cout << "\n=== Running GameplaySystem Phase 21 Tests ===" << std::endl;
    try {
        test_TutorialSystem_SequenceProgression();
        test_BeatmatchingLesson_Validation();
        test_EQMixingLesson_Transition();
        test_ProgressTracker_SaveLoad();
        test_TutorialSystem_HintGeneration();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
    }
}

} // namespace dj

int main() {
    dj::runAllTutorialTests();
    std::cout << "\n=== GameplaySystem Phase 21 Tests Complete ===" << std::endl;
    return 0;
}
