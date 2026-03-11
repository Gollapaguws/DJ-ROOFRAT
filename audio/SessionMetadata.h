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
};

} // namespace dj
