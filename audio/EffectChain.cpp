#include "audio/EffectChain.h"
#include "audio/Reverb.h"
#include "audio/Delay.h"

#include <algorithm>
#include <cmath>

namespace dj {

EffectChain::EffectChain() = default;

void EffectChain::setMode(Mode mode) {
    mode_ = mode;
}

EffectChain::Mode EffectChain::mode() const {
    return mode_;
}

void EffectChain::addEffect(std::shared_ptr<Reverb> reverb, float wetDryMix) {
    effects_.push_back({
        EffectSlot::Type::Reverb,
        reverb,
        nullptr,
        std::clamp(wetDryMix, 0.0f, 1.0f)
    });
}

void EffectChain::addEffect(std::shared_ptr<Delay> delay, float wetDryMix) {
    effects_.push_back({
        EffectSlot::Type::Delay,
        nullptr,
        delay,
        std::clamp(wetDryMix, 0.0f, 1.0f)
    });
}

std::array<float, 2> EffectChain::process(const std::array<float, 2>& input) {
    if (effects_.empty()) {
        return input;
    }
    
    if (mode_ == Mode::Serial) {
        // Serial: audio → effect1 → effect2 → ...
        std::array<float, 2> signal = input;
        
        for (const auto& slot : effects_) {
            std::array<float, 2> output = {0.0f, 0.0f};
            
            if (slot.type == EffectSlot::Type::Reverb && slot.reverb) {
                // Apply reverb with per-effect wet/dry mix
                std::array<float, 2> wetSignal = slot.reverb->process(signal);
                output[0] = signal[0] * (1.0f - slot.wetDryMix) + wetSignal[0] * slot.wetDryMix;
                output[1] = signal[1] * (1.0f - slot.wetDryMix) + wetSignal[1] * slot.wetDryMix;
            } else if (slot.type == EffectSlot::Type::Delay && slot.delay) {
                // Apply delay with per-effect wet/dry mix
                std::array<float, 2> wetSignal = slot.delay->process(signal);
                output[0] = signal[0] * (1.0f - slot.wetDryMix) + wetSignal[0] * slot.wetDryMix;
                output[1] = signal[1] * (1.0f - slot.wetDryMix) + wetSignal[1] * slot.wetDryMix;
            }
            
            signal = output;
        }
        
        return signal;
    } 
    else if (mode_ == Mode::Parallel) {
        // Parallel: audio → [effect1, effect2, ...] → mix
        std::array<float, 2> wetMix = {0.0f, 0.0f};
        float totalWetAmount = 0.0f;
        
        for (const auto& slot : effects_) {
            if (slot.type == EffectSlot::Type::Reverb && slot.reverb) {
                std::array<float, 2> wetSignal = slot.reverb->process(input);
                wetMix[0] += wetSignal[0] * slot.wetDryMix;
                wetMix[1] += wetSignal[1] * slot.wetDryMix;
                totalWetAmount += slot.wetDryMix;
            } else if (slot.type == EffectSlot::Type::Delay && slot.delay) {
                std::array<float, 2> wetSignal = slot.delay->process(input);
                wetMix[0] += wetSignal[0] * slot.wetDryMix;
                wetMix[1] += wetSignal[1] * slot.wetDryMix;
                totalWetAmount += slot.wetDryMix;
            }
        }
        
        // Calculate dry portion (inverse of total wet amount)
        float dryAmount = 1.0f - std::clamp(totalWetAmount, 0.0f, 1.0f);
        
        return {
            input[0] * dryAmount + wetMix[0],
            input[1] * dryAmount + wetMix[1]
        };
    }
    
    return input;
}

void EffectChain::clear() {
    effects_.clear();
}

} // namespace dj
