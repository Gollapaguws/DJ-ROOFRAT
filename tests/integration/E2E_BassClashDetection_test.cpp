// Phase 3: Integration E2E Test - Bass Clash Detection
// Tests: Load bass-heavy tracks → play on both decks → verify bass clash detection triggers

#include "audio/Deck.h"
#include "audio/AudioClip.h"
#include "audio/SpectrumAnalyzer.h"
#include "gameplay/MixQualityAnalyzer.h"

#include <cassert>
#include <iostream>
#include <cmath>
#include <vector>

namespace dj {

// E2E Test 1: Bass clash detection with overlapping bass frequencies
void test_E2E_BassClashDetection_Trigger() {
    std::cout << "[TEST 1] E2E_BassClashDetection_Trigger...\n";
    
    try {
        const int sampleRate = 44100;
        const std::size_t framesPerBlock = 512;
        
        // Step 1: Create test audio clips with overlapping bass frequencies
        // Bass frequency range: 20-250 Hz, using 80 Hz for both tracks
        auto clipA = AudioClip::generateTestTone(80.0f, 2.0f, sampleRate);   // Bass on Deck A
        auto clipB = AudioClip::generateTestTone(100.0f, 2.0f, sampleRate);  // Bass on Deck B
        
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
        
        // Step 4: Process frames to populate spectrum analyzers
        // Need to process enough frames to fill the FFT window (1024 samples)
        for (int i = 0; i < 1100; ++i) {
            deckA.nextFrame();
            deckB.nextFrame();
        }
        
        // Step 5: Get spectrum analyzers from decks
        const SpectrumAnalyzer* analyzerA = deckA.getSpectrumAnalyzer();
        const SpectrumAnalyzer* analyzerB = deckB.getSpectrumAnalyzer();
        
        assert(analyzerA != nullptr && "Deck A spectrum analyzer is null");
        assert(analyzerB != nullptr && "Deck B spectrum analyzer is null");
        
        // Step 6: Get spectrum data
        auto spectrumA = analyzerA->getFullSpectrum();
        auto spectrumB = analyzerB->getFullSpectrum();
        
        assert(!spectrumA.empty() && "Spectrum A is empty");
        assert(!spectrumB.empty() && "Spectrum B is empty");
        
        // Debug: Print bass bin averages
        float bassA = 0.0f, bassB = 0.0f;
        size_t bassBins = std::min<size_t>(21, spectrumA.size());
        for (size_t i = 0; i < bassBins; ++i) {
            bassA += spectrumA[i];
            bassB += spectrumB[i];
        }
        bassA /= static_cast<float>(bassBins);
        bassB /= static_cast<float>(bassBins);
        std::cout << "  Debug: Bass A avg = " << bassA << ", Bass B avg = " << bassB << "\n";
        
        // Step 7: Use MixQualityAnalyzer bass-clash detection on spectra
        MixQualityAnalyzer analyzer;
        const size_t numBins = std::min(spectrumA.size(), spectrumB.size());
        const bool bassClash = analyzer.detectBassClash(spectrumA.data(), spectrumB.data(), numBins);
        
        // Step 8: Verify bass clash is detected (both playing bass frequencies)
        assert(bassClash && "Bass clash should be detected when both decks play bass frequencies");
        
        std::cout << "  ✓ Bass clash detected successfully\n";
        std::cout << "  ✓ Spectrum A size: " << spectrumA.size() << "\n";
        std::cout << "  ✓ Spectrum B size: " << spectrumB.size() << "\n";
        std::cout << "PASS\n";
    } catch (const std::exception& e) {
        std::cout << "  ✗ Exception during bass clash detection: " << e.what() << "\n";
        assert(false && "Bass clash detection test failed");
    }
}

// E2E Test 2: No bass clash when only one deck plays bass
void test_E2E_BassClashDetection_NoTrigger() {
    std::cout << "[TEST 2] E2E_BassClashDetection_NoTrigger...\n";
    
    try {
        const int sampleRate = 44100;
        
        // Step 1: Create test audio clips - one with bass, one with treble
        auto clipA = AudioClip::generateTestTone(80.0f, 2.0f, sampleRate);    // Bass
        auto clipB = AudioClip::generateTestTone(2000.0f, 2.0f, sampleRate);  // Treble
        
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
        
        // Step 4: Process frames
        for (int i = 0; i < 1100; ++i) {
            deckA.nextFrame();
            deckB.nextFrame();
        }
        
        // Step 5: Get spectrum data
        const SpectrumAnalyzer* analyzerA = deckA.getSpectrumAnalyzer();
        const SpectrumAnalyzer* analyzerB = deckB.getSpectrumAnalyzer();
        
        auto spectrumA = analyzerA->getFullSpectrum();
        auto spectrumB = analyzerB->getFullSpectrum();
        
        // Step 6: Check for bass clash using MixQualityAnalyzer
        MixQualityAnalyzer analyzer;
        const size_t numBins = std::min(spectrumA.size(), spectrumB.size());
        const bool bassClash = analyzer.detectBassClash(spectrumA.data(), spectrumB.data(), numBins);
        
        // Step 7: Verify NO bass clash (different frequency ranges)
        assert(!bassClash && "Bass clash should NOT be detected when frequencies don't overlap in bass range");
        
        std::cout << "  ✓ No false positive bass clash detected\n";
        std::cout << "PASS\n";
    } catch (const std::exception& e) {
        std::cout << "  ✗ Exception during no-trigger test: " << e.what() << "\n";
        assert(false && "No-trigger test failed");
    }
}

void runAllE2EBassClashDetectionTests() {
    std::cout << "\n=== E2E Bass Clash Detection Tests ===\n";
    test_E2E_BassClashDetection_Trigger();
    test_E2E_BassClashDetection_NoTrigger();
    std::cout << "All E2E Bass Clash Detection tests passed!\n\n";
}

} // namespace dj

int main() {
    dj::runAllE2EBassClashDetectionTests();
    return 0;
}
