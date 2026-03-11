#pragma once

#include <vector>

namespace dj {

class LightingRig {
public:
    LightingRig();

    // Update the lighting state based on BPM, energy, and time delta.
    // dtSeconds: elapsed time in seconds since last update
    void update(float bpm, float energy, float dtSeconds);

    // Get the current bar intensities (32 bars, each in [0, 1])
    const std::vector<float>& intensities() const;

private:
    std::vector<float> intensities_;  // 32 bars
    float phaseTime_;                 // Accumulated phase time for BPM sync
};

} // namespace dj
