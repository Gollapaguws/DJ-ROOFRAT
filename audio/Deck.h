#pragma once

#include <array>
#include <cstddef>
#include <string>

#include "audio/AudioClip.h"
#include "audio/ThreeBandEQ.h"

namespace dj {

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

    void setCue(std::size_t frame);
    void jumpToCue();

    void configureLoop(std::size_t startFrame, std::size_t endFrame, bool enabled);
    void setSlipMode(bool enabled);
    void setVinylMode(bool enabled);

    std::array<float, 2> nextFrame();
    float recentEnergy() const;
    std::size_t currentFrame() const;

private:
    void advanceHeads(double step);
    std::array<float, 2> applyFilter(const std::array<float, 2>& input);

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
};

} // namespace dj
