#pragma once

#include <string>
#include <vector>

namespace dj {

class Deck;
class BeatGrid;
class EnergyCurve;
class CamelotAnalyzer;

/**
 * @brief Analyzes track structure and suggests optimal transition points.
 * 
 * Detects phrase boundaries (16/32 bars), analyzes energy context,
 * checks harmonic compatibility, and provides timed suggestions
 * with confidence scoring.
 */
class TransitionCoach {
public:
    struct Suggestion {
        double timestamp;          // When to mix (in seconds from track start)
        std::string reason;        // Human-readable coaching message
        float confidence;          // 0.0-1.0 (how confident is this suggestion?)
        bool urgency;              // True if transition window closing soon (<8 beats)
        float energyDelta;         // Energy change (+/- percentage)
        float harmonicScore;       // 0.0-1.0 compatibility score
    };

    TransitionCoach() = default;
    ~TransitionCoach() = default;

    /**
     * @brief Detect phrase boundaries (16/32 bar structure).
     * 
     * @param grid Beat grid to analyze
     * @return Vector of phrase boundary timestamps (in seconds)
     */
    std::vector<double> detectPhrases(const BeatGrid& grid) const;

    /**
     * @brief Suggest next transition point with context analysis.
     * 
     * @param outgoingDeck Current playing deck
     * @param incomingDeck Deck to mix in
     * @param energyCurve Energy analysis
     * @param camelot Harmonic analysis
     * @return Transition suggestion with confidence and reasoning
     */
    Suggestion suggestNextTransition(
        const Deck& outgoingDeck,
        const Deck& incomingDeck,
        const EnergyCurve& energyCurve,
        const CamelotAnalyzer& camelot) const;

    /**
     * @brief Calculate countdown to next phrase boundary.
     * 
     * @param currentPos Current playback position (seconds)
     * @param nextPhrase Next phrase boundary (seconds)
     * @param bpm Current BPM
     * @return Beats remaining until phrase boundary
     */
    double calculateCountdown(double currentPos, double nextPhrase, double bpm) const;

private:
    /**
     * @brief Find next phrase boundary after current position.
     * 
     * @param phrases Vector of phrase timestamps
     * @param currentPos Current position (seconds)
     * @return Next phrase timestamp, or -1.0 if none found
     */
    double findNextPhrase(const std::vector<double>& phrases, double currentPos) const;
};

} // namespace dj
