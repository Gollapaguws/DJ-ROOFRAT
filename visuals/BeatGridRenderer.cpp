#include "visuals/BeatGridRenderer.h"

#include "audio/BeatGrid.h"

#include <sstream>

namespace dj {

std::string BeatGridRenderer::render(const BeatGrid& grid, int width) const {
    std::stringstream output;

    // Get beats
    auto beats = grid.getBeats();
    if (beats.empty()) {
        return "No beats in grid\n";
    }

    // Render beat markers line
    output << renderBeatMarkers(grid, width) << "\n";

    // Render bar position numbers line
    output << renderBarNumbers(grid, width) << "\n";

    // Render phrase markers line
    output << renderPhraseMarkers(grid, width) << "\n";

    return output.str();
}

std::string BeatGridRenderer::renderBeatMarkers(const BeatGrid& grid, int width) const {
    auto beats = grid.getBeats();
    if (beats.empty()) {
        return "";
    }

    std::string line(width, ' ');

    // Determine time range (safe: already checked beats.empty())
    double minTime = beats[0].timestamp;
    double maxTime = beats[beats.size() - 1].timestamp;
    if (maxTime <= minTime) {
        maxTime = minTime + 10.0;
    }
    double timeRange = maxTime - minTime;

    // Map each beat to a position in the output line
    for (const auto& beat : beats) {
        double relativeTime = beat.timestamp - minTime;
        if (timeRange > 0.0) {
            int position = static_cast<int>((relativeTime / timeRange) * (width - 1));
            if (position >= 0 && position < width) {
                line[position] = '|';
            }
        }
    }

    return line;
}

std::string BeatGridRenderer::renderBarNumbers(const BeatGrid& grid, int width) const {
    auto beats = grid.getBeats();
    if (beats.empty()) {
        return "";
    }

    std::string line(width, ' ');

    // Determine time range (safe: already checked beats.empty())
    double minTime = beats[0].timestamp;
    double maxTime = beats[beats.size() - 1].timestamp;
    if (maxTime <= minTime) {
        maxTime = minTime + 10.0;
    }
    double timeRange = maxTime - minTime;

    // Place bar position numbers for beats (show pattern clearly)
    for (std::size_t i = 0; i < beats.size(); i++) {
        const auto& beat = beats[i];
        double relativeTime = beat.timestamp - minTime;
        if (timeRange > 0.0) {
            int position = static_cast<int>((relativeTime / timeRange) * (width - 1));
            if (position >= 0 && position < width) {
                char barChar = static_cast<char>('0' + beat.barPosition);
                line[position] = barChar;
            }
        }
    }

    return line;
}

std::string BeatGridRenderer::renderPhraseMarkers(const BeatGrid& grid, int width) const {
    auto beats = grid.getBeats();
    if (beats.empty()) {
        return "";
    }

    std::string line(width, ' ');

    // Determine time range (safe: already checked beats.empty())
    double minTime = beats[0].timestamp;
    double maxTime = beats[beats.size() - 1].timestamp;
    if (maxTime <= minTime) {
        maxTime = minTime + 10.0;
    }
    double timeRange = maxTime - minTime;

    // Mark phrase starts with brackets or special markers
    bool inPhrase = false;

    for (const auto& beat : beats) {
        if (beat.isPhraseStart) {
            double relativeTime = beat.timestamp - minTime;
            if (timeRange > 0.0) {
                int position = static_cast<int>((relativeTime / timeRange) * (width - 1));
                if (position >= 0 && position < width) {
                    line[position] = '[';
                    inPhrase = true;
                }
            }
        } else if (inPhrase) {
            // Continue phrase line
            double relativeTime = beat.timestamp - minTime;
            if (timeRange > 0.0) {
                int position = static_cast<int>((relativeTime / timeRange) * (width - 1));
                if (position >= 0 && position < width && line[position] == ' ') {
                    line[position] = '=';
                }
            }
        }
    }

    // Mark last beat as end of phrase
    if (!beats.empty()) {
        const auto& lastBeat = beats.back();
        double relativeTime = lastBeat.timestamp - minTime;
        if (timeRange > 0.0) {
            int position = static_cast<int>((relativeTime / timeRange) * (width - 1));
            if (position >= 0 && position < width) {
                line[position] = ']';
            }
        }
    }

    return line;
}

}  // namespace dj
