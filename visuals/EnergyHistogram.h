#pragma once

#include <string>
#include <vector>
#include <cstddef>

namespace dj {

class EnergyHistogram {
public:
    EnergyHistogram();
    ~EnergyHistogram() = default;
    
    // Render histogram from energy ratings
    // energyRatings: vector of 1-10 energy ratings
    // width: character width of each bar (default 80)
    // height: number of rows to display per energy level (default 10)
    std::string render(const std::vector<int>& energyRatings, 
                       int width = 80, 
                       int height = 10) const;
    
private:
    // Helper: Render a single bar for an energy level
    std::string renderBar(int count, int maxCount, int width) const;
    
    // Helper: Get energy label string
    std::string getEnergyLabel(int rating) const;
};

} // namespace dj
