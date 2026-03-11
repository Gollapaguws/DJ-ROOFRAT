#pragma once

#include <cstdint>
#include <vector>

namespace dj {

struct Vertex {
    float position[3];
    float normal[3];
};

class StageGeometry {
public:
    StageGeometry();
    ~StageGeometry() = default;

    // Generate procedural stage geometry
    void generate();

    // Get vertex and index data
    const std::vector<Vertex>& getVertices() const { return vertices_; }
    const std::vector<uint32_t>& getIndices() const { return indices_; }

    // Get geometry components
    void generateFloor();
    void generateWalls();
    void generateBooth();

private:
    std::vector<Vertex> vertices_;
    std::vector<uint32_t> indices_;
};

} // namespace dj
