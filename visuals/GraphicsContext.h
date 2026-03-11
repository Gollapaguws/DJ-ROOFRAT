#pragma once

#include <string>

namespace dj {

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
    bool renderFrame(float bpm, float energy, int mood, float crossfader);

    // Clean up graphics resources
    void shutdown();

private:
    bool available_;
    int width_;
    int height_;
};

} // namespace dj
