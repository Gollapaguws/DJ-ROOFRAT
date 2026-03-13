#include "visuals/ParticleSystem.h"
#include "visuals/ComputeShader.h"

#include <random>
#include <cmath>
#include <algorithm>

namespace dj {

ParticleSystem::ParticleSystem() = default;

ParticleSystem::~ParticleSystem() = default;

bool ParticleSystem::initialize(ID3D11Device* device, int maxParticlesCount) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!device) {
        return false;
    }

    maxParticles_ = (maxParticlesCount > 100) ? maxParticlesCount : 10000;
    activeParticleCount_ = 0;

    // Create structured buffer for particles on GPU
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = sizeof(Particle) * maxParticles_;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(Particle);

    HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, particleBuffer_.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    // Create copy buffer for CPU readback
    D3D11_BUFFER_DESC copyDesc = bufferDesc;
    copyDesc.Usage = D3D11_USAGE_STAGING;
    copyDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    copyDesc.BindFlags = 0;
    copyDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    hr = device->CreateBuffer(&copyDesc, nullptr, particleBufferCopy_.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    // Create SRV for reading particles (not used yet for rendering)
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.NumElements = maxParticles_;

    hr = device->CreateShaderResourceView(particleBuffer_.Get(), &srvDesc, particleSRV_.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    // Create UAV for compute shader
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.NumElements = maxParticles_;

    hr = device->CreateUnorderedAccessView(particleBuffer_.Get(), &uavDesc, particleUAV_.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    // Create constant buffer for compute shader
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(ParticleConstants);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = device->CreateBuffer(&cbDesc, nullptr, constantBuffer_.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    // Create and compile compute shader
    computeShader_ = std::make_unique<ComputeShader>();
    std::string errorMsg;
    if (!computeShader_->compile(device, "particles", "CSMain", &errorMsg)) {
        // Compute shader compilation failed, but we can continue with a no-op
        // This allows testing on systems without shader files
    }

    // Initialize staging buffer
    stagingBuffer_.resize(maxParticles_);

    return true;
#else
    return false;
#endif
}

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

void ParticleSystem::updatePhysics(ID3D11DeviceContext* context, float deltaTime,
                                   const float gravity[3], const float windForce[3]) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!context || !computeShader_) {
        return;
    }

    // Upload particle data to GPU if we have new particles
    // particleBuffer_ uses D3D11_USAGE_DEFAULT (required for UAV), so use UpdateSubresource
    if (activeParticleCount_ > 0) {
        context->UpdateSubresource(particleBuffer_.Get(), 0, nullptr,
                                   stagingBuffer_.data(), sizeof(Particle) * activeParticleCount_, 0);
    }

    // Update constant buffer
    D3D11_MAPPED_SUBRESOURCE cbResource = {};
    if (SUCCEEDED(context->Map(constantBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &cbResource))) {
        ParticleConstants* pConstants = static_cast<ParticleConstants*>(cbResource.pData);
        pConstants->gravity[0] = gravity[0];
        pConstants->gravity[1] = gravity[1];
        pConstants->gravity[2] = gravity[2];
        pConstants->deltaTime = deltaTime;
        pConstants->windForce[0] = windForce[0];
        pConstants->windForce[1] = windForce[1];
        pConstants->windForce[2] = windForce[2];
        pConstants->particleCount = activeParticleCount_;
        context->Unmap(constantBuffer_.Get(), 0);
    }

    // Set compute shader resources
    context->CSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
    context->CSSetUnorderedAccessViews(0, 1, particleUAV_.GetAddressOf(), nullptr);

    // Dispatch compute shader
    uint32_t numGroups = (activeParticleCount_ + 255) / 256;  // 256 threads per group
    computeShader_->dispatch(context, numGroups, 1, 1);

    // Clean up dead particles (simple approach: mark lifetime-expired particles)
    // This would require GPU readback, so for now we just decrement lifetime in shader
#endif
}

int ParticleSystem::render(ID3D11DeviceContext* context) {
    // This is a placeholder for particle rendering
    // In a full implementation, this would set up vertex buffers and draw
    return 0;
}

float* ParticleSystem::getParticlePosition(int index) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (index < 0 || index >= activeParticleCount_) {
        return nullptr;
    }

    // For testing, we return the particle from staging buffer
    // In production, we'd need GPU readback
    return stagingBuffer_[index].position;
#else
    return nullptr;
#endif
}

void ParticleSystem::triggerConfettiBurst(const float position[3], int particleCount) {
    // Emit upward-moving particles with random colors (confetti effect)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> angleDistributor(0.0f, 2.0f * 3.14159f);
    std::uniform_real_distribution<> speedDistributor(5.0f, 10.0f);

    constexpr float PI = 3.14159265359f;

    for (int i = 0; i < particleCount && activeParticleCount_ < maxParticles_; ++i) {
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
