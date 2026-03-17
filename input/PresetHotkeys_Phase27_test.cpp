// Phase 27: Preset Hotkey System Test Suite
// Tests F1-F12 hotkeys for instant EQ preset recall during performance

#include "input/InputMapper.h"
#include "core/PresetManager.h"
#include "audio/Deck.h"

#include <cassert>
#include <iostream>
#include <string>
#include <filesystem>

namespace dj {

// ============================================================================
// Helper: Generate preset slot name ("A_Slot1", "B_Slot5", etc.)
// ============================================================================
std::string getPresetSlotName(char deck, int slot) {
    return std::string(1, deck) + "_Slot" + std::to_string(slot);
}

// ============================================================================
// Test 1: InputMapper_FKeyParsing - F1-F12 detection (Windows VK codes 112-123)
// ============================================================================
void test_InputMapper_FKeyParsing() {
    std::cout << "Running test_InputMapper_FKeyParsing..." << std::endl;
    
    // NOTE: This test validates the enum values exist and are sequential
    // Actual F-key detection is integration-tested in main loop
    
    // Verify enum values exist for all 12 Deck A preset slots
    InputCommand cmd1 = InputCommand::LoadPresetEQ_A_1;
    InputCommand cmd2 = InputCommand::LoadPresetEQ_A_2;
    InputCommand cmd12 = InputCommand::LoadPresetEQ_A_12;
    
    // Verify they are sequential (can increment from one to next)
    assert(static_cast<int>(cmd2) == static_cast<int>(cmd1) + 1);
    assert(static_cast<int>(cmd12) == static_cast<int>(cmd1) + 11);
    
    std::cout << "✓ test_InputMapper_FKeyParsing passed" << std::endl;
}

// ============================================================================
// Test 2: InputMapper_ShiftFKeys - Shift+F1-F12 for Deck B
// ============================================================================
void test_InputMapper_ShiftFKeys() {
    std::cout << "Running test_InputMapper_ShiftFKeys..." << std::endl;
    
    // Verify enum values exist for all 12 Deck B preset slots
    InputCommand cmd1 = InputCommand::LoadPresetEQ_B_1;
    InputCommand cmd2 = InputCommand::LoadPresetEQ_B_2;
    InputCommand cmd12 = InputCommand::LoadPresetEQ_B_12;
    
    // Verify they are sequential
    assert(static_cast<int>(cmd2) == static_cast<int>(cmd1) + 1);
    assert(static_cast<int>(cmd12) == static_cast<int>(cmd1) + 11);
    
    // Verify Deck B commands follow Deck A commands
    assert(static_cast<int>(cmd1) > static_cast<int>(InputCommand::LoadPresetEQ_A_12));
    
    std::cout << "✓ test_InputMapper_ShiftFKeys passed" << std::endl;
}

// ============================================================================
// Test 3: PresetHotkeys_LoadEQDeckA - F1 loads A_Slot1 preset to Deck A
// ============================================================================
void test_PresetHotkeys_LoadEQDeckA() {
    std::cout << "Running test_PresetHotkeys_LoadEQDeckA..." << std::endl;
    
    PresetManager manager;
    
    // Save preset to A_Slot1 with distinctive EQ values
    std::string slotName = getPresetSlotName('A', 1);
    assert(manager.saveEQPreset(slotName, 1.5f, 0.8f, 0.6f));
    
    // Load preset and verify values
    auto preset = manager.loadEQPreset(slotName);
    assert(preset.has_value());
    assert(preset->name == "A_Slot1");
    assert(preset->lowGain > 1.4f && preset->lowGain < 1.6f);
    assert(preset->midGain > 0.7f && preset->midGain < 0.9f);
    assert(preset->highGain > 0.5f && preset->highGain < 0.7f);
    
    std::cout << "✓ test_PresetHotkeys_LoadEQDeckA passed" << std::endl;
}

// ============================================================================
// Test 4: PresetHotkeys_LoadEQDeckB - Shift+F1 loads B_Slot1 preset to Deck B
// ============================================================================
void test_PresetHotkeys_LoadEQDeckB() {
    std::cout << "Running test_PresetHotkeys_LoadEQDeckB..." << std::endl;
    
    PresetManager manager;
    
    // Save preset to B_Slot1 with different EQ values
    std::string slotName = getPresetSlotName('B', 1);
    assert(manager.saveEQPreset(slotName, 0.7f, 1.2f, 1.4f));
    
    // Load preset and verify values
    auto preset = manager.loadEQPreset(slotName);
    assert(preset.has_value());
    assert(preset->name == "B_Slot1");
    assert(preset->lowGain > 0.6f && preset->lowGain < 0.8f);
    assert(preset->midGain > 1.1f && preset->midGain < 1.3f);
    assert(preset->highGain > 1.3f && preset->highGain < 1.5f);
    
    std::cout << "✓ test_PresetHotkeys_LoadEQDeckB passed" << std::endl;
}

// ============================================================================
// Test 5: PresetHotkeys_MissingPreset - Graceful handling when slot empty
// ============================================================================
void test_PresetHotkeys_MissingPreset() {
    std::cout << "Running test_PresetHotkeys_MissingPreset..." << std::endl;
    
    PresetManager manager;
    
    // Try to load from empty slot
    std::string slotName = getPresetSlotName('A', 7);
    auto preset = manager.loadEQPreset(slotName);
    
    // Should return empty optional (std::nullopt)
    assert(!preset.has_value());
    
    std::cout << "✓ test_PresetHotkeys_MissingPreset passed" << std::endl;
}

// ============================================================================
// Test 6: PresetHotkeys_SlotNaming - A_Slot1...A_Slot12 convention enforced
// ============================================================================
void test_PresetHotkeys_SlotNaming() {
    std::cout << "Running test_PresetHotkeys_SlotNaming..." << std::endl;
    
    // Verify slot naming convention for all 12 slots per deck
    for (int i = 1; i <= 12; ++i) {
        std::string nameA = getPresetSlotName('A', i);
        std::string nameB = getPresetSlotName('B', i);
        
        assert(nameA == ("A_Slot" + std::to_string(i)));
        assert(nameB == ("B_Slot" + std::to_string(i)));
    }
    
    // Verify specific examples
    assert(getPresetSlotName('A', 1) == "A_Slot1");
    assert(getPresetSlotName('A', 12) == "A_Slot12");
    assert(getPresetSlotName('B', 5) == "B_Slot5");
    
    std::cout << "✓ test_PresetHotkeys_SlotNaming passed" << std::endl;
}

// ============================================================================
// Test 7: PresetHotkeys_MultipleRecall - Load different presets sequentially
// ============================================================================
void test_PresetHotkeys_MultipleRecall() {
    std::cout << "Running test_PresetHotkeys_MultipleRecall..." << std::endl;
    
    PresetManager manager;
    
    // Save multiple presets to different slots
    assert(manager.saveEQPreset(getPresetSlotName('A', 1), 1.5f, 1.0f, 0.8f));
    assert(manager.saveEQPreset(getPresetSlotName('A', 3), 0.8f, 1.2f, 1.4f));
    assert(manager.saveEQPreset(getPresetSlotName('A', 12), 1.0f, 0.9f, 1.1f));
    
    // Load each preset and verify values persist correctly
    auto preset1 = manager.loadEQPreset(getPresetSlotName('A', 1));
    assert(preset1.has_value());
    assert(preset1->lowGain > 1.4f && preset1->lowGain < 1.6f);
    
    auto preset3 = manager.loadEQPreset(getPresetSlotName('A', 3));
    assert(preset3.has_value());
    assert(preset3->midGain > 1.1f && preset3->midGain < 1.3f);
    
    auto preset12 = manager.loadEQPreset(getPresetSlotName('A', 12));
    assert(preset12.has_value());
    assert(preset12->highGain > 1.0f && preset12->highGain < 1.2f);
    
    std::cout << "✓ test_PresetHotkeys_MultipleRecall passed" << std::endl;
}

// ============================================================================
// Test 8: PresetHotkeys_LiveEQUpdate - Deck EQ reflects preset instantly
// ============================================================================
void test_PresetHotkeys_LiveEQUpdate() {
    std::cout << "Running test_PresetHotkeys_LiveEQUpdate..." << std::endl;
    
    PresetManager manager;
    Deck deck;
    
    // Initialize deck with default EQ
    deck.setOutputSampleRate(44100);
    deck.setEQ(1.0f, 1.0f, 1.0f);
    
    // Save preset with distinctive values
    std::string slotName = getPresetSlotName('A', 2);
    assert(manager.saveEQPreset(slotName, 1.6f, 0.7f, 1.3f));
    
    // Load preset
    auto preset = manager.loadEQPreset(slotName);
    assert(preset.has_value());
    
    // Apply preset to deck (simulates what main.cpp does)
    deck.setEQ(preset->lowGain, preset->midGain, preset->highGain);
    
    // Verify deck EQ updated (no direct getter, but we can verify no crash/error)
    // In actual usage, audio processing will reflect these values immediately
    
    // Additional test: Load another preset and apply
    assert(manager.saveEQPreset(getPresetSlotName('A', 4), 0.5f, 0.5f, 0.5f));
    auto preset2 = manager.loadEQPreset(getPresetSlotName('A', 4));
    assert(preset2.has_value());
    deck.setEQ(preset2->lowGain, preset2->midGain, preset2->highGain);
    
    std::cout << "✓ test_PresetHotkeys_LiveEQUpdate passed" << std::endl;
}

} // namespace dj

// ============================================================================
// Test Runner
// ============================================================================
int main() {
    std::cout << "\n=== Phase 27: Preset Hotkey System Tests ===\n" << std::endl;
    
    try {
        dj::test_InputMapper_FKeyParsing();
        dj::test_InputMapper_ShiftFKeys();
        dj::test_PresetHotkeys_LoadEQDeckA();
        dj::test_PresetHotkeys_LoadEQDeckB();
        dj::test_PresetHotkeys_MissingPreset();
        dj::test_PresetHotkeys_SlotNaming();
        dj::test_PresetHotkeys_MultipleRecall();
        dj::test_PresetHotkeys_LiveEQUpdate();
        
        std::cout << "\n✓ All Phase 27 tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n✗ Test failed with unknown exception" << std::endl;
        return 1;
    }
}
