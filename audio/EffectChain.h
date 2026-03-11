#pragma once

#include <array>
#include <memory>
#include <vector>
#include <functional>

namespace dj {

class Reverb;
class Delay;

class EffectChain {
public:
    enum class Mode {
        Serial,    // audio → effect1 → effect2 → ... → output
        Parallel   // audio → [effect1, effect2, ...] → mix → output
    };
    
    EffectChain();
    
    void setMode(Mode mode);
    Mode mode() const;
    
    // Add effect to chain with per-effect wet/dry mix
    void addEffect(std::shared_ptr<Reverb> reverb, float wetDryMix);
    void addEffect(std::shared_ptr<Delay> delay, float wetDryMix);
    
    std::array<float, 2> process(const std::array<float, 2>& input);
    
    void clear();

private:
    struct EffectSlot {
        enum class Type { Reverb, Delay };
        Type type;
        std::shared_ptr<Reverb> reverb;
        std::shared_ptr<Delay> delay;
        float wetDryMix;
    };
    
    Mode mode_ = Mode::Serial;
    std::vector<EffectSlot> effects_;
};

} // namespace dj
