#pragma once

#include <cstddef>

namespace dj {

class Deck;

/**
 * @brief Phase alignment engine for beat-synchronized deck synchronization.
 * 
 * Calculates the phase offset between two decks by comparing their current
 * playback positions relative to their beat grids. Used by auto-sync features
 * to align beats for seamless transitions.
 * 
 * Phase offset is normalized to [-0.5, +0.5] of a beat cycle, where:
 * - 0.0 = perfectly aligned
 * - +0.5 = target deck is half a beat ahead
 * - -0.5 = target deck is half a beat behind
 * 
 * Target accuracy: ±5ms (imperceptible phase alignment)
 */
class PhaseAligner {
public:
    PhaseAligner() = default;
    ~PhaseAligner() = default;

    /**
     * @brief Calculate phase offset between two decks in samples.
     * 
     * Finds the nearest beat marker in each deck's beat grid and computes
     * the phase difference. Positive offset means deckB is ahead of deckA.
     * 
     * @param deckA Source deck (reference)
     * @param deckB Target deck (to be aligned)
     * @param bpmA Current BPM of deck A
     * @param bpmB Current BPM of deck B
     * @return Phase offset in samples (+ = B ahead, - = B behind)
     */
    double calculatePhaseOffset(const Deck& deckA, const Deck& deckB,
                                 double bpmA, double bpmB) const;

    /**
     * @brief Calculate phase offset between two decks in beats.
     * 
     * Similar to calculatePhaseOffset but returns normalized beat offset
     * in the range [-0.5, +0.5] for easier interpretation.
     * 
     * @param deckA Source deck (reference)
     * @param deckB Target deck (to be aligned)
     * @param bpmA Current BPM of deck A
     * @param bpmB Current BPM of deck B
     * @return Phase offset in beats (+ = B ahead, - = B behind)
     */
    double calculatePhaseOffsetBeats(const Deck& deckA, const Deck& deckB,
                                      double bpmA, double bpmB) const;

private:
    /**
     * @brief Find the nearest beat marker to a given sample position.
     * 
     * @param position Current playback position in samples
     * @param bpm Current BPM
     * @param sampleRate Audio sample rate
     * @return Sample position of nearest beat, or position if no beat grid
     */
    std::size_t findNearestBeat(std::size_t position, double bpm, int sampleRate) const;

    /**
     * @brief Normalize phase offset to [-0.5, +0.5] beat range.
     * 
     * @param offsetSamples Raw phase offset in samples
     * @param bpm Current BPM
     * @param sampleRate Audio sample rate
     * @return Normalized phase offset in beats
     */
    double normalizePhaseOffset(double offsetSamples, double bpm, int sampleRate) const;
};

} // namespace dj
