#include "visuals/Camera.h"

#include <cmath>
#include <numbers>
#include <algorithm>

namespace dj {

Camera::Camera() = default;

void Camera::setProjection(int width, int height, float fovDegrees, float nearPlane, float farPlane) {
    width_ = width;
    height_ = height;
    fovDegrees_ = fovDegrees;
    nearPlane_ = nearPlane;
    farPlane_ = farPlane;

    currentZoom_ = baseZoom_;
    zoomTarget_ = baseZoom_;

    updateProjectionMatrix();
    updateViewMatrix();
}

void Camera::updateBeatReactive(float bpm, float deltaTime) {
    // Calculate beat interval
    if (bpm > 0.0f) {
        float beatInterval = 60.0f / bpm;
        beatTimer_ += deltaTime;

        // Trigger zoom on beat
        if (beatTimer_ >= beatInterval && lastBeatTime_ < 0.0f) {
            beatTimer_ -= beatInterval;
            lastBeatTime_ = 0.0f;
            zoomTarget_ = baseZoom_ * 0.85f; // Pull back on beat
        }

        if (lastBeatTime_ >= 0.0f) {
            lastBeatTime_ += deltaTime;
            if (lastBeatTime_ >= beatInterval * 0.25f) {
                lastBeatTime_ = -1.0f;
            }
        }
    }

    // Smooth interpolation back to base zoom
    const float zoomSpeed = 2.0f; // units per second
    float direction = (zoomTarget_ - currentZoom_);
    if (std::abs(direction) > 0.01f) {
        currentZoom_ += direction * zoomSpeed * deltaTime;
    } else {
        currentZoom_ = zoomTarget_;
    }

    // Clamp zoom
    currentZoom_ = std::clamp(currentZoom_, baseZoom_ * 0.5f, baseZoom_ * 2.0f);

    updateProjectionMatrix();
    updateViewMatrix();
}

void Camera::updateProjectionMatrix() {
    // Create perspective projection matrix (right-handed, DirectX convention)
    float aspectRatio = (float)width_ / (float)height_;
    float fovRadians = fovDegrees_ * (std::numbers::pi_v<float> / 180.0f);
    float tanHalfFov = std::tan(fovRadians / 2.0f);

    // Initialize to identity
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            projectionMatrix_.m[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }

    projectionMatrix_.m[0][0] = 1.0f / (aspectRatio * tanHalfFov) * currentZoom_;
    projectionMatrix_.m[1][1] = 1.0f / tanHalfFov * currentZoom_;
    projectionMatrix_.m[2][2] = farPlane_ / (farPlane_ - nearPlane_);
    projectionMatrix_.m[2][3] = 1.0f;
    projectionMatrix_.m[3][2] = -(farPlane_ * nearPlane_) / (farPlane_ - nearPlane_);
    projectionMatrix_.m[3][3] = 0.0f;
}

void Camera::updateViewMatrix() {
    // Create identity view matrix (camera at origin, looking down negative Z)
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            viewMatrix_.m[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }

    // Position camera
    viewMatrix_.m[3][1] = 5.0f; // Move up
    viewMatrix_.m[3][2] = 5.0f; // Move back

    // Create world matrix (identity)
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            worldMatrix_.m[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
}

} // namespace dj
