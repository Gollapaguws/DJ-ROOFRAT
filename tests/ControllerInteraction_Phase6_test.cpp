#include "visuals/ControllerInteraction.h"
#include "visuals/DJControllerGeometry.h"
#include "audio/Deck.h"
#include "audio/Mixer.h"
#include "audio/AudioClip.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>

namespace dj {

// Helper for floating-point comparison
bool floatsApproximatelyEqual(float a, float b, float epsilon = 0.01f) {
    return std::abs(a - b) < epsilon;
}

// ============================================================================
// Test 1: test_ControllerInteraction_JogWheelScratching
// ============================================================================
// Setup deck A with test audio loaded
// Fast drag on jog wheel (large deltaX like 200 pixels)
// Verify scratch velocity applied to deck (check tempo or scratch mode)
void test_ControllerInteraction_JogWheelScratching() {
    std::cout << "[TEST] ControllerInteraction_JogWheelScratching...\n";
    
    // Create decks and mixer
    Deck deckA;
    Deck deckB;
    Mixer mixer;
    
    // Load test clips
    AudioClip clipA = AudioClip::generateTestTone(440.0f, 5.0f, 44100);
    deckA.loadClip(clipA);
    deckA.setOutputSampleRate(44100);
    
    // Create controller interaction
    ControllerInteraction controller(deckA, deckB, mixer);
    
    // Record initial state
    float initialTempo = deckA.tempoPercent();
    
    // Perform fast drag on jog wheel (large deltaX like 200 pixels)
    // This should apply high scratch velocity
    controller.handleMouseDown(ControlID::JogWheelA);
    
    // Call updateJogWheelVelocity with large deltaX and deltaTime
    float fastDeltaX = 200.0f;
    float deltaTime = 0.016f;  // ~60 FPS frame
    controller.updateJogWheelVelocity(ControlID::JogWheelA, fastDeltaX, deltaTime);
    
    // Also perform the normal drag
    controller.handleMouseDrag(ControlID::JogWheelA, fastDeltaX, 0.0f);
    controller.handleMouseUp();
    
    // Verify tempo changed (scratching should affect playback)
    float newTempo = deckA.tempoPercent();
    assert(newTempo != initialTempo && "Jog wheel scratch should change tempo");
    
    std::cout << "  PASSED: Jog wheel scratching changed tempo from " << initialTempo 
              << "% to " << newTempo << "%\n";
}

// ============================================================================
// Test 2: test_DJControllerGeometry_HoverHighlightApplied
// ============================================================================
// Create DJControllerGeometry
// Call setHoverHighlight(ControlID::Crossfader)
// Verify crossfader vertices have increased brightness/intensity
void test_DJControllerGeometry_HoverHighlightApplied() {
    std::cout << "[TEST] DJControllerGeometry_HoverHighlightApplied...\n";
    
    // Create controller geometry
    DJControllerGeometry geometry;
    geometry.generateMesh();
    
    // Record original vertices
    const auto& verticesOriginal = geometry.getVertices();
    assert(!verticesOriginal.empty() && "Geometry should have vertices");
    
    // Get a snapshot of crossfader vertices before highlight
    std::vector<Vertex> beforeHighlight = verticesOriginal;
    
    // Apply hover highlight to crossfader
    geometry.setHoverHighlight(ControlID::Crossfader);
    
    // Get vertices after highlight
    const auto& verticesAfter = geometry.getVertices();
    
    // Verify that some vertices changed (highlight was applied)
    // Look for increased normal magnitude (brightness indicator) or changed normals
    bool hasChangedVertices = false;
    for (size_t i = 0; i < beforeHighlight.size() && i < verticesAfter.size(); ++i) {
        // Check if normal values changed (indicating highlight intensity change)
        float normalMagnitudeBefore = std::sqrt(
            beforeHighlight[i].normal[0] * beforeHighlight[i].normal[0] +
            beforeHighlight[i].normal[1] * beforeHighlight[i].normal[1] +
            beforeHighlight[i].normal[2] * beforeHighlight[i].normal[2]
        );
        float normalMagnitudeAfter = std::sqrt(
            verticesAfter[i].normal[0] * verticesAfter[i].normal[0] +
            verticesAfter[i].normal[1] * verticesAfter[i].normal[1] +
            verticesAfter[i].normal[2] * verticesAfter[i].normal[2]
        );
        
        // Allow small floating-point differences
        if (std::abs(normalMagnitudeBefore - normalMagnitudeAfter) > 0.01f) {
            hasChangedVertices = true;
            break;
        }
    }
    
    assert(hasChangedVertices && "Hover highlight should modify vertex normals for brightness");
    
    std::cout << "  PASSED: Hover highlight increased brightness on crossfader vertices\n";
}

// ============================================================================
// Test 3: test_DJControllerGeometry_VisualSmoothingLerp
// ============================================================================
// Set EQ knob position to 1.0
// Call updateVisuals() with target 2.0
// Verify knob position interpolates gradually (not instant snap)
void test_DJControllerGeometry_VisualSmoothingLerp() {
    std::cout << "[TEST] DJControllerGeometry_VisualSmoothingLerp...\n";
    
    // Create controller geometry and decks
    DJControllerGeometry geometry;
    Deck deckA;
    Deck deckB;
    Mixer mixer;
    
    geometry.generateMesh();
    
    // Load test audio
    AudioClip clipA = AudioClip::generateTestTone(440.0f, 5.0f, 44100);
    AudioClip clipB = AudioClip::generateTestTone(880.0f, 5.0f, 44100);
    deckA.loadClip(clipA);
    deckB.loadClip(clipB);
    deckA.setOutputSampleRate(44100);
    deckB.setOutputSampleRate(44100);
    
    // Set initial EQ state (starting at 1.0)
    deckA.setEQ(1.0f, 1.0f, 1.0f);
    deckB.setEQ(1.0f, 1.0f, 1.0f);
    
    // Record initial state
    const auto& verticesInitial = geometry.getVertices();
    std::vector<float> initialPositions;
    for (const auto& v : verticesInitial) {
        initialPositions.push_back(v.position[0]);
        initialPositions.push_back(v.position[1]);
        initialPositions.push_back(v.position[2]);
    }
    
    // Update visuals with deck A EQ at 2.0 (high value)
    // This should trigger gradual lerp
    deckA.setEQ(2.0f, 2.0f, 2.0f);
    geometry.updateVisuals(deckA, deckB, mixer);
    
    // Get positions after first update
    const auto& verticesAfter1 = geometry.getVertices();
    std::vector<float> positionsAfter1;
    for (const auto& v : verticesAfter1) {
        positionsAfter1.push_back(v.position[0]);
        positionsAfter1.push_back(v.position[1]);
        positionsAfter1.push_back(v.position[2]);
    }
    
    // Call updateVisuals again to see lerp progress
    geometry.updateVisuals(deckA, deckB, mixer);
    
    const auto& verticesAfter2 = geometry.getVertices();
    std::vector<float> positionsAfter2;
    for (const auto& v : verticesAfter2) {
        positionsAfter2.push_back(v.position[0]);
        positionsAfter2.push_back(v.position[1]);
        positionsAfter2.push_back(v.position[2]);
    }
    
    // Verify that positions are interpolated, not jumped to target immediately
    // After1 and After2 should be different (intermediate steps of lerp)
    bool hasInterpolation = false;
    for (size_t i = 0; i < positionsAfter1.size() && i < positionsAfter2.size(); ++i) {
        if (std::abs(positionsAfter1[i] - positionsAfter2[i]) > 0.0001f) {
            hasInterpolation = true;
            break;
        }
    }
    
    assert(hasInterpolation && "Visual smoothing should use lerp, not instant snapping");
    
    std::cout << "  PASSED: Visual smoothing applied lerp interpolation\n";
}

// Run all tests
void runAllTests() {
    std::cout << "\n=== ControllerInteraction Phase 6 Tests (TDD - RED state) ===\n\n";
    
    try {
        test_ControllerInteraction_JogWheelScratching();
        test_DJControllerGeometry_HoverHighlightApplied();
        test_DJControllerGeometry_VisualSmoothingLerp();
        
        std::cout << "\n✓ ALL TESTS PASSED\n\n";
    } catch (const std::exception& e) {
        std::cerr << "\n✗ TEST FAILED: " << e.what() << "\n\n";
        throw;
    }
}

}  // namespace dj

int main() {
    dj::runAllTests();
    return 0;
}
