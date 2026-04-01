#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <string>

#include "audio/AudioClip.h"
#include "audio/ThreeBandEQ.h"
#include "audio/SyncController.h"
#include "audio/SpectrumAnalyzer.h"

namespace dj {

class EffectChain;
class VinylSimulator;
class ScratchDetector;

/// EQ state structure for accessing three-band EQ gains
struct EQState {
    float low = 1.0f;   // Low band gain (default 1.0)
    float mid = 1.0f;   // Mid band gain (default 1.0)
    float high = 1.0f;  // High band gain (default 1.0)
};

class Deck {
public:
    /**
     * @brief Load an in-memory AudioClip for playback.
     * @param clip Audio clip to load (copying samples into internal buffer).
     * @return true on success, false if clip is empty or invalid.
     */
    bool loadClip(const AudioClip& clip);
    /**
     * @brief Load from file path with optional error output.
     * @param path Absolute or relative file path to audio file.
     * @param errorOut Optional pointer to receive error description on failure.
     * @return true on success, false on error.
     */
    [[nodiscard]] bool loadFromFile(const std::string& path, std::string* errorOut = nullptr);

    /**
     * @brief Check if a clip is currently loaded.
     * @return true if a clip has been loaded and is ready for playback.
     */
    bool hasClip() const;
    /**
     * @brief Get pointer to currently loaded clip.
     * @return Const pointer to AudioClip, or nullptr if no clip loaded.
     */
    const AudioClip* clip() const;

    /**
     * @brief Start playback from current position.
     */
    void play();
    /**
     * @brief Pause playback (freeze playhead without stopping).
     */
    void pause();
    /**
     * @brief Stop playback and reset playhead to start.
     */
    void stop();
    /**
     * @brief Check if playback is active.
     * @return true if playing, false if paused or stopped.
     */
    bool isPlaying() const;

    /**
     * @brief Set the output sample rate for playback calculations.
     * @param sampleRate Output sample rate in Hz (e.g., 44100, 48000).
     */
    void setOutputSampleRate(int sampleRate);
    /**
     * @brief Set playback tempo offset.
     * @param percent Offset from 0 (0% = original speed, 5.0 = +5%, -5.0 = -5%).
     */
    void setTempoPercent(float percent);
    /**
     * @brief Get current tempo offset.
     * @return Current tempo percent (0 = original, positive = faster, negative = slower).
     */
    float tempoPercent() const;

    /**
     * @brief Set loop length in beats.
     * @param beats Loop length (default 16, typical: 4, 8, 16, 32).
     */
    void setLoopBeats(int beats);
    /**
     * @brief Get current loop length in beats.
     * @return Loop length in beats.
     */
    int loopBeats() const;

    /**
     * @brief Set output trim (gain before EQ and effects).
     * @param trim Gain multiplier (default 1.0, range 0.0+).
     */
    void setTrim(float trim);
    /**
     * @brief Set 3-band EQ gains.
     * @param low Low band gain (default 1.0, typical 0.0–2.0).
     * @param mid Mid band gain (default 1.0, typical 0.0–2.0).
     * @param high High band gain (default 1.0, typical 0.0–2.0).
     */
    void setEQ(float low, float mid, float high);
    /**
     * @brief Set 3-band EQ crossover frequencies.
     * @param lowMid Frequency dividing low and mid bands (Hz, e.g., 250).
     * @param midHigh Frequency dividing mid and high bands (Hz, e.g., 4000).
     */
    void setEQFrequencies(float lowMid, float midHigh);
    /**
     * @brief Set filter cutoff normalized 0.0–1.0.
     * @param cutoff Filter cutoff: 0.0 = closed (no signal), 1.0 = fully open.
     */
    void setFilter(float cutoff);
    /**
     * @brief Enable/disable isolator bands (mute specific EQ bands).
     * @param low Enable low band isolator.
     * @param mid Enable mid band isolator.
     * @param high Enable high band isolator.
     */
    void setIsolatorMode(bool low, bool mid, bool high);
    /**
     * @brief Check if a specific isolator band is enabled.
     * @param band EQ band index (0=low, 1=mid, 2=high).
     * @return true if isolator is active on this band.
     */
    bool isIsolatorEnabled(int band) const;
    /**
     * @brief Set filter order (steepness).
     * @param order Filter order: 1 = single-pole (6dB/octave), 2 = Butterworth (12dB/octave).
     */
    void setFilterOrder(int order);
    /**
     * @brief Get current filter order.
     * @return Current filter order (1 or 2).
     */
    int getFilterOrder() const;

    // Phase 15: Effect chain integration
    /**
     * @brief Attach an effect chain to this deck.
     * @param effectChain Shared pointer to EffectChain for processing.
     */
    void setEffectChain(std::shared_ptr<EffectChain> effectChain);
    /**
     * @brief Set effect send level for processing.
     * @param sendLevel Send amount: 0.0 = no effect send, 1.0 = full effect.
     */
    void setEffectSendLevel(float sendLevel);  // 0.0 = no send, 1.0 = full send

    /**
     * @brief Set primary cue point.
     * @param frame Frame index to mark as cue point.
     */
    void setCue(std::size_t frame);
    /**
     * @brief Jump playhead to primary cue point.
     */
    void jumpToCue();
    // Phase 4: Multi-cue hotspot banks (A, B, C = banks 0, 1, 2)
    /**
     * @brief Set cue point in a multi-cue bank.
     * @param frame Frame index for cue.
     * @param bank Bank index (0=A, 1=B, 2=C).
     */
    void setCue(std::size_t frame, int bank);
    /**
     * @brief Jump playhead to cue point in a specific bank.
     * @param bank Bank index (0=A, 1=B, 2=C).
     */
    void jumpToCue(int bank);
    /**
     * @brief Set the active multi-cue bank.
     * @param bank Bank index (0=A, 1=B, 2=C).
     */
    void setActiveCueBank(int bank);

    /**
     * @brief Configure loop boundaries and enable/disable.
     * @param startFrame Loop start frame.
     * @param endFrame Loop end frame.
     * @param enabled Enable loop playback.
     */
    void configureLoop(std::size_t startFrame, std::size_t endFrame, bool enabled);
    // Phase 4: Quantized loop configuration with BPM
    /**
     * @brief Configure quantized loop with BPM-based alignment.
     * @param startFrame Loop start frame.
     * @param endFrame Loop end frame.
     * @param enabled Enable loop playback.
     * @param bpm Beats per minute for quantization.
     */
    void configureLoop(std::size_t startFrame, std::size_t endFrame, bool enabled, float bpm);
    /**
     * @brief Enable/disable slip mode (vinyl-style rewind without resetting playhead).
     * @param enabled Enable slip mode.
     */
    void setSlipMode(bool enabled);
    /**
     * @brief Enable/disable vinyl simulation effects.
     * @param enabled Enable vinyl mode.
     */
    void setVinylMode(bool enabled);
    
    // Phase 17: Scratch control for vinyl mode
    /**
     * @brief Set scratch velocity for vinyl mode.
     * @param velocity Scratch speed multiplier (0.0–2.0+).
     */
    void setScratchVelocity(float velocity);

    // Phase 4: Tempo ramping
    /**
     * @brief Enable/disable tempo ramp (smooth tempo transitions).
     * @param enabled Enable tempo ramping.
     */
    void setTempoRampEnabled(bool enabled);
    /**
     * @brief Set target tempo for ramping.
     * @param percent Target tempo offset percent.
     */
    void setTargetTempo(float percent);
    /**
     * @brief Set tempo ramp rate (speed of tempo transition).
     * @param rate Ramp rate per frame.
     */
    void setTempoRampRate(float rate);

    // Phase 38: Phase alignment for auto-sync
    /**
     * @brief Align phase with another deck for synchronization.
     * @param targetDeck Reference deck for phase alignment.
     * @param bpmA Current BPM of target deck.
     * @param bpmB Current BPM of this deck.
     */
    void alignPhaseWithDeck(const Deck& targetDeck, double bpmA, double bpmB);

    // Phase 39: Auto-sync control
    /**
     * @brief Enable auto-sync targeting another deck.
     * @param target Pointer to target deck for sync (null to disable).
     */
    void setAutoSyncTarget(Deck* target);
    /**
     * @brief Disable auto-sync control.
     */
    void disableAutoSync();
    /**
     * @brief Check if sync is currently enabled.
     * @return true if auto-sync is active.
     */
    bool isSyncEnabled() const;
    /**
     * @brief Jump forward or backward by N beats.
     * @param beats Number of beats (positive = forward, negative = backward).
     */
    void beatJump(int beats);
    
    // Phase 41: Sync state introspection
    /**
     * @brief Get current synchronization state.
     * @return SyncState enum (Off, Initializing, Locked, or Drifting).
     */
    SyncState getSyncState() const;
    /**
     * @brief Get current phase offset from target deck.
     * @return Phase offset in samples (positive = ahead, negative = behind).
     */
    double getPhaseOffset() const;

    // Phase 41: BPM warp (micro-tune)
    /**
     * @brief Apply micro-tuning warp (fine pitch adjustment).
     * @param warpPercent Pitch offset in percent (e.g., 0.01 = +0.01%, range ±0.5%).
     */
    void setWarp(float warpPercent);
    /**
     * @brief Get current micro-tuning warp.
     * @return Current warp offset in percent.
     */
    float getWarp() const;
    /**
     * @brief Clear micro-tuning warp (reset to 0%).
     */
    void clearWarp();

    /**
     * @brief Advance playback by one frame.
     * @return Stereo sample [L, R] after EQ and effects, ready for output.
     */
    std::array<float, 2> nextFrame();
    /**
     * @brief Get recent audio energy level.
     * @return Energy in range [0.0, 1.0].
     */
    float recentEnergy() const;
    /**
     * @brief Get current playback position in frames.
     * @return Frame index in loaded clip.
     */
    std::size_t currentFrame() const;
    /**
     * @brief Return estimated or detected BPM for the loaded clip.
     * @return BPM value, or 0 if not available.
     */
    float getBPM() const;

    // Phase 3: Spectrum analyzer access for bass clash detection
    /**
     * @brief Access the associated SpectrumAnalyzer for real-time spectrum data.
     * @return Const pointer to SpectrumAnalyzer (never null after loadClip).
     */
    const SpectrumAnalyzer* getSpectrumAnalyzer() const;

    // Phase 5: Visual feedback sync - get EQ state
    /**
     * @brief Get current 3-band EQ gains.
     * @return EQState struct with low, mid, high gain values.
     */
    EQState getEQ() const;

private:
    void advanceHeads(double step);
    std::array<float, 2> applyFilter(const std::array<float, 2>& input);
    // Phase 4: Quantize frame to nearest beat given BPM and sample rate
    std::size_t quantizeFrameToBeat(std::size_t frame, float bpm) const;

    AudioClip clip_;
    ThreeBandEQ eq_;

    double playbackHead_ = 0.0;
    double slipHead_ = 0.0;
    std::size_t cueFrame_ = 0;

    std::size_t loopStart_ = 0;
    std::size_t loopEnd_ = 0;
    bool loopEnabled_ = false;

    bool playing_ = false;
    bool slipMode_ = false;
    bool vinylMode_ = false;

    bool isolatorLow_ = false;
    bool isolatorMid_ = false;
    bool isolatorHigh_ = false;
    int filterOrder_ = 1;  // 1 = single-pole (default), 2 = Butterworth

    int outputSampleRate_ = 44100;
    float tempoPercent_ = 0.0f;
    int loopBeats_ = 16;
    float trim_ = 1.0f;
    float filterCutoff_ = 1.0f;

    float lowPassLeft_ = 0.0f;
    float lowPassRight_ = 0.0f;
    float recentEnergy_ = 0.0f;

    // Butterworth filter state (for 2nd-order filter)
    // Input history (x[n-1], x[n-2])
    float butterworthZ1Left_ = 0.0f;
    float butterworthZ2Left_ = 0.0f;
    float butterworthZ1Right_ = 0.0f;
    float butterworthZ2Right_ = 0.0f;
    // Output history (y[n-1], y[n-2])
    float butterworthYZ1Left_ = 0.0f;
    float butterworthYZ2Left_ = 0.0f;
    float butterworthYZ1Right_ = 0.0f;
    float butterworthYZ2Right_ = 0.0f;

    // Phase 4: Tempo ramping
    float targetTempo_ = 0.0f;
    float tempoRampRate_ = 0.0f;
    bool tempoRampEnabled_ = false;

    // Phase 4: Multi-cue hotspot banks (3 banks: A, B, C = indices 0, 1, 2)
    std::array<std::size_t, 3> cuePoints_ = {0, 0, 0};
    int activeCueBank_ = 0;

    // Phase 15: Effect chain processing
    std::shared_ptr<EffectChain> effectChain_;
    float effectSendLevel_ = 0.0f;

    // Phase 17: Vinyl simulation and scratch detection
    std::shared_ptr<VinylSimulator> vinylSimulator_;
    std::shared_ptr<ScratchDetector> scratchDetector_;
    float scratchVelocity_ = 0.0f;  // Current scratch velocity from user input

    // Phase 39: Auto-sync control
    std::shared_ptr<SyncController> syncController_;
    Deck* autoSyncTarget_ = nullptr;
    bool syncEnabled_ = false;

    // Phase 41: BPM warp (micro-adjustment)
    float warpAmount_ = 0.0f;  // Temporary BPM micro-adjustment (±0.05% max)

    // Phase 3: Spectrum analyzer for bass clash detection
    std::unique_ptr<SpectrumAnalyzer> spectrumAnalyzer_;
};

} // namespace dj
