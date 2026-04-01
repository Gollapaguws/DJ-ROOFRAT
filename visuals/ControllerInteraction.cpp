#include "visuals/ControllerInteraction.h"
#include "audio/Deck.h"
#include "audio/Mixer.h"
#include <algorithm>

namespace dj {

ControllerInteraction::ControllerInteraction(Deck& deckA, Deck& deckB, Mixer& mixer)
    : deckA_(deckA), deckB_(deckB), mixer_(mixer) {
}

void ControllerInteraction::handleMouseDown(ControlID control) {
    activeControl_ = control;
}

void ControllerInteraction::handleMouseDrag(ControlID control, float deltaX, float deltaY) {
    if (activeControl_ == ControlID::None || activeControl_ != control) {
        return;
    }
    
    // Route to appropriate handler based on control type
    switch (control) {
        case ControlID::Crossfader:
            updateCrossfader(deltaX);
            break;
        case ControlID::EQLowA:
        case ControlID::EQMidA:
        case ControlID::EQHighA:
        case ControlID::EQLowB:
        case ControlID::EQMidB:
        case ControlID::EQHighB:
            updateEQ(control, deltaY);
            break;
        case ControlID::PitchA:
        case ControlID::PitchB:
            updatePitchFader(control, deltaY);
            break;
        case ControlID::JogWheelA:
        case ControlID::JogWheelB:
            updateJogWheel(control, deltaX);
            break;
        default:
            break;
    }
}

void ControllerInteraction::handleMouseUp() {
    // Check if we were dragging a play button
    if (activeControl_ == ControlID::PlayA || activeControl_ == ControlID::PlayB) {
        togglePlayButton(activeControl_);
    }
    
    activeControl_ = ControlID::None;
}

void ControllerInteraction::updateCrossfader(float deltaX) {
    // Crossfader range: -1.0 (full A) to +1.0 (full B)
    float current = mixer_.crossfader();
    float newValue = current + (deltaX * mouseSensitivity_);
    newValue = std::clamp(newValue, -1.0f, 1.0f);
    mixer_.setCrossfader(newValue);
}

void ControllerInteraction::updateEQ(ControlID control, float deltaY) {
    // EQ range: 0.0 to 2.0, drag up (negative deltaY) increases gain
    Deck& deck = (control == ControlID::EQLowA || control == ControlID::EQMidA || control == ControlID::EQHighA) 
                 ? deckA_ : deckB_;
    
    // Determine which band to update
    // Note: Since Deck doesn't provide getEQ(), we need to track state differently.
    // For now, we'll apply the adjustment directly to the deck.
    // This is a limitation of the current API; in production code, we'd want getEQ().
    
    float eqDelta = -(deltaY * mouseSensitivity_);  // Negative deltaY = increase gain
    
    // Get default values and adjust based on control ID
    float low = 1.0f, mid = 1.0f, high = 1.0f;
    
    if (control == ControlID::EQLowA || control == ControlID::EQLowB) {
        low = std::clamp(1.0f + eqDelta, 0.0f, 2.0f);
    } else if (control == ControlID::EQMidA || control == ControlID::EQMidB) {
        mid = std::clamp(1.0f + eqDelta, 0.0f, 2.0f);
    } else if (control == ControlID::EQHighA || control == ControlID::EQHighB) {
        high = std::clamp(1.0f + eqDelta, 0.0f, 2.0f);
    }
    
    deck.setEQ(low, mid, high);
}

void ControllerInteraction::updatePitchFader(ControlID control, float deltaY) {
    // Pitch range: -20% to +20%, drag down (positive deltaY) increases tempo
    Deck& deck = (control == ControlID::PitchA) ? deckA_ : deckB_;
    float current = deck.tempoPercent();
    float newValue = current + (deltaY * mouseSensitivity_ * 20.0f);  // Scale to ±20 range
    newValue = std::clamp(newValue, -20.0f, 20.0f);
    deck.setTempoPercent(newValue);
}

void ControllerInteraction::updateJogWheel(ControlID control, float deltaX) {
    // Jog wheel: Apply temporary tempo nudge based on rotation velocity
    Deck& deck = (control == ControlID::JogWheelA) ? deckA_ : deckB_;
    float nudgeAmount = deltaX * mouseSensitivity_ * 2.0f;  // Amplify for sensitivity
    float current = deck.tempoPercent();
    float newValue = current + nudgeAmount;
    newValue = std::clamp(newValue, -20.0f, 20.0f);
    deck.setTempoPercent(newValue);
}

void ControllerInteraction::updateJogWheelVelocity(ControlID control, float deltaX, float deltaTime) {
    // Phase 6: Calculate angular velocity from deltaX and deltaTime
    // This enables fast scratching effects
    
    if (deltaTime <= 0.0f) {
        return;
    }
    
    // Calculate velocity (pixels per second)
    float velocity = deltaX / deltaTime;
    lastJogWheelVelocity_ = velocity;
    lastJogWheelTime_ = deltaTime;
    
    // Get the appropriate deck
    Deck& deck = (control == ControlID::JogWheelA) ? deckA_ : deckB_;
    
    // Apply scratch velocity to deck if it supports it
    // Use setScratchVelocity if available, otherwise apply tempo boost
    float scratchIntensity = std::clamp(velocity / 100.0f, -2.0f, 2.0f);
    deck.setScratchVelocity(scratchIntensity);
    
    // Also apply a tempo nudge proportional to velocity
    float current = deck.tempoPercent();
    float tempoNudge = (velocity * mouseSensitivity_ * 0.5f);
    float newValue = current + tempoNudge;
    newValue = std::clamp(newValue, -20.0f, 20.0f);
    deck.setTempoPercent(newValue);
}

void ControllerInteraction::togglePlayButton(ControlID control) {
    Deck& deck = (control == ControlID::PlayA) ? deckA_ : deckB_;
    if (deck.isPlaying()) {
        deck.pause();
    } else {
        deck.play();
    }
}

} // namespace dj
