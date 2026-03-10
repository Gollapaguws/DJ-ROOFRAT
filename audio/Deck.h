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

    void setTrim(float trim);
    void setEQ(float low, float mid, float high);
    void setEQFrequencies(float lowMid, float midHigh);
    void setFilter(float cutoff);

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

    int outputSampleRate_ = 44100;
    float tempoPercent_ = 0.0f;
    float trim_ = 1.0f;
    float filterCutoff_ = 1.0f;

    float lowPassLeft_ = 0.0f;
    float lowPassRight_ = 0.0f;
    float recentEnergy_ = 0.0f;
};

} // namespace dj
