#pragma once

namespace dj {

class ScratchDetector {
public:
    // Set sample rate
    void setSampleRate(int sampleRate);
    
    // Sensitivity: controls how much scratch input affects pitch (higher = more effect)
    void setSensitivity(float sensitivity);
    float sensitivity() const;
    
    // Deadzone: input magnitude below this is treated as zero
    void setDeadzone(float deadzone);
    float deadzone() const;
    
    // Process scratch input (velocity) over a time frame
    // Input: scratchVelocity (in revolution units per second, can be negative)
    // dt: time since last update (in seconds)
    // Output: pitch multiplier (1.0 = normal, negative = backward)
    float processScratchInput(float scratchVelocity, float dt);
    
    // Reset detector state
    void reset();

private:
    int sampleRate_ = 44100;
    float sensitivity_ = 1.0f;
    float deadzone_ = 0.05f;
};

} // namespace dj
