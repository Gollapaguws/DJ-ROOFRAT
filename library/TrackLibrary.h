#pragma once

#include <optional>
#include <string>
#include <vector>
#include <memory>

#include "audio/TrackMetadata.h"

namespace dj::library {

struct StoredTrack {
    int id;
    std::string path;
    std::string title;
    std::string artist;
    std::optional<float> bpm;
    std::optional<std::string> key;
    float durationSeconds;
    std::string genre;
};

class TrackLibrary {
public:
    TrackLibrary();
    ~TrackLibrary();

    // Initialize database (creates tables if needed)
    bool initialize(const std::string& dbPath, std::string* errorOut = nullptr);

    // CRUD Operations
    bool addTrack(const std::string& path, const TrackMetadata& metadata, int* outId = nullptr, std::string* errorOut = nullptr);
    std::optional<StoredTrack> getTrack(int id, std::string* errorOut = nullptr);
    bool removeTrack(int id, std::string* errorOut = nullptr);

    // Query Operations
    std::vector<StoredTrack> queryByBpm(float minBpm, float maxBpm, std::string* errorOut = nullptr);
    std::vector<StoredTrack> queryByKey(const std::string& key, std::string* errorOut = nullptr);
    std::vector<StoredTrack> queryByArtist(const std::string& artist, std::string* errorOut = nullptr);
    std::vector<StoredTrack> queryByGenre(const std::string& genre, std::string* errorOut = nullptr);
    std::vector<StoredTrack> getAllTracks(std::string* errorOut = nullptr);

    // Utility
    int getTrackCount(std::string* errorOut = nullptr);
    bool clearDatabase(std::string* errorOut = nullptr);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace dj::library
