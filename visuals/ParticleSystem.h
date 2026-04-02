#pragma once

#include <memory>

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();

#if defined(DJROOFRAT_OPENGL_MIGRATION)
    // OpenGL migration: initialize and update using OpenGL
    bool initialize(void* unused = nullptr, int maxParticles = 10000);
    void updatePhysics(void* unused, float deltaTime, const float gravity[3], const float windForce[3]);
#else
    // DirectX path
    bool initialize(ID3D11Device* device, int maxParticles = 10000);
    void updatePhysics(ID3D11DeviceContext* context, float deltaTime, const float gravity[3], const float windForce[3]);
#endif

    void emitParticles(const float position[3], int count, float lifetime, const float baseVelocity[3]);
    int render(
#if defined(DJROOFRAT_OPENGL_MIGRATION)
        void*
#else
        ID3D11DeviceContext* context
#endif
    );
    int getActiveParticleCount() const noexcept { return activeParticleCount_; }
    int getMaxParticles() const noexcept { return maxParticles_; }
    float* getParticlePosition(int index);
    void triggerConfettiBurst(const float position[3], int particleCount = 500);
    void reset();
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    ID3D11UnorderedAccessView* getParticleUAV() const { return particleUAV_.Get(); }
#endif

private:
    int maxParticles_ = 10000;
    int activeParticleCount_ = 0;
    int emissionCursor_ = 0;
#if defined(DJROOFRAT_OPENGL_MIGRATION)
    GLuint ssbo_ = 0;
    GLuint ubo_ = 0;
    std::unique_ptr<ComputeShader> computeShader_;
    std::vector<Particle> stagingBuffer_;
#elif defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    ComPtr<ID3D11Buffer> particleBuffer_;
    ComPtr<ID3D11Buffer> particleBufferCopy_;
    ComPtr<ID3D11ShaderResourceView> particleSRV_;
    ComPtr<ID3D11UnorderedAccessView> particleUAV_;
    ComPtr<ID3D11Buffer> constantBuffer_;
    std::unique_ptr<ComputeShader> computeShader_;
    std::vector<Particle> stagingBuffer_;
#endif
};

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
    bool initialize(void*, int = 10000) { return false; }
    void emitParticles(const float[3], int, float, const float[3]) {}
    void updatePhysics(void*, float, const float[3], const float[3]) {}
    int render(void*) { return 0; }
    int getActiveParticleCount() const { return 0; }
    int getMaxParticles() const { return 0; }
    float* getParticlePosition(int) { return nullptr; }
    void triggerConfettiBurst(const float[3], int = 500) {}
    void reset() {}
    void* getParticleUAV() const { return nullptr; }
};
} // namespace dj

#endif
