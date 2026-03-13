#pragma once

#include <memory>
#include <vector>

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#define NOMINMAX
#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace dj {

// Particle structure matching shader (must be 16-byte aligned)
struct Particle {
    float position[3];      // 12 bytes
    float lifetime;          // 4 bytes, total 16
    float velocity[3];       // 12 bytes
    float initialLife;       // 4 bytes, total 16
    float color[4];          // 16 bytes
    float size;              // 4 bytes
    float padding[3];        // 12 bytes padding to align to 16
};

// Constant buffer for compute shader (must be 16-byte aligned)
struct ParticleConstants {
    float gravity[3];        // 12 bytes
    float deltaTime;         // 4 bytes, total 16
    float windForce[3];      // 12 bytes
    int particleCount;       // 4 bytes, total 16
};

class ComputeShader;

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();

    // Initialize particle system with default capacity (10,000 particles)
    bool initialize(ID3D11Device* device, int maxParticles = 10000);

    // Emit particles from a position
    // position: emission point
    // count: number of particles to emit
    // lifetime: particle lifetime in seconds
    // baseVelocity: initial velocity direction
    void emitParticles(const float position[3], int count, float lifetime, const float baseVelocity[3]);

    // Update physics using compute shader
    void updatePhysics(ID3D11DeviceContext* context, float deltaTime,
                       const float gravity[3], const float windForce[3]);

    // Render particles (returns vertex/index count for caller to render)
    int render(ID3D11DeviceContext* context);

    // Get active particle count
    int getActiveParticleCount() const noexcept { return activeParticleCount_; }

    // Get max particle capacity
    int getMaxParticles() const noexcept { return maxParticles_; }

    // Get particle position (for testing)
    // Note: This requires GPU readback and is slow - use sparingly
    float* getParticlePosition(int index);

    // Trigger confetti burst for pyrotechnics
    void triggerConfettiBurst(const float position[3], int particleCount = 500);

    // Reset all particles
    void reset();

    // Get UAV for compute shader
    ID3D11UnorderedAccessView* getParticleUAV() const { return particleUAV_.Get(); }

private:
    int maxParticles_ = 10000;
    int activeParticleCount_ = 0;
    int emissionCursor_ = 0;
    
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // GPU resources
    ComPtr<ID3D11Buffer> particleBuffer_;              // Structured buffer on GPU
    ComPtr<ID3D11Buffer> particleBufferCopy_;          // Copy for CPU readback
    ComPtr<ID3D11ShaderResourceView> particleSRV_;     // For rendering
    ComPtr<ID3D11UnorderedAccessView> particleUAV_;    // For compute shader
    ComPtr<ID3D11Buffer> constantBuffer_;              // For compute shader constants
    
    std::unique_ptr<ComputeShader> computeShader_;
    
    // Temporary system memory buffer for CPU->GPU transfers
    std::vector<Particle> stagingBuffer_;
#endif
};

} // namespace dj

#else

namespace dj {
class ParticleSystem {
public:
    ParticleSystem() = default;
    ~ParticleSystem() = default;
    bool initialize(void* device, int maxParticles = 10000) { return false; }
    void emitParticles(const float position[3], int count, float lifetime, const float baseVelocity[3]) {}
    void updatePhysics(void* context, float deltaTime, const float gravity[3], const float windForce[3]) {}
    int render(void* context) { return 0; }
    int getActiveParticleCount() const { return 0; }
    int getMaxParticles() const { return 0; }
    float* getParticlePosition(int index) { return nullptr; }
    void triggerConfettiBurst(const float position[3], int particleCount = 500) {}
    void reset() {}
    void* getParticleUAV() const { return nullptr; }
};
} // namespace dj

#endif
