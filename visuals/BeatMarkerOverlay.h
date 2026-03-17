#pragma once

#include <string>
#include <vector>

namespace dj {

class BeatMarkerOverlay {
public:
    struct RenderOptions {
        char markerChar = '|';
        char highlightChar = 'v';  // Using 'v' instead of '▼' for console compatibility
        bool useColor = true;
    };

    BeatMarkerOverlay();
    explicit BeatMarkerOverlay(const RenderOptions& options);
    ~BeatMarkerOverlay() = default;

    // Render beat markers on waveform output
    // beatPositions: Beat positions in samples
    // currentPlaybackPosition: Current playback position in samples
    // windowStart: Start sample of the visible waveform window
    // windowEnd: End sample of the visible waveform window
    std::string render(
        const std::string& waveformOutput,
        const std::vector<size_t>& beatPositions,
        size_t currentPlaybackPosition,
        size_t windowStart,
        size_t windowEnd
    ) const;

    // Set render options
    void setOptions(const RenderOptions& options);
    const RenderOptions& getOptions() const;

private:
    std::string insertMarkers(
        const std::string& waveformOutput,
        const std::vector<size_t>& visibleBeats,
        size_t highlightedBeat,
        size_t windowStart,
        size_t windowEnd
    ) const;

    int calculateColumnPosition(size_t samplePosition, size_t windowStart, size_t windowEnd, int waveformWidth) const;
    int detectWaveformWidth(const std::string& waveformOutput) const;
    size_t findClosestBeat(const std::vector<size_t>& beatPositions, size_t playbackPosition) const;

    RenderOptions options_;
};

} // namespace dj
