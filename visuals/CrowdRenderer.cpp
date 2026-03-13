#include "visuals/CrowdRenderer.h"
#include "visuals/CrowdMesh.h"
#include "visuals/CrowdAnimator.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace dj {

CrowdRenderer::CrowdRenderer()
    : motionAmplitude_(0.0f), density_(0.0f), visibleSilhouettes_(0), animationPhase_(0.0f) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    crowdMesh_ = std::make_unique<CrowdMesh>();
    animator_ = std::make_unique<CrowdAnimator>();
#endif
}

CrowdRenderer::~CrowdRenderer() = default;

void CrowdRenderer::update(int mood, float energy, float dtSeconds) {
    // Clamp inputs to safe ranges
    const float clampedEnergy = std::clamp(energy, 0.0f, 1.0f);
    const float clampedDt = std::clamp(dtSeconds, 0.0f, 1.0f);

    // Clamp mood to valid range [0, 3] representing (Unimpressed, Calm, Grooving, Hyped)
    const int clampedMood = std::clamp(mood, 0, 3);

    // Map mood to amplitude scaling factor
    // 0 (Unimpressed) -> 0.1, 1 (Calm) -> 0.3, 2 (Grooving) -> 0.6, 3 (Hyped) -> 1.0
    float moodFactor = 0.0f;
    switch (clampedMood) {
    case 0:  // Unimpressed
        moodFactor = 0.1f;
        break;
    case 1:  // Calm
        moodFactor = 0.3f;
        break;
    case 2:  // Grooving
        moodFactor = 0.6f;
        break;
    case 3:  // Hyped
        moodFactor = 1.0f;
        break;
    }

    // Motion amplitude driven by mood * energy
    motionAmplitude_ = std::clamp(moodFactor * clampedEnergy, 0.0f, 1.0f);

    // Density driven mainly by energy with mood modulation
    density_ = std::clamp(clampedEnergy * (0.5f + moodFactor * 0.5f), 0.0f, 1.0f);

    // Update animation phase for smooth motion
    animationPhase_ += clampedDt * 3.0f;  // ~3 cycles per second base rate
    if (animationPhase_ > 2.0f * std::numbers::pi_v<float>) {
        animationPhase_ -= 2.0f * std::numbers::pi_v<float>;
    }

    // Visible silhouettes range from 0 to ~500 based on mood and energy
    // Hyped mood at full energy -> ~500 silhouettes
    // Unimpressed mood at zero energy -> ~0 silhouettes
    const float maxSilhouettes = 500.0f;
    int silhouetteCount = static_cast<int>(
        moodFactor * clampedEnergy * maxSilhouettes +
        std::abs(std::sin(animationPhase_)) * moodFactor * 50.0f  // Add some oscillation
    );
    visibleSilhouettes_ = (silhouetteCount > 0) ? silhouetteCount : 0;
}

float CrowdRenderer::motionAmplitude() const noexcept {
    return motionAmplitude_;
}

float CrowdRenderer::density() const noexcept {
    return density_;
}

int CrowdRenderer::visibleSilhouettes() const noexcept {
    return visibleSilhouettes_;
}

// ===== Phase 20: Instanced Rendering Implementation =====

bool CrowdRenderer::initialize(ID3D11Device* device) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!device || !crowdMesh_ || !animator_) {
        return false;
    }

    // Generate all LOD meshes
    if (!crowdMesh_->generateLOD(device, CrowdMesh::LODLevel::LOD0) ||
        !crowdMesh_->generateLOD(device, CrowdMesh::LODLevel::LOD1) ||
        !crowdMesh_->generateLOD(device, CrowdMesh::LODLevel::LOD2)) {
        return false;
    }

    // Create instance buffer
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = sizeof(InstanceData) * maxInstances_;
    ibDesc.Usage = D3D11_USAGE_DYNAMIC;
    ibDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    ibDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = device->CreateBuffer(&ibDesc, nullptr, instanceBuffer_.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    return true;
#else
    return false;
#endif
}

void CrowdRenderer::addInstance(const InstanceData& instance) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (instances_.size() < static_cast<size_t>(maxInstances_)) {
        instances_.push_back(instance);
    }
#endif
}

int CrowdRenderer::getInstanceCount() const noexcept {
    return static_cast<int>(instances_.size());
}

void CrowdRenderer::clearInstances() {
    instances_.clear();
}

int CrowdRenderer::calculateLODLevel(const float3& crowdPos, const float3& cameraPos) const noexcept {
    // Calculate distance
    float dx = crowdPos.x - cameraPos.x;
    float dy = crowdPos.y - cameraPos.y;
    float dz = crowdPos.z - cameraPos.z;
    float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

    // LOD selection based on distance
    if (distance < 5.0f) {
        return 0;  // LOD0 (high detail)
    } else if (distance < 15.0f) {
        return 1;  // LOD1 (medium detail)
    } else {
        return 2;  // LOD2 (low detail)
    }
}

bool CrowdRenderer::renderInstanced(ID3D11DeviceContext* context, const float3& cameraPos) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!context || !instanceBuffer_ || instances_.empty() || !crowdMesh_) {
        return false;
    }

    // Update instance buffer with current data
    D3D11_MAPPED_SUBRESOURCE resource = {};
    if (SUCCEEDED(context->Map(instanceBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource))) {
        std::memcpy(resource.pData, instances_.data(), sizeof(InstanceData) * instances_.size());
        context->Unmap(instanceBuffer_.Get(), 0);
    } else {
        return false;
    }

    // Render each instance with appropriate LOD
    for (size_t i = 0; i < instances_.size(); ++i) {
        const auto& instance = instances_[i];
        
        // Calculate LOD based on distance
        float3 instPos = {instance.position[0], instance.position[1], instance.position[2]};
        int lod = calculateLODLevel(instPos, cameraPos);
        lod = std::clamp(lod, 0, 2);

        CrowdMesh::LODLevel lodLevel = static_cast<CrowdMesh::LODLevel>(lod);

        // Get mesh buffers for this LOD
        ID3D11Buffer* vb = crowdMesh_->getVertexBuffer(lodLevel);
        ID3D11Buffer* ib = crowdMesh_->getIndexBuffer(lodLevel);

        if (!vb || !ib) {
            continue;
        }

        // Set vertex and instance buffers
        ID3D11Buffer* buffers[] = {vb, instanceBuffer_.Get()};
        UINT strides[] = {sizeof(Vertex), sizeof(InstanceData)};
        UINT offsets[] = {0, 0};
        context->IASetVertexBuffers(0, 2, buffers, strides, offsets);

        // Set index buffer
        context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

        // Draw indexed instanced
        int indexCount = crowdMesh_->getIndexCount(lodLevel);
        if (indexCount > 0) {
            context->DrawIndexedInstanced(indexCount, 1, 0, 0, i);
        }
    }

    return true;
#else
    return false;
#endif
}

} // namespace dj
