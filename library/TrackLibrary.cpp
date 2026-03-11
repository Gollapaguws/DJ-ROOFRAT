#include "library/TrackLibrary.h"

#include <algorithm>
#include <map>
#include <memory>
#include <vector>

namespace dj::library {

class TrackLibrary::Impl {
public:
    std::map<int, StoredTrack> tracks_;
    int nextId_ = 1;
    std::string dbPath_;

    ~Impl() = default;

    bool initializeDatabase(std::string* errorOut) {
        (void)errorOut;
        // In-memory implementation: nothing to initialize
        return true;
    }

    int addTrackImpl(const std::string& path, const TrackMetadata& metadata, std::string* errorOut) {
        (void)errorOut;
        StoredTrack track;
        track.id = nextId_++;
        track.path = path;
        track.title = metadata.title;
        track.artist = metadata.artist;
        track.bpm = metadata.bpm;
        track.key = metadata.key;
        track.durationSeconds = metadata.durationSeconds;
        track.genre = metadata.genre;
        tracks_[track.id] = track;
        return track.id;
    }

    std::optional<StoredTrack> getTrackImpl(int id, std::string* errorOut) {
        (void)errorOut;
        auto it = tracks_.find(id);
        if (it != tracks_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    bool removeTrackImpl(int id, std::string* errorOut) {
        (void)errorOut;
        return tracks_.erase(id) > 0;
    }

    std::vector<StoredTrack> getAllTracksImpl(std::string* errorOut) {
        (void)errorOut;
        std::vector<StoredTrack> result;
        result.reserve(tracks_.size());
        for (const auto& [id, track] : tracks_) {
            result.push_back(track);
        }
        return result;
    }

    std::vector<StoredTrack> queryByBpmImpl(float minBpm, float maxBpm, std::string* errorOut) {
        (void)errorOut;
        std::vector<StoredTrack> result;
        for (const auto& [id, track] : tracks_) {
            if (track.bpm.has_value()) {
                float bpm = track.bpm.value();
                if (bpm >= minBpm && bpm <= maxBpm) {
                    result.push_back(track);
                }
            }
        }
        std::sort(result.begin(), result.end(), [](const StoredTrack& a, const StoredTrack& b) {
            if (!a.bpm.has_value()) return false;
            if (!b.bpm.has_value()) return true;
            return a.bpm.value() < b.bpm.value();
        });
        return result;
    }

    std::vector<StoredTrack> queryByKeyImpl(const std::string& key, std::string* errorOut) {
        (void)errorOut;
        std::vector<StoredTrack> result;
        for (const auto& [id, track] : tracks_) {
            if (track.key.has_value() && track.key.value() == key) {
                result.push_back(track);
            }
        }
        return result;
    }

    std::vector<StoredTrack> queryByArtistImpl(const std::string& artist, std::string* errorOut) {
        (void)errorOut;
        std::vector<StoredTrack> result;
        for (const auto& [id, track] : tracks_) {
            if (track.artist == artist) {
                result.push_back(track);
            }
        }
        return result;
    }

    std::vector<StoredTrack> queryByGenreImpl(const std::string& genre, std::string* errorOut) {
        (void)errorOut;
        std::vector<StoredTrack> result;
        for (const auto& [id, track] : tracks_) {
            if (track.genre == genre) {
                result.push_back(track);
            }
        }
        return result;
    }

    int getTrackCountImpl(std::string* errorOut) {
        (void)errorOut;
        return static_cast<int>(tracks_.size());
    }

    bool clearDatabaseImpl(std::string* errorOut) {
        (void)errorOut;
        tracks_.clear();
        return true;
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

    pImpl->dbPath_ = dbPath;
    return pImpl->initializeDatabase(errorOut);
}

bool TrackLibrary::addTrack(const std::string& path, const TrackMetadata& metadata, int* outId, std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: database not initialized";
        }
        return false;
    }

    if (path.empty() || metadata.title.empty()) {
        if (errorOut) {
            *errorOut = "Path and title cannot be empty";
        }
        return false;
    }

    int id = pImpl->addTrackImpl(path, metadata, errorOut);
    if (id < 0) {
        return false;
    }

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

    return pImpl->getTrackImpl(id, errorOut);
}

bool TrackLibrary::removeTrack(int id, std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: implementation not initialized";
        }
        return false;
    }

    return pImpl->removeTrackImpl(id, errorOut);
}

std::vector<StoredTrack> TrackLibrary::queryByBpm(float minBpm, float maxBpm, std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: implementation not initialized";
        }
        return {};
    }

    return pImpl->queryByBpmImpl(minBpm, maxBpm, errorOut);
}

std::vector<StoredTrack> TrackLibrary::queryByKey(const std::string& key, std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: implementation not initialized";
        }
        return {};
    }

    return pImpl->queryByKeyImpl(key, errorOut);
}

std::vector<StoredTrack> TrackLibrary::queryByArtist(const std::string& artist, std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: implementation not initialized";
        }
        return {};
    }

    return pImpl->queryByArtistImpl(artist, errorOut);
}

std::vector<StoredTrack> TrackLibrary::queryByGenre(const std::string& genre, std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: implementation not initialized";
        }
        return {};
    }

    return pImpl->queryByGenreImpl(genre, errorOut);
}

std::vector<StoredTrack> TrackLibrary::getAllTracks(std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: implementation not initialized";
        }
        return {};
    }

    return pImpl->getAllTracksImpl(errorOut);
}

int TrackLibrary::getTrackCount(std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: implementation not initialized";
        }
        return 0;
    }

    return pImpl->getTrackCountImpl(errorOut);
}

bool TrackLibrary::clearDatabase(std::string* errorOut) {
    if (!pImpl) {
        if (errorOut) {
            *errorOut = "Internal error: implementation not initialized";
        }
        return false;
    }

    return pImpl->clearDatabaseImpl(errorOut);
}

} // namespace dj::library
