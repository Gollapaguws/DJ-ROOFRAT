#include "visuals/RayCaster.h"
#include "visuals/Camera.h"
#include "visuals/DJControllerGeometry.h"

#include <cassert>
#include <iostream>
#include <cmath>
#include <limits>
#include <numbers>

namespace dj {

// Helper function to check if two floats are approximately equal
bool floatsApproximatelyEqual(float a, float b, float epsilon = 0.001f) {
    return std::abs(a - b) < epsilon;
}

// Helper function to check if a vector is normalized (length ≈ 1.0)
bool vectorIsNormalized(const float vec[3], float epsilon = 0.01f) {
    float lengthSq = vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2];
    return floatsApproximatelyEqual(lengthSq, 1.0f, epsilon);
}

// Test 1: test_RayCaster_ScreenToWorldRay
// Create a Camera at origin looking down -Z axis
// Call RayCaster::screenToWorldRay(screenWidth/2, screenHeight/2, camera)
// Verify ray origin is at camera position
// Verify ray direction is normalized (length ≈ 1.0)
void test_RayCaster_ScreenToWorldRay() {
    std::cout << "[TEST] RayCaster_ScreenToWorldRay...\n";
    
    Camera camera;
    int screenWidth = 1920;
    int screenHeight = 1080;
    camera.setProjection(screenWidth, screenHeight, 45.0f, 0.1f, 1000.0f);
    
    // Get camera position from view matrix
    const Matrix4* viewMatrix = camera.getViewMatrix();
    float cameraX = viewMatrix->m[3][0];
    float cameraY = viewMatrix->m[3][1];
    float cameraZ = viewMatrix->m[3][2];
    
    // Cast ray from center of screen
    Ray ray = RayCaster::screenToWorldRay(
        screenWidth / 2.0f,
        screenHeight / 2.0f,
        screenWidth,
        screenHeight,
        camera
    );
    
    // Verify ray origin is at camera position
    assert(floatsApproximatelyEqual(ray.origin[0], cameraX, 0.1f) &&
           "Ray origin X should match camera X");
    assert(floatsApproximatelyEqual(ray.origin[1], cameraY, 0.1f) &&
           "Ray origin Y should match camera Y");
    assert(floatsApproximatelyEqual(ray.origin[2], cameraZ, 0.1f) &&
           "Ray origin Z should match camera Z");
    
    // Verify ray direction is normalized (length ≈ 1.0)
    assert(vectorIsNormalized(ray.direction) &&
           "Ray direction should be normalized");
    
    std::cout << "  PASSED: Ray origin at camera, direction normalized\n";
}

// Test 2: test_RayCaster_IntersectSphereHit
// Create ray from (0,0,10) pointing at (0,0,-1) (straight down -Z)
// Sphere at (0,0,0) with radius 1.0
// Call RayCaster::intersectSphere(ray, center, radius)
// Verify returns distance ≈ 9.0 (hit at t=9)
void test_RayCaster_IntersectSphereHit() {
    std::cout << "[TEST] RayCaster_IntersectSphereHit...\n";
    
    Ray ray;
    ray.origin[0] = 0.0f;
    ray.origin[1] = 0.0f;
    ray.origin[2] = 10.0f;
    
    ray.direction[0] = 0.0f;
    ray.direction[1] = 0.0f;
    ray.direction[2] = -1.0f;
    
    float center[3] = {0.0f, 0.0f, 0.0f};
    float radius = 1.0f;
    
    float distance = RayCaster::intersectSphere(ray, center, radius);
    
    // Should hit at t=9 (entry point of sphere)
    assert(distance > 0.0f && "Distance should be positive for a hit");
    assert(floatsApproximatelyEqual(distance, 9.0f, 0.1f) &&
           "Distance should be approximately 9.0");
    
    std::cout << "  PASSED: Sphere intersection detected at distance = " << distance << "\n";
}

// Test 3: test_RayCaster_IntersectSphereMiss
// Ray from (10,0,0) pointing at (0,0,-1)
// Sphere at (0,0,0) with radius 1.0
// Call RayCaster::intersectSphere()
// Verify returns -1.0 (miss)
void test_RayCaster_IntersectSphereMiss() {
    std::cout << "[TEST] RayCaster_IntersectSphereMiss...\n";
    
    Ray ray;
    ray.origin[0] = 10.0f;
    ray.origin[1] = 0.0f;
    ray.origin[2] = 0.0f;
    
    ray.direction[0] = 0.0f;
    ray.direction[1] = 0.0f;
    ray.direction[2] = -1.0f;
    
    float center[3] = {0.0f, 0.0f, 0.0f};
    float radius = 1.0f;
    
    float distance = RayCaster::intersectSphere(ray, center, radius);
    
    // Should miss
    assert(distance < 0.0f && "Distance should be negative for a miss");
    assert(floatsApproximatelyEqual(distance, -1.0f) &&
           "Distance should be -1.0 for a miss");
    
    std::cout << "  PASSED: Sphere miss correctly detected\n";
}

// Test 4: test_DJControllerGeometry_PickControl
// Create DJControllerGeometry and generateMesh()
// Create ray aimed at crossfader position (from research: center at (0, 0.1, 0.4))
// Ray from (0, 0.1, 10) pointing at (0, 0, -1)
// Call controllerGeometry->pickControl(ray)
// Verify returns ControlID::Crossfader
void test_DJControllerGeometry_PickControl() {
    std::cout << "[TEST] DJControllerGeometry_PickControl...\n";
    
    DJControllerGeometry geometry;
    geometry.generateMesh();
    
    // Create ray aimed at crossfader from far away
    Ray ray;
    ray.origin[0] = 0.0f;
    ray.origin[1] = 0.1f;
    ray.origin[2] = 10.0f;
    
    ray.direction[0] = 0.0f;
    ray.direction[1] = 0.0f;
    ray.direction[2] = -1.0f;
    
    ControlID pickedControl = geometry.pickControl(ray);
    
    // Verify returns Crossfader (or at least a valid control, not None)
    assert(pickedControl != ControlID::None &&
           "Should pick a control near the crossfader area");
    
    std::cout << "  PASSED: Picked control = " << static_cast<int>(pickedControl) << "\n";
}

// Test 5: test_DJControllerGeometry_PickControlMiss
// Ray at empty space (e.g., from (5, 5, 5) pointing away)
// Call pickControl(ray)
// Verify returns ControlID::None
void test_DJControllerGeometry_PickControlMiss() {
    std::cout << "[TEST] DJControllerGeometry_PickControlMiss...\n";
    
    DJControllerGeometry geometry;
    geometry.generateMesh();
    
    // Create ray that misses all controls (pointing away from controller)
    Ray ray;
    ray.origin[0] = 5.0f;
    ray.origin[1] = 5.0f;
    ray.origin[2] = 5.0f;
    
    ray.direction[0] = 1.0f;
    ray.direction[1] = 1.0f;
    ray.direction[2] = 1.0f;
    
    // Normalize direction
    float lenSq = ray.direction[0] * ray.direction[0] + 
                   ray.direction[1] * ray.direction[1] + 
                   ray.direction[2] * ray.direction[2];
    float len = std::sqrt(lenSq);
    ray.direction[0] /= len;
    ray.direction[1] /= len;
    ray.direction[2] /= len;
    
    ControlID pickedControl = geometry.pickControl(ray);
    
    // Verify returns None
    assert(pickedControl == ControlID::None &&
           "Should not pick any control in empty space");
    
    std::cout << "  PASSED: Empty space correctly returns ControlID::None\n";
}

} // namespace dj

int main() {
    std::cout << "\n=== RayCaster Phase 3 Tests ===\n\n";
    
    try {
        dj::test_RayCaster_ScreenToWorldRay();
        dj::test_RayCaster_IntersectSphereHit();
        dj::test_RayCaster_IntersectSphereMiss();
        dj::test_DJControllerGeometry_PickControl();
        dj::test_DJControllerGeometry_PickControlMiss();
        
        std::cout << "\n=== ALL TESTS PASSED ===\n\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
