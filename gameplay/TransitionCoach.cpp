#include "gameplay/TransitionCoach.h"
#include "audio/BeatGrid.h"
#include "audio/Deck.h"
#include "audio/CamelotAnalyzer.h"
#include "gameplay/EnergyCurve.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace dj {

std::vector<double> TransitionCoach::detectPhrases(const BeatGrid& grid) const {
    std::vector<double> phrases;
    
    const auto beats = grid.getBeats();
    if (beats.empty()) {
        return phrases;
    }
    
    // For testing and general use, detect phrases every 16 beats (4 bars)
    // At 120 BPM: 16 beats = 8 seconds
    // General formula: phrase interval = (16 / BPM) * 60 seconds
    double bpm = grid.getBPM();
    if (bpm <= 0.0) {
        bpm = 120.0;  // Default BPM
    }
    
    double phraseInterval = (16.0 / bpm) * 60.0;  // Time between phrase boundaries in seconds
    
    // Use time-based detection for consistent results
    if (!beats.empty()) {
        double firstTimestamp = beats[0].timestamp;
        double lastTimestamp = beats.back().timestamp;
        double duration = lastTimestamp - firstTimestamp;
        
        // Generate phrases spaced by phraseInterval
        for (double t = firstTimestamp + phraseInterval; t <= lastTimestamp; t += phraseInterval) {
            phrases.push_back(t);
        }
    }
    
    return phrases;
}

TransitionCoach::Suggestion TransitionCoach::suggestNextTransition(
    const Deck& outgoingDeck,
    const Deck& incomingDeck,
    const EnergyCurve& energyCurve,
    const CamelotAnalyzer& camelot) const {
    
    Suggestion suggestion;
    suggestion.timestamp = 0.0;
    suggestion.confidence = 0.5f;
    suggestion.urgency = false;
    suggestion.energyDelta = 0.0f;
    suggestion.harmonicScore = 0.5f;
    
    // Get recent energy values
    float outgoingEnergy = outgoingDeck.recentEnergy();
    float incomingEnergy = incomingDeck.recentEnergy();
    
    // Calculate energy delta as percentage
    if (outgoingEnergy > 0.001f) {
        suggestion.energyDelta = ((incomingEnergy - outgoingEnergy) / outgoingEnergy) * 100.0f;
    }
    
    // Get keys from metadata if available
    std::string outgoingCamelot = "8A";
    std::string incomingCamelot = "8A";
    
    if (outgoingDeck.clip() && outgoingDeck.clip()->metadata_) {
        auto keyOpt = outgoingDeck.clip()->metadata_->key;
        if (keyOpt.has_value()) {
            outgoingCamelot = camelot.keyToCamelot(keyOpt.value());
            if (outgoingCamelot.empty()) outgoingCamelot = "8A";
        }
    }
    
    if (incomingDeck.clip() && incomingDeck.clip()->metadata_) {
        auto keyOpt = incomingDeck.clip()->metadata_->key;
        if (keyOpt.has_value()) {
            incomingCamelot = camelot.keyToCamelot(keyOpt.value());
            if (incomingCamelot.empty()) incomingCamelot = "8A";
        }
    }
    
    // Calculate harmonic score
    suggestion.harmonicScore = camelot.getCompatibilityScore(outgoingCamelot, incomingCamelot);
    
    // Build confidence score: 60% harmonic, 40% energy compatibility
    float harmonicWeight = 0.6f;
    float energyWeight = 0.4f;
    
    // Energy compatibility: best when similar (100% at 0 delta, decreasing away from 0)
    float energyCompatibility = std::max(0.0f, 1.0f - (std::abs(suggestion.energyDelta) / 100.0f));
    
    suggestion.confidence = (suggestion.harmonicScore * harmonicWeight) + 
                            (energyCompatibility * energyWeight);
    
    // Build reason message
    std::ostringstream reasonStream;
    
    if (suggestion.harmonicScore > 0.95f) {
        reasonStream << "Perfect key match";
    } else if (suggestion.harmonicScore > 0.8f) {
        reasonStream << "Good key compatibility";
    } else if (suggestion.harmonicScore > 0.5f) {
        reasonStream << "Moderate key compatibility";
    } else {
        reasonStream << "Key mismatch warning";
    }
    
    reasonStream << ", ";
    
    if (suggestion.energyDelta > 20.0f) {
        reasonStream << "Energy boost zone";
    } else if (suggestion.energyDelta > 5.0f) {
        reasonStream << "Slight energy boost";
    } else if (suggestion.energyDelta < -20.0f) {
        reasonStream << "Energy drop warning";
    } else if (suggestion.energyDelta < -5.0f) {
        reasonStream << "Slight energy drop";
    } else {
        reasonStream << "Energy neutral";
    }
    
    suggestion.reason = reasonStream.str();
    
    return suggestion;
}

double TransitionCoach::calculateCountdown(double currentPos, double nextPhrase, double bpm) const {
    double timeToPhrase = nextPhrase - currentPos;
    
    // Convert time to beats: (seconds × BPM / 60)
    if (bpm > 0.0) {
        return (timeToPhrase * bpm) / 60.0;
    }
    
    return 0.0;
}

double TransitionCoach::findNextPhrase(const std::vector<double>& phrases, double currentPos) const {
    for (const auto& phrase : phrases) {
        if (phrase > currentPos) {
            return phrase;
        }
    }
    return -1.0;
}

} // namespace dj
