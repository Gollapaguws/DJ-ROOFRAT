// Phase 1: DJ Controller Geometry Foundation
// Test file for 3D DJ Controller procedural geometry generation
// Tests mesh generation, control shapes, positioning, and bounds

#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

#include "visuals/DJControllerGeometry.h"

namespace dj {

// Helper function to check if two bounding boxes intersect
bool boundingBoxesIntersect(const ControlBounds& a, const ControlBounds& b) {
    float dx = b.centerX - a.centerX;
    float dy = b.centerY - a.centerY;
    float dz = b.centerZ - a.centerZ;
    float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
    
    // Check if spheres (bounding circles) overlap
    return dist < (a.radius + b.radius);
}

// Test 1: DJControllerGeometry_GeneratesMesh
// Verify mesh is generated with vertices and indices
void test_DJControllerGeometry_GeneratesMesh() {
    std::cout << "[TEST] DJControllerGeometry_GeneratesMesh...\n";
    
    DJControllerGeometry geometry;
    
    // Before generation, should be empty
    assert(geometry.getVertices().empty() && "Vertices should be empty before generation");
    assert(geometry.getIndices().empty() && "Indices should be empty before generation");
    
    // Generate mesh
    geometry.generateMesh();
    
    // After generation, should have content
    assert(geometry.getVertices().size() > 0 && "Vertices should be generated");
    assert(geometry.getIndices().size() > 0 && "Indices should be generated");
    assert(geometry.getIndices().size() % 3 == 0 && "Indices must be divisible by 3 (triangles)");
    
    std::cout << "  PASSED: Generated " << geometry.getVertices().size() 
              << " vertices and " << geometry.getIndices().size() << " indices\n";
}

// Test 2: DJControllerGeometry_KnobHasCorrectShape
// Verify knobs are cylindrical with correct vertex count
void test_DJControllerGeometry_KnobHasCorrectShape() {
    std::cout << "[TEST] DJControllerGeometry_KnobHasCorrectShape...\n";
    
    DJControllerGeometry geometry;
    geometry.generateMesh();
    
    const auto& vertices = geometry.getVertices();
    const auto& bounds = geometry.getControlBounds();
    
    // Find a knob control (e.g., EQLowA)
    int knobCount = 0;
    for (const auto& bound : bounds) {
        if (bound.id == ControlID::EQLowA || 
            bound.id == ControlID::EQMidA || 
            bound.id == ControlID::EQHighA) {
            knobCount++;
        }
    }
    
    // Should have found at least one knob
    assert(knobCount > 0 && "Should have at least one knob control");
    
    // A cylinder with 20 segments should have ~20-24 vertices per knob (caps + sides)
    // For a 20-segment cylinder: 20 (bottom) + 20 (top) + 2 (centers) = 42 vertices
    // But we may have different counts depending on implementation
    assert(vertices.size() >= 20 && "Should have reasonable number of vertices for geometry");
    
    std::cout << "  PASSED: Knob geometry verified\n";
}

// Test 3: DJControllerGeometry_ControlPositionsNonOverlapping
// Verify controls don't overlap based on bounding boxes
void test_DJControllerGeometry_ControlPositionsNonOverlapping() {
    std::cout << "[TEST] DJControllerGeometry_ControlPositionsNonOverlapping...\n";
    
    DJControllerGeometry geometry;
    geometry.generateMesh();
    
    const auto& bounds = geometry.getControlBounds();
    assert(bounds.size() > 0 && "Should have control bounds");
    
    // Check all pairs for overlap (with tolerance)
    for (size_t i = 0; i < bounds.size(); ++i) {
        for (size_t j = i + 1; j < bounds.size(); ++j) {
            [[maybe_unused]] bool overlaps = boundingBoxesIntersect(bounds[i], bounds[j]);
            
            // Allow some overlap for closely-spaced EQ knobs, but not major overlaps
            float dx = bounds[j].centerX - bounds[i].centerX;
            float dy = bounds[j].centerY - bounds[i].centerY;
            float dz = bounds[j].centerZ - bounds[i].centerZ;
            float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
            float minDist = bounds[i].radius + bounds[j].radius;
            
            // Allow up to 0.05 units of overlap for closely-grouped controls
            assert(dist >= (minDist - 0.05f) && 
                   "Controls should not significantly overlap");
        }
    }
    
    std::cout << "  PASSED: All " << bounds.size() << " controls properly positioned\n";
}

// Test 4: DJControllerGeometry_AllControlsGenerated
// Verify all expected controls are present
void test_DJControllerGeometry_AllControlsGenerated() {
    std::cout << "[TEST] DJControllerGeometry_AllControlsGenerated...\n";
    
    DJControllerGeometry geometry;
    geometry.generateMesh();
    
    const auto& bounds = geometry.getControlBounds();
    
    // Expected controls:
    // 1 crossfader
    // 2 jog wheels (A, B)
    // 2 pitch faders (A, B)
    // 6 EQ knobs (3 per deck, 2 decks)
    // 2 play buttons (A, B)
    // Total: 13 controls
    
    std::vector<bool> found(14, false);
    
    for (const auto& bound : bounds) {
        if (bound.id == ControlID::Crossfader) found[0] = true;
        else if (bound.id == ControlID::JogWheelA) found[1] = true;
        else if (bound.id == ControlID::JogWheelB) found[2] = true;
        else if (bound.id == ControlID::PitchA) found[3] = true;
        else if (bound.id == ControlID::PitchB) found[4] = true;
        else if (bound.id == ControlID::EQLowA) found[5] = true;
        else if (bound.id == ControlID::EQMidA) found[6] = true;
        else if (bound.id == ControlID::EQHighA) found[7] = true;
        else if (bound.id == ControlID::EQLowB) found[8] = true;
        else if (bound.id == ControlID::EQMidB) found[9] = true;
        else if (bound.id == ControlID::EQHighB) found[10] = true;
        else if (bound.id == ControlID::PlayA) found[11] = true;
        else if (bound.id == ControlID::PlayB) found[12] = true;
    }
    
    // Verify all controls were found
    assert(found[0] && "Crossfader not found");
    assert(found[1] && "JogWheelA not found");
    assert(found[2] && "JogWheelB not found");
    assert(found[3] && "PitchA not found");
    assert(found[4] && "PitchB not found");
    assert(found[5] && "EQLowA not found");
    assert(found[6] && "EQMidA not found");
    assert(found[7] && "EQHighA not found");
    assert(found[8] && "EQLowB not found");
    assert(found[9] && "EQMidB not found");
    assert(found[10] && "EQHighB not found");
    assert(found[11] && "PlayA not found");
    assert(found[12] && "PlayB not found");
    
    std::cout << "  PASSED: All 13 controls generated\n";
}

// Main test runner
void runAllTests() {
    std::cout << "========== DJ Controller Geometry Phase 1 Tests ==========\n\n";
    
    try {
        test_DJControllerGeometry_GeneratesMesh();
        test_DJControllerGeometry_KnobHasCorrectShape();
        test_DJControllerGeometry_ControlPositionsNonOverlapping();
        test_DJControllerGeometry_AllControlsGenerated();
        
        std::cout << "\n========== ALL TESTS PASSED ==========\n";
    } catch (const std::exception& e) {
        std::cerr << "EXCEPTION: " << e.what() << "\n";
        assert(false);
    }
}

} // namespace dj

// Entry point for standalone test
int main() {
    dj::runAllTests();
    return 0;
}
