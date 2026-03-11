#include <cassert>
#include <iostream>
#include <sstream>
#include <cmath>

#include "MIDIMapping.h"
#include "MIDILearnMode.h"
#include "ControllerProfiles.h"
#include "MIDIMessage.h"

// ============================================================================
// Test 1: MIDIMapping LinearCurve
// ============================================================================
void test_MIDIMapping_LinearCurve() {
    std::cout << "Test 1: MIDIMapping_LinearCurve..." << std::endl;
    
    using namespace dj::input;
    
    // Test linear curve mapping: 0-127 -> 0.0-1.0
    MIDIMapping mapping;
    mapping.setMessageType(MIDICurveType::ControlChange);
    mapping.setControllerNumber(7);  // Volume CC
    mapping.setChannel(0);
    mapping.setCurveType(CurveType::Linear);
    mapping.setValueRange(0.0f, 1.0f);
    
    // Test min value (0)
    float result = mapping.applyMapping(0);
    assert(std::abs(result - 0.0f) < 0.01f && "CC value 0 should map to 0.0");
    std::cout << "  CC 0 -> " << result << " (expected 0.0): OK" << std::endl;
    
    // Test mid value (64)
    result = mapping.applyMapping(64);
    assert(std::abs(result - 0.5f) < 0.01f && "CC value 64 should map to ~0.5");
    std::cout << "  CC 64 -> " << result << " (expected 0.5): OK" << std::endl;
    
    // Test max value (127)
    result = mapping.applyMapping(127);
    assert(std::abs(result - 1.0f) < 0.01f && "CC value 127 should map to 1.0");
    std::cout << "  CC 127 -> " << result << " (expected 1.0): OK" << std::endl;
    
    std::cout << "  PASSED" << std::endl;
}

// ============================================================================
// Test 2: MIDIMapping LogarithmicCurve
// ============================================================================
void test_MIDIMapping_LogarithmicCurve() {
    std::cout << "Test 2: MIDIMapping_LogarithmicCurve..." << std::endl;
    
    using namespace dj::input;
    
    MIDIMapping mapping;
    mapping.setMessageType(MIDICurveType::ControlChange);
    mapping.setControllerNumber(7);  // Volume CC
    mapping.setChannel(0);
    mapping.setCurveType(CurveType::Logarithmic);
    mapping.setValueRange(0.0f, 1.0f);
    
    // Test min value (0) - should be ~0.0
    float result = mapping.applyMapping(0);
    assert(result >= -0.01f && "CC value 0 should map to ~0.0 or slightly negative");
    std::cout << "  CC 0 -> " << result << " (expected ~0.0): OK" << std::endl;
    
    // Test mid value (64) - logarithmic should be lower than linear mid (0.5)
    float midResult = mapping.applyMapping(64);
    assert(midResult > 0.0f && midResult < 0.5f && "CC value 64 should map to lower value with log curve");
    std::cout << "  CC 64 -> " << midResult << " (expected < 0.5 for log): OK" << std::endl;
    
    // Test max value (127) - should be ~1.0
    result = mapping.applyMapping(127);
    assert(std::abs(result - 1.0f) < 0.01f && "CC value 127 should map to ~1.0");
    std::cout << "  CC 127 -> " << result << " (expected 1.0): OK" << std::endl;
    
    std::cout << "  PASSED" << std::endl;
}

// ============================================================================
// Test 3: MIDIMapping Serialization (JSON)
// ============================================================================
void test_MIDIMapping_Serialization() {
    std::cout << "Test 3: MIDIMapping_Serialization..." << std::endl;
    
    using namespace dj::input;
    
    // Create a mapping
    MIDIMapping mapping1;
    mapping1.setMessageType(MIDICurveType::ControlChange);
    mapping1.setControllerNumber(7);
    mapping1.setChannel(0);
    mapping1.setCurveType(CurveType::Linear);
    mapping1.setValueRange(0.0f, 1.0f);
    mapping1.setTargetFunction("deckA_volume");
    
    // Serialize to JSON
    std::string json = mapping1.toJSON();
    assert(!json.empty() && "JSON serialization should not be empty");
    std::cout << "  JSON: " << json.substr(0, 50) << "..." << std::endl;
    
    // Deserialize from JSON
    auto mapping2 = MIDIMapping::fromJSON(json);
    assert(mapping2.has_value() && "JSON deserialization should succeed");
    
    // Verify equality
    assert(mapping2->getControllerNumber() == 7 && "CC number should be preserved");
    assert(mapping2->getChannel() == 0 && "Channel should be preserved");
    assert(mapping2->getCurveType() == CurveType::Linear && "Curve type should be preserved");
    assert(mapping2->getTargetFunction() == "deckA_volume" && "Target function should be preserved");
    
    std::cout << "  Serialization/Deserialization: OK" << std::endl;
    std::cout << "  PASSED" << std::endl;
}

// ============================================================================
// Test 4: MIDILearnMode Capture
// ============================================================================
void test_MIDILearnMode_Capture() {
    std::cout << "Test 4: MIDILearnMode_Capture..." << std::endl;
    
    using namespace dj::midi;
    using namespace dj::input;
    
    MIDILearnMode learner;
    
    // Start learning for a specific function
    learner.enterLearnMode("deckA_volume");
    assert(learner.isLearning() && "Should be in learning mode");
    std::cout << "  Entered learn mode for deckA_volume: OK" << std::endl;
    
    // Simulate a MIDI message: CC7 with value 100
    unsigned char bytes[3] = {0xB0, 0x07, 0x64};  // CC, CC7, value 100
    MIDIMessage msg = MIDIMessage::parseRawBytes(bytes, 3);
    
    // Process the message
    auto mapping = learner.processMIDIInput(msg);
    assert(mapping.has_value() && "Should return a mapping after MIDI input");
    assert(!learner.isLearning() && "Should exit learning mode after capturing");
    
    std::cout << "  MIDI input captured and mapping created: OK" << std::endl;
    std::cout << "  Exited learn mode: OK" << std::endl;
    
    // Verify the mapping
    assert(mapping->getControllerNumber() == 7 && "Mapping should capture CC number");
    assert(mapping->getTargetFunction() == "deckA_volume" && "Mapping should have target function");
    
    std::cout << "  PASSED" << std::endl;
}

// ============================================================================
// Test 5: ControllerProfiles PioneerDDJ
// ============================================================================
void test_ControllerProfiles_PioneerDDJ() {
    std::cout << "Test 5: ControllerProfiles_PioneerDDJ..." << std::endl;
    
    using namespace dj::input;
    
    auto profile = ControllerProfiles::getPioneerDDJ400Profile();
    assert(profile != nullptr && "Pioneer DDJ-400 profile should exist");
    
    // Check that expected mappings exist
    auto crossfaderMapping = profile->getMappingForFunction("crossfader");
    assert(crossfaderMapping.has_value() && "Crossfader mapping should exist");
    std::cout << "  Pioneer DDJ-400 crossfader mapping: OK" << std::endl;
    
    auto volumeAMapping = profile->getMappingForFunction("deckA_volume");
    assert(volumeAMapping.has_value() && "Deck A volume mapping should exist");
    std::cout << "  Pioneer DDJ-400 deckA volume mapping: OK" << std::endl;
    
    auto volumeBMapping = profile->getMappingForFunction("deckB_volume");
    assert(volumeBMapping.has_value() && "Deck B volume mapping should exist");
    std::cout << "  Pioneer DDJ-400 deckB volume mapping: OK" << std::endl;
    
    auto eqLowAMapping = profile->getMappingForFunction("deckA_eq_low");
    assert(eqLowAMapping.has_value() && "Deck A EQ low mapping should exist");
    std::cout << "  Pioneer DDJ-400 deckA EQ low mapping: OK" << std::endl;
    
    std::cout << "  PASSED" << std::endl;
}

// ============================================================================
// Test 6: MIDIMapping JogWheel
// ============================================================================
void test_MIDIMapping_Jogwheel() {
    std::cout << "Test 6: MIDIMapping_Jogwheel..." << std::endl;
    
    using namespace dj::input;
    
    // Create a jogwheel mapping with 14-bit resolution
    MIDIMapping jogMapping;
    jogMapping.setMessageType(MIDICurveType::PitchBend);
    jogMapping.setChannel(0);
    jogMapping.setCurveType(CurveType::Exponential);  // For velocity sensitivity
    jogMapping.setValueRange(-1.0f, 1.0f);  // -1.0 to 1.0 for pitch bend
    jogMapping.setTargetFunction("deckA_pitch");
    jogMapping.setJogWheelAcceleration(true);
    
    // Test slow wheel movement (small pitch bend delta)
    // Pitch bend neutral is 0x2000 (8192 in 14-bit), slow = 0x1E00 (7680)
    float slowBend = jogMapping.applyMapping(7680);
    assert(slowBend < 0.0f && "Slow downward wheel should give small negative pitch bend");
    std::cout << "  Slow wheel down (14-bit 7680) -> " << slowBend << ": OK" << std::endl;
    
    // Test fast wheel movement (large pitch bend)
    // Fast down = 0x0000 (minimum)
    float fastBend = jogMapping.applyMapping(0);
    assert(fastBend < slowBend && "Fast downward wheel should give larger negative pitch bend");
    std::cout << "  Fast wheel down (14-bit 0) -> " << fastBend << ": OK" << std::endl;
    
    // Test upward movement
    // Fast up = 0x3FFF (16383 maximum)
    float fastBendUp = jogMapping.applyMapping(16383);
    assert(fastBendUp > 0.0f && "Upward wheel should give positive pitch bend");
    std::cout << "  Fast wheel up (14-bit 16383) -> " << fastBendUp << ": OK" << std::endl;
    
    std::cout << "  PASSED" << std::endl;
}

int main() {
    std::cout << "\n=== Phase 13: MIDI Mapping System Tests ===\n" << std::endl;
    
    try {
        test_MIDIMapping_LinearCurve();
        test_MIDIMapping_LogarithmicCurve();
        test_MIDIMapping_Serialization();
        test_MIDILearnMode_Capture();
        test_ControllerProfiles_PioneerDDJ();
        test_MIDIMapping_Jogwheel();
        
        std::cout << "\n=== ALL TESTS PASSED ===\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test exception: " << e.what() << std::endl;
        return 1;
    }
}
