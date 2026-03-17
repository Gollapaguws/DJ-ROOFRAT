#include "audio/PhaseAligner.h"
#include "audio/Deck.h"

#include <cmath>
#include <algorithm>

namespace dj {

double PhaseAligner::calculatePhaseOffset(const Deck& deckA, const Deck& deckB,
                                           double bpmA, double bpmB) const {
    // Get current playback positions
    std::size_t posA = deckA.currentFrame();
    std::size_t posB = deckB.currentFrame();
    
    // Assume 44.1kHz for now (TODO: get from clip metadata)
    constexpr int sampleRate = 44100;
    
    // Find nearest beat markers
    std::size_t beatA = findNearestBeat(posA, bpmA, sampleRate);
    std::size_t beatB = findNearestBeat(posB, bpmB, sampleRate);
    
    // Calculate phase offset relative to beat positions
    double phaseA = static_cast<double>(posA) - static_cast<double>(beatA);
    double phaseB = static_cast<double>(posB) - static_cast<double>(beatB);
    
    // Offset in samples (positive = B ahead)
    double offsetSamples = phaseB - phaseA;
    
    return offsetSamples;
}

double PhaseAligner::calculatePhaseOffsetBeats(const Deck& deckA, const Deck& deckB,
                                                 double bpmA, double bpmB) const {
    constexpr int sampleRate = 44100;
    
    // Get offset in samples
    double offsetSamples = calculatePhaseOffset(deckA, deckB, bpmA, bpmB);
    
    // Normalize to beats using average BPM
    double avgBPM = (bpmA + bpmB) / 2.0;
    double offsetBeats = normalizePhaseOffset(offsetSamples, avgBPM, sampleRate);
    
    return offsetBeats;
}

std::size_t PhaseAligner::findNearestBeat(std::size_t position, double bpm, int sampleRate) const {
    // Calculate samples per beat
    double secondsPerBeat = 60.0 / bpm;
    double samplesPerBeat = secondsPerBeat * static_cast<double>(sampleRate);
    
    // Find nearest beat grid position
    // Assumes beat 0 is at sample 0 (simplified model)
    std::size_t beatIndex = static_cast<std::size_t>(
        std::round(static_cast<double>(position) / samplesPerBeat)
    );
    
    std::size_t beatPosition = static_cast<std::size_t>(
        static_cast<double>(beatIndex) * samplesPerBeat
    );
    
    return beatPosition;
}

double PhaseAligner::normalizePhaseOffset(double offsetSamples, double bpm, int sampleRate) const {
    // Convert samples to beats
    double secondsPerBeat = 60.0 / bpm;
    double samplesPerBeat = secondsPerBeat * static_cast<double>(sampleRate);
    double offsetBeats = offsetSamples / samplesPerBeat;
    
    // Normalize to [-0.5, +0.5] range
    // Wrap around beat boundaries
    while (offsetBeats > 0.5) {
        offsetBeats -= 1.0;
    }
    while (offsetBeats < -0.5) {
        offsetBeats += 1.0;
    }
    
    return offsetBeats;
}

} // namespace dj
