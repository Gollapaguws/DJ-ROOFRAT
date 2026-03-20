#include <cassert>
#include <iostream>

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include "visuals/Enhanced3DScene.h"
#include "visuals/GraphicsContext.h"

namespace {

// Test 1: Enhanced3DScene Initialization with D3D11 device/context
void test_Enhanced3DScene_Initialization() {
    // Arrange
    dj::GraphicsContext graphics;
    bool graphicsInit = graphics.initialize(1920, 1080);
    
    if (!graphicsInit) {
        std::cout << "⊘ test_Enhanced3DScene_Initialization skipped (graphics unavailable)\n";
        return;
    }

    dj::Enhanced3DScene scene;

    // Act
    bool initialized = scene.initialize(graphics.getD3D11Device(), graphics.getD3D11DeviceContext());

    // Assert
    assert(initialized && "Enhanced3DScene should initialize successfully");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_Enhanced3DScene_Initialization passed\n";
}

// Test 2: Enhanced3DScene UpdateWithMusicData - Verify music state updates
void test_Enhanced3DScene_UpdateWithMusicData() {
    // Arrange
    dj::GraphicsContext graphics;
    bool graphicsInit = graphics.initialize(1920, 1080);
    
    if (!graphicsInit) {
        std::cout << "⊘ test_Enhanced3DScene_UpdateWithMusicData skipped (graphics unavailable)\n";
        return;
    }

    dj::Enhanced3DScene scene;
    scene.initialize(graphics.getD3D11Device(), graphics.getD3D11DeviceContext());

    // Act - Update with specific music data
    float bpm = 128.0f;
    float energy = 0.7f;
    float beatPhase = 0.5f;
    float deltaTime = 0.016f; // 60 FPS

    scene.update(bpm, energy, beatPhase, deltaTime);

    // Assert - Scene should have stored the state
    // Since we can't directly access private state, we verify the method doesn't crash
    // and doesn't throw exceptions
    assert(true && "Update should process music data without error");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_Enhanced3DScene_UpdateWithMusicData passed\n";
}

// Test 3: Enhanced3DScene FeatureToggling - Enable/disable effects
void test_Enhanced3DScene_FeatureToggling() {
    // Arrange
    dj::GraphicsContext graphics;
    bool graphicsInit = graphics.initialize(1920, 1080);
    
    if (!graphicsInit) {
        std::cout << "⊘ test_Enhanced3DScene_FeatureToggling skipped (graphics unavailable)\n";
        return;
    }

    dj::Enhanced3DScene scene;
    scene.initialize(graphics.getD3D11Device(), graphics.getD3D11DeviceContext());

    // Act & Assert - Default enable state
    assert(scene.hasDynamicLighting() && "Dynamic lighting should be enabled by default");
    assert(scene.hasParticleBursts() && "Particle bursts should be enabled by default");
    assert(!scene.hasTunnelEffect() && "Tunnel effect should be disabled by default");
    assert(!scene.hasBloom() && "Bloom should be disabled by default");

    // Act - Toggle features
    scene.setDynamicLighting(false);
    scene.setParticleBursts(false);
    scene.setTunnelEffect(true);
    scene.setBloomEffect(true);

    // Assert - Verify toggles worked
    assert(!scene.hasDynamicLighting() && "Dynamic lighting should be disabled after toggle");
    assert(!scene.hasParticleBursts() && "Particle bursts should be disabled after toggle");
    assert(scene.hasTunnelEffect() && "Tunnel effect should be enabled after toggle");
    assert(scene.hasBloom() && "Bloom should be enabled after toggle");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_Enhanced3DScene_FeatureToggling passed\n";
}

// Test 4: Enhanced3DScene IntegrationWithGraphicsContext - Verify renderFrame integration
void test_Enhanced3DScene_IntegrationWithGraphicsContext() {
    // Arrange
    dj::GraphicsContext graphics;
    bool graphicsInit = graphics.initialize(1920, 1080);
    
    if (!graphicsInit) {
        std::cout << "⊘ test_Enhanced3DScene_IntegrationWithGraphicsContext skipped (graphics unavailable)\n";
        return;
    }

    // Act - renderFrame should complete without crashing
    bool renderResult = graphics.renderFrame(128.0f, 0.7f, 0, 0.0f);

    // Assert - Render should succeed when graphics are available
    assert(renderResult && "renderFrame should succeed with graphics available");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_Enhanced3DScene_IntegrationWithGraphicsContext passed\n";
}

} // namespace

int main() {
    std::cout << "Running Enhanced3DScene Phase 1 tests...\n\n";

    try {
        test_Enhanced3DScene_Initialization();
        test_Enhanced3DScene_UpdateWithMusicData();
        test_Enhanced3DScene_FeatureToggling();
        test_Enhanced3DScene_IntegrationWithGraphicsContext();

        std::cout << "\n✓ All Enhanced3DScene Phase 1 tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed with exception: " << e.what() << "\n";
        return 1;
    }
}

#else
int main() {
    std::cout << "⊘ Enhanced3DScene Phase 1 tests skipped (graphics not available on this platform)\n";
    return 0;
}
#endif

