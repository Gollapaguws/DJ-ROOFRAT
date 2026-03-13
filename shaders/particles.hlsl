// Particle structure (must match C++ Particle struct - 64 bytes)
struct Particle {
    float3 position;
    float lifetime;
    float3 velocity;
    float initialLife;
    float4 color;
    float size;
    float3 padding;
};

// Constant buffer for particle physics
cbuffer ParticleConstants : register(b0) {
    float3 Gravity;
    float DeltaTime;
    float3 WindForce;
    int ParticleCount;
};

// RW structured buffer for particles
RWStructuredBuffer<Particle> Particles : register(u0);

// Compute shader main function
// Launch with thread groups: (256, 1, 1) threads per group
// Number of groups: ceil(ParticleCount / 256)
[numthreads(256, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID) {
    uint idx = DTid.x;
    
    // Early exit for out-of-bounds threads
    if (idx >= ParticleCount) {
        return;
    }
    
    // Load particle
    Particle p = Particles[idx];
    
    // Skip dead particles (lifetime <= 0)
    if (p.lifetime <= 0.0f) {
        return;
    }
    
    // Apply forces
    p.velocity += Gravity * DeltaTime;
    p.velocity += WindForce * DeltaTime;
    
    // Update position
    p.position += p.velocity * DeltaTime;
    
    // Decay lifetime
    p.lifetime -= DeltaTime;
    
    // Fade out alpha as lifetime decreases
    p.color.a = max(0.0f, p.lifetime / p.initialLife);
    
    // Store updated particle
    Particles[idx] = p;
}
