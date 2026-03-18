#include "gameplay/TransitionCoach.h"
#include "audio/BeatGrid.h"
#include "audio/Deck.h"
#include "audio/AudioClip.h"
#include "audio/EnergyAnalyzer.h"
#include "audio/CamelotAnalyzer.h"
#include "gameplay/EnergyCurve.h"
#include "visuals/CoachingHUD.h"

#include <cassert>
#include <iostream>
#include <cmath>

namespace dj {

// Test 1: test_TransitionCoach_PhraseDetection
void test_TransitionCoach_PhraseDetection() {
    std::cout << "[TEST 1] TransitionCoach_PhraseDetection...\n";
    
    // Create a BeatGrid with 64 seconds of audio at 120 BPM
    BeatGrid grid;
    grid.generateFromBPM(120.0, 0.0, 64.0);
    
    auto beats = grid.getBeats();
    std::cout << "  Created " << beats.size() << " beats\n";
    std::cout << "  First beat at: " << beats[0].timestamp << "s\n";
    if (!beats.empty()) {
        std::cout << "  Last beat at: " << beats.back().timestamp << "s\n";
    }
    
    TransitionCoach coach;
    auto phrases = coach.detectPhrases(grid);
    
    // Should detect phrases every 8 seconds (16 beats at 120 BPM)
    // With 64 seconds total: phrases at 8, 16, 24, 32, 40, 48, 56 seconds
    assert(phrases.size() >= 4);  // At least 4 phrases
    
    // Verify spacing (should be ~8 seconds apart)
    for (size_t i = 1; i < phrases.size(); ++i) {
        double spacing = phrases[i] - phrases[i - 1];
        // 16 beats at 120 BPM = 8 seconds
        assert(std::abs(spacing - 8.0) < 1.0);  // Allow ±1 second tolerance
    }
    
    std::cout << "  Detected " << phrases.size() << " phrases\n";
    for (size_t i = 0; i < phrases.size(); ++i) {
        std::cout << "    Phrase " << (i + 1) << " at " << phrases[i] << "s\n";
    }
    std::cout << "PASS\n";
}

// Test 2: test_TransitionCoach_TransitionWindow
void test_TransitionCoach_TransitionWindow() {
    std::cout << "[TEST 2] TransitionCoach_TransitionWindow...\n";
    
    TransitionCoach coach;
    
    // Current position at 10 seconds, next phrase at 18 seconds
    double currentPos = 10.0;
    double nextPhrase = 18.0;
    double bpm = 120.0;
    
    double countdown = coach.calculateCountdown(currentPos, nextPhrase, bpm);
    
    // Time to next phrase: 8 seconds = 16 beats at 120 BPM
    // (8 seconds × 120 BPM / 60 = 16 beats)
    assert(std::abs(countdown - 16.0) < 0.5);
    
    std::cout << "  Time to phrase: " << (nextPhrase - currentPos) << " seconds\n";
    std::cout << "  Countdown: " << countdown << " beats\n";
    std::cout << "PASS\n";
}

// Test 3: test_TransitionCoach_EnergyContext
void test_TransitionCoach_EnergyContext() {
    std::cout << "[TEST 3] TransitionCoach_EnergyContext...\n";
    
    // Create two decks with energy metadata
    auto clipA = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    auto clipB = AudioClip::generateTestTone(880.0f, 10.0f, 44100);
    
    clipA.metadata_ = TrackMetadata{};
    clipA.metadata_->bpm = 120.0f;
    clipB.metadata_ = TrackMetadata{};
    clipB.metadata_->bpm = 120.0f;
    
    Deck deckA, deckB;
    deckA.loadClip(clipA);
    deckB.loadClip(clipB);
    
    deckA.setOutputSampleRate(44100);
    deckB.setOutputSampleRate(44100);
    
    deckA.play();
    deckB.play();
    
    // Simulate some playback to get energy readings
    for (int i = 0; i < 5000; ++i) {
        deckA.nextFrame();
        deckB.nextFrame();
    }
    
    EnergyAnalyzer analyzerA, analyzerB;
    EnergyCurve energyCurve(30.0, 1.0);
    CamelotAnalyzer camelot;
    TransitionCoach coach;
    
    // Get energy readings and add to curve
    float energyA = deckA.recentEnergy();
    float energyB = deckB.recentEnergy();
    
    energyCurve.addSample(energyA, 0.0, energyA);
    energyCurve.addSample(energyB, 1.0, energyB);
    
    // Generate suggestion
    auto suggestion = coach.suggestNextTransition(deckA, deckB, energyCurve, camelot);
    
    // Check that suggestion contains energy context
    assert(suggestion.confidence >= 0.0f && suggestion.confidence <= 1.0f);
    assert(!suggestion.reason.empty());
    
    std::cout << "  Deck A energy: " << energyA << "\n";
    std::cout << "  Deck B energy: " << energyB << "\n";
    std::cout << "  Energy delta: " << suggestion.energyDelta << "%\n";
    std::cout << "  Suggestion: " << suggestion.reason << "\n";
    std::cout << "  Confidence: " << (suggestion.confidence * 100.0f) << "%\n";
    std::cout << "PASS\n";
}

// Test 4: test_TransitionCoach_HarmonicCheck
void test_TransitionCoach_HarmonicCheck() {
    std::cout << "[TEST 4] TransitionCoach_HarmonicCheck...\n";
    
    CamelotAnalyzer camelot;
    
    // Check harmonic compatibility between two Camelot codes
    // 8A and 1A are incompatible (far apart on wheel)
    float score1 = camelot.getCompatibilityScore("8A", "1A");
    assert(score1 < 0.5f);  // Should be low compatibility
    
    // 8A and 8A are perfect match
    float score2 = camelot.getCompatibilityScore("8A", "8A");
    assert(score2 > 0.95f);  // Should be very high
    
    // Create decks with metadata
    auto clipA = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    auto clipB = AudioClip::generateTestTone(880.0f, 10.0f, 44100);
    
    clipA.metadata_ = TrackMetadata{};
    clipA.metadata_->bpm = 120.0f;
    clipA.metadata_->key = "C major";  // Maps to Camelot code
    
    clipB.metadata_ = TrackMetadata{};
    clipB.metadata_->bpm = 120.0f;
    clipB.metadata_->key = "F# major";  // Incompatible key
    
    Deck deckA, deckB;
    deckA.loadClip(clipA);
    deckB.loadClip(clipB);
    
    deckA.setOutputSampleRate(44100);
    deckB.setOutputSampleRate(44100);
    
    deckA.play();
    deckB.play();
    
    for (int i = 0; i < 1000; ++i) {
        deckA.nextFrame();
        deckB.nextFrame();
    }
    
    EnergyCurve energyCurve(30.0, 1.0);
    TransitionCoach coach;
    
    auto suggestion = coach.suggestNextTransition(deckA, deckB, energyCurve, camelot);
    
    // Suggestion should include harmonic score
    assert(suggestion.harmonicScore >= 0.0f && suggestion.harmonicScore <= 1.0f);
    
    std::cout << "  Harmonic score: " << (suggestion.harmonicScore * 100.0f) << "%\n";
    std::cout << "  Reason: " << suggestion.reason << "\n";
    std::cout << "PASS\n";
}

// Test 5: test_TransitionCoach_Countdown
void test_TransitionCoach_Countdown() {
    std::cout << "[TEST 5] TransitionCoach_Countdown...\n";
    
    TransitionCoach coach;
    double bpm = 120.0;
    
    // Simulate countdown from 16 beats to 0
    for (int beats = 16; beats >= 0; --beats) {
        // Convert beats to time
        double timeRemaining = (beats / bpm) * 60.0;  // Convert beats to seconds
        
        double currentPos = 10.0;
        double nextPhrase = currentPos + timeRemaining;
        
        double countdown = coach.calculateCountdown(currentPos, nextPhrase, bpm);
        
        assert(std::abs(countdown - static_cast<double>(beats)) < 0.5);
    }
    
    std::cout << "  Countdown verification: 16 beats -> 0 beats\n";
    std::cout << "PASS\n";
}

// Test 6: test_TransitionCoach_ConfidenceScore
void test_TransitionCoach_ConfidenceScore() {
    std::cout << "[TEST 6] TransitionCoach_ConfidenceScore...\n";
    
    auto clipA = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    auto clipB = AudioClip::generateTestTone(880.0f, 10.0f, 44100);
    
    // Test case 1: Good match (same key, similar energy)
    clipA.metadata_ = TrackMetadata{};
    clipA.metadata_->bpm = 120.0f;
    clipA.metadata_->key = "C major";
    
    clipB.metadata_ = TrackMetadata{};
    clipB.metadata_->bpm = 120.0f;
    clipB.metadata_->key = "C major";
    
    Deck deckA, deckB;
    deckA.loadClip(clipA);
    deckB.loadClip(clipB);
    
    deckA.setOutputSampleRate(44100);
    deckB.setOutputSampleRate(44100);
    
    deckA.play();
    deckB.play();
    
    for (int i = 0; i < 10000; ++i) {
        deckA.nextFrame();
        deckB.nextFrame();
    }
    
    EnergyCurve energyCurve(30.0, 1.0);
    CamelotAnalyzer camelot;
    TransitionCoach coach;
    
    auto suggestion1 = coach.suggestNextTransition(deckA, deckB, energyCurve, camelot);
    assert(suggestion1.confidence >= 0.0f && suggestion1.confidence <= 1.0f);
    
    std::cout << "  Good match confidence: " << (suggestion1.confidence * 100.0f) << "%\n";
    
    // Test case 2: Bad match (incompatible key)
    clipB.metadata_->key = "F# major";
    deckB.stop();
    deckB.loadClip(clipB);
    deckB.play();
    
    for (int i = 0; i < 10000; ++i) {
        deckA.nextFrame();
        deckB.nextFrame();
    }
    
    auto suggestion2 = coach.suggestNextTransition(deckA, deckB, energyCurve, camelot);
    assert(suggestion2.confidence >= 0.0f && suggestion2.confidence <= 1.0f);
    
    std::cout << "  Bad match confidence: " << (suggestion2.confidence * 100.0f) << "%\n";
    std::cout << "PASS\n";
}

// Test 7: test_TransitionCoach_Urgency
void test_TransitionCoach_Urgency() {
    std::cout << "[TEST 7] TransitionCoach_Urgency...\n";
    
    TransitionCoach coach;
    double bpm = 120.0;
    
    auto clipA = AudioClip::generateTestTone(440.0f, 10.0f, 44100);
    auto clipB = AudioClip::generateTestTone(880.0f, 10.0f, 44100);
    
    clipA.metadata_ = TrackMetadata{};
    clipA.metadata_->bpm = 120.0f;
    clipB.metadata_ = TrackMetadata{};
    clipB.metadata_->bpm = 120.0f;
    
    Deck deckA, deckB;
    deckA.loadClip(clipA);
    deckB.loadClip(clipB);
    
    deckA.setOutputSampleRate(44100);
    deckB.setOutputSampleRate(44100);
    
    deckA.play();
    deckB.play();
    
    for (int i = 0; i < 5000; ++i) {
        deckA.nextFrame();
        deckB.nextFrame();
    }
    
    EnergyCurve energyCurve(30.0, 1.0);
    CamelotAnalyzer camelot;
    
    // Test urgency when close to phrase boundary (1 beat away)
    double currentPos = 10.0;
    double nextPhrase1 = currentPos + (1.0 / bpm) * 60.0;  // 1 beat away
    
    double countdown1 = coach.calculateCountdown(currentPos, nextPhrase1, bpm);
    assert(countdown1 <= 1.5);  // Close to boundary
    
    // Test non-urgent when far from phrase boundary (20 beats away)
    double nextPhrase2 = currentPos + (20.0 / bpm) * 60.0;  // 20 beats away
    
    double countdown2 = coach.calculateCountdown(currentPos, nextPhrase2, bpm);
    assert(countdown2 >= 19.0);  // Far from boundary
    
    std::cout << "  Countdown when urgent: " << countdown1 << " beats\n";
    std::cout << "  Countdown when not urgent: " << countdown2 << " beats\n";
    std::cout << "PASS\n";
}

// Test 8: test_CoachingHUD_Render
void test_CoachingHUD_Render() {
    std::cout << "[TEST 8] CoachingHUD_Render...\n";
    
    CoachingHUD hud;
    TransitionCoach::Suggestion suggestion;
    
    suggestion.timestamp = 18.0;
    suggestion.reason = "Perfect key match, energy boost";
    suggestion.confidence = 0.85f;
    suggestion.urgency = false;
    suggestion.energyDelta = 10.5f;
    suggestion.harmonicScore = 0.95f;
    
    std::string output = hud.render(suggestion, 8.0);
    
    // Verify output contains key elements
    assert(!output.empty());
    assert(output.find("85") != std::string::npos ||
           output.find("0.85") != std::string::npos);  // Confidence percentage
    
    // Check for box drawing or visual elements
    bool hasBox = output.find("┌") != std::string::npos ||
                  output.find("─") != std::string::npos ||
                  output.find("└") != std::string::npos ||
                  output.find("[") != std::string::npos;
    assert(hasBox);
    
    std::cout << "  HUD Output:\n";
    std::cout << output << "\n";
    std::cout << "PASS\n";
}

// Test runner
void runAllTransitionCoachTests() {
    std::cout << "\n=== TRANSITION COACH PHASE 40 TESTS ===\n\n";
    
    try {
        test_TransitionCoach_PhraseDetection();
        test_TransitionCoach_TransitionWindow();
        test_TransitionCoach_EnergyContext();
        test_TransitionCoach_HarmonicCheck();
        test_TransitionCoach_Countdown();
        test_TransitionCoach_ConfidenceScore();
        test_TransitionCoach_Urgency();
        test_CoachingHUD_Render();
        
        std::cout << "\n=== ALL TESTS PASSED ===\n";
    } catch (const std::exception& e) {
        std::cout << "\n!!! TEST FAILED: " << e.what() << "\n";
        throw;
    }
}

} // namespace dj

int main() {
    dj::runAllTransitionCoachTests();
    return 0;
}
