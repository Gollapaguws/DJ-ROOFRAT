#include "audio/SyncController.h"
#include "audio/Deck.h"

#include <algorithm>
#include <cmath>

namespace dj {

void SyncController::update(Deck& syncDeck, const Deck& targetDeck, double bpmA, double bpmB) {
    if (state_ == SyncState::Off) {
        return;
    }

    // Phase 1: Initializing state - match BPM using setTargetTempo
    if (state_ == SyncState::Initializing) {
        // Calculate effective target BPM for sync deck
        float effectiveTargetBpm = static_cast<float>(bpmA);
        
        // Calculate what tempo percent we need to match target BPM
        float targetTempoPercent = ((effectiveTargetBpm / bpmB) - 1.0f) * 100.0f;
        targetTempoPercent = std::clamp(targetTempoPercent, -50.0f, 50.0f);
        
        // Use ramping if available to smoothly transition
        syncDeck.setTempoRampEnabled(true);
        syncDeck.setTargetTempo(targetTempoPercent);
        syncDeck.setTempoRampRate(0.02f);  // Smooth ramp rate
        
        // Check if we're close enough to target
        float currentEffective = bpmB * (1.0f + (syncDeck.tempoPercent() / 100.0f));
        if (std::abs(currentEffective - effectiveTargetBpm) < 0.1f) {
            state_ = SyncState::Locked;
            lockedFrameCount_ = 0;
        }
        
        framesSinceAdjustment_++;
        return;
    }

    // Phase 2: Locked state - maintain phase alignment
    if (state_ == SyncState::Locked) {
        framesSinceAdjustment_++;
        
        // Check phase every 4 beats (~1-2 seconds at 120 BPM)
        // At 44.1kHz, one beat at 120 BPM = 22050 samples
        const int PhaseCheckInterval = 88200;  // 4 beats worth of samples
        
        if (framesSinceAdjustment_ >= PhaseCheckInterval) {
            framesSinceAdjustment_ = 0;
            
            // Calculate phase offset
            double phaseOffsetSamples = aligner_.calculatePhaseOffset(targetDeck, syncDeck, bpmA, bpmB);
            
            // If drift exceeds 50 samples, make micro-adjustment
            if (std::abs(phaseOffsetSamples) > 50.0) {
                state_ = SyncState::Drifting;
                lockedFrameCount_ = 0;
            } else {
                // Still locked - increment stability counter
                lockedFrameCount_++;
            }
        }
        
        return;
    }

    // Phase 3: Drifting state - auto-correct drift
    if (state_ == SyncState::Drifting) {
        framesSinceAdjustment_++;
        
        // Calculate phase offset
        double phaseOffsetSamples = aligner_.calculatePhaseOffset(targetDeck, syncDeck, bpmA, bpmB);
        
        // Micro-adjust tempo by ±0.01% if drift > 50 samples
        if (std::abs(phaseOffsetSamples) > 50.0) {
            float adjustment = (phaseOffsetSamples > 0.0) ? 0.01f : -0.01f;
            float currentTempo = syncDeck.tempoPercent();
            syncDeck.setTempoPercent(std::clamp(currentTempo + adjustment, -50.0f, 50.0f));
        }
        
        // Check if drift has been corrected
        if (std::abs(phaseOffsetSamples) < 50.0 && framesSinceAdjustment_ > 1000) {
            state_ = SyncState::Locked;
            lockedFrameCount_ = 0;
            framesSinceAdjustment_ = 0;
        }
        
        return;
    }
}

void SyncController::enableSync(Deck& syncDeck, Deck* targetDeck) {
    if (targetDeck == nullptr) {
        disableSync();
        return;
    }
    
    targetDeck_ = targetDeck;
    state_ = SyncState::Initializing;
    framesSinceAdjustment_ = 0;
    lockedFrameCount_ = 0;
}

void SyncController::disableSync() {
    state_ = SyncState::Off;
    targetDeck_ = nullptr;
    framesSinceAdjustment_ = 0;
    lockedFrameCount_ = 0;
}

bool SyncController::isLocked() const {
    return state_ == SyncState::Locked && lockedFrameCount_ >= LOCK_STABILITY_THRESHOLD;
}

SyncState SyncController::getState() const {
    return state_;
}

} // namespace dj
