#pragma once

#include <string>
#include <vector>
#include <optional>

namespace dj {

struct DeckState {
    std::string trackPath;
    double playbackPosition = 0.0;  // Seconds
    float tempoBend = 0.0f;  // -0.1 to 0.1
    bool isPlaying = false;
    float lowGain = 1.0f;
    float midGain = 1.0f;
    float highGain = 1.0f;
};

struct SessionState {
    DeckState deckA;
    DeckState deckB;
    float crossfader = 0.0f;  // -1.0 (A) to 1.0 (B)
    int currentCareerTier = 1;
    float crowdEnergy = 0.5f;
    std::string venueId;
};

class SessionManager {
public:
    SessionManager();
    ~SessionManager() = default;
    
    // Save/load session
    bool saveSession(const std::string& filePath, const SessionState& state);
    std::optional<SessionState> loadSession(const std::string& filePath);
    
    // Auto-save management
    void enableAutoSave(bool enabled);
    bool isAutoSaveEnabled() const;
    void setAutoSaveInterval(int seconds);
    
private:
    bool autoSaveEnabled_ = false;
    int autoSaveIntervalSeconds_ = 120;
    
    std::string serializeToJSON(const SessionState& state) const;
    std::optional<SessionState> deserializeFromJSON(const std::string& json);
};

} // namespace dj
