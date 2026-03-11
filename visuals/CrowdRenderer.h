#pragma once

namespace dj {

class CrowdRenderer {
public:
    CrowdRenderer();
    ~CrowdRenderer() = default;

    // Update crowd silhouette animation based on mood and energy
    // mood: integer representing crowd mood (0=Unimpressed, 1=Calm, 2=Grooving, 3=Hyped)
    // energy: crowd energy level in range [0, 1]
    // dtSeconds: time delta since last update
    void update(int mood, float energy, float dtSeconds);

    // Get animation motion amplitude in [0, 1]
    // Higher mood and energy produce higher amplitude
    float motionAmplitude() const noexcept;

    // Get crowd density in [0, 1]
    // Higher energy produces higher density
    float density() const noexcept;

    // Get number of visible crowd silhouettes
    // Typically 0-550 depending on mood and energy
    int visibleSilhouettes() const noexcept;

private:
    float motionAmplitude_;
    float density_;
    int visibleSilhouettes_;
    float animationPhase_;  // Track animation phase over time
};

} // namespace dj
