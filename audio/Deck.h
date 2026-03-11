#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <string>

#include "audio/AudioClip.h"
#include "audio/ThreeBandEQ.h"

namespace dj {

class EffectChain;
class VinylSimulator;
class ScratchDetector;

class Deck {
public:
    bool loadClip(const AudioClip& clip);
    bool loadFromFile(const std::string& path, std::string* errorOut = nullptr);

    bool hasClip() const;
    const AudioClip* clip() const;

    void play();
    void pause();
    void stop();
    bool isPlaying() const;

    void setOutputSampleRate(int sampleRate);
    void setTempoPercent(float percent);
    float tempoPercent() const;

    void setLoopBeats(int beats);
    int loopBeats() const;

    void setTrim(float trim);
    void setEQ(float low, float mid, float high);
    void setEQFrequencies(float lowMid, float midHigh);
    void setFilter(float cutoff);
    void setIsolatorMode(bool low, bool mid, bool high);
    bool isIsolatorEnabled(int band) const;
    void setFilterOrder(int order);
    int getFilterOrder() const;

    // Phase 15: Effect chain integration
    void setEffectChain(std::shared_ptr<EffectChain> effectChain);
    void setEffectSendLevel(float sendLevel);  // 0.0 = no send, 1.0 = full send

    void setCue(std::size_t frame);
    void jumpToCue();
    // Phase 4: Multi-cue hotspot banks (A, B, C = banks 0, 1, 2)
    void setCue(std::size_t frame, int bank);
    void jumpToCue(int bank);
    void setActiveCueBank(int bank);

    void configureLoop(std::size_t startFrame, std::size_t endFrame, bool enabled);
    // Phase 4: Quantized loop configuration with BPM
    void configureLoop(std::size_t startFrame, std::size_t endFrame, bool enabled, float bpm);
    void setSlipMode(bool enabled);
    void setVinylMode(bool enabled);
    
    // Phase 17: Scratch control for vinyl mode
    void setScratchVelocity(float velocity);

    // Phase 4: Tempo ramping
    void setTempoRampEnabled(bool enabled);
    void setTargetTempo(float percent);
    void setTempoRampRate(float rate);

    std::array<float, 2> nextFrame();
    float recentEnergy() const;
    std::size_t currentFrame() const;

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
};

} // namespace dj
