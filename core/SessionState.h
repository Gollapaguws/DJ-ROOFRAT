#pragma once

#include <string>
#include <vector>
#include <optional>

namespace dj {

/**
 * @brief Persisted state for a single deck.
 *
 * Captures playback position, tempo, EQ, and track path for session save/load.
 */
struct DeckState {
    /// File path to loaded track
    std::string trackPath;
    /// Playback position in seconds
    double playbackPosition = 0.0;
    /// Tempo offset (-0.1 to 0.1, or unclamped)
    float tempoBend = 0.0f;
    /// Playback state
    bool isPlaying = false;
    /// Low EQ band gain
    float lowGain = 1.0f;
    /// Mid EQ band gain
    float midGain = 1.0f;
    /// High EQ band gain
    float highGain = 1.0f;
};

/**
 * @brief Complete persisted state for a DJ session.
 *
 * Captures both decks, mixing parameters, career state, and crowd context
 * for session serialization and restoration.
 */
struct SessionState {
    /// State of Deck A
    DeckState deckA;
    /// State of Deck B
    DeckState deckB;
    /// Crossfader position (-1.0 = A, 0.0 = center, 1.0 = B)
    float crossfader = 0.0f;
    /// Current career tier
    int currentCareerTier = 1;
    /// Crowd energy at session save
    float crowdEnergy = 0.5f;
    /// Venue ID or name
    std::string venueId;
};

/**
 * @brief JSON-based session save/load manager with optional auto-save.
 *
 * Serializes and deserializes DJ session state to/from JSON files. Supports
 * periodic auto-save for recovery after crashes.
 */
class SessionManager {
public:
    /**
     * @brief Construct manager.
     */
    SessionManager();
    /**
     * @brief Destructor.
     */
    ~SessionManager() = default;
    
    // Save/load session
    /**
     * @brief Serialize and write session to JSON file.
     * @param filePath Destination file path.
     * @param state Session state to save.
     * @return true on success, false on write error.
     */
    bool saveSession(const std::string& filePath, const SessionState& state);
    /**
     * @brief Deserialize session from JSON file.
     * @param filePath Source file path.
     * @return Loaded SessionState, or std::nullopt on failure.
     */
    std::optional<SessionState> loadSession(const std::string& filePath);
    
    // Auto-save management
    /**
     * @brief Enable or disable periodic auto-save.
     * @param enabled true to enable auto-save, false to disable.
     */
    void enableAutoSave(bool enabled);
    /**
     * @brief Returns true if auto-save is active.
     * @return Auto-save state.
     */
    bool isAutoSaveEnabled() const;
    /**
     * @brief Set auto-save interval.
     * @param seconds Interval in seconds (default 120).
     */
    void setAutoSaveInterval(int seconds);
    
private:
    bool autoSaveEnabled_ = false;
    int autoSaveIntervalSeconds_ = 120;
    
    std::string serializeToJSON(const SessionState& state) const;
    std::optional<SessionState> deserializeFromJSON(const std::string& json);
};

} // namespace dj
