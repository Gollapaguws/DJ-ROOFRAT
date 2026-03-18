#include "visuals/SyncIndicator.h"
#include "audio/SyncController.h"
#include <sstream>
#include <cmath>
#include <algorithm>

namespace dj {

std::string SyncIndicator::renderPhaseMeter(double phaseOffset) const {
    // Clamp phase offset to [-0.5, 0.5]
    phaseOffset = std::clamp(phaseOffset, -0.5, 0.5);
    
    // Map to 20-character bar position (0-20)
    int position = static_cast<int>((phaseOffset + 0.5) * 20);
    position = std::clamp(position, 0, 20);
    
    // Build bar: [====|====] format with center indicator
    std::string bar = "[";
    for (int i = 0; i < 10; ++i) {
        if (i == position) {
            bar += "█";
        } else if (i < position) {
            bar += "=";
        } else {
            bar += " ";
        }
    }
    bar += "|";
    for (int i = 10; i < 20; ++i) {
        if (i == position) {
            bar += "█";
        } else if (i < position) {
            bar += "=";
        } else {
            bar += " ";
        }
    }
    bar += "]";
    
    return bar;
}

std::string SyncIndicator::render(bool syncEnabled, SyncState state, double phaseOffset) const {
    if (!syncEnabled) {
        return "";
    }
    
    std::ostringstream oss;
    oss << "[SYNC ";
    
    // Add state indicator
    switch (state) {
    case SyncState::Initializing:
        oss << "⟳ MATCHING";
        break;
    case SyncState::Locked:
        oss << "✓ LOCKED";
        break;
    case SyncState::Drifting:
        oss << "⚠ DRIFT";
        break;
    case SyncState::Off:
    default:
        oss << " OFF";
        break;
    }
    
    oss << " ";
    oss << renderPhaseMeter(phaseOffset);
    oss << "]";
    
    return oss.str();
}

} // namespace dj
