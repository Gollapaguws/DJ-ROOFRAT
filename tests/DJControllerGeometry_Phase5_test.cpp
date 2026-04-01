#include <cassert>
#include <iostream>
#include <cmath>
#include <algorithm>

#include "visuals/DJControllerGeometry.h"
#include "audio/Deck.h"
#include "audio/Mixer.h"

namespace {

// Test 1: test_DJControllerGeometry_CrossfaderPositionSync
// Verify crossfader geometry position updates with mixer state
void test_DJControllerGeometry_CrossfaderPositionSync() {
    std::cout << "[TEST] test_DJControllerGeometry_CrossfaderPositionSync...\n";
    
    // Arrange
    dj::Deck deckA, deckB;
    dj::AudioClip clipA = dj::AudioClip::generateTestTone(440.0f, 1.0f, 44100);
    dj::AudioClip clipB = dj::AudioClip::generateTestTone(880.0f, 1.0f, 44100);
    
    bool loadedA = deckA.loadClip(clipA);
    bool loadedB = deckB.loadClip(clipB);
    assert(loadedA && loadedB);
    
    dj::Mixer mixer;
    dj::DJControllerGeometry controller;
    
    // Generate initial mesh
    controller.generateMesh();
    const auto& initialVertices = controller.getVertices();
    assert(!initialVertices.empty());
    
    // Store initial state: identify crossfader control vertices
    // The crossfader starts at default position, we'll look for vertices we can identify as crossfader
    float minX_initial = FLT_MAX, maxX_initial = -FLT_MAX;
    for (const auto& v : initialVertices) {
        if (v.position[2] > 0.35f && v.position[2] < 0.45f) {  // Roughly crossfader Z position
            minX_initial = std::min(minX_initial, v.position[0]);
            maxX_initial = std::max(maxX_initial, v.position[0]);
        }
    }
    
    // Act: Set crossfader halfway left (-0.5)
    mixer.setCrossfader(-0.5f);
    controller.updateVisuals(deckA, deckB, mixer);
    
    // Assert: Crossfader geometry should have changed position
    const auto& updatedVertices = controller.getVertices();
    assert(!updatedVertices.empty());
    
    // Verify at least some vertices changed
    bool verticesChanged = false;
    for (size_t i = 0; i < initialVertices.size() && !verticesChanged; ++i) {
        if (initialVertices[i].position[0] != updatedVertices[i].position[0]) {
            verticesChanged = true;
        }
    }
    assert(verticesChanged && "Crossfader vertex positions should have updated");
    
    std::cout << "✓ test_DJControllerGeometry_CrossfaderPositionSync passed\n";
}

// Test 2: test_DJControllerGeometry_PlayButtonGlow
// Verify play button vertices get color intensity when playing
void test_DJControllerGeometry_PlayButtonGlow() {
    std::cout << "[TEST] test_DJControllerGeometry_PlayButtonGlow...\n";
    
    // Arrange
    dj::Deck deckA, deckB;
    dj::AudioClip clipA = dj::AudioClip::generateTestTone(440.0f, 1.0f, 44100);
    dj::AudioClip clipB = dj::AudioClip::generateTestTone(880.0f, 1.0f, 44100);
    
    bool loadedA = deckA.loadClip(clipA);
    bool loadedB = deckB.loadClip(clipB);
    assert(loadedA && loadedB);
    
    dj::Mixer mixer;
    dj::DJControllerGeometry controller;
    controller.generateMesh();
    
    // Initial state: Deck A not playing
    assert(!deckA.isPlaying());
    controller.updateVisuals(deckA, deckB, mixer);
    
    // Act: Play Deck A
    deckA.play();
    assert(deckA.isPlaying());
    
    // Create a mutable copy to track normals (using normals as color proxy)
    const auto& verticesBefore = controller.getVertices();
    std::vector<float> normalsSumBefore;
    for (const auto& v : verticesBefore) {
        normalsSumBefore.push_back(v.normal[0] + v.normal[1] + v.normal[2]);
    }
    
    // Update visuals with playing deck
    controller.updateVisuals(deckA, deckB, mixer);
    const auto& verticesAfter = controller.getVertices();
    
    // Assert: Some vertices should have changed (representing button glow state change)
    bool verticesChanged = false;
    for (size_t i = 0; i < verticesBefore.size() && !verticesChanged; ++i) {
        float sumAfter = verticesAfter[i].normal[0] + verticesAfter[i].normal[1] + verticesAfter[i].normal[2];
        if (normalsSumBefore[i] != sumAfter) {
            verticesChanged = true;
        }
    }
    
    // At least some geometry should reflect the playing state
    assert(verticesChanged && "Play button state should be reflected in geometry");
    
    std::cout << "✓ test_DJControllerGeometry_PlayButtonGlow passed\n";
}

// Test 3: test_DJControllerGeometry_EQKnobRotation
// Verify EQ knob vertices rotate based on gain value
void test_DJControllerGeometry_EQKnobRotation() {
    std::cout << "[TEST] test_DJControllerGeometry_EQKnobRotation...\n";
    
    // Arrange
    dj::Deck deckA, deckB;
    dj::AudioClip clipA = dj::AudioClip::generateTestTone(440.0f, 1.0f, 44100);
    dj::AudioClip clipB = dj::AudioClip::generateTestTone(880.0f, 1.0f, 44100);
    
    bool loadedA = deckA.loadClip(clipA);
    bool loadedB = deckB.loadClip(clipB);
    assert(loadedA && loadedB);
    
    dj::Mixer mixer;
    dj::DJControllerGeometry controller;
    controller.generateMesh();
    
    // Initial state: default EQ (all gains = 1.0)
    deckA.setEQ(1.0f, 1.0f, 1.0f);
    controller.updateVisuals(deckA, deckB, mixer);
    
    const auto& verticesBefore = controller.getVertices();
    std::vector<float> positionsSumBefore;
    for (const auto& v : verticesBefore) {
        positionsSumBefore.push_back(v.position[0] + v.position[1] + v.position[2]);
    }
    
    // Act: Set EQ low gain to 1.5 (half turn from default 1.0)
    deckA.setEQ(1.5f, 1.0f, 1.0f);
    
    // Update visuals and check if knob rotated
    controller.updateVisuals(deckA, deckB, mixer);
    const auto& verticesAfter = controller.getVertices();
    
    // Assert: EQ knob vertices should have rotated (positions changed)
    bool verticesChanged = false;
    for (size_t i = 0; i < verticesBefore.size() && !verticesChanged; ++i) {
        float sumAfter = verticesAfter[i].position[0] + verticesAfter[i].position[1] + verticesAfter[i].position[2];
        if (std::abs(positionsSumBefore[i] - sumAfter) > 0.001f) {
            verticesChanged = true;
        }
    }
    
    assert(verticesChanged && "EQ knob rotation should change vertex positions");
    
    std::cout << "✓ test_DJControllerGeometry_EQKnobRotation passed\n";
}

// Test 4: test_DJControllerGeometry_PitchFaderSync
// Verify pitch fader geometry Y position changes with tempo
void test_DJControllerGeometry_PitchFaderSync() {
    std::cout << "[TEST] test_DJControllerGeometry_PitchFaderSync...\n";
    
    // Arrange
    dj::Deck deckA, deckB;
    dj::AudioClip clipA = dj::AudioClip::generateTestTone(440.0f, 1.0f, 44100);
    dj::AudioClip clipB = dj::AudioClip::generateTestTone(880.0f, 1.0f, 44100);
    
    bool loadedA = deckA.loadClip(clipA);
    bool loadedB = deckB.loadClip(clipB);
    assert(loadedA && loadedB);
    
    dj::Mixer mixer;
    dj::DJControllerGeometry controller;
    controller.generateMesh();
    
    // Initial state: tempo at 0%
    deckA.setTempoPercent(0.0f);
    controller.updateVisuals(deckA, deckB, mixer);
    
    const auto& verticesBefore = controller.getVertices();
    float minY_before = FLT_MAX, maxY_before = -FLT_MAX;
    for (const auto& v : verticesBefore) {
        // Pitch fader is around X = -0.5, Z = 0.3
        if (v.position[0] > -0.55f && v.position[0] < -0.45f &&
            v.position[2] > 0.25f && v.position[2] < 0.35f) {
            minY_before = std::min(minY_before, v.position[1]);
            maxY_before = std::max(maxY_before, v.position[1]);
        }
    }
    
    // Act: Set tempo to +10.0% (halfway to max +20%)
    deckA.setTempoPercent(10.0f);
    controller.updateVisuals(deckA, deckB, mixer);
    
    const auto& verticesAfter = controller.getVertices();
    float minY_after = FLT_MAX, maxY_after = -FLT_MAX;
    for (const auto& v : verticesAfter) {
        if (v.position[0] > -0.55f && v.position[0] < -0.45f &&
            v.position[2] > 0.25f && v.position[2] < 0.35f) {
            minY_after = std::min(minY_after, v.position[1]);
            maxY_after = std::max(maxY_after, v.position[1]);
        }
    }
    
    // Assert: Pitch fader Y position should have increased
    assert(minY_after > minY_before && "Pitch fader should move upward with positive tempo");
    assert(maxY_after > maxY_before && "Pitch fader range should shift upward");
    
    std::cout << "✓ test_DJControllerGeometry_PitchFaderSync passed\n";
}

}  // namespace

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    std::cout << "\n=== Running DJ Controller Geometry Phase 5 Tests ===\n\n";
    
    try {
        test_DJControllerGeometry_CrossfaderPositionSync();
        test_DJControllerGeometry_PlayButtonGlow();
        test_DJControllerGeometry_EQKnobRotation();
        test_DJControllerGeometry_PitchFaderSync();
        
        std::cout << "\n✓✓✓ All Phase 5 tests passed! ✓✓✓\n\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
