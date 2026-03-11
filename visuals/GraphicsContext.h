#pragma once

#include <memory>
#include <string>

namespace dj {

class LaserController;
class CrowdRenderer;

class GraphicsContext {
public:
    GraphicsContext();
    ~GraphicsContext();

    // Initialize graphics context. Returns true if graphics are available after init.
    bool initialize(int width, int height, std::string* errorOut = nullptr);

    // Check if graphics are currently available
    bool isAvailable() const;

    // Render a frame with current playback state.
    // Returns true if render succeeded; false if graphics unavailable.
    // Parameters: BPM, crowd energy [0,1], mood (0-3), crossfader [-1,1]
    bool renderFrame(float bpm, float energy, int mood, float crossfader);

    // Clean up graphics resources
    void shutdown();

private:
    bool available_;
    int width_;
    int height_;
    std::unique_ptr<LaserController> laserController_;
    std::unique_ptr<CrowdRenderer> crowdRenderer_;
};

} // namespace dj
