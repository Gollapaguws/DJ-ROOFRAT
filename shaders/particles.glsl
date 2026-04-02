#version 430

// Particle structure (std430 layout, matches C++ struct)
struct Particle {
    vec3 position;
    float lifetime;
    vec3 velocity;
    float initialLife;
    vec4 color;
    float size;
    vec3 padding;
};

// SSBO for particles
layout(std430, binding = 0) buffer ParticlesBuffer {
    Particle particles[];
};

// UBO for constants
layout(std140, binding = 0) uniform ParticleConstants {
    vec3 Gravity;
    float DeltaTime;
    vec3 WindForce;
    int ParticleCount;
};

// Main compute shader
layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;
void main() {
    uint idx = gl_GlobalInvocationID.x;
    if (idx >= uint(ParticleCount)) return;
    Particle p = particles[idx];
    if (p.lifetime <= 0.0) return;
    p.velocity += Gravity * DeltaTime;
    p.velocity += WindForce * DeltaTime;
    p.position += p.velocity * DeltaTime;
    p.lifetime -= DeltaTime;
    p.color.a = max(0.0, p.lifetime / p.initialLife);
    particles[idx] = p;
}
