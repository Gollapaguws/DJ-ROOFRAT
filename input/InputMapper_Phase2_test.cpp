#include "input/InputMapper.h"

#include <cassert>
#include <iostream>

namespace dj {

// Test: Verify isolator key mappings work correctly
void test_InputMapper_IsolatorKeys() {
    // Test Deck A isolator keys (Shift+Q/W/E -> uppercase Q/W/E)
    InputCommand cmdLowA = InputMapper::parseKey('Q');  // Uppercase = Shift
    assert(cmdLowA == InputCommand::IsolatorLowA);
    
    InputCommand cmdMidA = InputMapper::parseKey('W');
    assert(cmdMidA == InputCommand::IsolatorMidA);
    
    InputCommand cmdHighA = InputMapper::parseKey('E');
    assert(cmdHighA == InputCommand::IsolatorHighA);
    
    // Test Deck B isolator keys (Shift+D/F/G -> uppercase D/F/G)
    InputCommand cmdLowB = InputMapper::parseKey('D');
    assert(cmdLowB == InputCommand::IsolatorLowB);
    
    InputCommand cmdMidB = InputMapper::parseKey('F');
    assert(cmdMidB == InputCommand::IsolatorMidB);
    
    InputCommand cmdHighB = InputMapper::parseKey('G');
    assert(cmdHighB == InputCommand::IsolatorHighB);
    
    // Test filter order toggle
    InputCommand cmdFilterToggle = InputMapper::parseKey('U');
    assert(cmdFilterToggle == InputCommand::FilterOrderToggle);
    
    // Verify lowercase keys still work for normal EQ
    InputCommand cmdLowANormal = InputMapper::parseKey('q');  // lowercase
    assert(cmdLowANormal == InputCommand::DeckALowDown);
    
    std::cout << "✓ test_InputMapper_IsolatorKeys passed" << std::endl;
}

// Test: Verify non-isolated keys still work normally
void test_InputMapper_NormalKeys() {
    InputCommand cmdPlay = InputMapper::parseKey('a');
    assert(cmdPlay == InputCommand::PlayPauseA);
    
    InputCommand cmdQuit = InputMapper::parseKey('x');
    assert(cmdQuit == InputCommand::Quit);
    
    std::cout << "✓ test_InputMapper_NormalKeys passed" << std::endl;
}

void runAllPhase2InputMapperTests() {
    std::cout << "\n=== Running InputMapper Phase 2 Tests ===" << std::endl;
    try {
        test_InputMapper_IsolatorKeys();
        test_InputMapper_NormalKeys();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
    }
}

} // namespace dj

// Entry point for running tests
int main() {
    dj::runAllPhase2InputMapperTests();
    std::cout << "\n=== InputMapper Phase 2 Tests Complete ===" << std::endl;
    return 0;
}
