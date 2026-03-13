#include "visuals/CrowdMesh.h"

#include <cmath>
#include <numbers>

namespace dj {

CrowdMesh::CrowdMesh() = default;
CrowdMesh::~CrowdMesh() = default;

bool CrowdMesh::generateLOD(ID3D11Device* device, LODLevel lodLevel) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!device) {
        return false;
    }

    int lodIndex = static_cast<int>(lodLevel);
    if (lodIndex < 0 || lodIndex > 2) {
        return false;
    }

    // Generate geometry based on LOD level
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    switch (lodLevel) {
    case LODLevel::LOD0:
        generateLOD0Geometry(vertices, indices);
        break;
    case LODLevel::LOD1:
        generateLOD1Geometry(vertices, indices);
        break;
    case LODLevel::LOD2:
        generateLOD2Geometry(vertices, indices);
        break;
    }

    if (vertices.empty() || indices.empty()) {
        return false;
    }

    // Create vertex buffer
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(vertices.size());
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices.data();

    HRESULT hr = device->CreateBuffer(&vbDesc, &vbData, lodData_[lodIndex].vertexBuffer.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    // Create index buffer
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = sizeof(uint32_t) * static_cast<uint32_t>(indices.size());
    ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices.data();

    hr = device->CreateBuffer(&ibDesc, &ibData, lodData_[lodIndex].indexBuffer.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    // Store data for reference
    lodData_[lodIndex].vertices = vertices;
    lodData_[lodIndex].indices = indices;

    return true;
#else
    return false;
#endif
}

ID3D11Buffer* CrowdMesh::getVertexBuffer(LODLevel lodLevel) const {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    int lodIndex = static_cast<int>(lodLevel);
    if (lodIndex >= 0 && lodIndex <= 2) {
        return lodData_[lodIndex].vertexBuffer.Get();
    }
#endif
    return nullptr;
}

ID3D11Buffer* CrowdMesh::getIndexBuffer(LODLevel lodLevel) const {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    int lodIndex = static_cast<int>(lodLevel);
    if (lodIndex >= 0 && lodIndex <= 2) {
        return lodData_[lodIndex].indexBuffer.Get();
    }
#endif
    return nullptr;
}

int CrowdMesh::getVertexCount(LODLevel lodLevel) const {
    int lodIndex = static_cast<int>(lodLevel);
    if (lodIndex >= 0 && lodIndex <= 2) {
        return static_cast<int>(lodData_[lodIndex].vertices.size());
    }
    return 0;
}

int CrowdMesh::getIndexCount(LODLevel lodLevel) const {
    int lodIndex = static_cast<int>(lodLevel);
    if (lodIndex >= 0 && lodIndex <= 2) {
        return static_cast<int>(lodData_[lodIndex].indices.size());
    }
    return 0;
}

void CrowdMesh::generateLOD0Geometry(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
    // LOD0: High detail - simplified stick figure with sphere joints and cylinder limbs
    // Create a simple humanoid shape: head (sphere) + body (boxes) + limbs (cylinders)

    // Head - sphere
    createSphere(vertices, indices, 0.3f, 8, 8);

    // Body - box
    uint32_t bodyStart = static_cast<uint32_t>(vertices.size());
    createCube(vertices, indices, 0.4f, 0.6f, 0.2f);

    // Left arm
    uint32_t leftArmStart = static_cast<uint32_t>(vertices.size());
    createCube(vertices, indices, 0.2f, 0.5f, 0.15f);

    // Right arm
    uint32_t rightArmStart = static_cast<uint32_t>(vertices.size());
    createCube(vertices, indices, 0.2f, 0.5f, 0.15f);

    // Left leg
    uint32_t leftLegStart = static_cast<uint32_t>(vertices.size());
    createCube(vertices, indices, 0.15f, 0.6f, 0.15f);

    // Right leg
    uint32_t rightLegStart = static_cast<uint32_t>(vertices.size());
    createCube(vertices, indices, 0.15f, 0.6f, 0.15f);
}

void CrowdMesh::generateLOD1Geometry(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
    // LOD1: Medium detail - simplified capsule body with sphere head
    // Head
    createSphere(vertices, indices, 0.25f, 6, 6);

    // Body - simplified box
    createCube(vertices, indices, 0.3f, 0.5f, 0.15f);

    // Legs - simple box
    createCube(vertices, indices, 0.25f, 0.4f, 0.15f);
}

void CrowdMesh::generateLOD2Geometry(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
    // LOD2: Low detail - simple silhouette (quad)
    // Create a vertical quad facing camera

    Vertex v0 = {{-0.2f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}};
    Vertex v1 = {{0.2f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}};
    Vertex v2 = {{0.2f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}};
    Vertex v3 = {{-0.2f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}};

    uint32_t start = static_cast<uint32_t>(vertices.size());
    vertices.push_back(v0);
    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v3);

    indices.push_back(start + 0);
    indices.push_back(start + 1);
    indices.push_back(start + 2);
    indices.push_back(start + 0);
    indices.push_back(start + 2);
    indices.push_back(start + 3);
}

void CrowdMesh::createCube(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
                           float width, float height, float depth) {
    float hw = width * 0.5f;
    float hh = height * 0.5f;
    float hd = depth * 0.5f;

    uint32_t start = static_cast<uint32_t>(vertices.size());

    // Create 8 vertices of cube
    vertices.push_back({{-hw, -hh, -hd}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}});
    vertices.push_back({{hw, -hh, -hd}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}});
    vertices.push_back({{hw, hh, -hd}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}});
    vertices.push_back({{-hw, hh, -hd}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}});

    vertices.push_back({{-hw, -hh, hd}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}});
    vertices.push_back({{hw, -hh, hd}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}});
    vertices.push_back({{hw, hh, hd}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}});
    vertices.push_back({{-hw, hh, hd}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}});

    // Front face
    indices.push_back(start + 0);
    indices.push_back(start + 1);
    indices.push_back(start + 2);
    indices.push_back(start + 0);
    indices.push_back(start + 2);
    indices.push_back(start + 3);

    // Back face
    indices.push_back(start + 4);
    indices.push_back(start + 6);
    indices.push_back(start + 5);
    indices.push_back(start + 4);
    indices.push_back(start + 7);
    indices.push_back(start + 6);

    // Top face
    indices.push_back(start + 2);
    indices.push_back(start + 6);
    indices.push_back(start + 7);
    indices.push_back(start + 2);
    indices.push_back(start + 7);
    indices.push_back(start + 3);

    // Bottom face
    indices.push_back(start + 0);
    indices.push_back(start + 4);
    indices.push_back(start + 5);
    indices.push_back(start + 0);
    indices.push_back(start + 5);
    indices.push_back(start + 1);

    // Left face
    indices.push_back(start + 0);
    indices.push_back(start + 3);
    indices.push_back(start + 7);
    indices.push_back(start + 0);
    indices.push_back(start + 7);
    indices.push_back(start + 4);

    // Right face
    indices.push_back(start + 1);
    indices.push_back(start + 5);
    indices.push_back(start + 6);
    indices.push_back(start + 1);
    indices.push_back(start + 6);
    indices.push_back(start + 2);
}

void CrowdMesh::createSphere(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
                             float radius, int rings, int segments) {
    uint32_t start = static_cast<uint32_t>(vertices.size());

    // Generate sphere vertices
    for (int i = 0; i <= rings; ++i) {
        float phi = static_cast<float>(i) * std::numbers::pi_v<float> / rings;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        for (int j = 0; j <= segments; ++j) {
            float theta = static_cast<float>(j) * 2.0f * std::numbers::pi_v<float> / segments;
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);

            float x = radius * sinPhi * cosTheta;
            float y = radius * cosPhi;
            float z = radius * sinPhi * sinTheta;

            Vertex v;
            v.position[0] = x;
            v.position[1] = y;
            v.position[2] = z;
            v.normal[0] = x / radius;
            v.normal[1] = y / radius;
            v.normal[2] = z / radius;
            v.texCoord[0] = static_cast<float>(j) / segments;
            v.texCoord[1] = static_cast<float>(i) / rings;

            vertices.push_back(v);
        }
    }

    // Generate sphere indices
    for (int i = 0; i < rings; ++i) {
        for (int j = 0; j < segments; ++j) {
            uint32_t a = start + i * (segments + 1) + j;
            uint32_t b = a + 1;
            uint32_t c = start + (i + 1) * (segments + 1) + j;
            uint32_t d = c + 1;

            indices.push_back(a);
            indices.push_back(c);
            indices.push_back(b);
            indices.push_back(b);
            indices.push_back(c);
            indices.push_back(d);
        }
    }
}

} // namespace dj
