#include "visuals/CrowdAnimator.h"

#include <cmath>
#include <numbers>

namespace dj {

CrowdAnimator::CrowdAnimator()
    : animationType_(AnimationType::Idle), animationState_(0.0f), animationTime_(0.0f) {}

void CrowdAnimator::update(float bpm, float deltaTime) {
    // Convert BPM to beats per second
    float beatsPerSecond = bpm / 60.0f;
    
    // Animation speed: 2 beats per complete animation cycle
    float animSpeed = beatsPerSecond * 2.0f;
    
    // Update animation time
    animationTime_ += deltaTime * animSpeed;
    
    // Wrap animation state to [0, 1]
    if (animationTime_ > 1.0f) {
        animationTime_ -= 1.0f;
    }
    
    animationState_ = animationTime_;
}

InstanceData CrowdAnimator::getInstanceData(int crowdMemberId, const float position[3], 
                                            const float color[3]) const {
    InstanceData data = {};
    
    data.position[0] = position[0];
    data.position[1] = position[1];
    data.position[2] = position[2];
    data.padding1 = 0.0f;
    
    data.color[0] = color[0];
    data.color[1] = color[1];
    data.color[2] = color[2];
    data.animationType = static_cast<int>(animationType_);
    
    data.animationState = animationState_;
    data.lodLevel = 0.0f;
    data.padding2[0] = 0.0f;
    data.padding2[1] = 0.0f;
    
    return data;
}

} // namespace dj
