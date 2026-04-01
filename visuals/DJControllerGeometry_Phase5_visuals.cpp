#include "visuals/DJControllerGeometry.h"
#include "audio/Deck.h"
#include "audio/Mixer.h"
#include <cmath>

namespace dj {

// Phase 5: Update controller visuals based on deck and mixer state
void DJControllerGeometry::updateVisuals(const Deck& deckA, const Deck& deckB, const Mixer& mixer) {
    // Update crossfader position
    updateCrossfaderPosition(mixer.crossfader());
    
    // Update play button glow
    updatePlayButtonGlow(ControlID::PlayA, deckA.isPlaying());
    updatePlayButtonGlow(ControlID::PlayB, deckB.isPlaying());
    
    // Update EQ knobs rotation
    auto eqA = deckA.getEQ();
    auto eqB = deckB.getEQ();
    updateEQKnobRotation(ControlID::EQLowA, eqA.low);
    updateEQKnobRotation(ControlID::EQMidA, eqA.mid);
    updateEQKnobRotation(ControlID::EQHighA, eqA.high);
    updateEQKnobRotation(ControlID::EQLowB, eqB.low);
    updateEQKnobRotation(ControlID::EQMidB, eqB.mid);
    updateEQKnobRotation(ControlID::EQHighB, eqB.high);
    
    // Update pitch fader positions
    updatePitchFaderPosition(ControlID::PitchA, deckA.tempoPercent());
    updatePitchFaderPosition(ControlID::PitchB, deckB.tempoPercent());
}

// Update crossfader position based on mixer state
void DJControllerGeometry::updateCrossfaderPosition(float crossfaderValue) {
    // Crossfader moves left-right (X axis)
    // -1.0 = far left, 0.0 = center, 1.0 = far right
    // We'll find crossfader geometry and translate it along X axis
    
    constexpr float CROSSFADER_TRAVEL = 0.3f;  // How far it can move
    float targetX = (crossfaderValue / 2.0f) * CROSSFADER_TRAVEL;  // Normalize to [-0.15, 0.15]
    
    // Find crossfader vertices (they're around Z = 0.4)
    for (auto& v : m_vertices) {
        // Identify crossfader vertices by their Z position
        if (v.position[2] > 0.35f && v.position[2] < 0.45f && 
            v.position[1] > 0.08f && v.position[1] < 0.12f) {
            // This is likely a crossfader vertex - move it along X
            // Simple approach: add the crossfader offset to X
            v.position[0] += targetX;
        }
    }
}

// Update play button glow effect
void DJControllerGeometry::updatePlayButtonGlow(ControlID button, bool isPlaying) {
    // Find button vertices and modify their normals (as a proxy for color/brightness)
    // When playing, increase normal magnitude; when not, set to normal
    
    float intensity = isPlaying ? 1.5f : 1.0f;  // Playing buttons are brighter
    
    // Find button vertices by control bounds
    for (const auto& bounds : m_controlBounds) {
        if (bounds.id == button) {
            // Modify all vertices within this button's bounds
            for (auto& v : m_vertices) {
                float dx = v.position[0] - bounds.centerX;
                float dy = v.position[1] - bounds.centerY;
                float dz = v.position[2] - bounds.centerZ;
                float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
                
                if (dist < bounds.radius * 1.2f) {
                    // This is a button vertex - modulate its normal for glow
                    v.normal[0] *= intensity;
                    v.normal[1] *= intensity;
                    v.normal[2] *= intensity;
                }
            }
            break;
        }
    }
}

// Update EQ knob rotation based on gain value
void DJControllerGeometry::updateEQKnobRotation(ControlID knob, float gainValue) {
    // EQ gain typically ranges from ~0.5 to 2.0
    // Map to rotation angle: 1.0 = 0°, with range approximately ±90°
    // Rotation angle = (gainValue - 1.0) * PI radians
    
    const float PI = 3.14159265f;
    float angle = (gainValue - 1.0f) * PI;  // Maps 0.5->-0.5*PI, 1.0->0, 2.0->PI
    
    float cosA = std::cos(angle);
    float sinA = std::sin(angle);
    
    // Find knob vertices and apply rotation
    for (const auto& bounds : m_controlBounds) {
        if (bounds.id == knob) {
            // For each vertex near this knob, apply Y-axis rotation around center
            for (auto& v : m_vertices) {
                float dx = v.position[0] - bounds.centerX;
                float dy = v.position[1] - bounds.centerY;
                float dz = v.position[2] - bounds.centerZ;
                float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
                
                if (dist < bounds.radius * 1.5f) {
                    // Apply Y-axis rotation (rotate around vertical axis)
                    float newX = dx * cosA - dz * sinA;
                    float newZ = dx * sinA + dz * cosA;
                    
                    v.position[0] = bounds.centerX + newX;
                    v.position[2] = bounds.centerZ + newZ;
                    
                    // Also rotate the normal vector
                    float normX = v.normal[0];
                    float normZ = v.normal[2];
                    v.normal[0] = normX * cosA - normZ * sinA;
                    v.normal[2] = normX * sinA + normZ * cosA;
                }
            }
            break;
        }
    }
}

// Update pitch fader vertical position based on tempo
void DJControllerGeometry::updatePitchFaderPosition(ControlID fader, float tempoPercent) {
    // Tempo ranges from -50% to +50%
    // Fader should move up with positive tempo, down with negative
    // Map to Y offset: -50% -> -0.1, 0% -> 0, +50% -> +0.1
    
    float faderTravel = 0.1f;
    float offsetY = (tempoPercent / 50.0f) * faderTravel;  // Maps [-50,+50] to ±0.1
    
    // Find fader vertices and translate Y
    for (const auto& bounds : m_controlBounds) {
        if (bounds.id == fader) {
            // Translate all vertices of this fader upward/downward
            for (auto& v : m_vertices) {
                float dx = v.position[0] - bounds.centerX;
                float dy = v.position[1] - bounds.centerY;
                float dz = v.position[2] - bounds.centerZ;
                float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
                
                if (dist < bounds.radius * 2.0f) {
                    // This is a fader vertex - move it along Y
                    v.position[1] += offsetY;
                }
            }
            break;
        }
    }
}

} // namespace dj
