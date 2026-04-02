#include "visuals/DJControllerGeometry.h"
#include "audio/Deck.h"
#include "audio/Mixer.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace dj {

DJControllerGeometry::DJControllerGeometry() = default;

void DJControllerGeometry::generateMesh() {
    // Clear existing geometry
    m_vertices.clear();
    m_indices.clear();
    m_controlBounds.clear();
    
    // Create base controller surface
    float halfWidth = CONTROLLER_WIDTH / 2.0f;
    float halfDepth = CONTROLLER_DEPTH / 2.0f;
    
    // Base panel vertices (flat rectangle at y = BASE_HEIGHT)
    uint32_t baseStart = static_cast<uint32_t>(m_vertices.size());
    
    // Four corners of the controller base
    addVertex(-halfWidth, BASE_HEIGHT, -halfDepth, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);  // 0: front-left
    addVertex(halfWidth, BASE_HEIGHT, -halfDepth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);   // 1: front-right
    addVertex(halfWidth, BASE_HEIGHT, halfDepth, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);    // 2: back-right
    addVertex(-halfWidth, BASE_HEIGHT, halfDepth, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);   // 3: back-left
    
    // Add base panel triangles
    addTriangle(baseStart, baseStart + 1, baseStart + 2);
    addTriangle(baseStart, baseStart + 2, baseStart + 3);
    
    // ===== DECK A (Left) =====
    
    // Jog wheel at (-0.5, 0.1, -0.2)
    createJogWheel(DECK_A_X, CONTROL_Y, JOG_WHEEL_Z, JOG_WHEEL_RADIUS, ControlID::JogWheelA);
    
    // Pitch fader at (-0.5, 0.1, 0.3)
    createFader(DECK_A_X, CONTROL_Y, PITCH_FADER_Z, PITCH_FADER_WIDTH, PITCH_FADER_HEIGHT, ControlID::PitchA);
    
    // EQ knobs at (-0.7, 0.1, 0.0/0.1/0.2) for Low/Mid/High
    createKnob(DECK_A_X - 0.2f, CONTROL_Y, EQ_CENTER_Z, KNOB_RADIUS, KNOB_HEIGHT, ControlID::EQLowA);
    createKnob(DECK_A_X - 0.2f, CONTROL_Y, EQ_CENTER_Z + EQ_SPACING_Z, KNOB_RADIUS, KNOB_HEIGHT, ControlID::EQMidA);
    createKnob(DECK_A_X - 0.2f, CONTROL_Y, EQ_CENTER_Z + 2.0f * EQ_SPACING_Z, KNOB_RADIUS, KNOB_HEIGHT, ControlID::EQHighA);
    
    // Play button at (-0.3, 0.1, -0.2)
    createButton(DECK_A_X + 0.2f, CONTROL_Y, BUTTON_Z, BUTTON_SIZE, ControlID::PlayA);
    
    // ===== DECK B (Right) =====
    
    // Jog wheel at (0.5, 0.1, -0.2)
    createJogWheel(DECK_B_X, CONTROL_Y, JOG_WHEEL_Z, JOG_WHEEL_RADIUS, ControlID::JogWheelB);
    
    // Pitch fader at (0.5, 0.1, 0.3)
    createFader(DECK_B_X, CONTROL_Y, PITCH_FADER_Z, PITCH_FADER_WIDTH, PITCH_FADER_HEIGHT, ControlID::PitchB);
    
    // EQ knobs at (0.7, 0.1, 0.0/0.1/0.2) for Low/Mid/High
    createKnob(DECK_B_X + 0.2f, CONTROL_Y, EQ_CENTER_Z, KNOB_RADIUS, KNOB_HEIGHT, ControlID::EQLowB);
    createKnob(DECK_B_X + 0.2f, CONTROL_Y, EQ_CENTER_Z + EQ_SPACING_Z, KNOB_RADIUS, KNOB_HEIGHT, ControlID::EQMidB);
    createKnob(DECK_B_X + 0.2f, CONTROL_Y, EQ_CENTER_Z + 2.0f * EQ_SPACING_Z, KNOB_RADIUS, KNOB_HEIGHT, ControlID::EQHighB);
    
    // Play button at (0.3, 0.1, -0.2)
    createButton(DECK_B_X - 0.2f, CONTROL_Y, BUTTON_Z, BUTTON_SIZE, ControlID::PlayB);
    
    // ===== CENTER CROSSFADER =====
    createCrossfader(0.0f, CONTROL_Y, CROSSFADER_Z, CROSSFADER_WIDTH, ControlID::Crossfader);
}

void DJControllerGeometry::addVertex(float x, float y, float z, 
                                     float nx, float ny, float nz,
                                     float u, float v) {
    Vertex vertex;
    vertex.position[0] = x;
    vertex.position[1] = y;
    vertex.position[2] = z;
    vertex.normal[0] = nx;
    vertex.normal[1] = ny;
    vertex.normal[2] = nz;
    vertex.texCoord[0] = u;
    vertex.texCoord[1] = v;
    m_vertices.push_back(vertex);
}

void DJControllerGeometry::addTriangle(uint32_t i0, uint32_t i1, uint32_t i2) {
    m_indices.push_back(i0);
    m_indices.push_back(i1);
    m_indices.push_back(i2);
}

void DJControllerGeometry::addControlBounds(ControlID id, float x, float y, float z, float radius) {
    m_controlBounds.push_back({id, x, y, z, radius});
}

void DJControllerGeometry::createKnob(float x, float y, float z, float radius, float height, ControlID id) {
    // Create a cylinder for the knob (20 segments)
    const uint32_t segments = 20;
    const float PI = 3.14159265f;
    uint32_t vertexStart = static_cast<uint32_t>(m_vertices.size());
    
    // Bottom cap center
    uint32_t bottomCenter = vertexStart;
    addVertex(x, y, z, 0.0f, -1.0f, 0.0f, 0.5f, 0.5f);
    
    // Top cap center
    uint32_t topCenter = vertexStart + 1;
    addVertex(x, y + height, z, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f);
    
    // Side vertices
    for (uint32_t i = 0; i <= segments; ++i) {
        float angle = 2.0f * PI * i / segments;
        float cx = std::cos(angle) * radius;
        float cz = std::sin(angle) * radius;
        
        // Bottom vertex
        addVertex(x + cx, y, z + cz, cx, 0.0f, cz, i / static_cast<float>(segments), 0.0f);
        
        // Top vertex
        addVertex(x + cx, y + height, z + cz, cx, 0.0f, cz, i / static_cast<float>(segments), 1.0f);
    }
    
    // Bottom cap triangles
    for (uint32_t i = 0; i < segments; ++i) {
        uint32_t v0 = vertexStart + 2 + i * 2;
        uint32_t v1 = vertexStart + 2 + (i + 1) * 2;
        addTriangle(bottomCenter, v1, v0);
    }
    
    // Top cap triangles
    for (uint32_t i = 0; i < segments; ++i) {
        uint32_t v0 = vertexStart + 2 + i * 2 + 1;
        uint32_t v1 = vertexStart + 2 + (i + 1) * 2 + 1;
        addTriangle(topCenter, v0, v1);
    }
    
    // Side triangles
    for (uint32_t i = 0; i < segments; ++i) {
        uint32_t b0 = vertexStart + 2 + i * 2;
        uint32_t t0 = vertexStart + 2 + i * 2 + 1;
        uint32_t b1 = vertexStart + 2 + (i + 1) * 2;
        uint32_t t1 = vertexStart + 2 + (i + 1) * 2 + 1;
        
        addTriangle(b0, t1, t0);
        addTriangle(b0, b1, t1);
    }
    
    // Add control bounds
    addControlBounds(id, x, y + height / 2.0f, z, radius * 0.8f);
}

void DJControllerGeometry::createFader(float x, float y, float z, float width, float height, ControlID id) {
    // Create a rectangular box for the fader
    uint32_t vertexStart = static_cast<uint32_t>(m_vertices.size());
    
    float halfWidth = width / 2.0f;
    float depth = 0.02f;  // Small depth for 2D-like fader
    float halfDepth = depth / 2.0f;
    
    // Front face
    addVertex(x - halfWidth, y, z - halfDepth, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);           // 0
    addVertex(x + halfWidth, y, z - halfDepth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);           // 1
    addVertex(x + halfWidth, y + height, z - halfDepth, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f); // 2
    addVertex(x - halfWidth, y + height, z - halfDepth, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f); // 3
    
    // Back face
    addVertex(x - halfWidth, y, z + halfDepth, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);            // 4
    addVertex(x + halfWidth, y, z + halfDepth, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);            // 5
    addVertex(x + halfWidth, y + height, z + halfDepth, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);  // 6
    addVertex(x - halfWidth, y + height, z + halfDepth, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);  // 7
    
    // Front face triangles
    addTriangle(vertexStart + 0, vertexStart + 1, vertexStart + 2);
    addTriangle(vertexStart + 0, vertexStart + 2, vertexStart + 3);
    
    // Back face triangles
    addTriangle(vertexStart + 4, vertexStart + 6, vertexStart + 5);
    addTriangle(vertexStart + 4, vertexStart + 7, vertexStart + 6);
    
    // Top face triangles
    addTriangle(vertexStart + 3, vertexStart + 2, vertexStart + 6);
    addTriangle(vertexStart + 3, vertexStart + 6, vertexStart + 7);
    
    // Bottom face triangles
    addTriangle(vertexStart + 0, vertexStart + 5, vertexStart + 1);
    addTriangle(vertexStart + 0, vertexStart + 4, vertexStart + 5);
    
    // Left face triangles
    addTriangle(vertexStart + 0, vertexStart + 3, vertexStart + 7);
    addTriangle(vertexStart + 0, vertexStart + 7, vertexStart + 4);
    
    // Right face triangles
    addTriangle(vertexStart + 1, vertexStart + 5, vertexStart + 6);
    addTriangle(vertexStart + 1, vertexStart + 6, vertexStart + 2);
    
    // Add control bounds
    addControlBounds(id, x, y + height / 2.0f, z, halfWidth);
}

void DJControllerGeometry::createButton(float x, float y, float z, float size, ControlID id) {
    // Create a small cube for the button
    uint32_t vertexStart = static_cast<uint32_t>(m_vertices.size());
    
    float halfSize = size / 2.0f;
    
    // Front face
    addVertex(x - halfSize, y, z - halfSize, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);            // 0
    addVertex(x + halfSize, y, z - halfSize, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);            // 1
    addVertex(x + halfSize, y + size, z - halfSize, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);    // 2
    addVertex(x - halfSize, y + size, z - halfSize, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);    // 3
    
    // Back face
    addVertex(x - halfSize, y, z + halfSize, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);             // 4
    addVertex(x + halfSize, y, z + halfSize, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);             // 5
    addVertex(x + halfSize, y + size, z + halfSize, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);     // 6
    addVertex(x - halfSize, y + size, z + halfSize, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);     // 7
    
    // Front face triangles
    addTriangle(vertexStart + 0, vertexStart + 1, vertexStart + 2);
    addTriangle(vertexStart + 0, vertexStart + 2, vertexStart + 3);
    
    // Back face triangles
    addTriangle(vertexStart + 4, vertexStart + 6, vertexStart + 5);
    addTriangle(vertexStart + 4, vertexStart + 7, vertexStart + 6);
    
    // Top face triangles
    addTriangle(vertexStart + 3, vertexStart + 2, vertexStart + 6);
    addTriangle(vertexStart + 3, vertexStart + 6, vertexStart + 7);
    
    // Bottom face triangles
    addTriangle(vertexStart + 0, vertexStart + 5, vertexStart + 1);
    addTriangle(vertexStart + 0, vertexStart + 4, vertexStart + 5);
    
    // Left face triangles
    addTriangle(vertexStart + 0, vertexStart + 3, vertexStart + 7);
    addTriangle(vertexStart + 0, vertexStart + 7, vertexStart + 4);
    
    // Right face triangles
    addTriangle(vertexStart + 1, vertexStart + 5, vertexStart + 6);
    addTriangle(vertexStart + 1, vertexStart + 6, vertexStart + 2);
    
    // Add control bounds
    addControlBounds(id, x, y + halfSize, z, halfSize);
}

void DJControllerGeometry::createJogWheel(float x, float y, float z, float radius, ControlID id) {
    // Create a larger cylinder for the jog wheel (64 segments for smoothness)
    const uint32_t segments = 64;
    const float PI = 3.14159265f;
    const float thickness = 0.02f;
    
    uint32_t vertexStart = static_cast<uint32_t>(m_vertices.size());
    
    // Bottom cap center
    uint32_t bottomCenter = vertexStart;
    addVertex(x, y, z, 0.0f, -1.0f, 0.0f, 0.5f, 0.5f);
    
    // Top cap center
    uint32_t topCenter = vertexStart + 1;
    addVertex(x, y + thickness, z, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f);
    
    // Side vertices
    for (uint32_t i = 0; i <= segments; ++i) {
        float angle = 2.0f * PI * i / segments;
        float cx = std::cos(angle) * radius;
        float cz = std::sin(angle) * radius;
        
        // Bottom vertex
        addVertex(x + cx, y, z + cz, cx, 0.0f, cz, i / static_cast<float>(segments), 0.0f);
        
        // Top vertex
        addVertex(x + cx, y + thickness, z + cz, cx, 0.0f, cz, i / static_cast<float>(segments), 1.0f);
    }
    
    // Bottom cap triangles
    for (uint32_t i = 0; i < segments; ++i) {
        uint32_t v0 = vertexStart + 2 + i * 2;
        uint32_t v1 = vertexStart + 2 + (i + 1) * 2;
        addTriangle(bottomCenter, v1, v0);
    }
    
    // Top cap triangles
    for (uint32_t i = 0; i < segments; ++i) {
        uint32_t v0 = vertexStart + 2 + i * 2 + 1;
        uint32_t v1 = vertexStart + 2 + (i + 1) * 2 + 1;
        addTriangle(topCenter, v0, v1);
    }
    
    // Side triangles
    for (uint32_t i = 0; i < segments; ++i) {
        uint32_t b0 = vertexStart + 2 + i * 2;
        uint32_t t0 = vertexStart + 2 + i * 2 + 1;
        uint32_t b1 = vertexStart + 2 + (i + 1) * 2;
        uint32_t t1 = vertexStart + 2 + (i + 1) * 2 + 1;
        
        addTriangle(b0, t1, t0);
        addTriangle(b0, b1, t1);
    }
    
    // Add control bounds
    addControlBounds(id, x, y + thickness / 2.0f, z, radius * 0.9f);
}

void DJControllerGeometry::createCrossfader(float x, float y, float z, float width, ControlID id) {
    // Create a long rectangular box for the crossfader
    uint32_t vertexStart = static_cast<uint32_t>(m_vertices.size());
    
    float halfWidth = width / 2.0f;
    float height = 0.03f;
    float depth = 0.02f;
    float halfDepth = depth / 2.0f;
    
    // Front face
    addVertex(x - halfWidth, y, z - halfDepth, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);           // 0
    addVertex(x + halfWidth, y, z - halfDepth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);           // 1
    addVertex(x + halfWidth, y + height, z - halfDepth, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f); // 2
    addVertex(x - halfWidth, y + height, z - halfDepth, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f); // 3
    
    // Back face
    addVertex(x - halfWidth, y, z + halfDepth, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);            // 4
    addVertex(x + halfWidth, y, z + halfDepth, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);            // 5
    addVertex(x + halfWidth, y + height, z + halfDepth, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);  // 6
    addVertex(x - halfWidth, y + height, z + halfDepth, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);  // 7
    
    // Front face triangles
    addTriangle(vertexStart + 0, vertexStart + 1, vertexStart + 2);
    addTriangle(vertexStart + 0, vertexStart + 2, vertexStart + 3);
    
    // Back face triangles
    addTriangle(vertexStart + 4, vertexStart + 6, vertexStart + 5);
    addTriangle(vertexStart + 4, vertexStart + 7, vertexStart + 6);
    
    // Top face triangles
    addTriangle(vertexStart + 3, vertexStart + 2, vertexStart + 6);
    addTriangle(vertexStart + 3, vertexStart + 6, vertexStart + 7);
    
    // Bottom face triangles
    addTriangle(vertexStart + 0, vertexStart + 5, vertexStart + 1);
    addTriangle(vertexStart + 0, vertexStart + 4, vertexStart + 5);
    
    // Left face triangles
    addTriangle(vertexStart + 0, vertexStart + 3, vertexStart + 7);
    addTriangle(vertexStart + 0, vertexStart + 7, vertexStart + 4);
    
    // Right face triangles
    addTriangle(vertexStart + 1, vertexStart + 5, vertexStart + 6);
    addTriangle(vertexStart + 1, vertexStart + 6, vertexStart + 2);
    
    // Add control bounds
    addControlBounds(id, x, y + height / 2.0f, z, halfWidth);
}

void DJControllerGeometry::updateCrossfaderPosition(float crossfaderValue) {
    (void)crossfaderValue;
    // Phase 5/6: Update crossfader control position based on value [-1, 1]
    // For now, just a placeholder to avoid linker errors
    // In full implementation, this would update vertex positions
}

void DJControllerGeometry::updatePlayButtonGlow(ControlID button, bool isPlaying) {
    (void)button;
    (void)isPlaying;
    // Phase 5/6: Update play button visual state based on playback
    // For now, just a placeholder
}

void DJControllerGeometry::updateEQKnobRotation(ControlID knob, float gainValue) {
    // Phase 5/6: Update EQ knob rotation based on gain value
    // Apply lerped rotation to actual vertices
    
    if (m_verticesOriginal.empty()) {
        return;
    }
    
    // Try to find the range, but if not available, update vertices near control bounds
    auto it = m_vertexRanges.find(knob);
    
    if (it != m_vertexRanges.end()) {
        // Use pre-computed vertex range
        const auto& range = it->second;
        if (range.start < m_verticesOriginal.size() && 
            range.start + range.count <= m_verticesOriginal.size()) {
            
            float normalizedGain = std::clamp(gainValue, 0.0f, 2.0f) / 2.0f;  // 0.0-1.0
            
            for (uint32_t i = range.start; i < range.start + range.count; ++i) {
                if (i < m_vertices.size() && i < m_verticesOriginal.size()) {
                    const auto& orig = m_verticesOriginal[i];
                    auto& current = m_vertices[i];
                    
                    float heightOffset = (normalizedGain - 0.5f) * 0.05f;
                    current.position[1] = orig.position[1] + heightOffset;
                    current.normal[1] = orig.normal[1] * (0.8f + normalizedGain * 0.4f);
                }
            }
            return;
        }
    }
    
    // Fallback: find vertices by proximity to control bounds
    for (const auto& bounds : m_controlBounds) {
        if (bounds.id == knob) {
            float normalizedGain = std::clamp(gainValue, 0.0f, 2.0f) / 2.0f;
            
            for (size_t i = 0; i < m_vertices.size() && i < m_verticesOriginal.size(); ++i) {
                const auto& orig = m_verticesOriginal[i];
                auto& current = m_vertices[i];
                
                // Check proximity to control
                float dx = orig.position[0] - bounds.centerX;
                float dy = orig.position[1] - bounds.centerY;
                float dz = orig.position[2] - bounds.centerZ;
                float distSq = dx*dx + dy*dy + dz*dz;
                
                if (distSq < (bounds.radius * 1.5f) * (bounds.radius * 1.5f)) {
                    float heightOffset = (normalizedGain - 0.5f) * 0.05f;
                    current.position[1] = orig.position[1] + heightOffset;
                    current.normal[1] = orig.normal[1] * (0.8f + normalizedGain * 0.4f);
                }
            }
            break;
        }
    }
}

void DJControllerGeometry::updatePitchFaderPosition(ControlID fader, float tempoPercent) {
    // Phase 5/6: Update pitch fader position based on tempo
    // Find and update vertices for this fader control
    
    auto it = m_vertexRanges.find(fader);
    if (it == m_vertexRanges.end() || m_verticesOriginal.empty()) {
        return;
    }
    
    const auto& range = it->second;
    if (range.start >= m_verticesOriginal.size() || 
        range.start + range.count > m_verticesOriginal.size()) {
        return;
    }
    
    // Normalize tempo to 0-1 range (±20% -> 0.0-1.0)
    float normalizedTempo = (tempoPercent + 20.0f) / 40.0f;  // -20 to +20 -> 0 to 1
    normalizedTempo = std::clamp(normalizedTempo, 0.0f, 1.0f);
    
    for (uint32_t i = range.start; i < range.start + range.count; ++i) {
        if (i < m_vertices.size() && i < m_verticesOriginal.size()) {
            const auto& orig = m_verticesOriginal[i];
            auto& current = m_vertices[i];
            
            // Move fader vertically based on tempo
            float faderOffset = (normalizedTempo - 0.5f) * 0.1f;  // ±0.05 offset
            current.position[1] = orig.position[1] + faderOffset;
        }
    }
}

void DJControllerGeometry::updateVisuals(const Deck& deckA, const Deck& deckB, const Mixer& mixer) {
    // Phase 6: Update controller visuals with lerp smoothing
    
    // First, copy vertices to original backup if needed
    if (m_verticesOriginal.empty()) {
        m_verticesOriginal = m_vertices;
        
        // Initialize currentRotations to the current state (not target)
        // This allows proper lerping on first and subsequent calls
        EQState eqStateA = deckA.getEQ();
        EQState eqStateB = deckB.getEQ();
        
        currentRotations_[ControlID::EQLowA] = eqStateA.low;
        currentRotations_[ControlID::EQMidA] = eqStateA.mid;
        currentRotations_[ControlID::EQHighA] = eqStateA.high;
        
        currentRotations_[ControlID::EQLowB] = eqStateB.low;
        currentRotations_[ControlID::EQMidB] = eqStateB.mid;
        currentRotations_[ControlID::EQHighB] = eqStateB.high;
    }
    
    // Set target rotations based on current deck/mixer state
    // EQ knobs rotation based on EQ gains
    EQState eqStateA = deckA.getEQ();
    EQState eqStateB = deckB.getEQ();
    
    targetRotations_[ControlID::EQLowA] = eqStateA.low;
    targetRotations_[ControlID::EQMidA] = eqStateA.mid;
    targetRotations_[ControlID::EQHighA] = eqStateA.high;
    
    targetRotations_[ControlID::EQLowB] = eqStateB.low;
    targetRotations_[ControlID::EQMidB] = eqStateB.mid;
    targetRotations_[ControlID::EQHighB] = eqStateB.high;
    
    // Apply smooth lerp interpolation
    applyLerpInterpolation();
    
    // Update EQ knob rotations based on lerped values
    for (const auto& [id, currentRotation] : currentRotations_) {
        updateEQKnobRotation(id, currentRotation);
    }
    
    // Update other controls
    updateCrossfaderPosition(mixer.crossfader());
    updatePlayButtonGlow(ControlID::PlayA, deckA.isPlaying());
    updatePlayButtonGlow(ControlID::PlayB, deckB.isPlaying());
    updatePitchFaderPosition(ControlID::PitchA, deckA.tempoPercent());
    updatePitchFaderPosition(ControlID::PitchB, deckB.tempoPercent());
}

ControlID DJControllerGeometry::pickControl(const Ray& ray) const {
    float closestDist = std::numeric_limits<float>::max();
    ControlID closestControl = ControlID::None;
    
    // Test ray against all control bounding spheres
    for (const auto& bounds : m_controlBounds) {
        float center[3] = {bounds.centerX, bounds.centerY, bounds.centerZ};
        float dist = RayCaster::intersectSphere(ray, center, bounds.radius);
        
        if (dist > 0.0f && dist < closestDist) {
            closestDist = dist;
            closestControl = bounds.id;
        }
    }
    
    return closestControl;
}

void DJControllerGeometry::setHoverHighlight(ControlID control) {
    hoveredControl_ = control;
    
    // Find vertices for this control and increase their brightness
    // We do this by scaling the normals (normal magnitude indicates brightness)
    for (auto& bounds : m_controlBounds) {
        if (bounds.id == control) {
            // Look for vertices near this control center
            for (auto& vertex : m_vertices) {
                // Simple proximity check: if vertex is close to control center, highlight it
                float dx = vertex.position[0] - bounds.centerX;
                float dy = vertex.position[1] - bounds.centerY;
                float dz = vertex.position[2] - bounds.centerZ;
                float distSq = dx*dx + dy*dy + dz*dz;
                
                // If within 2x control radius, apply highlight
                if (distSq < (bounds.radius * 2.0f) * (bounds.radius * 2.0f)) {
                    // Increase normal magnitude for brighter lighting (1.0 -> 1.5)
                    float scale = 1.5f;
                    vertex.normal[0] *= scale;
                    vertex.normal[1] *= scale;
                    vertex.normal[2] *= scale;
                }
            }
            break;
        }
    }
}

void DJControllerGeometry::clearHoverHighlight() {
    hoveredControl_ = ControlID::None;
    
    // Restore original vertices to remove highlight
    if (!m_verticesOriginal.empty()) {
        m_vertices = m_verticesOriginal;
    }
}

void DJControllerGeometry::applyLerpInterpolation() {
    // Apply smooth lerp transitions to control rotations
    // This ensures knobs, faders, etc. don't "snap" to new positions
    
    const float lerpSpeed = 0.1f;  // ~100ms to fully interpolate (10% per frame @ 60FPS)
    
    for (auto& [id, target] : targetRotations_) {
        auto it = currentRotations_.find(id);
        if (it != currentRotations_.end()) {
            // Explicit lerp: current += (target - current) * lerpSpeed
            it->second = it->second + (target - it->second) * lerpSpeed;
            
            // Update vertex positions for this control based on currentRotation
            // This is handled by updateEQKnobRotation and other position update methods
        }
    }
}

} // namespace dj

