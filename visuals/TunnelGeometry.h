#pragma once

#include <cstdint>
#include <vector>

namespace dj {

/// Vertex structure with position, normal, and UV coordinates
#ifndef DJROOFRAT_VERTEX_STRUCT_DEFINED
#define DJROOFRAT_VERTEX_STRUCT_DEFINED
struct Vertex {
    float position[3];
    float normal[3];
    float texCoord[2];  // UV coordinates in [0,1] range
};
#endif

/// TunnelGeometry generates procedural cylindrical tunnel mesh
/// for music-reactive visual effects with scrolling textures
class TunnelGeometry {
public:
    TunnelGeometry();
    ~TunnelGeometry() = default;

    /// Generate procedural tunnel geometry
    /// @param segments Number of angular divisions around the cylinder (e.g., 32)
    /// @param rings Number of axial divisions along the cylinder (e.g., 16)
    /// @param radius Cylinder radius in world units (e.g., 5.0f)
    void generate(uint32_t segments, uint32_t rings, float radius);

    /// Get vertex data
    const std::vector<Vertex>& getVertices() const { return vertices_; }
    
    /// Get index data for triangle rendering
    const std::vector<uint32_t>& getIndices() const { return indices_; }
    
    /// Get geometry parameters
    uint32_t getSegmentCount() const { return segmentCount_; }
    uint32_t getRingCount() const { return ringCount_; }
    float getRadius() const { return radius_; }

private:
    std::vector<Vertex> vertices_;
    std::vector<uint32_t> indices_;
    
    uint32_t segmentCount_ = 0;
    uint32_t ringCount_ = 0;
    float radius_ = 5.0f;
    
    /// Generate cylindrical surface with inward-facing normals
    void generateCylinderSurface();
};

} // namespace dj
