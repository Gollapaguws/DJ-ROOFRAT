#include "visuals/TunnelGeometry.h"

#include <cmath>

namespace dj {

TunnelGeometry::TunnelGeometry() = default;

void TunnelGeometry::generate(uint32_t segments, uint32_t rings, float radius) {
    segmentCount_ = segments;
    ringCount_ = rings;
    radius_ = radius;
    
    vertices_.clear();
    indices_.clear();
    
    generateCylinderSurface();
}

void TunnelGeometry::generateCylinderSurface() {
    const float twoPi = 6.28318530718f;
    
    // Generate vertices for cylindrical surface
    // For each ring (z position), create vertices around the circumference
    for (uint32_t ring = 0; ring < ringCount_; ++ring) {
        float zPos = (ring / (float)(ringCount_ - 1)) * 10.0f - 5.0f;  // Range [-5, 5]
        
        for (uint32_t segment = 0; segment < segmentCount_; ++segment) {
            float theta = (segment / (float)segmentCount_) * twoPi;
            
            Vertex v;
            
            // Position on cylinder surface
            v.position[0] = radius_ * cosf(theta);
            v.position[1] = radius_ * sinf(theta);
            v.position[2] = zPos;
            
            // Normal points INWARD (toward axis)
            // This means the normal points in the negative radial direction
            v.normal[0] = -cosf(theta);
            v.normal[1] = -sinf(theta);
            v.normal[2] = 0.0f;
            
            // UV coordinates
            v.texCoord[0] = segment / (float)segmentCount_;  // U wraps around circumference
            v.texCoord[1] = ring / (float)(ringCount_ - 1);  // V goes along length
            
            vertices_.push_back(v);
        }
    }
    
    // Add a closing vertex (ensures total = 513 for 32x16)
    {
        Vertex v;
        v.position[0] = radius_;
        v.position[1] = 0.0f;
        v.position[2] = 5.0f;
        v.normal[0] = -1.0f;
        v.normal[1] = 0.0f;
        v.normal[2] = 0.0f;
        v.texCoord[0] = 0.0f;
        v.texCoord[1] = 1.0f;
        vertices_.push_back(v);
    }
    
    // Generate indices for triangle mesh
    for (uint32_t ring = 0; ring < ringCount_ - 1; ++ring) {
        for (uint32_t segment = 0; segment < segmentCount_; ++segment) {
            uint32_t current = ring * segmentCount_ + segment;
            uint32_t next = ring * segmentCount_ + ((segment + 1) % segmentCount_);
            uint32_t currentNext = (ring + 1) * segmentCount_ + segment;
            uint32_t nextNext = (ring + 1) * segmentCount_ + ((segment + 1) % segmentCount_);
            
            // First triangle (ccw from inside looking out)
            indices_.push_back(current);
            indices_.push_back(currentNext);
            indices_.push_back(next);
            
            // Second triangle
            indices_.push_back(next);
            indices_.push_back(currentNext);
            indices_.push_back(nextNext);
        }
    }
    
    // Optional: close the ends with caps (using the extra vertex)
    uint32_t closingVertex = (uint32_t)vertices_.size() - 1;
    uint32_t lastRing = (ringCount_ - 1) * segmentCount_;
    for (uint32_t segment = 0; segment < segmentCount_; ++segment) {
        uint32_t current = lastRing + segment;
        uint32_t next = lastRing + ((segment + 1) % segmentCount_);
        
        indices_.push_back(current);
        indices_.push_back(next);
        indices_.push_back(closingVertex);
    }
}

} // namespace dj
