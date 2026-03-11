#pragma once

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#include <wrl.h>
#endif

#include <string>
#include <array>

namespace dj {

// 4x4 matrix for transformations
struct Matrix4 {
    float m[4][4]{};
    
    Matrix4() = default;
    
    Matrix4(float value) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                m[i][j] = (i == j) ? value : 0.0f;
            }
        }
    }

    // Operator for array access
    float* operator[](int index) { return m[index]; }
    const float* operator[](int index) const { return m[index]; }
};

class Camera {
public:
    Camera();
    ~Camera() = default;

    // Set perspective projection
    void setProjection(int width, int height, float fovDegrees, float nearPlane, float farPlane);

    // Update camera with beat-reactive zoom
    void updateBeatReactive(float bpm, float deltaTime);

    // Get transformation matrices
    const Matrix4& getProjectionMatrix() const { return projectionMatrix_; }
    const Matrix4* getViewMatrix() const { return &viewMatrix_; }
    const Matrix4* getWorldMatrix() const { return &worldMatrix_; }

    // Get current zoom level
    float getZoom() const { return currentZoom_; }

    // Set base zoom level
    void setBaseZoom(float zoom) { baseZoom_ = zoom; }

private:
    Matrix4 projectionMatrix_;
    Matrix4 viewMatrix_;
    Matrix4 worldMatrix_;

    float baseZoom_ = 1.0f;
    float currentZoom_ = 1.0f;
    float zoomTarget_ = 1.0f;
    float beatTimer_ = 0.0f;
    float lastBeatTime_ = -1.0f;

    void updateProjectionMatrix();
    void updateViewMatrix();

    int width_ = 1920;
    int height_ = 1080;
    float fovDegrees_ = 45.0f;
    float nearPlane_ = 0.1f;
    float farPlane_ = 1000.0f;
};

} // namespace dj
