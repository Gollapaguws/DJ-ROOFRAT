#pragma once

#include <string>
#include <memory>
#include <vector>

namespace dj {

// Forward declarations
class Deck;
class Mixer;

class Lesson {
public:
    virtual ~Lesson() = default;
    
    // Setup the lesson with initial state
    virtual void setup(float baselineBpmA, float baselineBpmB) {}
    
    // Setup for EQ mixing lesson with deck references
    virtual void setup(Deck& deckA, Deck& deckB, Mixer& mixer) {}
    
    // Validate the current state (block-based progress)
    // Returns true when lesson is complete
    virtual bool validate(float bpmA, float bpmB, int blockNumber) {
        return false;
    }
    
    // Validate for EQ mixing lesson
    virtual bool validate(Deck& deckA, Deck& deckB, Mixer& mixer, int blockNumber) {
        return false;
    }
    
    // Teardown and cleanup
    virtual void teardown() {}
    
    // Get hint text for current state
    virtual std::string getHint() const = 0;
    
    // Get lesson name
    virtual std::string getName() const = 0;
    
    // Get lesson objective
    virtual std::string getObjective() const = 0;
};

} // namespace dj
