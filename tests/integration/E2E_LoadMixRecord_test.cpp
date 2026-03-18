// Phase 1: Integration E2E Test - Load, Mix, Record Workflow
// Tests: Load audio file → play on both decks → crossfade → record to buffer → verify output

#include "audio/Deck.h"
#include "audio/Mixer.h"
#include "audio/AudioClip.h"
#include "audio/Recorder.h"

#include <cassert>
#include <iostream>
#include <cmath>
#include <vector>

namespace dj {

// E2E Test 1: Full workflow - Load, Mix, and Record
void test_E2E_LoadMixRecord_FullWorkflow() {
    std::cout << "[TEST 1] E2E_LoadMixRecord_FullWorkflow...\n";
    
    try {
        const int sampleRate = 44100;
        const std::size_t framesPerBlock = 512;
        
        // Step 1: Create test audio clips with different frequencies
        auto clipA = AudioClip::generateTestTone(440.0f, 2.0f, sampleRate);  // A4: 440 Hz
        auto clipB = AudioClip::generateTestTone(880.0f, 2.0f, sampleRate);  // A5: 880 Hz
        
        assert(!clipA.empty() && "Failed to generate clipA");
        assert(!clipB.empty() && "Failed to generate clipB");
        
        // Step 2: Load into decks
        Deck deckA, deckB;
        deckA.setOutputSampleRate(sampleRate);
        deckB.setOutputSampleRate(sampleRate);
        
        bool loadedA = deckA.loadClip(clipA);
        bool loadedB = deckB.loadClip(clipB);
        assert(loadedA && loadedB && "Failed to load clips into decks");
        
        // Step 3: Start playback
        deckA.play();
        deckB.play();
        assert(deckA.isPlaying() && deckB.isPlaying() && "Decks failed to start playing");
        
        // Step 4: Set up mixer and recorder
        Mixer mixer;
        mixer.setCrossfader(-1.0f);  // Start at DeckA
        
        Recorder recorder(sampleRate, 2, 1000);  // Record 1 second worth
        recorder.start();
        
        // Step 5: Simulate mixing with crossfade
        float numBlocks = 2.0f;  // Small number for quick test
        for (int blockNum = 0; blockNum < static_cast<int>(numBlocks); ++blockNum) {
            // Crossfade from -1.0 (DeckA only) to +1.0 (DeckB only)
            float progress = static_cast<float>(blockNum) / numBlocks;
            float crossfadePos = (progress * 2.0f) - 1.0f;  // -1.0 to +1.0
            mixer.setCrossfader(crossfadePos);
            
            // Mix block
            MixMetrics metrics;
            std::vector<float> mixedBlock = mixer.mixBlock(deckA, deckB, framesPerBlock, metrics);
            
            assert(mixedBlock.size() == framesPerBlock * 2 && "Mixer produced incorrect block size");
            
            // Record the mixed output
            recorder.submitFrames(mixedBlock.data(), framesPerBlock);
        }
        
        // Step 6: Stop recording
        recorder.stop();
        deckA.stop();
        deckB.stop();
        
        // Step 7: Verify recorded data
        const auto& recordedData = recorder.getRecordedData();
        assert(!recordedData.empty() && "Recorder captured no data");
        assert(recordedData.size() >= framesPerBlock * 2 * static_cast<int>(numBlocks) &&
               "Recorder captured less data than expected");
        
        // Verify output is not all zeros
        bool hasSignal = false;
        for (float sample : recordedData) {
            if (std::abs(sample) > 0.01f) {
                hasSignal = true;
                break;
            }
        }
        assert(hasSignal && "Recorded output is all silence");
        
        std::cout << "  ✓ Load->Mix->Record workflow completed successfully\n";
        std::cout << "  ✓ Recorded " << recordedData.size() / 2 << " frames\n";
        std::cout << "PASS\n";
    } catch (const std::exception& e) {
        std::cout << "  ✗ Exception during E2E workflow: " << e.what() << "\n";
        assert(false && "E2E workflow test failed");
    }
}

// E2E Test 2: Recording filename customization and auto fallback
void test_E2E_Recording_CustomFilename() {
    std::cout << "[TEST 2] E2E_Recording_CustomFilename...\n";

    try {
        const int sampleRate = 44100;
        const std::size_t framesPerBlock = 512;

        Recorder recorder(sampleRate, 2, 60);

        // Verify custom filename path is used when set
        recorder.setExportFilename("phase3_custom_mix.wav");
        assert(recorder.getExportFilename() == "phase3_custom_mix.wav" &&
               "Custom export filename not stored correctly");
        assert(recorder.getExportFilenameOrDefault("auto_fallback.wav") == "phase3_custom_mix.wav" &&
               "Custom export filename should override fallback");

        // Verify fallback path is used when custom filename is cleared
        recorder.setExportFilename("");
        assert(recorder.getExportFilename().empty() && "Custom export filename should be cleared");
        assert(recorder.getExportFilenameOrDefault("auto_fallback.wav") == "auto_fallback.wav" &&
               "Fallback filename should be used when no custom filename is set");

        // Ensure recording pipeline remains functional with filename operations
        recorder.start();
        std::vector<float> block(framesPerBlock * 2, 0.15f);
        recorder.submitFrames(block.data(), framesPerBlock);
        recorder.stop();

        assert(recorder.getDuration() > 0.0f && "Recorder duration should be > 0 after capture");

        std::cout << "  ✓ Custom recording filename path works\n";
        std::cout << "  ✓ Auto fallback filename path works\n";
        std::cout << "PASS\n";
    } catch (const std::exception& e) {
        std::cout << "  ✗ Exception during custom filename test: " << e.what() << "\n";
        assert(false && "Custom filename test failed");
    }
}

void runAllE2ELoadMixRecordTests() {
    std::cout << "\n=== Running E2E Load/Mix/Record Tests ===\n";
    try {
        test_E2E_LoadMixRecord_FullWorkflow();
        test_E2E_Recording_CustomFilename();
    } catch (const std::exception& e) {
        std::cerr << "E2E test failed with exception: " << e.what() << "\n";
    }
}

} // namespace dj

// Entry point
int main() {
    dj::runAllE2ELoadMixRecordTests();
    std::cout << "\n=== E2E Load/Mix/Record Tests Complete ===\n";
    return 0;
}
