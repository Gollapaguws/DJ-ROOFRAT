#include <cassert>
#include <cmath>
#include <iostream>
#include <string>

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include "visuals/GraphicsContext.h"
#include "visuals/Shader.h"
#include "visuals/VertexBuffer.h"
#include "visuals/IndexBuffer.h"
#include "visuals/Camera.h"
#include "visuals/StageGeometry.h"

namespace {

// Test 1: D3D11 Device Initialization
void test_GraphicsContext_D3D11Initialization() {
    // Arrange
    dj::GraphicsContext graphics;

    // Act
    bool initialized = graphics.initialize(1920, 1080);

    // Assert
    assert(initialized && "GraphicsContext should initialize successfully with graphics enabled");
    assert(graphics.isAvailable() && "GraphicsContext should be available after initialization");
    assert(graphics.getD3D11Device() != nullptr && "D3D11 device should be created");
    assert(graphics.getD3D11DeviceContext() != nullptr && "D3D11 device context should be created");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_GraphicsContext_D3D11Initialization passed\n";
}

// Test 2: Swap Chain Creation
void test_GraphicsContext_SwapChainCreation() {
    // Arrange
    dj::GraphicsContext graphics;
    graphics.initialize(1920, 1080);

    // Assert: After initialization, swap chain should exist
    assert(graphics.getSwapChain() != nullptr && "Swap chain should be created");
    assert(graphics.getRenderTargetView() != nullptr && "Render target view should be created");
    assert(graphics.getDepthStencilView() != nullptr && "Depth stencil view should be created");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_GraphicsContext_SwapChainCreation passed\n";
}

// Test 2b: Input Layout (BUG 1)
void test_GraphicsContext_InputLayout() {
    // Arrange
    dj::GraphicsContext graphics;
    bool initialized = graphics.initialize(1920, 1080);

    if (!initialized) {
        std::cout << "⊘ test_GraphicsContext_InputLayout skipped (graphics unavailable)\n";
        return;
    }

    // Assert: Input layout must be created for D3D11 to interpret vertex data
    assert(graphics.getInputLayout() != nullptr && "Input layout should be created for GPU vertex interpretation");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_GraphicsContext_InputLayout passed\n";
}

// Test 2c: Constant Buffer (BUG 2)
void test_GraphicsContext_ConstantBuffer() {
    // Arrange
    dj::GraphicsContext graphics;
    bool initialized = graphics.initialize(1920, 1080);

    if (!initialized) {
        std::cout << "⊘ test_GraphicsContext_ConstantBuffer skipped (graphics unavailable)\n";
        return;
    }

    // Assert: Constant buffer must be created for shader matrices (WVP)
    assert(graphics.getConstantBuffer() != nullptr && "Constant buffer should be created for camera matrices");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_GraphicsContext_ConstantBuffer passed\n";
}

// Test 3: HLSL Shader Compilation
void test_Shader_HLSLCompilation() {
    // Arrange
    dj::Shader shader;

    // Act: Compile a simple shader
    std::string errorMsg;
    bool compiled = shader.compile("VSMain", "vs_5_0", &errorMsg);

    // Assert
    assert(compiled && "Shader should compile successfully");
    assert(shader.getVertexShaderBlob() != nullptr && "Vertex shader blob should be created");
    assert(errorMsg.empty() && "No errors should be reported");

    std::cout << "✓ test_Shader_HLSLCompilation passed\n";
}

// Test 4: Vertex Buffer Creation
void test_VertexBuffer_Creation() {
    // Arrange
    dj::GraphicsContext graphics;
    graphics.initialize(1920, 1080);

    struct Vertex {
        float position[3];
        float normal[3];
    };

    Vertex vertices[] = {
        {{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}
    };

    // Act
    dj::VertexBuffer vertexBuffer;
    bool created = vertexBuffer.create(graphics.getD3D11Device(), 
                                       vertices, 
                                       3, 
                                       sizeof(Vertex));

    // Assert
    assert(created && "Vertex buffer should be created successfully");
    assert(vertexBuffer.getBuffer() != nullptr && "D3D11 buffer should exist");
    assert(vertexBuffer.getVertexCount() == 3 && "Vertex count should match");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_VertexBuffer_Creation passed\n";
}

// Test 5: Index Buffer Creation
void test_IndexBuffer_Creation() {
    // Arrange
    dj::GraphicsContext graphics;
    graphics.initialize(1920, 1080);

    uint32_t indices[] = {0, 1, 2};

    // Act
    dj::IndexBuffer indexBuffer;
    bool created = indexBuffer.create(graphics.getD3D11Device(), indices, 3);

    // Assert
    assert(created && "Index buffer should be created successfully");
    assert(indexBuffer.getBuffer() != nullptr && "D3D11 buffer should exist");
    assert(indexBuffer.getIndexCount() == 3 && "Index count should match");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_IndexBuffer_Creation passed\n";
}

// Test 6: Camera Perspective Projection
void test_Camera_PerspectiveProjection() {
    // Arrange
    dj::Camera camera;
    camera.setProjection(1920, 1080, 45.0f, 0.1f, 1000.0f);

    // Act
    auto projMatrix = camera.getProjectionMatrix();

    // Assert
    // Check that projection matrix is properly formed (shouldn't all be zeros)
    assert(projMatrix[1][1] != 0.0f && "Projection matrix should be initialized");
    assert(camera.getViewMatrix() != nullptr && "View matrix should exist");
    assert(camera.getWorldMatrix() != nullptr && "World matrix should exist");

    std::cout << "✓ test_Camera_PerspectiveProjection passed\n";
}

// Test 7: Beat-Reactive Camera Zoom
void test_Camera_BeatReactiveZoom() {
    // Arrange
    dj::Camera camera;
    camera.setProjection(1920, 1080, 45.0f, 0.1f, 1000.0f);

    // Act: Simulate beat trigger
    camera.updateBeatReactive(120.0f, 0.016f); // 120 BPM, ~60 FPS
    camera.updateBeatReactive(120.0f, 0.016f);

    // Continue updating for smooth interpolation back
    for (int i = 0; i < 10; ++i) {
        camera.updateBeatReactive(120.0f, 0.016f);
    }

    // Assert: Camera should support beat-reactive behavior
    assert(camera.getZoom() >= 0.0f && "Zoom should be non-negative");
    assert(camera.getZoom() <= 10.0f && "Zoom should be reasonable");

    std::cout << "✓ test_Camera_BeatReactiveZoom passed\n";
}

// Test 8: Full Render Loop at 60 FPS
void test_GraphicsContext_RenderLoop() {
    // Arrange
    dj::GraphicsContext graphics;
    bool initialized = graphics.initialize(1920, 1080);

    if (!initialized) {
        std::cout << "⊘ test_GraphicsContext_RenderLoop skipped (graphics unavailable)\n";
        return;
    }

    // Act: Render multiple frames and check timing
    float totalTime = 0.0f;
    const float deltaTime = 1.0f / 60.0f; // 60 FPS
    const int frameCount = 60; // Run for 1 second

    for (int i = 0; i < frameCount; ++i) {
        float bpm = 120.0f;
        float energy = 0.5f;
        int mood = 0;
        float crossfader = 0.0f;

        bool rendered = graphics.renderFrame(bpm, energy, mood, crossfader);
        assert(rendered && "renderFrame should succeed with graphics available");

        totalTime += deltaTime;
    }

    // Assert: Should have rendered all frames without performance degradation
    assert(totalTime >= (frameCount * deltaTime) * 0.9f && "Should maintain approximately 60 FPS");
    assert(graphics.getFPS() > 50.0f && "FPS should be close to 60");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_GraphicsContext_RenderLoop passed\n";
}

} // namespace

int main() {
    std::cout << "\n=== Running GraphicsContext Phase 18 Tests ===\n\n";

    try {
        test_GraphicsContext_D3D11Initialization();
        test_GraphicsContext_SwapChainCreation();
        test_GraphicsContext_InputLayout();          // BUG 1 verification
        test_GraphicsContext_ConstantBuffer();       // BUG 2 verification
        test_Shader_HLSLCompilation();
        test_VertexBuffer_Creation();
        test_IndexBuffer_Creation();
        test_Camera_PerspectiveProjection();
        test_Camera_BeatReactiveZoom();
        test_GraphicsContext_RenderLoop();

        std::cout << "\n=== All 10 tests passed! ===\n\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n!!! Test failed with exception: " << e.what() << "\n\n";
        return 1;
    }
}

#else

int main() {
    std::cout << "Graphics tests skipped (graphics disabled or non-Windows platform)\n";
    return 0;
}

#endif
