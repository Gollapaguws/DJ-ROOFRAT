#pragma once

#include "visuals/DJControllerGeometry.h"
#include <memory>

namespace dj {

class Deck;
class Mixer;

/// Handles mouse interaction with 3D DJ controller
class ControllerInteraction {
public:
    ControllerInteraction(Deck& deckA, Deck& deckB, Mixer& mixer);
    
    // Mouse event handlers
    void handleMouseDown(ControlID control);
    void handleMouseDrag(ControlID control, float deltaX, float deltaY);
    void handleMouseUp();
    
    // Phase 6: Update jog wheel velocity for scratching
    void updateJogWheelVelocity(ControlID control, float deltaX, float deltaTime);
    
    // Get currently interacting control
    ControlID getActiveControl() const { return activeControl_; }
    
private:
    // Map mouse delta to parameter change
    void updateCrossfader(float deltaX);
    void updateEQ(ControlID control, float deltaY);
    void updatePitchFader(ControlID control, float deltaY);
    void updateJogWheel(ControlID control, float deltaX);
    void togglePlayButton(ControlID control);
    
    Deck& deckA_;
    Deck& deckB_;
    Mixer& mixer_;
    
    ControlID activeControl_ = ControlID::None;
    float mouseSensitivity_ = 0.01f;  // Pixels to parameter units
    
    // Phase 6: Track jog wheel velocity for scratching
    float lastJogWheelTime_ = 0.0f;
    float lastJogWheelVelocity_ = 0.0f;
};

} // namespace dj
