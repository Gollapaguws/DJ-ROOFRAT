#include "gameplay/Venue.h"
#include <vector>

namespace dj {

namespace {
std::vector<Venue> venues = {
    Venue("Basement Bar", 0, 50, 0.5f),
    Venue("City Club", 1, 200, 0.8f),
    Venue("Rooftop Session", 2, 800, 1.0f),
    Venue("Arena Showcase", 3, 5000, 1.5f),
    Venue("Festival Main Stage", 4, 50000, 2.0f)
};
}

const Venue& getVenueByTier(int tier) {
    if (tier < 0 || tier >= static_cast<int>(venues.size())) {
        return venues[0];
    }
    return venues[tier];
}

int getVenueCount() {
    return static_cast<int>(venues.size());
}

} // namespace dj
