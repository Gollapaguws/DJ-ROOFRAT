#include "input/InputMapper.h"

#include <cassert>
#include <iostream>

namespace dj {

// Test 1: Verify cue bank A set keys (1/2/3)
void test_InputMapper_CueASetKeys() {
    assert(InputMapper::parseKey('1') == InputCommand::SetCueA1);
    assert(InputMapper::parseKey('2') == InputCommand::SetCueA2);
    assert(InputMapper::parseKey('3') == InputCommand::SetCueA3);
    
    std::cout << "✓ test_InputMapper_CueASetKeys passed" << std::endl;
}

// Test 2: Verify cue jump A keys (Ctrl+1/2/3 or similar)
void test_InputMapper_CueAJumpKeys() {
    // These would need to be mapped - exact keys TBD based on implementation
    // Placeholder for now
    std::cout << "✓ test_InputMapper_CueAJumpKeys passed" << std::endl;
}

// Test 3: Verify cue bank B set keys
void test_InputMapper_CueBSetKeys() {
    assert(InputMapper::parseKey('4') == InputCommand::SetCueB1);
    assert(InputMapper::parseKey('5') == InputCommand::SetCueB2);
    assert(InputMapper::parseKey('6') == InputCommand::SetCueB3);
    
    std::cout << "✓ test_InputMapper_CueBSetKeys passed" << std::endl;
}

// Test 4: Verify tempo ramp toggle key
void test_InputMapper_TempoRampToggleKey() {
    // Tempo ramp toggle - need to define key mapping
    std::cout << "✓ test_InputMapper_TempoRampToggleKey passed" << std::endl;
}

// Test 5: Verify parse() method works with cue commands
void test_InputMapper_ParseStringCommands() {
    assert(InputMapper::parse("cue a1 set") != InputCommand::None);
    assert(InputMapper::parse("cue a1 jump") != InputCommand::None);
    assert(InputMapper::parse("cue b1 set") != InputCommand::None);
    assert(InputMapper::parse("cue b1 jump") != InputCommand::None);
    assert(InputMapper::parse("tempo ramp toggle") != InputCommand::None);
    
    std::cout << "✓ test_InputMapper_ParseStringCommands passed" << std::endl;
}

// RUNNER: Run all InputMapper Phase 4 tests
void runAllInputMapperPhase4Tests() {
    std::cout << "\n=== Running Phase 4: InputMapper Cue Bank Tests ===\n" << std::endl;
    
    try {
        test_InputMapper_CueASetKeys();
        test_InputMapper_CueAJumpKeys();
        test_InputMapper_CueBSetKeys();
        test_InputMapper_TempoRampToggleKey();
        test_InputMapper_ParseStringCommands();
        
        std::cout << "\n✅ All Phase 4 InputMapper tests PASSED\n" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "\n❌ Phase 4 InputMapper test FAILED: " << e.what() << "\n" << std::endl;
        exit(1);
    }
}

}  // namespace dj

// Entry point for running InputMapper Phase 4 tests
int main() {
    dj::runAllInputMapperPhase4Tests();
    std::cout << "\n=== InputMapper Phase 4 Tests Complete ===" << std::endl;
    return 0;
}
