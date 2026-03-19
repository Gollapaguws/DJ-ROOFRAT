#pragma once

#include <cstddef>
#include <utility>
#include <vector>

#include "audio/Deck.h"

namespace dj {

/**
 * @brief Real-time metrics from a mixBlock() call.
 *
 * Contains RMS energy, transition smoothness, and per-deck energy measurements
 * for analyzing the quality and energy of the current mix.
 */
struct MixMetrics {
    /// RMS energy of mixed output (0.0–1.0)
    float rms = 0.0f;
    /// Mix transition smoothness (0.0–1.0, where 1.0 is seamless)
    float transitionSmoothness = 1.0f;
    /// Energy level of Deck A (0.0–1.0)
    float deckAEnergy = 0.0f;
    /// Energy level of Deck B (0.0–1.0)
    float deckBEnergy = 0.0f;
};

/**
 * @brief Equal-power crossfader and gain staging for two decks.
 *
 * Implements equal-power crossfading to maintain constant loudness as
 * the fader transitions between two decks. Supports per-deck trim and
 * master gain control.
 */
class Mixer {
public:
    /**
     * @brief Set crossfader position.
     * @param value Crossfader position: -1.0 = full Deck A, 0.0 = center, 1.0 = full Deck B.
     *              Values outside [-1.0, 1.0] are clamped.
     */
    void setCrossfader(float value);
    /**
     * @brief Returns current crossfader position.
     * @return Crossfader value in range [-1.0, 1.0].
     */
    float crossfader() const;

    /**
     * @brief Set per-deck trim gains.
     * @param a Deck A trim multiplier (default 1.0, typically 0.5–2.0).
     * @param b Deck B trim multiplier (default 1.0, typically 0.5–2.0).
     */
    void setDeckTrim(float a, float b);
    /**
     * @brief Set master output gain.
     * @param gain Gain multiplier (default 1.0, typically 0.0–2.0).
     */
    void setMasterGain(float gain);

    /**
     * @brief Returns current [gainA, gainB] using equal-power law.
     * @return Pair of gains [deckA, deckB] such that gainA^2 + gainB^2 ≈ 1.0.
     */
    std::pair<float, float> crossfadeGains() const;
    /**
     * @brief Mix N frames from both decks.
     * @param deckA Source deck A.
     * @param deckB Source deck B.
     * @param frames Number of frames to mix.
     * @param metrics Output metrics for this block (RMS, smoothness, per-deck energy).
     * @return Interleaved stereo float buffer at combined output level after crossfade and trim.
     */
    std::vector<float> mixBlock(Deck& deckA, Deck& deckB, std::size_t frames, MixMetrics& metrics);

private:
    float crossfader_ = -1.0f;
    float previousCrossfader_ = -1.0f;
    float deckTrimA_ = 1.0f;
    float deckTrimB_ = 1.0f;
    float masterGain_ = 1.0f;
};

} // namespace dj
