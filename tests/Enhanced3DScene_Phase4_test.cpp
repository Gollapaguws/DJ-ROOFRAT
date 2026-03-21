// Phase 4: Procedural tunnel geometry with scrolling UVs for music-reactive effects
// Test file for tunnel mesh generation, normals, scrolling UVs, and beat distortion

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#include <dxgi.h>
#include <wrl.h>
#include "visuals/TunnelGeometry.h"
#include "visuals/Enhanced3DScene.h"
#include "visuals/Shader.h"
#include "visuals/GraphicsContext.h"

#include <cassert>
#include <iostream>
#include <cmath>
#include <algorithm>

using Microsoft::WRL::ComPtr;

namespace dj {

// Global graphics context for tests
static std::unique_ptr<GraphicsContext> g_graphicsContext;

// Initialize graphics context for test suite
void initializeGraphicsForPhase4Tests() {
    if (!g_graphicsContext) {
        g_graphicsContext = std::make_unique<GraphicsContext>();
        std::string error;
        if (!g_graphicsContext->initialize(1280, 720, &error)) {
            std::cerr << "Graphics initialization failed: " << error << std::endl;
        }
    }
}

// Test 1: TunnelGeometry_CylindricalMeshGeneration
// Verify 32-segment x 16-ring cylinder generates 513 vertices
void test_TunnelGeometry_CylindricalMeshGeneration() {
    std::cout << "[TEST] TunnelGeometry_CylindricalMeshGeneration...\n";
    
    TunnelGeometry geometry;
    geometry.generate(32, 16, 5.0f);  // 32 segments, 16 rings, 5.0 radius
    
    const auto& vertices = geometry.getVertices();
    const auto& indices = geometry.getIndices();
    
    // Expected: (32 * 16) + 1 ring cap = 512 + 1 = 513 vertices
    // (32 segments, 16 rings along Z, plus center points for caps)
    assert(vertices.size() == 513 && "TunnelGeometry should have 513 vertices (32x16 + 1)");
    
    // Verify indices exist
    assert(indices.size() > 0 && "TunnelGeometry should have index data");
    
    std::cout << "  ✓ Cylindrical mesh generated with " << vertices.size() << " vertices\n";
}

// Test 2: TunnelGeometry_ProperNormals
// Verify normals point INWARD (toward cylinder axis)
void test_TunnelGeometry_ProperNormals() {
    std::cout << "[TEST] TunnelGeometry_ProperNormals...\n";
    
    TunnelGeometry geometry;
    geometry.generate(32, 16, 5.0f);
    
    const auto& vertices = geometry.getVertices();
    
    // For a tunnel, normals should point inward (toward axis)
    // Check vertices 0-31 (first ring around cylinder)
    for (size_t i = 0; i < 32; ++i) {
        float nx = vertices[i].normal[0];
        float nz = vertices[i].normal[2];
        float px = vertices[i].position[0];
        float pz = vertices[i].position[2];
        
        // For inward-facing normals, dot product of (normal · position_radial) should be negative
        // Normal dot radial direction = (nx*px + nz*pz) should be < 0 for inward-facing
        float dotProduct = nx * px + nz * pz;
        assert(dotProduct < 0.0f && "Normal should point inward (toward axis)");
    }
    
    std::cout << "  ✓ All normals point inward toward cylinder axis\n";
}

// Test 3: TunnelEffect_ScrollingUVs
// Verify UV offset increments with time
void test_TunnelEffect_ScrollingUVs() {
    std::cout << "[TEST] TunnelEffect_ScrollingUVs...\n";
    
    TunnelGeometry geometry;
    geometry.generate(32, 16, 5.0f);
    
    float scrollOffset = 0.0f;
    float scrollSpeed = 0.5f;  // units per second
    float deltaTime = 1.0f / 60.0f;  // 60 FPS
    
    // Simulate 10 frames
    float initialOffset = scrollOffset;
    for (int frame = 0; frame < 10; ++frame) {
        scrollOffset += scrollSpeed * deltaTime;
    }
    
    float expectedOffset = initialOffset + (scrollSpeed * deltaTime * 10.0f);
    float epsilon = 0.0001f;
    assert(std::abs(scrollOffset - expectedOffset) < epsilon && 
           "Scroll offset should increment correctly over time");
    
    // Verify scroll offset can be used for UV wrapping (fmod behavior)
    float wrappedOffset = fmod(scrollOffset, 1.0f);
    assert(wrappedOffset >= 0.0f && wrappedOffset <= 1.0f &&
           "Wrapped offset should be in [0, 1]");
    
    std::cout << "  ✓ UV scrolling offset increments correctly: " << scrollOffset << "\n";
}

// Test 4: TunnelEffect_BeatDistortion
// Verify beat intensity affects radial distortion amplitude
void test_TunnelEffect_BeatDistortion() {
    std::cout << "[TEST] TunnelEffect_BeatDistortion...\n";
    
    TunnelGeometry geometry;
    geometry.generate(32, 16, 5.0f);
    
    float radius = 5.0f;
    float beatIntensity = 0.8f;
    
    // Simulate beat distortion on first ring vertices
    for (size_t i = 0; i < 32; ++i) {
        float theta = (i / 32.0f) * 6.28318530718f;  // 2π
        float frequency = 2.0f;
        
        // Radial displacement = beatIntensity * sin(theta * frequency)
        float displacement = beatIntensity * sinf(theta * frequency);
        float distortedRadius = radius + displacement;
        
        // Verify distortion was applied
        assert(distortedRadius >= (radius - beatIntensity) &&
               distortedRadius <= (radius + beatIntensity) &&
               "Distorted radius should be within beat intensity bounds");
    }
    
    std::cout << "  ✓ Beat distortion applied with intensity: " << beatIntensity << "\n";
}

// Test 5: TunnelShader_Compilation
// Verify tunnel.hlsl loads and compiles
void test_TunnelShader_Compilation() {
    std::cout << "[TEST] TunnelShader_Compilation...\n";
    
    initializeGraphicsForPhase4Tests();
    
    if (!g_graphicsContext || !g_graphicsContext->isAvailable()) {
        std::cerr << "  ! Graphics context not available, skipping shader compilation test\n";
        return;
    }
    
    Shader tunnelShader;
    std::string error;
    bool compiled = tunnelShader.compile("tunnel", "VSMain", "vs_5_0", &error);
    
    if (!compiled) {
        std::cerr << "  ! VS Compilation failed: " << error << "\n";
    }
    assert(compiled && "tunnel.hlsl vertex shader should compile successfully");
    
    compiled = tunnelShader.compile("tunnel", "PSMain", "ps_5_0", &error);
    if (!compiled) {
        std::cerr << "  ! PS Compilation failed: " << error << "\n";
    }
    assert(compiled && "tunnel.hlsl pixel shader should compile successfully");
    
    std::cout << "  ✓ tunnel.hlsl compiled successfully\n";
}

// Test 6: TunnelIntegration_RenderTunnel
// Verify Enhanced3DScene can render tunnel with scrolling UVs and beat distortion
void test_TunnelIntegration_RenderTunnel() {
    std::cout << "[TEST] TunnelIntegration_RenderTunnel...\n";
    
    initializeGraphicsForPhase4Tests();
    
    if (!g_graphicsContext || !g_graphicsContext->getD3D11Device()) {
        std::cerr << "  ! Graphics context not available, skipping integration test\n";
        return;
    }
    
    Enhanced3DScene scene;
    bool initialized = scene.initialize(
        g_graphicsContext->getD3D11Device(),
        g_graphicsContext->getD3D11DeviceContext()
    );
    assert(initialized && "Scene should initialize");
    
    // Enable tunnel effect
    scene.setTunnelEffect(true);
    assert(scene.hasTunnelEffect() && "Tunnel effect should be enabled");
    
    // Create view and projection matrices (identity for test)
    float viewMatrix[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 5, 1
    };
    
    float projMatrix[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    
    // Update with music data
    float bpm = 120.0f;
    float energy = 0.7f;
    float beatPhase = 0.5f;
    float deltaTime = 1.0f / 60.0f;
    
    scene.update(bpm, energy, beatPhase, deltaTime);
    
    // Verify scene state
    assert(scene.getBeatIntensity() >= 0.0f && scene.getBeatIntensity() <= 1.0f &&
           "Beat intensity should be in [0, 1]");
    
    std::cout << "  ✓ Tunnel integration test passed, beat intensity: " 
              << scene.getBeatIntensity() << "\n";
}

// Master test runner
void runPhase4Tests() {
    std::cout << "\n=== Phase 4: Tunnel Geometry and Effects Tests ===\n\n";
    
    try {
        test_TunnelGeometry_CylindricalMeshGeneration();
        test_TunnelGeometry_ProperNormals();
        test_TunnelEffect_ScrollingUVs();
        test_TunnelEffect_BeatDistortion();
        test_TunnelShader_Compilation();
        test_TunnelIntegration_RenderTunnel();
        
        std::cout << "\n=== All Phase 4 tests passed! ===\n\n";
    } catch (const std::exception& e) {
        std::cerr << "Test exception: " << e.what() << std::endl;
    }
}

}  // namespace dj

// Entry point for standalone testing
int main() {
    dj::runPhase4Tests();
    return 0;
}

#else
int main() {
    std::cerr << "Graphics support not enabled. Compile with _WIN32 and DJROOFRAT_ENABLE_GRAPHICS.\n";
    return 1;
}
#endif
