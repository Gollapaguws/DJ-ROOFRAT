#pragma once

#include <memory>
#include <vector>

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace dj {

struct Vertex {
    float position[3];
    float normal[3];
    float texCoord[2];
};

class CrowdMesh {
public:
    enum class LODLevel {
        LOD0 = 0,  // High detail (~500 vertices, full articulation)
        LOD1 = 1,  // Medium detail (~100 vertices, simplified)
        LOD2 = 2   // Low detail (~20 vertices, silhouette)
    };

    CrowdMesh();
    ~CrowdMesh();

    // Generate LOD mesh and create GPU buffers
    bool generateLOD(ID3D11Device* device, LODLevel lodLevel);

    // Get vertex buffer for a LOD
    ID3D11Buffer* getVertexBuffer(LODLevel lodLevel) const;
    ID3D11Buffer* getIndexBuffer(LODLevel lodLevel) const;

    // Get vertex/index counts for a LOD
    int getVertexCount(LODLevel lodLevel) const;
    int getIndexCount(LODLevel lodLevel) const;

private:
    struct LODData {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        ComPtr<ID3D11Buffer> vertexBuffer;
        ComPtr<ID3D11Buffer> indexBuffer;
    };

    // Generate procedural LOD geometry
    void generateLOD0Geometry(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    void generateLOD1Geometry(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    void generateLOD2Geometry(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

    // Helper: Create cube mesh
    void createCube(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, 
                    float width, float height, float depth);

    // Helper: Create sphere mesh
    void createSphere(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
                      float radius, int rings, int segments);

    LODData lodData_[3];  // One for each LOD level
};

} // namespace dj

#else

namespace dj {
class CrowdMesh {
public:
    enum class LODLevel { LOD0 = 0, LOD1 = 1, LOD2 = 2 };
    CrowdMesh() = default;
    ~CrowdMesh() = default;
    bool generateLOD(void* device, LODLevel lodLevel) { return false; }
    void* getVertexBuffer(LODLevel lodLevel) const { return nullptr; }
    void* getIndexBuffer(LODLevel lodLevel) const { return nullptr; }
    int getVertexCount(LODLevel lodLevel) const { return 0; }
    int getIndexCount(LODLevel lodLevel) const { return 0; }
};
} // namespace dj

#endif
