#include "core/ConfigManager.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace dj {

ConfigManager::ConfigManager()
    : config_()
{
}

const AppConfig& ConfigManager::getConfig() const {
    return config_;
}

void ConfigManager::setConfig(const AppConfig& config) {
    config_ = config;
    
    // Clamp values to valid ranges
    config_.masterVolume = std::clamp(config_.masterVolume, 0.0f, 1.0f);
    config_.defaultCrossfader = std::clamp(config_.defaultCrossfader, -1.0f, 1.0f);
    config_.autosaveIntervalSeconds = std::clamp(config_.autosaveIntervalSeconds, 10, 3600);
}

int ConfigManager::getSampleRate() const {
    return config_.sampleRate;
}

float ConfigManager::getMasterVolume() const {
    return config_.masterVolume;
}

void ConfigManager::setMasterVolume(float volume) {
    config_.masterVolume = std::clamp(volume, 0.0f, 1.0f);
}

bool ConfigManager::loadConfig(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return deserializeFromJSON(buffer.str());
}

bool ConfigManager::saveConfig(const std::string& filePath) const {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    file << serializeToJSON();
    file.close();
    
    return true;
}

std::string ConfigManager::serializeToJSON() const {
    std::ostringstream oss;
    
    oss << "{\n";
    oss << "  \"sampleRate\": " << config_.sampleRate << ",\n";
    oss << "  \"bufferSize\": " << config_.bufferSize << ",\n";
    oss << "  \"masterVolume\": " << config_.masterVolume << ",\n";
    oss << "  \"enableGraphics\": " << (config_.enableGraphics ? "true" : "false") << ",\n";
    oss << "  \"graphicsWidth\": " << config_.graphicsWidth << ",\n";
    oss << "  \"graphicsHeight\": " << config_.graphicsHeight << ",\n";
    oss << "  \"fullscreen\": " << (config_.fullscreen ? "true" : "false") << ",\n";
    oss << "  \"defaultCrossfader\": " << config_.defaultCrossfader << ",\n";
    oss << "  \"autoGainEnabled\": " << (config_.autoGainEnabled ? "true" : "false") << ",\n";
    oss << "  \"autosaveIntervalSeconds\": " << config_.autosaveIntervalSeconds << "\n";
    oss << "}\n";
    
    return oss.str();
}

bool ConfigManager::deserializeFromJSON(const std::string& json) {
    try {
        // Validate JSON structure - must have opening and closing braces
        if (json.find("{") == std::string::npos || json.find("}") == std::string::npos) {
            return false;
        }
        
        // Parse sampleRate
        {
            size_t pos = json.find("\"sampleRate\":");
            if (pos != std::string::npos) {
                size_t colonPos = json.find(":", pos);
                if (colonPos != std::string::npos) {
                    size_t commaPos = json.find(",", colonPos);
                    if (commaPos == std::string::npos) {
                        return false;  // If no comma, this field couldn't be parsed correctly
                    }
                    std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                    // Trim whitespace
                    valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                    valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                    int val = std::stoi(valueStr);
                    config_.sampleRate = (val > 0) ? val : 44100;
                } else {
                    return false;
                }
            } else {
                return false;  // Required field not found
            }
        }
        
        // Parse bufferSize
        {
            size_t pos = json.find("\"bufferSize\":");
            if (pos != std::string::npos) {
                size_t colonPos = json.find(":", pos);
                if (colonPos != std::string::npos) {
                    size_t commaPos = json.find(",", colonPos);
                    if (commaPos != std::string::npos) {
                        std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                        valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                        valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                        int val = std::stoi(valueStr);
                        config_.bufferSize = (val > 0) ? val : 2048;
                    }
                }
            } else {
                return false;  // Required field not found
            }
        }
        
        // Parse masterVolume
        {
            size_t pos = json.find("\"masterVolume\":");
            if (pos != std::string::npos) {
                size_t colonPos = json.find(":", pos);
                if (colonPos != std::string::npos) {
                    size_t commaPos = json.find(",", colonPos);
                    if (commaPos != std::string::npos) {
                        std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                        valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                        valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                        config_.masterVolume = std::clamp(std::stof(valueStr), 0.0f, 1.0f);
                    }
                }
            }
        }
        
        // Parse enableGraphics
        {
            size_t pos = json.find("\"enableGraphics\":");
            if (pos != std::string::npos) {
                size_t colonPos = json.find(":", pos);
                if (colonPos != std::string::npos) {
                    size_t commaPos = json.find(",", colonPos);
                    size_t endPos = (commaPos != std::string::npos) ? commaPos : json.find("}", colonPos);
                    std::string valueStr = json.substr(colonPos + 1, endPos - colonPos - 1);
                    valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                    valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                    config_.enableGraphics = (valueStr == "true");
                }
            }
        }
        
        // Parse graphicsWidth
        {
            size_t pos = json.find("\"graphicsWidth\":");
            if (pos != std::string::npos) {
                size_t colonPos = json.find(":", pos);
                if (colonPos != std::string::npos) {
                    size_t commaPos = json.find(",", colonPos);
                    if (commaPos != std::string::npos) {
                        std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                        valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                        valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                        int val = std::stoi(valueStr);
                        config_.graphicsWidth = (val > 0) ? val : 1920;
                    }
                }
            }
        }
        
        // Parse graphicsHeight
        {
            size_t pos = json.find("\"graphicsHeight\":");
            if (pos != std::string::npos) {
                size_t colonPos = json.find(":", pos);
                if (colonPos != std::string::npos) {
                    size_t commaPos = json.find(",", colonPos);
                    if (commaPos != std::string::npos) {
                        std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                        valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                        valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                        int val = std::stoi(valueStr);
                        config_.graphicsHeight = (val > 0) ? val : 1080;
                    }
                }
            }
        }
        
        // Parse fullscreen
        {
            size_t pos = json.find("\"fullscreen\":");
            if (pos != std::string::npos) {
                size_t colonPos = json.find(":", pos);
                if (colonPos != std::string::npos) {
                    size_t commaPos = json.find(",", colonPos);
                    size_t endPos = (commaPos != std::string::npos) ? commaPos : json.find("}", colonPos);
                    std::string valueStr = json.substr(colonPos + 1, endPos - colonPos - 1);
                    valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                    valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                    config_.fullscreen = (valueStr == "true");
                }
            }
        }
        
        // Parse defaultCrossfader
        {
            size_t pos = json.find("\"defaultCrossfader\":");
            if (pos != std::string::npos) {
                size_t colonPos = json.find(":", pos);
                if (colonPos != std::string::npos) {
                    size_t commaPos = json.find(",", colonPos);
                    if (commaPos != std::string::npos) {
                        std::string valueStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
                        valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                        valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                        config_.defaultCrossfader = std::clamp(std::stof(valueStr), -1.0f, 1.0f);
                    }
                }
            }
        }
        
        // Parse autoGainEnabled
        {
            size_t pos = json.find("\"autoGainEnabled\":");
            if (pos != std::string::npos) {
                size_t colonPos = json.find(":", pos);
                if (colonPos != std::string::npos) {
                    size_t commaPos = json.find(",", colonPos);
                    size_t endPos = (commaPos != std::string::npos) ? commaPos : json.find("}", colonPos);
                    std::string valueStr = json.substr(colonPos + 1, endPos - colonPos - 1);
                    valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                    valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                    config_.autoGainEnabled = (valueStr == "true");
                }
            }
        }
        
        // Parse autosaveIntervalSeconds
        {
            size_t pos = json.find("\"autosaveIntervalSeconds\":");
            if (pos != std::string::npos) {
                size_t colonPos = json.find(":", pos);
                if (colonPos != std::string::npos) {
                    size_t bracePos = json.find("}", colonPos);
                    if (bracePos != std::string::npos) {
                        std::string valueStr = json.substr(colonPos + 1, bracePos - colonPos - 1);
                        valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
                        valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);
                        int val = std::stoi(valueStr);
                        config_.autosaveIntervalSeconds = std::clamp(val, 10, 3600);
                    }
                }
            }
        }
        
        return true;
    } catch (const std::invalid_argument& e) {
        // Failed to parse configuration value
        return false;
    } catch (const std::out_of_range& e) {
        // Configuration value out of valid range
        return false;
    } catch (const std::exception& e) {
        // Other parsing errors
        return false;
    } catch (...) {
        // Unknown error
        return false;
    }
}

} // namespace dj
