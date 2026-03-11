#include "audio/VinylSimulator.h"

#include <algorithm>
#include <cmath>

namespace dj {

void VinylSimulator::setSampleRate(int sampleRate) {
    sampleRate_ = std::max(8000, sampleRate);
}

void VinylSimulator::setMotorOn(bool on) {
    motorOn_ = on;
    if (on) {
        motorElapsedTime_ = 0.0f;  // Reset timer when motor starts
        motorStartVelocity_ = currentVelocity_;  // Store current velocity for smooth ramp
    }
}

bool VinylSimulator::isMotorOn() const {
    return motorOn_;
}

void VinylSimulator::setTargetVelocity(float velocity) {
    float newTarget = std::clamp(velocity, -2.0f, 2.0f);
    
    // Only reset ramp if target actually changed (prevent ramp reset on repeated calls)
    if (motorOn_ && std::abs(newTarget - targetVelocity_) > 0.001f) {
        motorStartVelocity_ = currentVelocity_;
        motorElapsedTime_ = 0.0f;  // Restart the ramp timer
    }
    
    targetVelocity_ = newTarget;
}

float VinylSimulator::targetVelocity() const {
    return targetVelocity_;
}

void VinylSimulator::setMotorStartTime(float seconds) {
    motorStartTime_ = std::clamp(seconds, 0.1f, 5.0f);
}

float VinylSimulator::motorStartTime() const {
    return motorStartTime_;
}

void VinylSimulator::setFrictionCoefficient(float coeff) {
    frictionCoefficient_ = std::clamp(coeff, 0.0f, 1.0f);
}

float VinylSimulator::frictionCoefficient() const {
    return frictionCoefficient_;
}

float VinylSimulator::platterVelocity() const {
    return currentVelocity_;
}

void VinylSimulator::update(float dt) {
    if (motorOn_) {
        // Motor is on: ramp velocity from motorStartVelocity_ toward target over motorStartTime_
        motorElapsedTime_ += dt;
        
        // Calculate fraction of ramp complete
        float rampFraction = motorElapsedTime_ / motorStartTime_;
        
        if (rampFraction >= 1.0f) {
            // Ramp complete - hold at target velocity
            currentVelocity_ = targetVelocity_;
        } else {
            // Linear ramp from motorStartVelocity_ to targetVelocity_
            currentVelocity_ = motorStartVelocity_ + (targetVelocity_ - motorStartVelocity_) * rampFraction;
        }
    } else {
        // Motor is off: apply friction decay
        // Exponential decay: velocity *= (1 - friction)^dt
        // Using simplified version: velocity *= decay_factor
        float decayFactor = std::pow(1.0f - frictionCoefficient_, dt);
        currentVelocity_ *= decayFactor;
        
        // Clamp to zero when very small
        if (std::abs(currentVelocity_) < 0.001f) {
            currentVelocity_ = 0.0f;
        }
    }
}

void VinylSimulator::reset() {
    currentVelocity_ = 0.0f;
    motorElapsedTime_ = 0.0f;
    motorStartVelocity_ = 0.0f;
    motorOn_ = false;
}

} // namespace dj
