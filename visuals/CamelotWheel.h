#pragma once

#include <string>
#include <vector>

namespace dj {

class CamelotWheel {
public:
    // Render ASCII art Camelot Wheel for a given Camelot key
    // Shows inner ring (minor keys A) and outer ring (major keys B)
    // Current key highlighted with ** markers
    // Compatible keys marked with + indicators
    // Returns multi-line string suitable for terminal output
    std::string render(const std::string& currentCamelotKey) const;

    // Get list of compatible Camelot codes for a given key
    // Includes: self, adjacent keys, and relative minor/major
    std::vector<std::string> getCompatibleKeys(const std::string& camelotKey) const;

    // Get musical note name for a Camelot code (e.g., "5A" -> "C")
    // Returns the root note of the key
    std::string getNoteName(const std::string& camelotCode) const;

private:
    // Helper: Get the 12 Camelot positions as a list for rendering
    static std::vector<std::string> getWheelPositions(char ringType);

    // Helper: Determine if a key should be highlighted on the display
    static bool shouldHighlight(const std::string& wheelKey, const std::string& currentKey);

    // Helper: Determine if a key is compatible with current
    static bool isCompatibleWith(const std::string& wheelKey, const std::string& currentKey);
};

}  // namespace dj
