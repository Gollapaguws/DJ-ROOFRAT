#include "audio/BeatGrid.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace dj {

// BeatGridData serialization
std::string BeatGridData::serialize() const {
    std::stringstream ss;
    ss << "BeatGridData|";
    ss << firstBeatOffset << "|";
    ss << bpm << "|";
    ss << barsPerPhrase << "|";

    // Serialize nudges
    ss << manualNudges.size() << "|";
    for (double nudge : manualNudges) {
        ss << nudge << ",";
    }

    return ss.str();
}

BeatGridData BeatGridData::deserialize(const std::string& data) {
    BeatGridData result;
    result.firstBeatOffset = 0.0;
    result.bpm = 120.0;
    result.barsPerPhrase = 16;

    std::stringstream ss(data);
    std::string token;

    // Read header
    if (!std::getline(ss, token, '|'))
        return result;

    // Read firstBeatOffset
    if (!std::getline(ss, token, '|'))
        return result;
    try {
        result.firstBeatOffset = std::stod(token);
    } catch (...) {
    }

    // Read bpm
    if (!std::getline(ss, token, '|'))
        return result;
    try {
        result.bpm = std::stod(token);
    } catch (...) {
    }

    // Read barsPerPhrase
    if (!std::getline(ss, token, '|'))
        return result;
    try {
        result.barsPerPhrase = std::stoi(token);
    } catch (...) {
    }

    // Read nudges count
    if (!std::getline(ss, token, '|'))
        return result;
    std::size_t nudgeCount = 0;
    try {
        nudgeCount = std::stoul(token);
    } catch (...) {
    }

    // Read nudges
    if (nudgeCount > 0 && std::getline(ss, token)) {
        std::stringstream nudgeStream(token);
        std::string nudgeVal;
        while (std::getline(nudgeStream, nudgeVal, ',') && result.manualNudges.size() < nudgeCount) {
            if (!nudgeVal.empty()) {
                try {
                    result.manualNudges.push_back(std::stod(nudgeVal));
                } catch (...) {
                }
            }
        }
    }

    return result;
}

// BeatGrid implementation
BeatGrid::BeatGrid()
    : firstBeatOffset_(0.0)
    , bpm_(120.0)
    , barsPerPhrase_(16)
{
}

std::vector<BeatMarker> BeatGrid::getBeats() const {
    return beats_;
}

void BeatGrid::generateFromBPM(double bpm, double firstBeatOffset, double trackDuration) {
    bpm_ = bpm;
    firstBeatOffset_ = firstBeatOffset;
    beats_.clear();
    manualNudges_.clear();

    // Calculate seconds per beat
    double secondsPerBeat = 60.0 / bpm;

    // Generate beats from firstBeatOffset to trackDuration
    int beatIndex = 0;
    double timestamp = firstBeatOffset;

    while (timestamp < trackDuration) {
        BeatMarker marker;
        marker.timestamp = timestamp;
        marker.barPosition = (beatIndex % 4) + 1;
        marker.isDownbeat = (beatIndex % 4 == 0);
        marker.isPhraseStart = false;  // Will be set by markPhrases

        beats_.push_back(marker);
        manualNudges_.push_back(0.0);  // Initialize with no nudge

        timestamp += secondsPerBeat;
        beatIndex++;
    }
}

void BeatGrid::nudgeBeat(std::size_t beatIndex, double offsetMs) {
    if (beatIndex < beats_.size()) {
        // Convert offset from ms to seconds
        double offsetSeconds = offsetMs / 1000.0;
        beats_[beatIndex].timestamp += offsetSeconds;

        // Track the nudge adjustment
        if (beatIndex < manualNudges_.size()) {
            manualNudges_[beatIndex] += offsetMs;
        }
    }
}

void BeatGrid::nudgeAll(double offsetMs) {
    // Convert offset from ms to seconds
    double offsetSeconds = offsetMs / 1000.0;

    // Shift all beats uniformly
    for (auto& beat : beats_) {
        beat.timestamp += offsetSeconds;
    }

    // Update first beat offset as well
    firstBeatOffset_ += offsetSeconds;

    // Track nudge
    for (auto& nudge : manualNudges_) {
        nudge += offsetMs;
    }
}

void BeatGrid::snapToOnset(std::size_t beatIndex, const std::vector<double>& onsets) {
    if (beatIndex >= beats_.size() || onsets.empty()) {
        return;
    }

    // Find nearest onset to current beat timestamp
    double currentTimestamp = beats_[beatIndex].timestamp;
    double nearestOnset = onsets[0];
    double minDistance = std::abs(currentTimestamp - onsets[0]);

    for (double onset : onsets) {
        double distance = std::abs(currentTimestamp - onset);
        if (distance < minDistance) {
            minDistance = distance;
            nearestOnset = onset;
        }
    }

    // Snap to nearest onset
    double offsetSeconds = nearestOnset - currentTimestamp;
    nudgeBeat(beatIndex, offsetSeconds * 1000.0);
}

void BeatGrid::markPhrases(int barsPerPhrase) {
    barsPerPhrase_ = barsPerPhrase;
    int beatsPerPhrase = barsPerPhrase * 4;  // 4/4 time signature

    for (std::size_t i = 0; i < beats_.size(); ++i) {
        // Mark phrase starts (every N bars = N*4 beats)
        beats_[i].isPhraseStart = (i % beatsPerPhrase == 0);
    }
}

double BeatGrid::getBPM() const {
    return bpm_;
}

double BeatGrid::getFirstBeatOffset() const {
    return firstBeatOffset_;
}

BeatGridData BeatGrid::toData() const {
    BeatGridData data;
    data.firstBeatOffset = firstBeatOffset_;
    data.bpm = bpm_;
    data.barsPerPhrase = barsPerPhrase_;
    data.manualNudges = manualNudges_;
    return data;
}

void BeatGrid::fromData(const BeatGridData& data, double trackDuration) {
    bpm_ = data.bpm;
    firstBeatOffset_ = data.firstBeatOffset;
    barsPerPhrase_ = data.barsPerPhrase;
    manualNudges_ = data.manualNudges;

    // Regenerate beat grid
    generateFromBPM(bpm_, firstBeatOffset_, trackDuration);

    // Reapply manual nudges
    for (std::size_t i = 0; i < manualNudges_.size() && i < beats_.size(); ++i) {
        if (manualNudges_[i] != 0.0) {
            double offsetSeconds = manualNudges_[i] / 1000.0;
            beats_[i].timestamp += offsetSeconds;
        }
    }

    // Mark phrases
    markPhrases(barsPerPhrase_);
}

void BeatGrid::updateBeatPositions(double trackDuration) {
    // Regenerate beats after modifications
    generateFromBPM(bpm_, firstBeatOffset_, trackDuration);
}

}  // namespace dj
