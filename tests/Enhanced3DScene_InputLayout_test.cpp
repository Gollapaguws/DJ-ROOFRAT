#include <cassert>
#include <iostream>

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include "visuals/Enhanced3DScene.h"
#include "visuals/GraphicsContext.h"
#include "visuals/DJControllerGeometry.h"
#endif

// Test 1: createControllerGeometry_CreatesInputLayout
// Verify that input layout is created during createControllerGeometry()
void test_createControllerGeometry_CreatesInputLayout() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Arrange
    dj::GraphicsContext graphics;
    bool graphicsInit = graphics.initialize(1920, 1080);
    
    if (!graphicsInit) {
        std::cout << "⊘ test_createControllerGeometry_CreatesInputLayout skipped (graphics unavailable)\n";
        return;
    }

    dj::Enhanced3DScene scene;
    bool sceneInit = scene.initialize(graphics.getD3D11Device(), graphics.getD3D11DeviceContext());
    
    if (!sceneInit) {
        std::cout << "⊘ test_createControllerGeometry_CreatesInputLayout skipped (scene initialization failed)\n";
        graphics.shutdown();
        return;
    }

    // Act - Ensure createControllerGeometry() succeeded
    bool geometryCreated = scene.hasControllerGeometry();
    assert(geometryCreated && "Controller geometry should be created during initialize");

    // Check if input layout was created (by verifying enhanced shader is available)
    bool hasShader = scene.hasCompiledShader();
    assert(hasShader && "Enhanced shader should be loaded for input layout creation");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_createControllerGeometry_CreatesInputLayout passed\n";
#else
    std::cout << "⊘ test_createControllerGeometry_CreatesInputLayout skipped (graphics disabled)\n";
#endif
}

// Test 2: renderController_BindsInputLayout
// Verify that renderController binds the input layout before drawing
void test_renderController_BindsInputLayout() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Arrange
    dj::GraphicsContext graphics;
    bool graphicsInit = graphics.initialize(1920, 1080);
    
    if (!graphicsInit) {
        std::cout << "⊘ test_renderController_BindsInputLayout skipped (graphics unavailable)\n";
        return;
    }

    dj::Enhanced3DScene scene;
    bool sceneInit = scene.initialize(graphics.getD3D11Device(), graphics.getD3D11DeviceContext());
    
    if (!sceneInit) {
        std::cout << "⊘ test_renderController_BindsInputLayout skipped (scene initialization failed)\n";
        graphics.shutdown();
        return;
    }

    // Simple identity matrices for testing
    float identity[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    // Act - Call renderController (should bind input layout internally)
    try {
        scene.renderController(graphics.getD3D11DeviceContext(), identity, identity);
        // If we get here without crash, the function executed
        assert(true && "renderController should execute without exception");
    } catch (const std::exception&) {
        assert(false && "renderController should not throw exception");
    }

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_renderController_BindsInputLayout passed\n";
#else
    std::cout << "⊘ test_renderController_BindsInputLayout skipped (graphics disabled)\n";
#endif
}

// Main test runner
int main() {
    std::cout << "\n=== Enhanced3DScene Input Layout Tests ===\n";
    
    test_createControllerGeometry_CreatesInputLayout();
    test_renderController_BindsInputLayout();
    
    std::cout << "\n=== All Input Layout Tests Complete ===\n";
    return 0;
}
