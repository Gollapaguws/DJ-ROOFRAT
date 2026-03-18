#pragma once

#include <string>

namespace dj {

enum class SyncState;

/**
 * @brief Renders sync status indicator for terminal display.
 * 
 * Shows SYNC badge with current state and phase meter
 * indicating beat alignment.
 */
class SyncIndicator {
public:
    SyncIndicator() = default;
    ~SyncIndicator() = default;

    /**
     * @brief Render sync indicator string.
     * 
     * @param syncEnabled Whether sync is enabled
     * @param state Current sync state
     * @param phaseOffset Phase offset in beats (-0.5 to +0.5)
     * @return Formatted sync indicator (e.g., "[SYNC ✓ LOCKED [====|====]]")
     */
    std::string render(bool syncEnabled, SyncState state, double phaseOffset) const;

private:
    /**
     * @brief Render phase meter bar.
     * 
     * @param phaseOffset Phase offset in beats
     * @return ASCII bar like [====|====] with position indicator
     */
    std::string renderPhaseMeter(double phaseOffset) const;
};

} // namespace dj
