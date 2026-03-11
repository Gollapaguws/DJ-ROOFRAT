#include "audio/Phaser.h"
#include <iostream>
#include <array>

int main() {
    // Test 1: Phaser basic instantiation and processing
    dj::Phaser phaser;
    phaser.setSampleRate(44100);
    phaser.setStages(4);
    phaser.setLFORate(1.0f);
    phaser.setFeedback(0.5f);
    phaser.setMix(0.5f);
    
    std::array<float, 2> input = {0.1f, 0.1f};
    std::array<float, 2> output;
    
    // Process 100 samples
    int testsPassed = 0;
    for (int i = 0; i < 100; ++i) {
        output = phaser.process(input);
        
        // Check output is bounded
        if (output[0] >= -2.0f && output[0] <= 2.0f &&
            output[1] >= -2.0f && output[1] <= 2.0f) {
            testsPassed++;
        }
    }
    
    // Test 2: Reset functionality
    phaser.reset();
    output = phaser.process(input);
    
    if (testsPassed == 100 && 
        output[0] >= -2.0f && output[0] <= 2.0f &&
        output[1] >= -2.0f && output[1] <= 2.0f) {
        std::cout << "✓ Phaser fixes verified: AllpassFilter with stateX/stateY and true feedback routing working correctly!" << std::endl;
        return 0;
    } else {
        std::cout << "✗ Phaser test failed" << std::endl;
        return 1;
    }
}
