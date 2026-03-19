#pragma once

#include <string>

namespace dj {

/**
 * @brief Top-level play mode.
 */
enum class PlayMode {
    Career,   /// Career progression with venues and tiers
    Sandbox,  /// Freeform DJ practice
};

/**
 * @brief Per-frame scoring engine based on crowd energy and transition quality.
 *
 * Accumulates points based on real-time mix quality and crowd response metrics.
 */
class ScoringSystem {
public:
    /**
     * @brief Reset score to zero.
     */
    void reset();
    /**
     * @brief Add points for this frame.
     * @param crowdEnergy Current crowd energy [0.0–1.0].
     * @param transitionSmoothness Transition smoothness [0.0–1.0].
     * @return Updated total score.
     */
    int update(float crowdEnergy, float transitionSmoothness);
    /**
     * @brief Get current accumulated score.
     * @return Total score (0 to max int).
     */
    int score() const;

private:
    int score_ = 0;
};

/**
 * @brief Career tier and reputation tracker driven by crowd energy.
 *
 * Tracks DJ career progression through venues at different tiers, reputation
 * points, and peak tier reached. Tier advancement is monotonic.
 */
class CareerProgression {
public:
    /**
     * @brief Reset to tier 0 with default reputation.
     */
    void reset();
    /**
     * @brief Advance career based on crowd response.
     * @param crowdEnergy Current crowd energy [0.0–1.0].
     */
    void update(float crowdEnergy);
    /**
     * @brief Get current career tier.
     * @return Tier (0=starting, higher=more advanced).
     */
    int tier() const;
    /**
     * @brief Get name of current venue at this tier.
     * @return Venue name string.
     */
    std::string currentVenueName() const;
    
    // NEW METHODS for Phase 23
    /**
     * @brief Get reputation score.
     * @return Reputation (starts at 10.0).
     */
    float reputation() const;
    /**
     * @brief Get highest tier ever reached.
     * @return Peak tier.
     */
    int peakTier() const;
    /**
     * @brief Check if a venue at the given tier is accessible.
     * @param venueTier Tier to check.
     * @return true if currently at that tier or higher.
     */
    bool isVenueUnlocked(int venueTier) const;

private:
    float reputation_ = 10.0f;
    int peakTier_ = 0;
};

} // namespace dj
