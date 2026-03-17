#pragma once

#include <ctime>
#include <string>
#include <vector>

namespace dj {

struct TrackEntry {
    float timestamp;  // seconds from start
    std::string trackName;
    float bpm = 0.0f;
    std::string key;
};

struct TransitionMarker {
    float timestamp;  // seconds from start
    std::string type;  // "crossfade_smooth", "crossfade_tight", etc.
};

struct CueMarker {
    double timestamp;  // seconds from start
    char deck;         // 'A' or 'B'
    int bank;          // 0, 1, or 2 (banks A, B, C)
    std::size_t frame; // frame position in audio
};

class SessionMetadata {
public:
    SessionMetadata();
    ~SessionMetadata() = default;
    
    // Set recording metadata
    void setRecordingName(const std::string& name);
    void setStartTime(std::time_t time);
    void setDuration(float seconds);
    void setArtist(const std::string& artist);
    void setVenue(const std::string& venue);
    
    // Add track entry (timestamp, name, BPM, optional key)
    void addTrack(float timestamp, const std::string& trackName, float bpm = 0.0f, const std::string& key = "");
    
    // Add transition marker
    void addTransition(float timestamp, const std::string& type);
    
    // Add cue marker (Phase 29)
    void addCueMarker(double timestamp, char deck, int bank, std::size_t frame);
    
    // Get metadata as JSON string
    std::string toJSON() const;
    
    // Export metadata to file
    bool exportToFile(const std::string& filename) const;
    
    // Import metadata from file
    static SessionMetadata importFromFile(const std::string& filename);
    
private:
    std::string recordingName_;
    std::time_t startTime_;
    float duration_;
    std::string artist_;
    std::string venue_;
    
    std::vector<TrackEntry> tracks_;
    std::vector<TransitionMarker> transitions_;
    std::vector<CueMarker> cueMarkers_;  // Phase 29: Cue point tracking
};

} // namespace dj
