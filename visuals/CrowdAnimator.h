#pragma once

#include <array>

namespace dj {

// Forward declare for graphics-independent API
struct float3 {
    float x, y, z;
    float3() : x(0), y(0), z(0) {}
    float3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

struct InstanceData {
    float position[3];
    float padding1;
    float color[3];
    int animationType;  // 0=idle, 1=jump, 2=wave, 3=dance
    float animationState;  // [0,1] cycle through animation
    float lodLevel;     // 0/1/2
    float padding2[2];
};

enum class AnimationType {
    Idle = 0,
    Jump = 1,
    Wave = 2,
    Dance = 3
};

class CrowdAnimator {
public:
    CrowdAnimator();
    ~CrowdAnimator() = default;

    // Update animation based on BPM and delta time
    void update(float bpm, float deltaTime);

    // Set animation type
    void setAnimationType(AnimationType type) noexcept { animationType_ = type; }
    
    // Get current animation type
    AnimationType getAnimationType() const noexcept { return animationType_; }

    // Get animation state [0,1]
    float getAnimationState() const noexcept { return animationState_; }

    // Get current animation state as InstanceData for rendering
    InstanceData getInstanceData(int crowdMemberId, const float position[3], const float color[3]) const;

private:
    AnimationType animationType_ = AnimationType::Idle;
    float animationState_ = 0.0f;  // [0,1]
    float animationTime_ = 0.0f;   // Total time accumulated
};

} // namespace dj
