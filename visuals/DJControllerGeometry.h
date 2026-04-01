#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include "RayCaster.h"

namespace dj {

// Forward declarations
class Deck;
class Mixer;

#ifndef DJROOFRAT_VERTEX_STRUCT_DEFINED
#define DJROOFRAT_VERTEX_STRUCT_DEFINED
struct Vertex {
    float position[3];
    float normal[3];
    float texCoord[2];  // UV coordinates in [0,1] range
};
#endif

enum class ControlID {
    None,
    Crossfader,
    JogWheelA, JogWheelB,
    PitchA, PitchB,
    EQLowA, EQMidA, EQHighA,
    EQLowB, EQMidB, EQHighB,
    PlayA, PlayB
};

struct ControlBounds {
    ControlID id;
    float centerX;
    float centerY;
    float centerZ;
    float radius;
};

class DJControllerGeometry {
public:
    DJControllerGeometry();
    ~DJControllerGeometry() = default;
    
    // Generate procedural controller geometry
    void generateMesh();
    
    // Get vertex and index data
    const std::vector<Vertex>& getVertices() const { return m_vertices; }
    const std::vector<uint32_t>& getIndices() const { return m_indices; }
    const std::vector<ControlBounds>& getControlBounds() const { return m_controlBounds; }
    
    // Ray picking: detect which controller component is under the cursor
    ControlID pickControl(const Ray& ray) const;
    
    // Phase 5: Update controller visuals to reflect deck and mixer state
    void updateVisuals(const Deck& deckA, const Deck& deckB, const Mixer& mixer);
    
    // Phase 6: Hover highlight and visual smoothing
    void setHoverHighlight(ControlID control);
    void clearHoverHighlight();
    
private:
    // Primitive generators
    void createKnob(float x, float y, float z, float radius, float height, ControlID id);
    void createFader(float x, float y, float z, float width, float height, ControlID id);
    void createButton(float x, float y, float z, float size, ControlID id);
    void createJogWheel(float x, float y, float z, float radius, ControlID id);
    void createCrossfader(float x, float y, float z, float width, ControlID id);
    
    // Helper for adding vertices
    void addVertex(float x, float y, float z, 
                   float nx, float ny, float nz,
                   float u, float v);
    
    // Helper for adding triangle indices
    void addTriangle(uint32_t i0, uint32_t i1, uint32_t i2);
    
    // Helper for adding control bounds
    void addControlBounds(ControlID id, float x, float y, float z, float radius);
    
    // Phase 5: Update helper methods for visuals
    void updateCrossfaderPosition(float crossfaderValue);
    void updatePlayButtonGlow(ControlID button, bool isPlaying);
    void updateEQKnobRotation(ControlID knob, float gainValue);
    void updatePitchFaderPosition(ControlID fader, float tempoPercent);
    
    // Phase 6: Helper for lerp interpolation
    void applyLerpInterpolation();
    
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    std::vector<ControlBounds> m_controlBounds;
    
    // Phase 5: Store original vertex positions for updates and track vertex ranges per control
    std::vector<Vertex> m_verticesOriginal;
    struct VertexRange {
        uint32_t start;
        uint32_t count;
    };
    std::map<ControlID, VertexRange> m_vertexRanges;
    
    // Phase 6: Hover highlight state
    ControlID hoveredControl_ = ControlID::None;
    
    // Phase 6: Lerp interpolation for smooth transitions
    std::map<ControlID, float> targetRotations_;  // Target rotation values for knobs
    std::map<ControlID, float> currentRotations_; // Current rotation values (lerp)
    
    // Constants for geometry (SCALED UP 10x for visibility)
    static constexpr float CONTROLLER_WIDTH = 40.0f;
    static constexpr float CONTROLLER_DEPTH = 30.0f;
    static constexpr float BASE_HEIGHT = 0.05f;
    
    // Control dimensions (SCALED UP 10x for visibility)
    static constexpr float KNOB_RADIUS = 1.5f;
    static constexpr float KNOB_HEIGHT = 1.2f;
    static constexpr float JOG_WHEEL_RADIUS = 3.0f;
    static constexpr float PITCH_FADER_WIDTH = 0.8f;
    static constexpr float PITCH_FADER_HEIGHT = 6.0f;
    static constexpr float BUTTON_SIZE = 1.2f;
    static constexpr float CROSSFADER_WIDTH = 8.0f;
    
    // Control positions (MOVE FORWARD so camera at Z=-8 can see it)
    // Controller will be centered at Z=20, spanning Z=5 to Z=35
    static constexpr float DECK_A_X = -15.0f;
    static constexpr float DECK_B_X = 15.0f;
    static constexpr float CONTROL_Y = 5.0f;
    static constexpr float JOG_WHEEL_Z = 10.0f;   // Changed from -10.0f - jog wheels at front
    static constexpr float PITCH_FADER_Z = 30.0f;  // Changed from 10.0f - faders at back
    static constexpr float EQ_CENTER_Z = 19.7f;    // Changed from -0.3f - EQ knobs in middle
    static constexpr float EQ_SPACING_Z = 3.0f;    // Changed from 0.3f - scaled spacing
    static constexpr float BUTTON_Z = 9.0f;        // Changed from -1.0f - buttons near jog wheels
    static constexpr float CROSSFADER_Z = 20.8f;   // Changed from 0.8f - crossfader center
};

} // namespace dj
