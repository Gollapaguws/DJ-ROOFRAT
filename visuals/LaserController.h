#pragma once

#include <array>
#include <vector>

namespace dj {

// Vertex structure for laser beam geometry
struct LaserVertex {
    std::array<float, 3> position;
    std::array<float, 2> texCoord;
    std::array<float, 4> color;
};

// Beam geometry returned by getBeamGeometry()
struct BeamGeometry {
    std::vector<LaserVertex> vertices;
    std::vector<unsigned int> indices;
};

class LaserController {
public:
    LaserController();
    ~LaserController() = default;

    // Update laser state based on BPM, crossfader position, and time delta
    // bpm: beats per minute (typically 60-200)
    // crossfader: position in range [-1.0, 1.0] where -1 is left deck, 0 is center, 1 is right deck
    // dtSeconds: time delta since last update
    void update(float bpm, float crossfader, float dtSeconds);

    // Get primary laser beam angle in degrees, roughly [-45, +45] following crossfader
    float primaryAngleDegrees() const noexcept;

    // Get secondary laser beam angle in degrees, [-180, 180], oscillates with BPM
    float secondaryAngleDegrees() const noexcept;

    // Get laser intensity in [0, 1], increases with BPM/sync activity
    float intensity() const noexcept;

    // Get volumetric beam geometry (quad strip) for rendering
    // width: beam width in world units
    // Returns quad of 4 vertices with 6 indices (2 triangles)
    BeamGeometry getBeamGeometry(float width) const noexcept;

private:
    float primaryAngle_;
    float secondaryAngle_;
    float intensity_;
    float phaseTime_;  // Track oscillation phase over time
};

} // namespace dj
