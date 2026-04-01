#include "visuals/RayCaster.h"
#include "visuals/Camera.h"

#include <cmath>
#include <limits>

namespace dj {

Ray RayCaster::screenToWorldRay(float screenX, float screenY, int screenWidth, int screenHeight, const Camera& camera) {
    Ray ray{};
    
    // Get camera matrices
    const Matrix4& projMatrix = camera.getProjectionMatrix();
    const Matrix4* viewMatrix = camera.getViewMatrix();
    
    // Convert screen coordinates to normalized device coordinates (NDC)
    // NDC: x in [-1, 1], y in [-1, 1]
    // Note: screen Y is flipped (top = 0, bottom = screenHeight)
    float ndcX = (2.0f * screenX) / screenWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * screenY) / screenHeight;  // Flipped Y
    
    // Create homogeneous clip-space point
    // We use z = 1.0 (near plane) and w = 1.0 for a point in front of camera
    float clipSpacePoint[4] = {ndcX, ndcY, 1.0f, 1.0f};
    
    // Compute inverse of (projection * view) matrix
    float projViewMatrix[4][4];
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            projViewMatrix[i][j] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                projViewMatrix[i][j] += projMatrix.m[i][k] * viewMatrix->m[k][j];
            }
        }
    }
    
    float invProjView[4][4];
    if (!invertMatrix4x4(projViewMatrix, invProjView)) {
        // If inversion fails, create default ray pointing down -Z
        ray.origin[0] = 0.0f;
        ray.origin[1] = 0.0f;
        ray.origin[2] = 0.0f;
        ray.direction[0] = 0.0f;
        ray.direction[1] = 0.0f;
        ray.direction[2] = -1.0f;
        return ray;
    }
    
    // Transform clip-space point to world space
    float worldPoint[4];
    multiplyMatrixVector(invProjView, clipSpacePoint, worldPoint);
    
    // Homogeneous divide
    if (worldPoint[3] != 0.0f) {
        worldPoint[0] /= worldPoint[3];
        worldPoint[1] /= worldPoint[3];
        worldPoint[2] /= worldPoint[3];
    }
    
    // Ray origin = camera position (from view matrix translation)
    ray.origin[0] = viewMatrix->m[3][0];
    ray.origin[1] = viewMatrix->m[3][1];
    ray.origin[2] = viewMatrix->m[3][2];
    
    // Ray direction = normalize(worldPoint - cameraPos)
    ray.direction[0] = worldPoint[0] - ray.origin[0];
    ray.direction[1] = worldPoint[1] - ray.origin[1];
    ray.direction[2] = worldPoint[2] - ray.origin[2];
    
    normalize(ray.direction);
    
    return ray;
}

float RayCaster::intersectSphere(const Ray& ray, const float center[3], float radius) {
    // Vector from ray origin to sphere center
    float L[3];
    L[0] = center[0] - ray.origin[0];
    L[1] = center[1] - ray.origin[1];
    L[2] = center[2] - ray.origin[2];
    
    // Project L onto ray direction
    float tca = dot(L, ray.direction);
    
    // If tca < 0, sphere is behind ray origin
    if (tca < 0.0f) {
        return -1.0f;
    }
    
    // Distance from sphere center to ray line
    // d² = |L|² - tca²
    float L_lengthSq = dot(L, L);
    float dSq = L_lengthSq - tca * tca;
    float radiusSq = radius * radius;
    
    // If d² > radius², ray misses sphere
    if (dSq > radiusSq) {
        return -1.0f;
    }
    
    // Calculate intersection distance
    // t = tca - sqrt(radius² - d²)
    float discriminant = radiusSq - dSq;
    float thc = std::sqrt(discriminant);
    
    float t0 = tca - thc;  // Entry point
    float t1 = tca + thc;  // Exit point
    
    // Return closest intersection in front of ray
    if (t0 > 0.0f) {
        return t0;
    } else if (t1 > 0.0f) {
        return t1;
    }
    
    return -1.0f;
}

float RayCaster::intersectBox(const Ray& ray, const float boxMin[3], const float boxMax[3]) {
    // Slab method for axis-aligned bounding box intersection
    // https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
    
    float tMin = (std::numeric_limits<float>::lowest)();
    float tMax = (std::numeric_limits<float>::max)();
    
    // Test each slab (X, Y, Z)
    for (int i = 0; i < 3; ++i) {
        float rayOrigin = ray.origin[i];
        float rayDir = ray.direction[i];
        float bMin = boxMin[i];
        float bMax = boxMax[i];
        
        if (std::abs(rayDir) < 1e-6f) {
            // Ray is parallel to slab
            if (rayOrigin < bMin || rayOrigin > bMax) {
                return -1.0f;  // Ray misses box
            }
        } else {
            float t1 = (bMin - rayOrigin) / rayDir;
            float t2 = (bMax - rayOrigin) / rayDir;
            
            if (t1 > t2) {
                std::swap(t1, t2);
            }
            
            tMin = (std::max)(tMin, t1);
            tMax = (std::min)(tMax, t2);
            
            if (tMin > tMax) {
                return -1.0f;  // Ray misses box
            }
        }
    }
    
    // Return closest intersection in front of ray
    if (tMin > 0.0f) {
        return tMin;
    } else if (tMax > 0.0f) {
        return tMax;
    }
    
    return -1.0f;
}

void RayCaster::normalize(float vec[3]) {
    float lengthSq = vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2];
    
    if (lengthSq < 1e-6f) {
        // Very small vector, return zero
        vec[0] = 0.0f;
        vec[1] = 0.0f;
        vec[2] = 0.0f;
        return;
    }
    
    float length = std::sqrt(lengthSq);
    vec[0] /= length;
    vec[1] /= length;
    vec[2] /= length;
}

float RayCaster::dot(const float a[3], const float b[3]) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void RayCaster::multiplyMatrixVector(const float mat[4][4], const float vec[4], float result[4]) {
    for (int i = 0; i < 4; ++i) {
        result[i] = 0.0f;
        for (int j = 0; j < 4; ++j) {
            result[i] += mat[i][j] * vec[j];
        }
    }
}

bool RayCaster::invertMatrix4x4(const float mat[4][4], float invMat[4][4]) {
    // Using Gaussian elimination to compute inverse
    // Create augmented matrix [mat | I]
    float augmented[4][8];
    
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            augmented[i][j] = mat[i][j];
            augmented[i][j + 4] = (i == j) ? 1.0f : 0.0f;
        }
    }
    
    // Forward elimination
    for (int col = 0; col < 4; ++col) {
        // Find pivot
        int pivotRow = col;
        float maxVal = std::abs(augmented[col][col]);
        
        for (int row = col + 1; row < 4; ++row) {
            if (std::abs(augmented[row][col]) > maxVal) {
                maxVal = std::abs(augmented[row][col]);
                pivotRow = row;
            }
        }
        
        // Check for singular matrix
        if (maxVal < 1e-6f) {
            return false;
        }
        
        // Swap rows
        if (pivotRow != col) {
            for (int j = 0; j < 8; ++j) {
                std::swap(augmented[col][j], augmented[pivotRow][j]);
            }
        }
        
        // Scale pivot row
        float scale = augmented[col][col];
        for (int j = 0; j < 8; ++j) {
            augmented[col][j] /= scale;
        }
        
        // Eliminate column in other rows
        for (int row = 0; row < 4; ++row) {
            if (row != col) {
                float factor = augmented[row][col];
                for (int j = 0; j < 8; ++j) {
                    augmented[row][j] -= factor * augmented[col][j];
                }
            }
        }
    }
    
    // Extract inverse from augmented matrix
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            invMat[i][j] = augmented[i][j + 4];
        }
    }
    
    return true;
}

} // namespace dj
