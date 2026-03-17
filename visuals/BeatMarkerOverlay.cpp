#include "visuals/BeatMarkerOverlay.h"
#include <algorithm>
#include <sstream>
#include <cmath>

namespace dj {

BeatMarkerOverlay::BeatMarkerOverlay()
    : options_()
{
}

BeatMarkerOverlay::BeatMarkerOverlay(const RenderOptions& options)
    : options_(options)
{
}

std::string BeatMarkerOverlay::render(
    const std::string& waveformOutput,
    const std::vector<size_t>& beatPositions,
    size_t currentPlaybackPosition,
    size_t windowStart,
    size_t windowEnd
) const {
    if (waveformOutput.empty() || beatPositions.empty()) {
        return waveformOutput;
    }

    // Filter beats to only those visible in the window
    std::vector<size_t> visibleBeats;
    for (size_t beatPos : beatPositions) {
        if (beatPos >= windowStart && beatPos <= windowEnd) {
            visibleBeats.push_back(beatPos);
        }
    }

    if (visibleBeats.empty()) {
        return waveformOutput;
    }

    // Find the closest beat to current playback position (for highlighting)
    size_t highlightedBeat = findClosestBeat(beatPositions, currentPlaybackPosition);

    // Insert markers into the waveform output
    return insertMarkers(waveformOutput, visibleBeats, highlightedBeat, windowStart, windowEnd);
}

void BeatMarkerOverlay::setOptions(const RenderOptions& options) {
    options_ = options;
}

const BeatMarkerOverlay::RenderOptions& BeatMarkerOverlay::getOptions() const {
    return options_;
}

std::string BeatMarkerOverlay::insertMarkers(
    const std::string& waveformOutput,
    const std::vector<size_t>& visibleBeats,
    size_t highlightedBeat,
    size_t windowStart,
    size_t windowEnd
) const {
    // Detect waveform width from the output
    int waveformWidth = detectWaveformWidth(waveformOutput);
    if (waveformWidth <= 0) {
        return waveformOutput;  // Can't parse waveform, return as-is
    }

    // Split waveform into lines
    std::vector<std::string> lines;
    std::stringstream ss(waveformOutput);
    std::string line;
    while (std::getline(ss, line)) {
        lines.push_back(line);
    }

    if (lines.empty()) {
        return waveformOutput;
    }

    // Create a marker line (bottom of waveform)
    std::string markerLine(waveformWidth, ' ');

    // Place markers at beat positions
    for (size_t beatPos : visibleBeats) {
        int col = calculateColumnPosition(beatPos, windowStart, windowEnd, waveformWidth);
        if (col >= 0 && col < waveformWidth) {
            // Use highlight char if this is the current beat
            if (beatPos == highlightedBeat) {
                markerLine[col] = options_.highlightChar;
            } else {
                markerLine[col] = options_.markerChar;
            }
        }
    }

    // Append marker line to the waveform output
    lines.push_back(markerLine);

    // Reconstruct output with markers
    std::ostringstream result;
    for (size_t i = 0; i < lines.size(); ++i) {
        result << lines[i];
        if (i < lines.size() - 1) {
            result << "\n";
        }
    }

    return result.str();
}

int BeatMarkerOverlay::calculateColumnPosition(
    size_t samplePosition,
    size_t windowStart,
    size_t windowEnd,
    int waveformWidth
) const {
    if (windowEnd <= windowStart || waveformWidth <= 0) {
        return -1;
    }

    // Calculate relative position within window (0.0 to 1.0)
    double relativePosition = static_cast<double>(samplePosition - windowStart) /
                              static_cast<double>(windowEnd - windowStart);

    // Map to column position (0 to waveformWidth-1)
    int column = static_cast<int>(relativePosition * waveformWidth);

    return std::clamp(column, 0, waveformWidth - 1);
}

int BeatMarkerOverlay::detectWaveformWidth(const std::string& waveformOutput) const {
    // Find the first newline to determine line width
    size_t firstNewline = waveformOutput.find('\n');
    if (firstNewline == std::string::npos) {
        // No newlines, assume entire string is one line
        return static_cast<int>(waveformOutput.length());
    }

    return static_cast<int>(firstNewline);
}

size_t BeatMarkerOverlay::findClosestBeat(
    const std::vector<size_t>& beatPositions,
    size_t playbackPosition
) const {
    if (beatPositions.empty()) {
        return 0;
    }

    size_t closestBeat = beatPositions[0];
    size_t minDistance = std::abs(static_cast<long long>(beatPositions[0]) - static_cast<long long>(playbackPosition));

    for (size_t beatPos : beatPositions) {
        size_t distance = std::abs(static_cast<long long>(beatPos) - static_cast<long long>(playbackPosition));
        if (distance < minDistance) {
            minDistance = distance;
            closestBeat = beatPos;
        }
    }

    return closestBeat;
}

} // namespace dj
