#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>

namespace dj {

struct EQPreset {
    std::string name;
    float lowGain;
    float midGain;
    float highGain;
};

struct EffectPreset {
    std::string name;
    std::string effectType;  // "reverb", "delay", "filter", etc.
    std::map<std::string, float> parameters;
};

class PresetManager {
public:
    PresetManager();
    ~PresetManager() = default;
    
    // EQ presets
    bool saveEQPreset(const std::string& name, float low, float mid, float high);
    std::optional<EQPreset> loadEQPreset(const std::string& name);
    std::vector<std::string> listEQPresets() const;
    bool deleteEQPreset(const std::string& name);
    
    // Effect presets
    bool saveEffectPreset(const EffectPreset& preset);
    std::optional<EffectPreset> loadEffectPreset(const std::string& name);
    std::vector<std::string> listEffectPresets() const;
    bool deleteEffectPreset(const std::string& name);
    
    // Persistence
    bool saveToFile(const std::string& filePath);
    bool loadFromFile(const std::string& filePath);
    
private:
    std::vector<EQPreset> eqPresets_;
    std::vector<EffectPreset> effectPresets_;
    
    std::string serializeToJSON() const;
    bool deserializeFromJSON(const std::string& json);
};

} // namespace dj
