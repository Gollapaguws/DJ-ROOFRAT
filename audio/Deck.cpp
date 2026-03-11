#include "audio/Deck.h"

#include <algorithm>
#include <cmath>

#include "audio/TrackLoader.h"
#include "audio/EffectChain.h"
#include "audio/VinylSimulator.h"
#include "audio/ScratchDetector.h"

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
    butterworthZ1Left_ = 0.0f;
    butterworthZ2Left_ = 0.0f;
    butterworthZ1Right_ = 0.0f;
    butterworthZ2Right_ = 0.0f;
    butterworthYZ1Left_ = 0.0f;
    butterworthYZ2Left_ = 0.0f;
    butterworthYZ1Right_ = 0.0f;
    butterworthYZ2Right_ = 0.0f;
    recentEnergy_ = 0.0f;
    playing_ = false;
    
    // Phase 4: Reset tempo ramp and cue state
    targetTempo_ = 0.0f;
    tempoRampRate_ = 0.0f;
    tempoRampEnabled_ = false;
    cuePoints_ = {0, 0, 0};
    activeCueBank_ = 0;
    
    // Phase 17: Initialize vinyl simulator and scratch detector
    if (!vinylSimulator_) {
        vinylSimulator_ = std::make_shared<VinylSimulator>();
    }
    vinylSimulator_->setSampleRate(clip_.sampleRate);
    vinylSimulator_->setMotorOn(false);
    vinylSimulator_->reset();
    
    if (!scratchDetector_) {
        scratchDetector_ = std::make_shared<ScratchDetector>();
    }
    scratchDetector_->setSampleRate(clip_.sampleRate);
    scratchDetector_->reset();
    
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
        
        // Phase 17: Start vinyl motor if in vinyl mode
        if (vinylMode_ && vinylSimulator_) {
            vinylSimulator_->setMotorOn(true);
            vinylSimulator_->setTargetVelocity(1.0f);
        }
    }
}

void Deck::pause() {
    playing_ = false;
}

void Deck::stop() {
    playing_ = false;
    playbackHead_ = 0.0;
    slipHead_ = 0.0;
    
    // Phase 17: Stop vinyl motor if in vinyl mode
    if (vinylMode_ && vinylSimulator_) {
        vinylSimulator_->setMotorOn(false);
    }
}

bool Deck::isPlaying() const {
    return playing_;
}

void Deck::setOutputSampleRate(int sampleRate) {
    outputSampleRate_ = std::max(8000, sampleRate);
}

void Deck::setTempoPercent(float percent) {
    tempoPercent_ = std::clamp(percent, -50.0f, 50.0f);

    // Keep ramp target aligned with direct/manual tempo changes so nudges are not
    // immediately pulled back on the next frame when ramp mode is enabled.
    if (tempoRampEnabled_) {
        targetTempo_ = tempoPercent_;
    }
}

float Deck::tempoPercent() const {
    return tempoPercent_;
}

void Deck::setLoopBeats(int beats) {
    // Only accept valid values: 8, 16, or 32
    if (beats == 8 || beats == 16 || beats == 32) {
        loopBeats_ = beats;
    } else {
        // Clamp to nearest valid value
        if (beats < 8) {
            loopBeats_ = 8;
        } else if (beats < 16) {
            loopBeats_ = 8;
        } else if (beats < 32) {
            loopBeats_ = 16;
        } else {
            loopBeats_ = 32;
        }
    }
}

int Deck::loopBeats() const {
    return loopBeats_;
}

void Deck::setTrim(float trim) {
    trim_ = std::clamp(trim, 0.0f, 2.0f);
}

void Deck::setEQ(float low, float mid, float high) {
    // Apply isolator logic: force 0.0 gain when isolator is enabled for each band
    if (isolatorLow_) {
        low = 0.0f;
    }
    if (isolatorMid_) {
        mid = 0.0f;
    }
    if (isolatorHigh_) {
        high = 0.0f;
    }
    
    eq_.setGains(low, mid, high);
}

void Deck::setEQFrequencies(float lowMid, float midHigh) {
    eq_.setLowMidCrossover(lowMid);
    eq_.setMidHighCrossover(midHigh);
}

void Deck::setFilter(float cutoff) {
    filterCutoff_ = std::clamp(cutoff, 0.0f, 1.0f);
}

void Deck::setIsolatorMode(bool low, bool mid, bool high) {
    isolatorLow_ = low;
    isolatorMid_ = mid;
    isolatorHigh_ = high;
}

bool Deck::isIsolatorEnabled(int band) const {
    if (band == 0) {
        return isolatorLow_;
    }
    if (band == 1) {
        return isolatorMid_;
    }
    if (band == 2) {
        return isolatorHigh_;
    }
    return false;
}

void Deck::setFilterOrder(int order) {
    filterOrder_ = std::clamp(order, 1, 2);
}

int Deck::getFilterOrder() const {
    return filterOrder_;
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

// Phase 4: Multi-cue hotspot - set cue point for a specific bank
void Deck::setCue(std::size_t frame, int bank) {
    if (!hasClip()) {
        return;
    }
    
    // Clamp bank to valid range (0-2)
    const int normalizedBank = std::clamp(bank, 0, 2);
    
    // Clamp frame to valid range
    cuePoints_[normalizedBank] = std::min(frame, clip_.frameCount() - 1U);
}

// Phase 4: Multi-cue hotspot - jump to cue in a specific bank
void Deck::jumpToCue(int bank) {
    // Clamp bank to valid range (0-2)
    const int normalizedBank = std::clamp(bank, 0, 2);
    
    playbackHead_ = static_cast<double>(cuePoints_[normalizedBank]);
    if (!slipMode_) {
        slipHead_ = playbackHead_;
    }
}

// Phase 4: Set the active cue bank
void Deck::setActiveCueBank(int bank) {
    activeCueBank_ = std::clamp(bank, 0, 2);
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

// Phase 4: Configure loop with beat quantization
void Deck::configureLoop(std::size_t startFrame, std::size_t endFrame, bool enabled, float bpm) {
    if (!hasClip() || endFrame <= startFrame || endFrame > clip_.frameCount()) {
        loopEnabled_ = false;
        return;
    }

    // If BPM is provided and valid, quantize the boundaries to beat grid
    if (bpm > 0.0f) {
        startFrame = quantizeFrameToBeat(startFrame, bpm);
        endFrame = quantizeFrameToBeat(endFrame, bpm);
        
        // Ensure valid range after quantization
        if (endFrame <= startFrame) {
            endFrame = startFrame + 1;
        }
    }

    // Additional bounds check after quantization
    if (endFrame > clip_.frameCount()) {
        endFrame = clip_.frameCount();
    }

    loopStart_ = startFrame;
    loopEnd_ = endFrame;
    loopEnabled_ = enabled;
}

// Phase 4: Helper to quantize a frame to the nearest beat
std::size_t Deck::quantizeFrameToBeat(std::size_t frame, float bpm) const {
    if (!hasClip() || bpm <= 0.0f) {
        return frame;
    }

    const double secondsPerBeat = 60.0 / static_cast<double>(bpm);
    const double framesPerBeat = secondsPerBeat * static_cast<double>(clip_.sampleRate);
    
    if (framesPerBeat < 1.0) {
        return frame;  // Prevent division by near-zero
    }

    // Calculate nearest beat boundary: round(frame / framesPerBeat) * framesPerBeat
    const double frameAsDouble = static_cast<double>(frame);
    const std::size_t nearestBeat = static_cast<std::size_t>(
        std::round(frameAsDouble / framesPerBeat)
    );
    const std::size_t quantizedFrame = static_cast<std::size_t>(
        nearestBeat * framesPerBeat
    );

    return quantizedFrame;
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

void Deck::setScratchVelocity(float velocity) {
    scratchVelocity_ = std::clamp(velocity, -1.0f, 1.0f);
}

// Phase 4: Enable/disable tempo ramping
void Deck::setTempoRampEnabled(bool enabled) {
    tempoRampEnabled_ = enabled;
    if (enabled) {
        // When enabling, set target to current tempo for smooth transition
        targetTempo_ = tempoPercent_;
    }
}

// Phase 4: Set target tempo for ramping (will be interpolated to)
void Deck::setTargetTempo(float percent) {
    targetTempo_ = std::clamp(percent, -50.0f, 50.0f);
}

// Phase 4: Set the rate at which tempo ramps (0.0 to 1.0+, reasonable values are 0.001 to 0.1)
void Deck::setTempoRampRate(float rate) {
    // Clamp to reasonable range: prevent negative values and extreme values
    tempoRampRate_ = std::clamp(rate, 0.0f, 1.0f);
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

    // Phase 15: Apply effect chain if configured
    if (effectChain_ && effectSendLevel_ > 0.0f) {
        std::array<float, 2> effected = effectChain_->process(eqOut);
        eqOut[0] = eqOut[0] * (1.0f - effectSendLevel_) + effected[0] * effectSendLevel_;
        eqOut[1] = eqOut[1] * (1.0f - effectSendLevel_) + effected[1] * effectSendLevel_;
    }

    // Phase 4: Apply tempo ramping if enabled
    if (tempoRampEnabled_) {
        // Interpolate tempo toward target using ramp rate
        tempoPercent_ += (targetTempo_ - tempoPercent_) * tempoRampRate_;
        // Clamp to valid range to prevent drift
        tempoPercent_ = std::clamp(tempoPercent_, -50.0f, 50.0f);
    }

    const double sourceToOutputRate = static_cast<double>(clip_.sampleRate) / static_cast<double>(outputSampleRate_);
    double tempoScale = std::max(0.05, 1.0 + static_cast<double>(tempoPercent_) / 100.0);

    // Phase 17: Vinyl mode uses platter velocity instead of fixed tempoScale
    if (vinylMode_) {
        if (vinylSimulator_) {
            // Update vinyl physics (e.g., motor ramp, friction decay)
            // Use sample-accurate timestep: 1 frame = 1 sample
            const float vinylUpdateDt = 1.0f / static_cast<float>(outputSampleRate_);
            
            // If scratch velocity is active, use it to set target velocity
            // Otherwise, use normal forward speed (1.0)
            if (std::abs(scratchVelocity_) > 0.001f) {
                vinylSimulator_->setTargetVelocity(scratchVelocity_);
                
                // Call scratch detector to process input
                if (scratchDetector_) {
                    scratchDetector_->processScratchInput(scratchVelocity_, vinylUpdateDt);
                }
            } else {
                // No scratch input - use normal motor speed
                vinylSimulator_->setTargetVelocity(1.0f);
            }
            
            vinylSimulator_->update(vinylUpdateDt);
            
            // Use platter velocity directly as tempo scale
            tempoScale = vinylSimulator_->platterVelocity();
        }
        
        if (!playing_) {
            tempoScale = 0.0;
        }
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

    if (filterOrder_ == 2) {
        // 2nd-order Butterworth filter using biquad architecture
        // Normalized cutoff frequency (0.0 to 1.0 maps to 0 to Nyquist)
        const float wc = filterCutoff_ * 3.14159265f;  // Maps to 0-pi
        const float c = std::cos(wc);
        const float s = std::sin(wc);
        const float alpha = s / (2.0f * 0.7071f);  // 0.7071 = sqrt(2)/2 for Butterworth Q
        
        // Biquad coefficients for lowpass
        const float b0 = (1.0f - c) / 2.0f;
        const float b1 = 1.0f - c;
        const float b2 = (1.0f - c) / 2.0f;
        const float a0 = 1.0f + alpha;
        const float a1 = -2.0f * c;
        const float a2 = 1.0f - alpha;
        
        // Normalized coefficients
        const float b0_norm = b0 / a0;
        const float b1_norm = b1 / a0;
        const float b2_norm = b2 / a0;
        const float a1_norm = a1 / a0;
        const float a2_norm = a2 / a0;
        
        // Process left channel (Direct Form I biquad)
        const float outLeft = b0_norm * input[0] + b1_norm * butterworthZ1Left_ + b2_norm * butterworthZ2Left_
                             - a1_norm * butterworthYZ1Left_ - a2_norm * butterworthYZ2Left_;
        butterworthZ2Left_ = butterworthZ1Left_;
        butterworthZ1Left_ = input[0];
        butterworthYZ2Left_ = butterworthYZ1Left_;
        butterworthYZ1Left_ = outLeft;
        lowPassLeft_ = outLeft;
        
        // Process right channel (Direct Form I biquad)
        const float outRight = b0_norm * input[1] + b1_norm * butterworthZ1Right_ + b2_norm * butterworthZ2Right_
                              - a1_norm * butterworthYZ1Right_ - a2_norm * butterworthYZ2Right_;
        butterworthZ2Right_ = butterworthZ1Right_;
        butterworthZ1Right_ = input[1];
        butterworthYZ2Right_ = butterworthYZ1Right_;
        butterworthYZ1Right_ = outRight;
        lowPassRight_ = outRight;
        
        const float dryMix = filterCutoff_;
        const float wetMix = 1.0f - dryMix;
        
        return {
            (input[0] * dryMix) + (outLeft * wetMix),
            (input[1] * dryMix) + (outRight * wetMix),
        };
    } else {
        // Single-pole filter (default, order 1)
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
}

// Phase 15: Effect chain integration
void Deck::setEffectChain(std::shared_ptr<EffectChain> effectChain) {
    effectChain_ = effectChain;
}

void Deck::setEffectSendLevel(float sendLevel) {
    effectSendLevel_ = std::clamp(sendLevel, 0.0f, 1.0f);
}

} // namespace dj
