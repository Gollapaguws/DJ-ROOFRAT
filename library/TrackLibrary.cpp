#include "library/TrackLibrary.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

namespace dj::library {

class TrackLibrary::Impl {
public:
    std::map<int, StoredTrack> tracks;
    int nextId = 1;
    std::string dbPath;

    int addTrackImpl(const std::string& path, const TrackMetadata& metadata) {
        StoredTrack track;
        track.id = nextId;
        track.path = path;
        track.title = metadata.title;
        track.artist = metadata.artist;
        track.bpm = metadata.bpm;
        track.key = metadata.key;
        track.durationSeconds = metadata.durationSeconds;
        track.genre = metadata.genre;

        tracks[nextId] = track;
        return nextId++;
    }

    std::optional<StoredTrack> getTrackImpl(int id) {
        auto it = tracks.find(id);
        if (it != tracks.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    bool removeTrackImpl(int id) {
        return tracks.erase(id) > 0;
    }

    std::vector<StoredTrack> getAllTracksImpl() {
        std::vector<StoredTrack> result;
        for (const auto& [id, track] : tracks) {
            result.push_back(track);
        }
        return result;
    }

    std::vector<StoredTrack> queryByBpmImpl(float minBpm, float maxBpm) {
        std::vector<StoredTrack> result;
        for (const auto& [id, track] : tracks) {
            if (track.bpm.has_value()) {
                float bpm = track.bpm.value();
                if (bpm >= minBpm && bpm <= maxBpm) {
                    result.push_back(track);
                }
            }
        }
        return result;
    }

    std::vector<StoredTrack> queryByKeyImpl(const std::string& key) {
        std::vector<StoredTrack> result;
        for (const auto& [id, track] : tracks) {
            if (track.key.has_value() && track.key.value() == key) {
                result.push_back(track);
            }
        }
        return result;
    }

    std::vector<StoredTrack> queryByArtistImpl(const std::string& artist) {
        std::vector<StoredTrack> result;
        for (const auto& [id, track] : tracks) {
            if (track.artist == artist) {
                result.push_back(track);
            }
        }
        return result;
    }

    std::vector<StoredTrack> queryByGenreImpl(const std::string& genre) {
        std::vector<StoredTrack> result;
        for (const auto& [id, track] : tracks) {
            if (track.genre == genre) {
                result.push_back(track);
            }
        }
        return result;
    }

    int getTrackCountImpl() {
        return static_cast<int>(tracks.size());
    }

    void clearDatabaseImpl() {
        tracks.clear();
        nextId = 1;
    }
};

// ============================================================================
// TrackLibrary Implementation
// ============================================================================

TrackLibrary::TrackLibrary() : pImpl(std::make_unique<Impl>()) {}

TrackLibrary::~TrackLibrary() = default;

bool TrackLibrary::initialize(const std::string& dbPath, std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: implementation not initialized";
        }
        return false;
    }

    pImpl->dbPath = dbPath;

    // In production, this would create SQLite database and tables
    // For now, we initialize the in-memory storage
    pImpl->tracks.clear();
    pImpl->nextId = 1;

    return true;
}

bool TrackLibrary::addTrack(const std::string& path, const TrackMetadata& metadata, int* outId, std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: implementation not initialized";
        }
        return false;
    }

    if (path.empty() || metadata.title.empty()) {
        if (errorOut) {
            *errorOut = "Path and title cannot be empty";
        }
        return false;
    }

    int id = pImpl->addTrackImpl(path, metadata);
    if (outId) {
        *outId = id;
    }

    return true;
}

std::optional<StoredTrack> TrackLibrary::getTrack(int id, std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: implementation not initialized";
        }
        return std::nullopt;
    }

    return pImpl->getTrackImpl(id);
}

bool TrackLibrary::removeTrack(int id, std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: implementation not initialized";
        }
        return false;
    }

    return pImpl->removeTrackImpl(id);
}

std::vector<StoredTrack> TrackLibrary::queryByBpm(float minBpm, float maxBpm, std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: implementation not initialized";
        }
        return {};
    }

    return pImpl->queryByBpmImpl(minBpm, maxBpm);
}

std::vector<StoredTrack> TrackLibrary::queryByKey(const std::string& key, std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: implementation not initialized";
        }
        return {};
    }

    return pImpl->queryByKeyImpl(key);
}

std::vector<StoredTrack> TrackLibrary::queryByArtist(const std::string& artist, std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: implementation not initialized";
        }
        return {};
    }

    return pImpl->queryByArtistImpl(artist);
}

std::vector<StoredTrack> TrackLibrary::queryByGenre(const std::string& genre, std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: implementation not initialized";
        }
        return {};
    }

    return pImpl->queryByGenreImpl(genre);
}

std::vector<StoredTrack> TrackLibrary::getAllTracks(std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: implementation not initialized";
        }
        return {};
    }

    return pImpl->getAllTracksImpl();
}

int TrackLibrary::getTrackCount(std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: implementation not initialized";
        }
        return 0;
    }

    return pImpl->getTrackCountImpl();
}

bool TrackLibrary::clearDatabase(std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: implementation not initialized";
        }
        return false;
    }

    pImpl->clearDatabaseImpl();
    return true;
}

} // namespace dj::library
