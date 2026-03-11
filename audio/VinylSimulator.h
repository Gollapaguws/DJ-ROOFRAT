#pragma once

namespace dj {

class VinylSimulator {
public:
    // Set sample rate for time calculations
    void setSampleRate(int sampleRate);
    
    // Motor control
    void setMotorOn(bool on);
    bool isMotorOn() const;
    
    // Target velocity (1.0 = normal speed, negative = backward)
    void setTargetVelocity(float velocity);
    float targetVelocity() const;
    
    // Motor start time: duration to reach target velocity (in seconds)
    void setMotorStartTime(float seconds);
    float motorStartTime() const;
    
    // Friction coefficient (0.0 = no friction, 1.0 = heavy friction)
    void setFrictionCoefficient(float coeff);
    float frictionCoefficient() const;
    
    // Get current platter velocity
    float platterVelocity() const;
    
    // Update physics for a time step (dt in seconds)
    void update(float dt);
    
    // Reset to initial state
    void reset();

private:
    int sampleRate_ = 44100;
    bool motorOn_ = false;
    float targetVelocity_ = 0.0f;
    float currentVelocity_ = 0.0f;
    float motorStartTime_ = 0.15f;  // 0.15 seconds to reach full speed by default
    float frictionCoefficient_ = 0.8f;
    
    // Motor ramp tracking
    float motorElapsedTime_ = 0.0f;
    float motorStartVelocity_ = 0.0f;  // Velocity when motor was turned on or target changed
};

} // namespace dj
