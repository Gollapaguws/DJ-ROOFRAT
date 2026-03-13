#pragma once

#include <vector>
#include <array>

namespace dj {

// Light source for multi-light rendering
struct Light {
    enum class Type { Point, Spot };

    Type type;
    std::array<float, 3> position;
    std::array<float, 3> direction;
    std::array<float, 3> color;
    float intensity;
    float spotAngle;       // For spot lights, in degrees
    float attenuation;     // Fall-off rate
};

class LightingRig {
public:
    LightingRig();

    // Update the lighting state based on BPM, energy, and time delta.
    // dtSeconds: elapsed time in seconds since last update
    void update(float bpm, float energy, float dtSeconds);

    // Get the current bar intensities (32 bars, each in [0, 1])
    const std::vector<float>& intensities() const;

    // Multi-light system: Add spot light
    // Returns light index, or -1 if limit (8 lights) exceeded
    int addSpotLight(const std::array<float, 3>& position,
                     const std::array<float, 3>& direction,
                     const std::array<float, 3>& color,
                     float spotAngleDegrees,
                     float intensity);

    // Multi-light system: Add point light
    // Returns light index, or -1 if limit (8 lights) exceeded
    int addPointLight(const std::array<float, 3>& position,
                      const std::array<float, 3>& color,
                      float intensity);

    // Get all lights
    const std::vector<Light>& getLights() const noexcept { return lights_; }

    // Clear all lights
    void clearLights() noexcept { lights_.clear(); }

    // Get strobe intensity (0 to 1) for beat-synchronized strobes
    // Returns 0 when not strobing, peaks to 1 on beat flash
    float getStrobeIntensity() const noexcept { return strobeIntensity_; }

private:
    // Bar intensities for visualization
    std::vector<float> intensities_;  // 32 bars
    float phaseTime_;                 // Accumulated phase time for BPM sync

    // Multi-light system (up to 8 lights)
    std::vector<Light> lights_;
    static constexpr int MAX_LIGHTS = 8;

    // Strobe effect
    float strobeIntensity_;         // Current strobe brightness [0, 1]
    float strobePhase_;             // Phase of strobe effect
    float lastBeatTime_;            // Track beat timing for strobes
};

} // namespace dj
