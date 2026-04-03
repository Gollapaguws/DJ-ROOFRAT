#include "visuals/ParticleSystem.h"
#include "visuals/ComputeShader.h"


#if defined(DJROOFRAT_OPENGL_MIGRATION)
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <fstream>
#endif

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)

#include <random>
#include <cmath>
#include <algorithm>

namespace dj {

ParticleSystem::ParticleSystem() = default;

ParticleSystem::~ParticleSystem() = default;


#if defined(DJROOFRAT_OPENGL_MIGRATION)
bool ParticleSystem::initialize(void*, int maxParticlesCount) {
    maxParticles_ = (maxParticlesCount > 0) ? maxParticlesCount : 10000;
    activeParticleCount_ = 0;
    stagingBuffer_.resize(maxParticles_);
    // OpenGL: create SSBO for particles
    glGenBuffers(1, &ssbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particle) * maxParticles_, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_);
    // Load and compile GLSL compute shader
    std::ifstream file("shaders/particles.glsl");
    std::string src((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    computeShader_ = std::make_unique<ComputeShader>();
    std::string errorMsg;
    if (!computeShader_->compileSource(src.c_str(), &errorMsg)) {
        // Log errorMsg if needed
    }
    // Create UBO for constants
    glGenBuffers(1, &ubo_);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_);
    struct ParticleConstantsGL {
        float gravity[3]; float deltaTime;
        float windForce[3]; int particleCount;
    };
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ParticleConstantsGL), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_);
    return true;
}
#else
bool ParticleSystem::initialize(ID3D11Device* device, int maxParticlesCount) {
    if (!device) {
        return false;
    }
    maxParticles_ = (maxParticlesCount > 0) ? maxParticlesCount : 10000;
    activeParticleCount_ = 0;
    // ...existing code...
    return true;
}
#endif

void ParticleSystem::emitParticles(const float position[3], int count, float lifetime, const float baseVelocity[3]) {
    count = std::clamp(count, 0, maxParticles_);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distributor(-0.5f, 0.5f);
    std::uniform_real_distribution<> lifeVariation(lifetime * 0.8f, lifetime * 1.2f);

    for (int i = 0; i < count; ++i) {
        // Ring-buffer slot: wraps around so dead particles are naturally overwritten
        int slot = emissionCursor_ % maxParticles_;
        emissionCursor_++;

        Particle p = {};
        p.position[0] = position[0] + static_cast<float>(distributor(gen)) * 0.2f;
        p.position[1] = position[1] + static_cast<float>(distributor(gen)) * 0.2f;
        p.position[2] = position[2] + static_cast<float>(distributor(gen)) * 0.2f;

        // Add random variation to base velocity
        p.velocity[0] = baseVelocity[0] + static_cast<float>(distributor(gen)) * 2.0f;
        p.velocity[1] = baseVelocity[1] + static_cast<float>(distributor(gen)) * 2.0f;
        p.velocity[2] = baseVelocity[2] + static_cast<float>(distributor(gen)) * 2.0f;

        p.lifetime = static_cast<float>(lifeVariation(gen));
        p.initialLife = p.lifetime;

        // Random color based on type (confetti is colorful)
        p.color[0] = static_cast<float>(distributor(gen)) + 0.5f;
        p.color[1] = static_cast<float>(distributor(gen)) + 0.5f;
        p.color[2] = static_cast<float>(distributor(gen)) + 0.5f;
        p.color[3] = 1.0f;  // Start opaque

        p.size = 0.05f;

        stagingBuffer_[slot] = p;
        if (activeParticleCount_ < maxParticles_) {
            activeParticleCount_++;
        }
    }
}

#if defined(DJROOFRAT_OPENGL_MIGRATION)
void ParticleSystem::updatePhysics(void*, float deltaTime, const float gravity[3], const float windForce[3]) {
    if (!computeShader_ || activeParticleCount_ <= 0) return;
    // Upload particle data to SSBO
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Particle) * activeParticleCount_, stagingBuffer_.data());
    // Update UBO constants
    struct ParticleConstantsGL {
        float gravity[3]; float deltaTime;
        float windForce[3]; int particleCount;
    } c;
    c.gravity[0] = gravity[0]; c.gravity[1] = gravity[1]; c.gravity[2] = gravity[2];
    c.deltaTime = deltaTime;
    c.windForce[0] = windForce[0]; c.windForce[1] = windForce[1]; c.windForce[2] = windForce[2];
    c.particleCount = activeParticleCount_;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ParticleConstantsGL), &c);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_);
    // Dispatch compute shader
    computeShader_->use();
    uint32_t numGroups = (activeParticleCount_ + 255) / 256;
    computeShader_->dispatch(numGroups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    // Optionally read back for CPU-side queries
}
#else
void ParticleSystem::updatePhysics(ID3D11DeviceContext* context, float deltaTime, const float gravity[3], const float windForce[3]) {
    if (!context || !computeShader_) {
        return;
    }
    if (activeParticleCount_ <= 0) {
        return;
    }
    // ...existing code...
}
#endif

#if defined(DJROOFRAT_OPENGL_MIGRATION)
int ParticleSystem::render(void* context) {
    (void)context;  // Placeholder - unused in current implementation
    //This is a placeholder for particle rendering
    // In a full implementation, this would set up vertex buffers and draw
    return 0;
}
#else
int ParticleSystem::render(ID3D11DeviceContext* context) {
    (void)context; // Placeholder - unused in current implementation
    // This is a placeholder for particle rendering
    // In a full implementation, this would set up vertex buffers and draw
    return 0;
}
#endif

float* ParticleSystem::getParticlePosition(int index) {
    if (index < 0 || index >= activeParticleCount_) {
        return nullptr;
    }

    // For testing, we return the particle from staging buffer
    // In production, we'd need GPU readback
    return stagingBuffer_[index].position;
}

void ParticleSystem::triggerConfettiBurst(const float position[3], int particleCount) {
    // Emit upward-moving particles with random colors (confetti effect)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> angleDistributor(0.0f, 2.0f * 3.14159f);
    std::uniform_real_distribution<> speedDistributor(5.0f, 10.0f);

    particleCount = std::clamp(particleCount, 0, maxParticles_);

    for (int i = 0; i < particleCount; ++i) {
        float angle = static_cast<float>(angleDistributor(gen));
        float speed = static_cast<float>(speedDistributor(gen));

        float velocity[3] = {
            std::cos(angle) * speed * 0.5f,
            speed * 2.0f,  // Upward
            std::sin(angle) * speed * 0.5f
        };

        emitParticles(position, 1, 3.0f, velocity);
    }
}

void ParticleSystem::reset() {
    activeParticleCount_ = 0;
    emissionCursor_ = 0;
    stagingBuffer_.clear();
    stagingBuffer_.resize(maxParticles_);
}

} // namespace dj

#endif // defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
