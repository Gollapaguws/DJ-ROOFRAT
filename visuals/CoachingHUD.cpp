#include "visuals/CoachingHUD.h"

#include <iomanip>
#include <sstream>
#include <cmath>
#include <algorithm>

namespace dj {

std::string CoachingHUD::renderProgressBar(double beatsRemaining, double maxBeats) const {
    if (maxBeats <= 0.0) {
        return "[────────────────]";
    }
    
    // Clamp to max
    double progress = std::min(beatsRemaining / maxBeats, 1.0);
    progress = std::max(progress, 0.0);
    
    int barWidth = 16;
    int filledCount = static_cast<int>(progress * barWidth);
    
    std::string bar = "[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < filledCount) {
            bar += "█";
        } else {
            bar += "─";
        }
    }
    bar += "]";
    
    return bar;
}

std::string CoachingHUD::render(const TransitionCoach::Suggestion& suggestion,
                                double beatsRemaining) const {
    std::ostringstream oss;
    
    // Header box
    oss << "┌─────────────────────────────────────────────┐\n";
    oss << "│         TRANSITION COACHING OVERLAY         │\n";
    oss << "├─────────────────────────────────────────────┤\n";
    
    // Main suggestion text
    oss << "│ " << suggestion.reason << "\n";
    oss << "├─────────────────────────────────────────────┤\n";
    
    // Confidence score
    int confidencePercent = static_cast<int>(suggestion.confidence * 100.0f);
    oss << "│ Confidence: " << std::setw(3) << confidencePercent << "% ";
    
    // Confidence indicator
    if (suggestion.confidence > 0.7f) {
        oss << "✓ High";
    } else if (suggestion.confidence > 0.5f) {
        oss << "~ Medium";
    } else {
        oss << "✗ Low";
    }
    oss << "\n";
    
    // Energy indicator
    oss << "│ Energy: ";
    if (suggestion.energyDelta > 10.0f) {
        oss << "↑ Boost (" << std::fixed << std::setprecision(1) 
            << suggestion.energyDelta << "%)";
    } else if (suggestion.energyDelta < -10.0f) {
        oss << "↓ Drop (" << std::fixed << std::setprecision(1) 
            << suggestion.energyDelta << "%)";
    } else {
        oss << "→ Neutral";
    }
    oss << "\n";
    
    // Harmonic indicator
    int harmonicPercent = static_cast<int>(suggestion.harmonicScore * 100.0f);
    oss << "│ Harmonic Score: " << std::setw(3) << harmonicPercent << "% ";
    if (suggestion.harmonicScore > 0.8f) {
        oss << "✓";
    } else if (suggestion.harmonicScore > 0.5f) {
        oss << "~";
    } else {
        oss << "✗";
    }
    oss << "\n";
    
    oss << "├─────────────────────────────────────────────┤\n";
    
    // Countdown bar
    std::string bar = renderProgressBar(beatsRemaining, 16.0);
    oss << "│ Countdown: " << bar << " ";
    
    int beatsInt = static_cast<int>(beatsRemaining);
    if (beatsInt <= 0) {
        oss << "MIX NOW!";
    } else {
        oss << beatsInt << "b";
    }
    oss << "\n";
    
    // Urgency indicator
    if (suggestion.urgency) {
        oss << "│ ⚠ URGENT - Mix point approaching!           │\n";
    } else {
        oss << "│                                             │\n";
    }
    
    oss << "└─────────────────────────────────────────────┘\n";
    
    return oss.str();
}

} // namespace dj
