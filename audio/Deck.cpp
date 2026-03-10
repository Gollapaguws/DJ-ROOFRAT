#include "audio/Deck.h"

#include <algorithm>
#include <cmath>

#include "audio/TrackLoader.h"

namespace dj {

bool Deck::loadClip(const AudioClip& clip) {
    if (clip.empty()) {
        return false;
    }

    clip_ = clip;
    eq_.setSampleRate(clip_.sampleRate);
    eq_.reset();

    playbackHead_ = 0.0;
    slipHead_ = 0.0;
    cueFrame_ = 0;
    loopEnabled_ = false;
    loopStart_ = 0;
    loopEnd_ = 0;
    lowPassLeft_ = 0.0f;
    lowPassRight_ = 0.0f;
    recentEnergy_ = 0.0f;
    playing_ = false;
    return true;
}

bool Deck::loadFromFile(const std::string& path, std::string* errorOut) {
    const auto loaded = TrackLoader::loadFile(path, errorOut);
    if (!loaded.has_value()) {
        return false;
    }
    return loadClip(*loaded);
}

bool Deck::hasClip() const {
    return !clip_.empty();
}

const AudioClip* Deck::clip() const {
    if (!hasClip()) {
        return nullptr;
    }
    return &clip_;
}

void Deck::play() {
    if (hasClip()) {
        playing_ = true;
    }
}

void Deck::pause() {
    playing_ = false;
}

void Deck::stop() {
    playing_ = false;
    playbackHead_ = 0.0;
    slipHead_ = 0.0;
}

bool Deck::isPlaying() const {
    return playing_;
}

void Deck::setOutputSampleRate(int sampleRate) {
    outputSampleRate_ = std::max(8000, sampleRate);
}

void Deck::setTempoPercent(float percent) {
    tempoPercent_ = std::clamp(percent, -50.0f, 50.0f);
}

float Deck::tempoPercent() const {
    return tempoPercent_;
}

void Deck::setTrim(float trim) {
    trim_ = std::clamp(trim, 0.0f, 2.0f);
}

void Deck::setEQ(float low, float mid, float high) {
    eq_.setGains(low, mid, high);
}

void Deck::setEQFrequencies(float lowMid, float midHigh) {
    eq_.setLowMidCrossover(lowMid);
    eq_.setMidHighCrossover(midHigh);
}

void Deck::setFilter(float cutoff) {
    filterCutoff_ = std::clamp(cutoff, 0.0f, 1.0f);
}

void Deck::setCue(std::size_t frame) {
    if (!hasClip()) {
        cueFrame_ = 0;
        return;
    }
    cueFrame_ = std::min(frame, clip_.frameCount() - 1U);
}

void Deck::jumpToCue() {
    playbackHead_ = static_cast<double>(cueFrame_);
    if (!slipMode_) {
        slipHead_ = playbackHead_;
    }
}

void Deck::configureLoop(std::size_t startFrame, std::size_t endFrame, bool enabled) {
    if (!hasClip() || endFrame <= startFrame || endFrame > clip_.frameCount()) {
        loopEnabled_ = false;
        return;
    }

    loopStart_ = startFrame;
    loopEnd_ = endFrame;
    loopEnabled_ = enabled;
}

void Deck::setSlipMode(bool enabled) {
    if (enabled && !slipMode_) {
        slipHead_ = playbackHead_;
    }

    if (!enabled && slipMode_) {
        playbackHead_ = slipHead_;
    }

    slipMode_ = enabled;
}

void Deck::setVinylMode(bool enabled) {
    vinylMode_ = enabled;
}

std::array<float, 2> Deck::nextFrame() {
    if (!playing_ || !hasClip()) {
        return {0.0f, 0.0f};
    }

    if (playbackHead_ >= static_cast<double>(clip_.frameCount() - 1U)) {
        playing_ = false;
        return {0.0f, 0.0f};
    }

    const auto dry = clip_.frameAt(playbackHead_);
    auto eqOut = eq_.process(dry);
    eqOut = applyFilter(eqOut);

    eqOut[0] *= trim_;
    eqOut[1] *= trim_;

    const float instantEnergy = 0.5f * ((eqOut[0] * eqOut[0]) + (eqOut[1] * eqOut[1]));
    recentEnergy_ = (recentEnergy_ * 0.92f) + (instantEnergy * 0.08f);

    const double sourceToOutputRate = static_cast<double>(clip_.sampleRate) / static_cast<double>(outputSampleRate_);
    double tempoScale = std::max(0.05, 1.0 + static_cast<double>(tempoPercent_) / 100.0);

    if (vinylMode_ && !playing_) {
        tempoScale = 0.0;
    }

    const double step = sourceToOutputRate * tempoScale;
    advanceHeads(step);
    return eqOut;
}

float Deck::recentEnergy() const {
    return std::clamp(recentEnergy_, 0.0f, 1.0f);
}

std::size_t Deck::currentFrame() const {
    if (!hasClip()) {
        return 0;
    }
    const double clamped = std::clamp(playbackHead_, 0.0, static_cast<double>(clip_.frameCount() - 1U));
    return static_cast<std::size_t>(clamped);
}

void Deck::advanceHeads(double step) {
    if (!hasClip()) {
        playing_ = false;
        return;
    }

    playbackHead_ += step;
    if (slipMode_) {
        slipHead_ += step;
    } else {
        slipHead_ = playbackHead_;
    }

    if (loopEnabled_ && loopEnd_ > loopStart_) {
        const double loopStart = static_cast<double>(loopStart_);
        const double loopEnd = static_cast<double>(loopEnd_);
        while (playbackHead_ >= loopEnd) {
            playbackHead_ = loopStart + (playbackHead_ - loopEnd);
        }
    }

    const double lastFrame = static_cast<double>(clip_.frameCount() - 1U);
    if (slipHead_ > lastFrame) {
        slipHead_ = lastFrame;
    }

    if (!loopEnabled_ && playbackHead_ >= lastFrame) {
        playing_ = false;
        playbackHead_ = lastFrame;
    }
}

std::array<float, 2> Deck::applyFilter(const std::array<float, 2>& input) {
    if (filterCutoff_ >= 0.995f) {
        return input;
    }

    const float alpha = 0.02f + (filterCutoff_ * 0.45f);
    lowPassLeft_ += alpha * (input[0] - lowPassLeft_);
    lowPassRight_ += alpha * (input[1] - lowPassRight_);

    const float dryMix = filterCutoff_;
    const float wetMix = 1.0f - dryMix;

    return {
        (input[0] * dryMix) + (lowPassLeft_ * wetMix),
        (input[1] * dryMix) + (lowPassRight_ * wetMix),
    };
}

} // namespace dj
