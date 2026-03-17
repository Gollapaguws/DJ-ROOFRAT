#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>

#include "core/ConfigManager.h"
#include "core/SessionState.h"
#include "core/PresetManager.h"

namespace dj {

// ============================================================================
// Test 1: ConfigManager_SaveLoad - Save default config, load, verify values match
// ============================================================================
void test_ConfigManager_SaveLoad() {
    std::cout << "Running test_ConfigManager_SaveLoad..." << std::endl;
    
    std::string testFile = "test_config_phase25.json";
    
    // Create and save
    {
        ConfigManager manager;
        auto config = manager.getConfig();
        
        // Verify defaults
        assert(config.sampleRate == 44100);
        assert(config.bufferSize == 2048);
        assert(config.masterVolume > 0.7f && config.masterVolume < 0.9f);
        assert(config.enableGraphics == true);
        
        // Save to file
        assert(manager.saveConfig(testFile));
    }
    
    // Load and verify
    {
        ConfigManager manager;
        assert(manager.loadConfig(testFile));
        
        auto config = manager.getConfig();
        assert(config.sampleRate == 44100);
        assert(config.bufferSize == 2048);
        assert(config.masterVolume > 0.7f && config.masterVolume < 0.9f);
        assert(config.enableGraphics == true);
        assert(config.graphicsWidth == 1920);
        assert(config.graphicsHeight == 1080);
    }
    
    // Cleanup
    std::filesystem::remove(testFile);
    std::cout << "✓ test_ConfigManager_SaveLoad passed" << std::endl;
}

// ============================================================================
// Test 2: ConfigManager_InvalidJSON - Load corrupted JSON, verify returns false
// ============================================================================
void test_ConfigManager_InvalidJSON() {
    std::cout << "Running test_ConfigManager_InvalidJSON..." << std::endl;
    
    std::string testFile = "test_invalid_config.json";
    
    // Create corrupted JSON
    {
        std::ofstream file(testFile);
        file << "{ \"sampleRate\": CORRUPTED }";
        file.close();
    }
    
    // Try to load - should fail gracefully
    {
        ConfigManager manager;
        bool loaded = manager.loadConfig(testFile);
        assert(!loaded);  // Should fail on corrupted JSON
    }
    
    // Cleanup
    std::filesystem::remove(testFile);
    std::cout << "✓ test_ConfigManager_InvalidJSON passed" << std::endl;
}

// ============================================================================
// Test 3: SessionState_SaveLoad - Save session with deck states, load, verify all fields
// ============================================================================
void test_SessionState_SaveLoad() {
    std::cout << "Running test_SessionState_SaveLoad..." << std::endl;
    
    std::string testFile = "test_session_phase25.json";
    
    // Create session with data
    SessionState sessionState;
    sessionState.deckA.trackPath = "tracks/song1.wav";
    sessionState.deckA.playbackPosition = 45.3;
    sessionState.deckA.isPlaying = true;
    sessionState.deckA.lowGain = 1.0f;
    sessionState.deckA.midGain = 0.8f;
    sessionState.deckA.highGain = 1.2f;
    
    sessionState.deckB.trackPath = "tracks/song2.wav";
    sessionState.deckB.playbackPosition = 12.1;
    sessionState.deckB.isPlaying = false;
    sessionState.deckB.lowGain = 1.1f;
    sessionState.deckB.midGain = 1.0f;
    sessionState.deckB.highGain = 0.9f;
    
    sessionState.crossfader = 0.3f;
    sessionState.currentCareerTier = 2;
    sessionState.crowdEnergy = 0.75f;
    sessionState.venueId = "warehouse";
    
    // Save session
    {
        SessionManager manager;
        assert(manager.saveSession(testFile, sessionState));
    }
    
    // Load and verify
    {
        SessionManager manager;
        auto loaded = manager.loadSession(testFile);
        
        assert(loaded.has_value());
        auto state = loaded.value();
        
        // Verify deckA
        assert(state.deckA.trackPath == "tracks/song1.wav");
        assert(state.deckA.playbackPosition > 45.0 && state.deckA.playbackPosition < 46.0);
        assert(state.deckA.isPlaying == true);
        assert(state.deckA.lowGain > 0.9f && state.deckA.lowGain < 1.1f);
        
        // Verify deckB
        assert(state.deckB.trackPath == "tracks/song2.wav");
        assert(state.deckB.playbackPosition > 12.0 && state.deckB.playbackPosition < 12.2);
        assert(state.deckB.isPlaying == false);
        
        // Verify session fields
        assert(state.crossfader > 0.2f && state.crossfader < 0.4f);
        assert(state.currentCareerTier == 2);
        assert(state.crowdEnergy > 0.7f && state.crowdEnergy < 0.8f);
        assert(state.venueId == "warehouse");
    }
    
    // Cleanup
    std::filesystem::remove(testFile);
    std::cout << "✓ test_SessionState_SaveLoad passed" << std::endl;
}

// ============================================================================
// Test 4: SessionState_NestedObjects - Test deck A/B nested JSON parsing
// ============================================================================
void test_SessionState_NestedObjects() {
    std::cout << "Running test_SessionState_NestedObjects..." << std::endl;
    
    std::string testFile = "test_nested_session.json";
    
    // Create complex session with distinctive values
    SessionState sessionState;
    sessionState.deckA.trackPath = "path/with/special/chars.wav";
    sessionState.deckA.tempoBend = 0.05f;
    sessionState.deckB.trackPath = "another/path.wav";
    sessionState.deckB.tempoBend = -0.03f;
    sessionState.crowdEnergy = 0.99f;
    sessionState.venueId = "big_stadium";
    
    // Save and reload multiple times to verify nested object integrity
    for (int iteration = 0; iteration < 2; ++iteration) {
        SessionManager manager;
        
        if (iteration == 0) {
            assert(manager.saveSession(testFile, sessionState));
        } else {
            // Load, modify, save again
            auto loaded = manager.loadSession(testFile);
            assert(loaded.has_value());
            sessionState = loaded.value();
            assert(manager.saveSession(testFile, sessionState));
        }
    }
    
    // Final verification
    {
        SessionManager manager;
        auto loaded = manager.loadSession(testFile);
        assert(loaded.has_value());
        
        auto state = loaded.value();
        assert(state.deckA.trackPath == "path/with/special/chars.wav");
        assert(state.deckB.trackPath == "another/path.wav");
        assert(state.venueId == "big_stadium");
    }
    
    // Cleanup
    std::filesystem::remove(testFile);
    std::cout << "✓ test_SessionState_NestedObjects passed" << std::endl;
}

// ============================================================================
// Test 5: PresetManager_EQPresets - Save/load/list/delete EQ preset
// ============================================================================
void test_PresetManager_EQPresets() {
    std::cout << "Running test_PresetManager_EQPresets..." << std::endl;
    
    PresetManager manager;
    
    // Save EQ preset
    assert(manager.saveEQPreset("bass_boost", 1.5f, 1.0f, 0.8f));
    assert(manager.saveEQPreset("treble_cut", 1.0f, 1.0f, 0.5f));
    
    // List presets
    auto presets = manager.listEQPresets();
    assert(presets.size() >= 2);
    assert(std::find(presets.begin(), presets.end(), "bass_boost") != presets.end());
    assert(std::find(presets.begin(), presets.end(), "treble_cut") != presets.end());
    
    // Load and verify
    auto loaded = manager.loadEQPreset("bass_boost");
    assert(loaded.has_value());
    assert(loaded->name == "bass_boost");
    assert(loaded->lowGain > 1.4f && loaded->lowGain < 1.6f);
    assert(loaded->midGain > 0.9f && loaded->midGain < 1.1f);
    assert(loaded->highGain > 0.7f && loaded->highGain < 0.9f);
    
    // Delete preset
    assert(manager.deleteEQPreset("treble_cut"));
    auto afterDelete = manager.listEQPresets();
    assert(std::find(afterDelete.begin(), afterDelete.end(), "treble_cut") == afterDelete.end());
    
    // Try to load deleted - should fail
    auto shouldBeEmpty = manager.loadEQPreset("treble_cut");
    assert(!shouldBeEmpty.has_value());
    
    std::cout << "✓ test_PresetManager_EQPresets passed" << std::endl;
}

// ============================================================================
// Test 6: PresetManager_EffectPresets - Save/load effect preset with parameters
// ============================================================================
void test_PresetManager_EffectPresets() {
    std::cout << "Running test_PresetManager_EffectPresets..." << std::endl;
    
    PresetManager manager;
    
    // Create effect preset
    EffectPreset reverbPreset;
    reverbPreset.name = "large_hall";
    reverbPreset.effectType = "reverb";
    reverbPreset.parameters["roomSize"] = 0.8f;
    reverbPreset.parameters["damping"] = 0.5f;
    reverbPreset.parameters["wetLevel"] = 0.3f;
    reverbPreset.parameters["dryLevel"] = 0.7f;
    
    // Save
    assert(manager.saveEffectPreset(reverbPreset));
    
    // Load and verify
    auto loaded = manager.loadEffectPreset("large_hall");
    assert(loaded.has_value());
    assert(loaded->name == "large_hall");
    assert(loaded->effectType == "reverb");
    assert(loaded->parameters.count("roomSize") > 0);
    assert(loaded->parameters["roomSize"] > 0.7f && loaded->parameters["roomSize"] < 0.9f);
    assert(loaded->parameters["wetLevel"] > 0.2f && loaded->parameters["wetLevel"] < 0.4f);
    
    std::cout << "✓ test_PresetManager_EffectPresets passed" << std::endl;
}

// ============================================================================
// Test 7: PresetManager_MultiplePresets - Save 3 presets, verify all persist after file reload
// ============================================================================
void test_PresetManager_MultiplePresets() {
    std::cout << "Running test_PresetManager_MultiplePresets..." << std::endl;
    
    std::string testFile = "test_presets_phase25.json";
    
    // Create and save 3 presets
    {
        PresetManager manager;
        
        assert(manager.saveEQPreset("preset_1", 1.0f, 1.0f, 1.0f));
        assert(manager.saveEQPreset("preset_2", 1.2f, 0.9f, 0.8f));
        assert(manager.saveEQPreset("preset_3", 0.8f, 1.1f, 1.3f));
        
        // Add effect presets
        EffectPreset effect1;
        effect1.name = "effect_1";
        effect1.effectType = "delay";
        effect1.parameters["time"] = 0.5f;
        effect1.parameters["feedback"] = 0.3f;
        
        EffectPreset effect2;
        effect2.name = "effect_2";
        effect2.effectType = "filter";
        effect2.parameters["frequency"] = 1000.0f;
        effect2.parameters["resonance"] = 0.5f;
        
        assert(manager.saveEffectPreset(effect1));
        assert(manager.saveEffectPreset(effect2));
        
        // Save to file
        assert(manager.saveToFile(testFile));
    }
    
    // Load from file
    {
        PresetManager manager;
        assert(manager.loadFromFile(testFile));
        
        // Verify EQ presets persisted
        auto eqList = manager.listEQPresets();
        assert(eqList.size() >= 3);
        assert(std::find(eqList.begin(), eqList.end(), "preset_1") != eqList.end());
        assert(std::find(eqList.begin(), eqList.end(), "preset_2") != eqList.end());
        assert(std::find(eqList.begin(), eqList.end(), "preset_3") != eqList.end());
        
        // Verify effect presets persisted
        auto effectList = manager.listEffectPresets();
        assert(effectList.size() >= 2);
        assert(std::find(effectList.begin(), effectList.end(), "effect_1") != effectList.end());
        assert(std::find(effectList.begin(), effectList.end(), "effect_2") != effectList.end());
        
        // Load specific preset and verify data integrity
        auto preset2 = manager.loadEQPreset("preset_2");
        assert(preset2.has_value());
        assert(preset2->lowGain > 1.1f && preset2->lowGain < 1.3f);
        
        auto effect2 = manager.loadEffectPreset("effect_2");
        assert(effect2.has_value());
        assert(effect2->parameters["frequency"] > 900.0f && effect2->parameters["frequency"] < 1100.0f);
    }
    
    // Cleanup
    std::filesystem::remove(testFile);
    std::cout << "✓ test_PresetManager_MultiplePresets passed" << std::endl;
}

// ============================================================================
// Test 8: PresetManager_EmptyFile - Load from non-existent file, verify empty lists
// ============================================================================
void test_PresetManager_EmptyFile() {
    std::cout << "Running test_PresetManager_EmptyFile..." << std::endl;
    
    std::string nonExistentFile = "this_file_does_not_exist_phase25.json";
    
    // Ensure file doesn't exist
    std::filesystem::remove(nonExistentFile);
    
    PresetManager manager;
    
    // Try to load from non-existent file
    bool loaded = manager.loadFromFile(nonExistentFile);
    assert(!loaded);  // Should return false but not crash
    
    // Lists should be empty
    auto eqList = manager.listEQPresets();
    auto effectList = manager.listEffectPresets();
    assert(eqList.empty());
    assert(effectList.empty());
    
    // Manager should still be usable - add a preset
    assert(manager.saveEQPreset("new_preset", 1.0f, 1.0f, 1.0f));
    assert(manager.listEQPresets().size() > 0);
    
    std::cout << "✓ test_PresetManager_EmptyFile passed" << std::endl;
}

// ============================================================================
// Main test runner
// ============================================================================
void runAllConfigSystemTests() {
    std::cout << "\n=== Phase 25: Configuration & State Management Tests ===\n" << std::endl;
    
    try {
        test_ConfigManager_SaveLoad();
        test_ConfigManager_InvalidJSON();
        test_SessionState_SaveLoad();
        test_SessionState_NestedObjects();
        test_PresetManager_EQPresets();
        test_PresetManager_EffectPresets();
        test_PresetManager_MultiplePresets();
        test_PresetManager_EmptyFile();
        
        std::cout << "\n✓✓✓ All Phase 25 tests passed! ✓✓✓\n" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "\n✗✗✗ Test exception: " << e.what() << " ✗✗✗\n" << std::endl;
        throw;
    }
}

} // namespace dj

int main() {
    dj::runAllConfigSystemTests();
    return 0;
}
