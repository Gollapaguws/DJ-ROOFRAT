#pragma once

#include "gameplay/TransitionCoach.h"
#include <string>

namespace dj {

/**
 * @brief Renders coaching overlay in terminal ASCII format.
 * 
 * Displays transition suggestions, countdown bars, energy/harmonic
 * indicators, and confidence scores.
 */
class CoachingHUD {
public:
    CoachingHUD() = default;
    ~CoachingHUD() = default;

    /**
     * @brief Render coaching overlay as ASCII string.
     * 
     * @param suggestion Current transition suggestion
     * @param beatsRemaining Countdown in beats
     * @return Multiline ASCII string for terminal display
     */
    std::string render(const TransitionCoach::Suggestion& suggestion,
                       double beatsRemaining) const;

private:
    /**
     * @brief Render progress bar for countdown.
     * 
     * @param beatsRemaining Beats until transition
     * @param maxBeats Full countdown duration (e.g., 16 beats)
     * @return ASCII bar like [████████────────]
     */
    std::string renderProgressBar(double beatsRemaining, double maxBeats) const;
};

} // namespace dj
