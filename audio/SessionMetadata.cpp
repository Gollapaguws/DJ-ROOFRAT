#include "audio/SessionMetadata.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace dj {

SessionMetadata::SessionMetadata()
    : startTime_(0)
    , duration_(0.0f)
{
}

void SessionMetadata::setRecordingName(const std::string& name) {
    recordingName_ = name;
}

void SessionMetadata::setStartTime(std::time_t time) {
    startTime_ = time;
}

void SessionMetadata::setDuration(float seconds) {
    duration_ = seconds;
}

void SessionMetadata::setArtist(const std::string& artist) {
    artist_ = artist;
}

void SessionMetadata::setVenue(const std::string& venue) {
    venue_ = venue;
}

void SessionMetadata::addTrack(float timestamp, const std::string& trackName, float bpm, const std::string& key) {
    tracks_.push_back({timestamp, trackName, bpm, key});
}

void SessionMetadata::addTransition(float timestamp, const std::string& type) {
    transitions_.push_back({timestamp, type});
}

std::string SessionMetadata::toJSON() const {
    std::ostringstream json;
    
    json << "{\n";
    json << "  \"recording_name\": \"" << recordingName_ << "\",\n";
    
    // Format timestamp
    char timeStr[100];
    std::time_t t = startTime_;
    std::tm* tm_info = std::localtime(&t);
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
    json << "  \"start_time\": \"" << timeStr << "\",\n";
    
    json << "  \"duration_seconds\": " << duration_ << ",\n";
    
    if (!artist_.empty()) {
        json << "  \"artist\": \"" << artist_ << "\",\n";
    }
    
    if (!venue_.empty()) {
        json << "  \"venue\": \"" << venue_ << "\",\n";
    }
    
    // Track list
    json << "  \"tracks\": [\n";
    for (std::size_t i = 0; i < tracks_.size(); ++i) {
        const auto& track = tracks_[i];
        json << "    {\n";
        json << "      \"timestamp\": " << track.timestamp << ",\n";
        json << "      \"name\": \"" << track.trackName << "\"";
        
        if (track.bpm > 0.0f) {
            json << ",\n      \"bpm\": " << track.bpm;
        }
        
        if (!track.key.empty()) {
            json << ",\n      \"key\": \"" << track.key << "\"";
        }
        
        json << "\n    }";
        if (i < tracks_.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    json << "  ],\n";
    
    // Transitions
    json << "  \"transitions\": [\n";
    for (std::size_t i = 0; i < transitions_.size(); ++i) {
        const auto& trans = transitions_[i];
        json << "    {\n";
        json << "      \"timestamp\": " << trans.timestamp << ",\n";
        json << "      \"type\": \"" << trans.type << "\"\n";
        json << "    }";
        if (i < transitions_.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    json << "  ]\n";
    
    json << "}\n";
    
    return json.str();
}

bool SessionMetadata::exportToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << toJSON();
    file.close();
    
    return true;
}

SessionMetadata SessionMetadata::importFromFile(const std::string& filename) {
    SessionMetadata metadata;
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        return metadata;
    }
    
    // For now, just read the file without parsing JSON
    // Full JSON parsing would require additional dependencies
    // In production, this would use a proper JSON library
    
    std::string line;
    while (std::getline(file, line)) {
        // Basic parsing for recording name
        if (line.find("recording_name") != std::string::npos) {
            auto start = line.find("\"") + 1;
            auto end = line.rfind("\"");
            if (start > 0 && end != std::string::npos) {
                metadata.recordingName_ = line.substr(start, end - start);
            }
        }
        
        // Basic parsing for duration
        if (line.find("duration_seconds") != std::string::npos) {
            auto colonPos = line.find(":");
            if (colonPos != std::string::npos) {
                std::string valueStr = line.substr(colonPos + 1);
                try {
                    metadata.duration_ = std::stof(valueStr);
                } catch (...) {
                    // Ignore parse errors
                }
            }
        }
    }
    
    file.close();
    return metadata;
}

} // namespace dj
