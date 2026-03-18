#include "input/InputMapper.h"

#include <cassert>
#include <iostream>

namespace dj {

// Test 1: Verify tempo A keyboard commands parse correctly
void test_InputMapper_TempoAKeys() {
    // Tempo A nudge up (z)
    InputCommand cmdTempoAUp = InputMapper::parseKey('z');
    assert(cmdTempoAUp == InputCommand::NudgeTempoAUp);
    
    // Tempo A nudge down (x)
    InputCommand cmdTempoADown = InputMapper::parseKey('x');
    assert(cmdTempoADown == InputCommand::NudgeTempoADown);
    
    // Tempo A reset (c)
    InputCommand cmdTempoAReset = InputMapper::parseKey('c');
    assert(cmdTempoAReset == InputCommand::ResetTempoA);
    
    std::cout << "✓ test_InputMapper_TempoAKeys passed" << std::endl;
}

// Test 2: Verify beat-jump keys parse correctly
void test_InputMapper_BeatJumpKeys() {
    // Deck B beat jump backward (;)
    InputCommand cmdBeatJumpBackB = InputMapper::parseKey(';');
    assert(cmdBeatJumpBackB == InputCommand::BeatJumpBackwardB);
    
    // Deck B beat jump forward (')
    InputCommand cmdBeatJumpForwardB = InputMapper::parseKey('\'');
    assert(cmdBeatJumpForwardB == InputCommand::BeatJumpForwardB);
    
    std::cout << "✓ test_InputMapper_BeatJumpKeys passed" << std::endl;
}

// Test 3: Verify existing tempo B keys still work
void test_InputMapper_TempoBKeysStillWork() {
    InputCommand cmdTempoBUp = InputMapper::parseKey('i');
    assert(cmdTempoBUp == InputCommand::NudgeTempoBUp);
    
    InputCommand cmdTempoBDown = InputMapper::parseKey('k');
    assert(cmdTempoBDown == InputCommand::NudgeTempoBDown);
    
    InputCommand cmdTempoBReset = InputMapper::parseKey('o');
    assert(cmdTempoBReset == InputCommand::ResetTempoB);
    
    std::cout << "✓ test_InputMapper_TempoBKeysStillWork passed" << std::endl;
}

// Test 4: Verify recording utility commands parse correctly
void test_InputMapper_RecordingUtilityKeys() {
    InputCommand cmdSaveRecording = InputMapper::parseKey('V');
    assert(cmdSaveRecording == InputCommand::SaveRecording);

    InputCommand cmdSetRecordingFilename = InputMapper::parseKey('N');
    assert(cmdSetRecordingFilename == InputCommand::SetRecordingFilename);

    InputCommand cmdSaveRecordingText = InputMapper::parse("save recording");
    assert(cmdSaveRecordingText == InputCommand::SaveRecording);

    InputCommand cmdSetRecordingFilenameText = InputMapper::parse("set recording filename");
    assert(cmdSetRecordingFilenameText == InputCommand::SetRecordingFilename);

    std::cout << "✓ test_InputMapper_RecordingUtilityKeys passed" << std::endl;
}

void runAllPhase3InputMapperTests() {
    std::cout << "\n=== Running InputMapper Phase 3 Tests ===" << std::endl;
    try {
        test_InputMapper_TempoAKeys();
        test_InputMapper_BeatJumpKeys();
        test_InputMapper_TempoBKeysStillWork();
        test_InputMapper_RecordingUtilityKeys();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
    }
}

} // namespace dj

// Entry point for running tests
int main() {
    dj::runAllPhase3InputMapperTests();
    std::cout << "\n=== InputMapper Phase 3 Tests Complete ===" << std::endl;
    return 0;
}
