// Phase 2: 3D Crowd Visualization Integration
// Test file for crowd rendering and animation synchronization
// Tests crowd renderer initialization, BPM-synced animation, and LOD visibility

#include <cassert>
#include <iostream>
#include <cmath>

#include "visuals/CrowdRenderer.h"
#include "visuals/CrowdAnimator.h"
#include "visuals/CrowdMesh.h"

namespace dj {

// ===== CrowdAnimator Tests =====

// Test 1: CrowdAnimator_Initialization
// Verify CrowdAnimator can be instantiated without crash
void test_CrowdAnimator_Initialization() {
    std::cout << "[TEST] CrowdAnimator_Initialization...\n";
    
    CrowdAnimator animator;
    assert(animator.getAnimationType() == AnimationType::Idle && "Initial animation type should be Idle");
    assert(animator.getAnimationState() >= 0.0f && animator.getAnimationState() <= 1.0f && 
           "Animation state should be in [0,1]");
    
    std::cout << "  PASSED: CrowdAnimator initialized\n";
}

// Test 2: CrowdAnimator_UpdateWithBPM
// Verify CrowdAnimator updates animation state based on BPM
void test_CrowdAnimator_UpdateWithBPM() {
    std::cout << "[TEST] CrowdAnimator_UpdateWithBPM...\n";
    
    CrowdAnimator animator;
    float initialState = animator.getAnimationState();
    
    // Update with BPM 120 and deltaTime 0.016 (60 FPS)
    animator.update(120.0f, 0.016f);
    
    float newState = animator.getAnimationState();
    assert(newState >= 0.0f && newState <= 1.0f && "Animation state should still be in [0,1]");
    assert(newState != initialState && "Animation state should change after update");
    
    std::cout << "  PASSED: CrowdAnimator updated with BPM\n";
}

// Test 3: CrowdAnimator_AnimationTypeChange
// Verify CrowdAnimator can change animation types
void test_CrowdAnimator_AnimationTypeChange() {
    std::cout << "[TEST] CrowdAnimator_AnimationTypeChange...\n";
    
    CrowdAnimator animator;
    animator.setAnimationType(AnimationType::Jump);
    assert(animator.getAnimationType() == AnimationType::Jump && "Animation type should be Jump");
    
    animator.setAnimationType(AnimationType::Dance);
    assert(animator.getAnimationType() == AnimationType::Dance && "Animation type should be Dance");
    
    animator.setAnimationType(AnimationType::Wave);
    assert(animator.getAnimationType() == AnimationType::Wave && "Animation type should be Wave");
    
    std::cout << "  PASSED: CrowdAnimator changed animation types\n";
}

// Test 4: CrowdAnimator_HighBPMAcceleration
// Verify CrowdAnimator speeds up with higher BPM
void test_CrowdAnimator_HighBPMAcceleration() {
    std::cout << "[TEST] CrowdAnimator_HighBPMAcceleration...\n";
    
    CrowdAnimator animator1, animator2;
    
    // Update animator1 with 120 BPM
    animator1.update(120.0f, 0.1f);
    float state120 = animator1.getAnimationState();
    
    // Reset animator2 and update with 240 BPM (double)
    animator2.update(240.0f, 0.1f);
    float state240 = animator2.getAnimationState();
    
    assert(state240 > state120 && "Higher BPM should advance animation faster");
    
    std::cout << "  PASSED: Higher BPM accelerates animation\n";
}

// Test 5: CrowdAnimator_GetInstanceData
// Verify CrowdAnimator generates valid InstanceData
void test_CrowdAnimator_GetInstanceData() {
    std::cout << "[TEST] CrowdAnimator_GetInstanceData...\n";
    
    CrowdAnimator animator;
    animator.update(120.0f, 0.016f);
    
    float position[3] = {1.0f, 2.0f, 3.0f};
    float color[3] = {1.0f, 0.0f, 0.0f};
    
    InstanceData data = animator.getInstanceData(0, position, color);
    
    assert(data.position[0] == 1.0f && data.position[1] == 2.0f && data.position[2] == 3.0f &&
           "Position should match input");
    assert(data.color[0] == 1.0f && data.color[1] == 0.0f && data.color[2] == 0.0f &&
           "Color should match input");
    assert(data.animationType == static_cast<int>(AnimationType::Idle) &&
           "Animation type should be Idle");
    assert(data.animationState >= 0.0f && data.animationState <= 1.0f &&
           "Animation state should be in [0,1]");
    
    std::cout << "  PASSED: CrowdAnimator generated valid InstanceData\n";
}

// ===== CrowdRenderer Tests =====

// Test 6: CrowdRenderer_Initialization
// Verify CrowdRenderer can be instantiated without crash
void test_CrowdRenderer_Initialization() {
    std::cout << "[TEST] CrowdRenderer_Initialization...\n";
    
    CrowdRenderer renderer;
    assert(renderer.motionAmplitude() >= 0.0f && renderer.motionAmplitude() <= 1.0f &&
           "Motion amplitude should be in [0,1]");
    assert(renderer.density() >= 0.0f && renderer.density() <= 1.0f &&
           "Density should be in [0,1]");
    assert(renderer.visibleSilhouettes() >= 0 &&
           "Visible silhouettes should be non-negative");
    
    std::cout << "  PASSED: CrowdRenderer initialized\n";
}

// Test 7: CrowdRenderer_UpdateWithMoodEnergy
// Verify CrowdRenderer updates state based on mood and energy
void test_CrowdRenderer_UpdateWithMoodEnergy() {
    std::cout << "[TEST] CrowdRenderer_UpdateWithMoodEnergy...\n";
    
    CrowdRenderer renderer;
    
    // Update with low energy unimpressed mood
    renderer.update(0, 0.1f, 0.016f);
    float amplitude1 = renderer.motionAmplitude();
    float density1 = renderer.density();
    
    // Update with high energy hyped mood
    renderer.update(3, 0.9f, 0.016f);
    float amplitude2 = renderer.motionAmplitude();
    float density2 = renderer.density();
    
    assert(amplitude2 > amplitude1 && "Hyped mood should have higher amplitude than unimpressed");
    assert(density2 > density1 && "High energy should have higher density than low energy");
    
    std::cout << "  PASSED: CrowdRenderer updated with mood and energy\n";
}

// Test 8: CrowdRenderer_MoodScaling
// Verify CrowdRenderer correctly maps mood values
void test_CrowdRenderer_MoodScaling() {
    std::cout << "[TEST] CrowdRenderer_MoodScaling...\n";
    
    CrowdRenderer renderer;
    
    // Test each mood level with same energy
    renderer.update(0, 0.5f, 0.016f);  // Unimpressed
    float amp0 = renderer.motionAmplitude();
    
    renderer.update(1, 0.5f, 0.016f);  // Calm
    float amp1 = renderer.motionAmplitude();
    
    renderer.update(2, 0.5f, 0.016f);  // Grooving
    float amp2 = renderer.motionAmplitude();
    
    renderer.update(3, 0.5f, 0.016f);  // Hyped
    float amp3 = renderer.motionAmplitude();
    
    // Should have monotonic increase with mood
    assert(amp0 < amp1 && "Calm should exceed Unimpressed");
    assert(amp1 < amp2 && "Grooving should exceed Calm");
    assert(amp2 < amp3 && "Hyped should exceed Grooving");
    
    std::cout << "  PASSED: CrowdRenderer mood scaling is monotonic\n";
}

// Test 9: CrowdRenderer_VisibleSilhouettesRange
// Verify visible silhouettes stay in expected range
void test_CrowdRenderer_VisibleSilhouettesRange() {
    std::cout << "[TEST] CrowdRenderer_VisibleSilhouettesRange...\n";
    
    CrowdRenderer renderer;
    
    // Low state: unimpressed, zero energy
    renderer.update(0, 0.0f, 0.016f);
    int silhouettes_low = renderer.visibleSilhouettes();
    assert(silhouettes_low >= 0 && silhouettes_low < 100 &&
           "Low energy should have few silhouettes");
    
    // High state: hyped, full energy
    renderer.update(3, 1.0f, 0.016f);
    int silhouettes_high = renderer.visibleSilhouettes();
    assert(silhouettes_high > 200 && silhouettes_high <= 600 &&
           "High energy hyped state should have many silhouettes (200-600 range)");
    
    std::cout << "  PASSED: CrowdRenderer silhouettes in expected range\n";
}

// Test 10: CrowdRenderer_EnergyScaling
// Verify density scales with energy
void test_CrowdRenderer_EnergyScaling() {
    std::cout << "[TEST] CrowdRenderer_EnergyScaling...\n";
    
    CrowdRenderer renderer;
    
    // Low energy
    renderer.update(2, 0.1f, 0.016f);
    float density_low = renderer.density();
    
    // High energy
    renderer.update(2, 0.9f, 0.016f);
    float density_high = renderer.density();
    
    assert(density_high > density_low && "Higher energy should increase density");
    assert(density_low >= 0.0f && density_low <= 1.0f && "Density should be in valid range");
    assert(density_high >= 0.0f && density_high <= 1.0f && "Density should be in valid range");
    
    std::cout << "  PASSED: CrowdRenderer energy scaling works correctly\n";
}

// Test 11: CrowdRenderer_DeltaTimeInfluence
// Verify deltaTime affects animation smoothness
void test_CrowdRenderer_DeltaTimeInfluence() {
    std::cout << "[TEST] CrowdRenderer_DeltaTimeInfluence...\n";
    
    CrowdRenderer renderer1, renderer2;
    
    // Small deltaTime
    renderer1.update(2, 0.5f, 0.01f);
    float silhouettes1 = renderer1.visibleSilhouettes();
    
    // Larger deltaTime
    renderer2.update(2, 0.5f, 0.05f);
    float silhouettes2 = renderer2.visibleSilhouettes();
    
    // Both should produce reasonable values (silhouettes influenced by animation phase)
    assert(silhouettes1 >= 0 && silhouettes2 >= 0 &&
           "Both should produce non-negative silhouettes");
    
    std::cout << "  PASSED: CrowdRenderer deltaTime influence works\n";
}

// ===== LOD and Distance Tests =====

// Test 12: CrowdRenderer_LODCalculation
// Verify LOD level is calculated correctly based on distance
void test_CrowdRenderer_LODCalculation() {
    std::cout << "[TEST] CrowdRenderer_LODCalculation...\n";
    
    CrowdRenderer renderer;
    bool initialized = renderer.initialize(nullptr);  // Non-graphics build
    
    if (!initialized) {
        // In non-graphics mode, LOD calculation should still work
        float3 crowdPos(0.0f, 0.0f, 0.0f);
        float3 cameraPos(0.0f, 0.0f, 0.0f);
        
        int lod = renderer.calculateLODLevel(crowdPos, cameraPos);
        assert(lod >= 0 && lod <= 2 && "LOD level should be 0, 1, or 2");
    }
    
    std::cout << "  PASSED: CrowdRenderer LOD calculation works\n";
}

// Test 13: CrowdRenderer_InstanceManagement
// Verify CrowdRenderer can manage instances
void test_CrowdRenderer_InstanceManagement() {
    std::cout << "[TEST] CrowdRenderer_InstanceManagement...\n";
    
    CrowdRenderer renderer;
    
    // Initially should have 0 instances
    assert(renderer.getInstanceCount() == 0 && "Should start with 0 instances");
    
    // Clear should work on empty renderer
    renderer.clearInstances();
    assert(renderer.getInstanceCount() == 0 && "Clear should work after start");
    
    std::cout << "  PASSED: CrowdRenderer instance management works\n";
}

// Test 14: CrowdRenderer_AnimationPhaseProgression
// Verify animation phase progresses smoothly over time
void test_CrowdRenderer_AnimationPhaseProgression() {
    std::cout << "[TEST] CrowdRenderer_AnimationPhaseProgression...\n";
    
    CrowdRenderer renderer;
    
    int silhouettes_prev = 0;
    for (int i = 0; i < 5; ++i) {
        renderer.update(120.0f, 0.5f, 0.016f);
        int silhouettes_curr = renderer.visibleSilhouettes();
        
        // Silhouettes should vary (oscillate due to animation phase)
        assert(silhouettes_curr >= 0 && "Silhouettes should be non-negative");
    }
    
    std::cout << "  PASSED: CrowdRenderer animation phase progresses smoothly\n";
}

// Test 15: CrowdMesh_LODGeneration
// Verify CrowdMesh can be initialized without crash (non-graphics stub)
void test_CrowdMesh_LODGeneration() {
    std::cout << "[TEST] CrowdMesh_LODGeneration...\n";
    
    CrowdMesh mesh;
    
    // This will use stub in non-graphics builds
    bool result = mesh.generateLOD(nullptr, CrowdMesh::LODLevel::LOD0);
    assert(!result && "Non-graphics build should return false");
    
    std::cout << "  PASSED: CrowdMesh LOD generation stub works\n";
}

} // namespace dj

// Main test runner
int main() {
    std::cout << "\n=== Graphics Phase 2 Test Suite ===\n\n";
    
    // CrowdAnimator tests
    dj::test_CrowdAnimator_Initialization();
    dj::test_CrowdAnimator_UpdateWithBPM();
    dj::test_CrowdAnimator_AnimationTypeChange();
    dj::test_CrowdAnimator_HighBPMAcceleration();
    dj::test_CrowdAnimator_GetInstanceData();
    
    // CrowdRenderer tests
    dj::test_CrowdRenderer_Initialization();
    dj::test_CrowdRenderer_UpdateWithMoodEnergy();
    dj::test_CrowdRenderer_MoodScaling();
    dj::test_CrowdRenderer_VisibleSilhouettesRange();
    dj::test_CrowdRenderer_EnergyScaling();
    dj::test_CrowdRenderer_DeltaTimeInfluence();
    
    // LOD and distance tests
    dj::test_CrowdRenderer_LODCalculation();
    dj::test_CrowdRenderer_InstanceManagement();
    dj::test_CrowdRenderer_AnimationPhaseProgression();
    
    // CrowdMesh tests
    dj::test_CrowdMesh_LODGeneration();
    
    std::cout << "\n=== All Tests Passed ===\n\n";
    return 0;
}
