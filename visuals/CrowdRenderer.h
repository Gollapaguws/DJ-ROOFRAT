#pragma once

#include <vector>
#include <memory>

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;
#endif

namespace dj {

struct InstanceData;
struct float3;
class CrowdMesh;
class CrowdAnimator;

class CrowdRenderer {
public:
    CrowdRenderer();
    ~CrowdRenderer();

    // Update crowd silhouette animation based on mood and energy
    // mood: integer representing crowd mood (0=Unimpressed, 1=Calm, 2=Grooving, 3=Hyped)
    // energy: crowd energy level in range [0, 1]
    // dtSeconds: time delta since last update
    void update(int mood, float energy, float dtSeconds);

    // Get animation motion amplitude in [0, 1]
    // Higher mood and energy produce higher amplitude
    float motionAmplitude() const noexcept;

    // Get crowd density in [0, 1]
    // Higher energy produces higher density
    float density() const noexcept;

    // Get number of visible crowd silhouettes
    // Typically 0-550 depending on mood and energy
    int visibleSilhouettes() const noexcept;

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // ===== Phase 20: Instanced Rendering Support =====

    // Initialize GPU resources for instanced rendering
    bool initialize(ID3D11Device* device);

    // Add instance to be rendered
    void addInstance(const InstanceData& instance);

    // Get number of instances
    int getInstanceCount() const noexcept;

    // Clear all instances
    void clearInstances();

    // Calculate LOD level based on distance
    // Returns 0 (LOD0), 1 (LOD1), or 2 (LOD2)
    int calculateLODLevel(const float3& crowdPos, const float3& cameraPos) const noexcept;

    // Render instanced crowd
    bool renderInstanced(ID3D11DeviceContext* context, const float3& cameraPos);
#else
    // Non-graphics stubs
    bool initialize(void* device) { return false; }
    void addInstance(const InstanceData& instance) {}
    int getInstanceCount() const noexcept { return 0; }
    void clearInstances() {}
    int calculateLODLevel(const struct float3& crowdPos, const struct float3& cameraPos) const noexcept { return 0; }
    bool renderInstanced(void* context, const struct float3& cameraPos) { return false; }
#endif

private:
    float motionAmplitude_;
    float density_;
    int visibleSilhouettes_;
    float animationPhase_;  // Track animation phase over time

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    std::unique_ptr<CrowdMesh> crowdMesh_;
    std::unique_ptr<CrowdAnimator> animator_;

    std::vector<InstanceData> instances_;
    ComPtr<ID3D11Buffer> instanceBuffer_;  // GPU instance buffer
    int maxInstances_ = 1000;
#endif
};

} // namespace dj
