#include "core/PresetManager.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace dj {

PresetManager::PresetManager()
    : eqPresets_()
    , effectPresets_()
{
}

bool PresetManager::saveEQPreset(const std::string& name, float low, float mid, float high) {
    // Check if already exists, remove it first
    auto it = std::find_if(eqPresets_.begin(), eqPresets_.end(),
        [&name](const EQPreset& p) { return p.name == name; });
    if (it != eqPresets_.end()) {
        eqPresets_.erase(it);
    }
    
    // Add new preset
    eqPresets_.push_back({name, low, mid, high});
    return true;
}

std::optional<EQPreset> PresetManager::loadEQPreset(const std::string& name) {
    auto it = std::find_if(eqPresets_.begin(), eqPresets_.end(),
        [&name](const EQPreset& p) { return p.name == name; });
    
    if (it != eqPresets_.end()) {
        return *it;
    }
    return std::nullopt;
}

std::vector<std::string> PresetManager::listEQPresets() const {
    std::vector<std::string> names;
    for (const auto& preset : eqPresets_) {
        names.push_back(preset.name);
    }
    return names;
}

bool PresetManager::deleteEQPreset(const std::string& name) {
    auto it = std::find_if(eqPresets_.begin(), eqPresets_.end(),
        [&name](const EQPreset& p) { return p.name == name; });
    
    if (it != eqPresets_.end()) {
        eqPresets_.erase(it);
        return true;
    }
    return false;
}

bool PresetManager::saveEffectPreset(const EffectPreset& preset) {
    // Check if already exists, remove it first
    auto it = std::find_if(effectPresets_.begin(), effectPresets_.end(),
        [&preset](const EffectPreset& p) { return p.name == preset.name; });
    if (it != effectPresets_.end()) {
        effectPresets_.erase(it);
    }
    
    // Add new preset
    effectPresets_.push_back(preset);
    return true;
}

std::optional<EffectPreset> PresetManager::loadEffectPreset(const std::string& name) {
    auto it = std::find_if(effectPresets_.begin(), effectPresets_.end(),
        [&name](const EffectPreset& p) { return p.name == name; });
    
    if (it != effectPresets_.end()) {
        return *it;
    }
    return std::nullopt;
}

std::vector<std::string> PresetManager::listEffectPresets() const {
    std::vector<std::string> names;
    for (const auto& preset : effectPresets_) {
        names.push_back(preset.name);
    }
    return names;
}

bool PresetManager::deleteEffectPreset(const std::string& name) {
    auto it = std::find_if(effectPresets_.begin(), effectPresets_.end(),
        [&name](const EffectPreset& p) { return p.name == name; });
    
    if (it != effectPresets_.end()) {
        effectPresets_.erase(it);
        return true;
    }
    return false;
}

bool PresetManager::saveToFile(const std::string& filePath) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    file << serializeToJSON();
    file.close();
    
    return true;
}

bool PresetManager::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return deserializeFromJSON(buffer.str());
}

std::string PresetManager::serializeToJSON() const {
    std::ostringstream oss;
    
    oss << "{\n";
    
    // EQ Presets array
    oss << "  \"eqPresets\": [\n";
    for (std::size_t i = 0; i < eqPresets_.size(); ++i) {
        oss << "    {\n";
        oss << "      \"name\": \"" << eqPresets_[i].name << "\",\n";
        oss << "      \"lowGain\": " << eqPresets_[i].lowGain << ",\n";
        oss << "      \"midGain\": " << eqPresets_[i].midGain << ",\n";
        oss << "      \"highGain\": " << eqPresets_[i].highGain << "\n";
        oss << "    }";
        if (i < eqPresets_.size() - 1) {
            oss << ",";
        }
        oss << "\n";
    }
    oss << "  ],\n";
    
    // Effect Presets array
    oss << "  \"effectPresets\": [\n";
    for (std::size_t i = 0; i < effectPresets_.size(); ++i) {
        oss << "    {\n";
        oss << "      \"name\": \"" << effectPresets_[i].name << "\",\n";
        oss << "      \"effectType\": \"" << effectPresets_[i].effectType << "\",\n";
        oss << "      \"parameters\": {\n";
        
        // Serialize parameters map
        std::size_t paramCount = 0;
        for (const auto& param : effectPresets_[i].parameters) {
            oss << "        \"" << param.first << "\": " << param.second;
            ++paramCount;
            if (paramCount < effectPresets_[i].parameters.size()) {
                oss << ",";
            }
            oss << "\n";
        }
        
        oss << "      }\n";
        oss << "    }";
        if (i < effectPresets_.size() - 1) {
            oss << ",";
        }
        oss << "\n";
    }
    oss << "  ]\n";
    
    oss << "}\n";
    
    return oss.str();
}

bool PresetManager::deserializeFromJSON(const std::string& json) {
    try {
        // Clear existing presets
        eqPresets_.clear();
        effectPresets_.clear();
        
        // Parse EQ Presets array
        {
            size_t eqStart = json.find("\"eqPresets\":");
            if (eqStart != std::string::npos) {
                size_t arrayStart = json.find("[", eqStart);
                size_t arrayEnd = json.find("]", arrayStart);
                
                if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
                    std::string eqArray = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
                    
                    // Find all preset objects
                    size_t objStart = 0;
                    while ((objStart = eqArray.find("{", objStart)) != std::string::npos) {
                        size_t objEnd = eqArray.find("}", objStart);
                        if (objEnd == std::string::npos) break;
                        
                        std::string eqObj = eqArray.substr(objStart, objEnd - objStart + 1);
                        
                        EQPreset preset;
                        
                        // Parse name
                        {
                            size_t pos = eqObj.find("\"name\": \"");
                            if (pos != std::string::npos) {
                                size_t valueStart = pos + 9;  // "name": " = 9 characters
                                size_t quoteEnd = eqObj.find("\"", valueStart);
                                if (quoteEnd != std::string::npos) {
                                    preset.name = eqObj.substr(valueStart, quoteEnd - valueStart);
                                }
                            }
                        }
                        
                        // Parse lowGain
                        {
                            size_t pos = eqObj.find("\"lowGain\": ");
                            if (pos != std::string::npos) {
                                size_t colonPos = eqObj.find(":", pos);
                                if (colonPos != std::string::npos) {
                                    size_t commaPos = eqObj.find(",", colonPos);
                                    if (commaPos != std::string::npos) {
                                        std::string valueStr = eqObj.substr(colonPos + 1, commaPos - colonPos - 1);
                                        valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                                        valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                                        preset.lowGain = std::stof(valueStr);
                                    }
                                }
                            }
                        }
                        
                        // Parse midGain
                        {
                            size_t pos = eqObj.find("\"midGain\": ");
                            if (pos != std::string::npos) {
                                size_t colonPos = eqObj.find(":", pos);
                                if (colonPos != std::string::npos) {
                                    size_t commaPos = eqObj.find(",", colonPos);
                                    if (commaPos != std::string::npos) {
                                        std::string valueStr = eqObj.substr(colonPos + 1, commaPos - colonPos - 1);
                                        valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                                        valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                                        preset.midGain = std::stof(valueStr);
                                    }
                                }
                            }
                        }
                        
                        // Parse highGain
                        {
                            size_t pos = eqObj.find("\"highGain\": ");
                            if (pos != std::string::npos) {
                                size_t colonPos = eqObj.find(":", pos);
                                if (colonPos != std::string::npos) {
                                    size_t commaOrBrace = eqObj.find_first_of(",}", colonPos);
                                    if (commaOrBrace != std::string::npos) {
                                        std::string valueStr = eqObj.substr(colonPos + 1, commaOrBrace - colonPos - 1);
                                        valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                                        valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                                        preset.highGain = std::stof(valueStr);
                                    }
                                }
                            }
                        }
                        
                        eqPresets_.push_back(preset);
                        objStart = objEnd + 1;
                    }
                }
            }
        }
        
        // Parse Effect Presets array
        {
            size_t effectStart = json.find("\"effectPresets\":");
            if (effectStart != std::string::npos) {
                size_t arrayStart = json.find("[", effectStart);
                size_t arrayEnd = json.find("]", arrayStart);
                
                if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
                    std::string effectArray = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
                    
                    // Find all preset objects
                    size_t objStart = 0;
                    while ((objStart = effectArray.find("{", objStart)) != std::string::npos) {
                        size_t objEnd = effectArray.find("}", objStart);
                        if (objEnd == std::string::npos) break;
                        
                        std::string effectObj = effectArray.substr(objStart, objEnd - objStart + 1);
                        
                        EffectPreset preset;
                        
                        // Parse name
                        {
                            size_t pos = effectObj.find("\"name\": \"");
                            if (pos != std::string::npos) {
                                size_t valueStart = pos + 9;  // "name": " = 9 characters
                                size_t quoteEnd = effectObj.find("\"", valueStart);
                                if (quoteEnd != std::string::npos) {
                                    preset.name = effectObj.substr(valueStart, quoteEnd - valueStart);
                                }
                            }
                        }
                        
                        // Parse effectType
                        {
                            size_t pos = effectObj.find("\"effectType\": \"");
                            if (pos != std::string::npos) {
                                size_t valueStart = pos + 15;  // "effectType": " = 15 characters
                                size_t quoteEnd = effectObj.find("\"", valueStart);
                                if (quoteEnd != std::string::npos) {
                                    preset.effectType = effectObj.substr(valueStart, quoteEnd - valueStart);
                                }
                            }
                        }
                        
                        // Parse parameters map
                        {
                            size_t paramStart = effectObj.find("\"parameters\": {");
                            if (paramStart != std::string::npos) {
                                size_t paramObjStart = effectObj.find("{", paramStart);
                                size_t paramObjEnd = effectObj.find("}", paramObjStart);
                                
                                if (paramObjStart != std::string::npos && paramObjEnd != std::string::npos) {
                                    std::string paramStr = effectObj.substr(paramObjStart + 1, paramObjEnd - paramObjStart - 1);
                                    
                                    // Split by quotes to find key-value pairs
                                    size_t paramPos = 0;
                                    while ((paramPos = paramStr.find("\"", paramPos)) != std::string::npos) {
                                        size_t keyEnd = paramStr.find("\"", paramPos + 1);
                                        if (keyEnd == std::string::npos) break;
                                        
                                        std::string key = paramStr.substr(paramPos + 1, keyEnd - paramPos - 1);
                                        
                                        // Find the value
                                        size_t colonPos = paramStr.find(":", keyEnd);
                                        if (colonPos == std::string::npos) break;
                                        
                                        size_t valueEnd = paramStr.find(",", colonPos);
                                        if (valueEnd == std::string::npos) {
                                            valueEnd = paramStr.find("}", colonPos);
                                        }
                                        if (valueEnd == std::string::npos) break;
                                        
                                        std::string valueStr = paramStr.substr(colonPos + 1, valueEnd - colonPos - 1);
                                        valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                                        valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                                        
                                        try {
                                            preset.parameters[key] = std::stof(valueStr);
                                        } catch (...) {
                                            // Skip invalid values
                                        }
                                        
                                        paramPos = keyEnd + 1;
                                    }
                                }
                            }
                        }
                        
                        effectPresets_.push_back(preset);
                        objStart = objEnd + 1;
                    }
                }
            }
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace dj
