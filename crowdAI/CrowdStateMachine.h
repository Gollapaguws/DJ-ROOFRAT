#pragma once

#include <string>

namespace dj {

/**
 * @brief Crowd mood state ordered by ascending energy.
 */
enum class CrowdMood {
    Unimpressed,  /// Lowest energy, not engaged
    Calm,         /// Relaxed, moderate interest
    Grooving,     /// Active, dancing
    Hyped,        /// Peak energy, fully engaged
};

/**
 * @brief Crowd personality archetype affecting energy response.
 */
enum class CrowdPersonality {
    Default,  /// Balanced response to all music
    Rave,     /// Prefers high-energy electronic music
    Jazz,     /// Prefers smooth, complex musicality
    EDM,      /// Prefers steady-beat dance music
};

/**
 * @brief Output from a CrowdStateMachine update step.
 *
 * Contains real-time crowd state and reaction information.
 */
struct CrowdOutput {
    /// Current crowd energy meter (0.0–1.0)
    float energyMeter = 0.0f;
    /// Current crowd reaction text
    std::string reaction;
    /// Current crowd mood
    CrowdMood mood = CrowdMood::Calm;
};

/**
 * @brief State machine modeling crowd energy and mood during a DJ set.
 *
 * Simulates crowd response to BPM, mix quality, track energy, and beatmatch accuracy.
 * Supports different crowd personalities with distinct energy curves.
 */
class CrowdStateMachine {
public:
    /**
     * @brief Construct state machine.
     * @param personality Crowd archetype (default: Default).
     */
    explicit CrowdStateMachine(CrowdPersonality personality = CrowdPersonality::Default);

    // Phase 6: Extended signature with beatmatch delta input
    /**
     * @brief Advance crowd state by one step.
     * @param bpm Current mix BPM.
     * @param transitionSmoothness Mix quality [0.0–1.0].
     * @param trackEnergy Track energy [0.0–1.0].
     * @param beatmatchDelta BPM deviation between decks (0=perfect, 2=neutral, >2=poor).
     * @return Current crowd output snapshot.
     */
    CrowdOutput update(float bpm, float transitionSmoothness, float trackEnergy, float beatmatchDelta);

    // Phase 5 backward compatibility: default beatmatchDelta = 2.0f (neutral)
    /**
     * @brief Backward compatibility: default beatmatchDelta = 2.0f (neutral).
     * @param bpm Current mix BPM.
     * @param transitionSmoothness Mix quality [0.0–1.0].
     * @param trackEnergy Track energy [0.0–1.0].
     * @return Current crowd output snapshot.
     */
    CrowdOutput update(float bpm, float transitionSmoothness, float trackEnergy) {
        return update(bpm, transitionSmoothness, trackEnergy, 2.0f);
    }

private:
    CrowdPersonality personality_;
    float energy_ = 0.35f;
    CrowdMood mood_ = CrowdMood::Calm;
    float energyBuffer_ = 0.35f;
    int hysteresisCounter_ = 0;
    int reactionIndex_ = 0;  // For round-robin reaction selection
};

} // namespace dj
