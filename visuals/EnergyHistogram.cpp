#include "visuals/EnergyHistogram.h"

#include <algorithm>
#include <sstream>
#include <iomanip>

namespace dj {

EnergyHistogram::EnergyHistogram() = default;

std::string EnergyHistogram::getEnergyLabel(int rating) const {
    if (rating <= 2) return "Ambient";
    if (rating <= 4) return "Chill";
    if (rating <= 6) return "Moderate";
    if (rating <= 8) return "Upbeat";
    return "Peak Hour";
}

std::string EnergyHistogram::renderBar(int count, int maxCount, int width) const {
    if (maxCount == 0) return "";
    
    // Calculate bar length proportional to count
    int barLength = (count * width) / maxCount;
    std::string bar;
    
    // Use box-drawing characters for bar
    for (int i = 0; i < barLength; ++i) {
        bar += "█";
    }
    
    return bar;
}

std::string EnergyHistogram::render(const std::vector<int>& energyRatings,
                                     int width,
                                     int height) const {
    std::ostringstream ss;
    
    if (energyRatings.empty()) {
        return "No energy data available.";
    }
    
    // Count occurrences of each energy level (1-10)
    std::vector<int> counts(11, 0);  // Index 0 unused, 1-10 for ratings
    for (int rating : energyRatings) {
        if (rating >= 1 && rating <= 10) {
            counts[rating]++;
        }
    }
    
    // Find max count for scaling
    int maxCount = *std::max_element(counts.begin() + 1, counts.end());
    if (maxCount == 0) {
        return "No valid energy ratings found.";
    }
    
    // Render histogram from highest to lowest energy
    ss << "Energy Distribution (" << energyRatings.size() << " tracks):\n";
    
    for (int rating = 10; rating >= 1; --rating) {
        int count = counts[rating];
        std::string bar = renderBar(count, maxCount, width / 2);  // Half width for readability
        
        ss << std::setw(2) << rating << " " << bar;
        
        // Add count and label
        ss << " " << count << " track";
        if (count != 1) ss << "s";
        ss << " (" << getEnergyLabel(rating) << ")\n";
    }
    
    return ss.str();
}

} // namespace dj
