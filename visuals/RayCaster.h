#pragma once

#include <array>

namespace dj {

struct Ray {
    float origin[3];
    float direction[3];  // Must be normalized
};

struct Matrix4;  // Forward declaration
class Camera;    // Forward declaration

class RayCaster {
public:
    // Convert screen coordinates to world-space ray
    // screenX, screenY: normalized to screen dimensions
    // screenWidth, screenHeight: viewport dimensions
    // Returns Ray with origin at camera position and normalized direction
    static Ray screenToWorldRay(float screenX, float screenY, int screenWidth, int screenHeight, const Camera& camera);
    
    // Ray-sphere intersection test (geometric approach)
    // Returns distance along ray to intersection, or -1.0 if no hit
    // Uses algorithm from: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html
    static float intersectSphere(const Ray& ray, const float center[3], float radius);
    
    // Ray-box intersection test (AABB using slab method)
    // Returns distance along ray to intersection, or -1.0 if no hit
    static float intersectBox(const Ray& ray, const float boxMin[3], const float boxMax[3]);
    
private:
    // Helper: Normalize 3D vector in-place
    static void normalize(float vec[3]);
    
    // Helper: Dot product of two 3D vectors
    static float dot(const float a[3], const float b[3]);
    
    // Helper: Multiply 4x4 matrix by 4D vector
    static void multiplyMatrixVector(const float mat[4][4], const float vec[4], float result[4]);
    
    // Helper: Invert a 4x4 matrix
    static bool invertMatrix4x4(const float mat[4][4], float invMat[4][4]);
};

} // namespace dj
