#include "visuals/StageGeometry.h"

#include <cmath>

namespace dj {

StageGeometry::StageGeometry() = default;

void StageGeometry::generate() {
    vertices_.clear();
    indices_.clear();

    generateFloor();
    generateWalls();
    generateBooth();
}

void StageGeometry::generateFloor() {
    const float width = 8.0f;
    const float depth = 4.0f;
    const int subdivisionsX = 16;
    const int subdivisionsZ = 8;

    uint32_t vertexIndexOffset = static_cast<uint32_t>(vertices_.size());

    // Create floor grid
    for (int z = 0; z <= subdivisionsZ; ++z) {
        for (int x = 0; x <= subdivisionsX; ++x) {
            float xPos = (x / (float)subdivisionsX - 0.5f) * width;
            float zPos = (z / (float)subdivisionsZ - 0.5f) * depth;
            
            Vertex v;
            v.position[0] = xPos;
            v.position[1] = 0.0f;
            v.position[2] = zPos;
            v.normal[0] = 0.0f;
            v.normal[1] = 1.0f;
            v.normal[2] = 0.0f;
            
            vertices_.push_back(v);
        }
    }

    // Generate floor indices (triangle strips)
    for (int z = 0; z < subdivisionsZ; ++z) {
        for (int x = 0; x < subdivisionsX; ++x) {
            uint32_t topLeft = vertexIndexOffset + z * (subdivisionsX + 1) + x;
            uint32_t topRight = topLeft + 1;
            uint32_t bottomLeft = topLeft + (subdivisionsX + 1);
            uint32_t bottomRight = bottomLeft + 1;

            // First triangle
            indices_.push_back(topLeft);
            indices_.push_back(bottomLeft);
            indices_.push_back(topRight);

            // Second triangle
            indices_.push_back(topRight);
            indices_.push_back(bottomLeft);
            indices_.push_back(bottomRight);
        }
    }
}

void StageGeometry::generateWalls() {
    const float width = 8.0f;
    const float depth = 4.0f;
    const float height = 3.0f;
    const int subdivisions = 8;

    uint32_t vertexIndexOffset = static_cast<uint32_t>(vertices_.size());

    // Left wall
    for (int i = 0; i <= subdivisions; ++i) {
        for (int j = 0; j <= 1; ++j) {
            float xPos = -width / 2.0f;
            float yPos = j * height;
            float zPos = (i / (float)subdivisions - 0.5f) * depth;
            
            Vertex v;
            v.position[0] = xPos;
            v.position[1] = yPos;
            v.position[2] = zPos;
            v.normal[0] = -1.0f;
            v.normal[1] = 0.0f;
            v.normal[2] = 0.0f;
            
            vertices_.push_back(v);
        }
    }

    // Right wall
    for (int i = 0; i <= subdivisions; ++i) {
        for (int j = 0; j <= 1; ++j) {
            float xPos = width / 2.0f;
            float yPos = j * height;
            float zPos = (i / (float)subdivisions - 0.5f) * depth;
            
            Vertex v;
            v.position[0] = xPos;
            v.position[1] = yPos;
            v.position[2] = zPos;
            v.normal[0] = 1.0f;
            v.normal[1] = 0.0f;
            v.normal[2] = 0.0f;
            
            vertices_.push_back(v);
        }
    }

    // Generate wall indices
    // Left wall
    for (int i = 0; i < subdivisions; ++i) {
        uint32_t v0 = vertexIndexOffset + i * 2;
        uint32_t v1 = v0 + 1;
        uint32_t v2 = v0 + 2;
        uint32_t v3 = v2 + 1;

        indices_.push_back(v0);
        indices_.push_back(v2);
        indices_.push_back(v1);

        indices_.push_back(v1);
        indices_.push_back(v2);
        indices_.push_back(v3);
    }

    // Right wall
    uint32_t rightWallOffset = vertexIndexOffset + (subdivisions + 1) * 2;
    for (int i = 0; i < subdivisions; ++i) {
        uint32_t v0 = rightWallOffset + i * 2;
        uint32_t v1 = v0 + 1;
        uint32_t v2 = v0 + 2;
        uint32_t v3 = v2 + 1;

        indices_.push_back(v1);
        indices_.push_back(v2);
        indices_.push_back(v0);

        indices_.push_back(v3);
        indices_.push_back(v2);
        indices_.push_back(v1);
    }
}

void StageGeometry::generateBooth() {
    const float boothSize = 2.0f;
    const float boothHeight = 1.0f;

    uint32_t vertexIndexOffset = static_cast<uint32_t>(vertices_.size());

    // DJ booth top surface
    float positions[4][3] = {
        {-boothSize / 2.0f, boothHeight, -boothSize / 2.0f},
        { boothSize / 2.0f, boothHeight, -boothSize / 2.0f},
        {-boothSize / 2.0f, boothHeight,  boothSize / 2.0f},
        { boothSize / 2.0f, boothHeight,  boothSize / 2.0f}
    };

    for (int i = 0; i < 4; ++i) {
        Vertex v;
        v.position[0] = positions[i][0];
        v.position[1] = positions[i][1];
        v.position[2] = positions[i][2];
        v.normal[0] = 0.0f;
        v.normal[1] = 1.0f;
        v.normal[2] = 0.0f;
        vertices_.push_back(v);
    }

    // Booth top indices
    indices_.push_back(vertexIndexOffset + 0);
    indices_.push_back(vertexIndexOffset + 2);
    indices_.push_back(vertexIndexOffset + 1);

    indices_.push_back(vertexIndexOffset + 1);
    indices_.push_back(vertexIndexOffset + 2);
    indices_.push_back(vertexIndexOffset + 3);

    // Booth sides
    uint32_t sideVertexOffset = static_cast<uint32_t>(vertices_.size());
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j <= 1; ++j) {
            float y = (1.0f - j) * boothHeight;
            Vertex v;
            v.position[0] = positions[i][0];
            v.position[1] = y;
            v.position[2] = positions[i][2];
            
            // Normal pointing outward
            float nx = (i == 0 || i == 2) ? -1.0f : 1.0f;
            float nz = (i == 0 || i == 1) ? -1.0f : 1.0f;
            v.normal[0] = nx;
            v.normal[1] = 0.0f;
            v.normal[2] = nz;
            
            vertices_.push_back(v);
        }
    }
}

} // namespace dj
