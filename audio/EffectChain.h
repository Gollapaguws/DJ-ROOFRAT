#pragma once

#include <array>
#include <memory>
#include <vector>
#include <functional>

namespace dj {

class Reverb;
class Delay;

/**
 * @brief Serial or parallel effect chain attached to a deck.
 *
 * Allows routing audio through multiple effects with per-effect wet/dry mixing.
 * Supports serial (effects in sequence) or parallel (effects in parallel, then mixed)
 * processing topologies.
 */
class EffectChain {
public:
    /**
     * @brief Processing topology.
     */
    enum class Mode {
        Serial,    // audio → effect1 → effect2 → ... → output
        Parallel   // audio → [effect1, effect2, ...] → mix → output
    };
    
    /**
     * @brief Construct effect chain (default serial mode).
     */
    EffectChain();
    
    /**
     * @brief Set serial or parallel processing topology.
     * @param mode Processing mode (Serial or Parallel).
     */
    void setMode(Mode mode);
    /**
     * @brief Get current processing mode.
     * @return Current Mode (Serial or Parallel).
     */
    Mode mode() const;
    
    // Add effect to chain with per-effect wet/dry mix
    /**
     * @brief Add a reverb effect slot with per-effect wet/dry mix.
     * @param reverb Reverb instance to add.
     * @param wetDryMix Wet/dry mix: 0.0=dry (no effect), 1.0=full wet (effect only).
     */
    void addEffect(std::shared_ptr<Reverb> reverb, float wetDryMix);
    /**
     * @brief Add a delay effect slot with per-effect wet/dry mix.
     * @param delay Delay instance to add.
     * @param wetDryMix Wet/dry mix: 0.0=dry (no effect), 1.0=full wet (effect only).
     */
    void addEffect(std::shared_ptr<Delay> delay, float wetDryMix);
    
    /**
     * @brief Process one stereo frame through the chain.
     * @param input Stereo input sample [L, R].
     * @return Processed stereo output after all effects.
     */
    std::array<float, 2> process(const std::array<float, 2>& input);
    
    /**
     * @brief Remove all effect slots from the chain.
     */
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
