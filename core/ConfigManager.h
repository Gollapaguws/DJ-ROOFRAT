#pragma once

#include <string>
#include <optional>
#include <filesystem>

namespace dj {

struct AppConfig {
    // Audio settings
    int sampleRate = 44100;
    int bufferSize = 2048;
    float masterVolume = 0.8f;
    
    // Visual settings
    bool enableGraphics = true;
    int graphicsWidth = 1920;
    int graphicsHeight = 1080;
    bool fullscreen = false;
    
    // Gameplay settings
    float defaultCrossfader = 0.0f;  // -1.0 to 1.0
    bool autoGainEnabled = true;
    int autosaveIntervalSeconds = 120;  // 2 minutes default
};

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager() = default;
    
    // Load/save application config
    bool loadConfig(const std::string& filePath);
    bool saveConfig(const std::string& filePath) const;
    
    // Accessors
    const AppConfig& getConfig() const;
    void setConfig(const AppConfig& config);
    
    // Quick accessors for common settings
    int getSampleRate() const;
    float getMasterVolume() const;
    void setMasterVolume(float volume);
    
private:
    AppConfig config_;
    std::string serializeToJSON() const;
    bool deserializeFromJSON(const std::string& json);
};

} // namespace dj
