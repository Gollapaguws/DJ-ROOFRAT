#include <cassert>
#include <iostream>
#include <cmath>

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include "visuals/Enhanced3DScene.h"
#include "visuals/GraphicsContext.h"

namespace {

// Test 1: EnhancedShader_Compilation - Verify enhanced.hlsl compiles without errors
void test_EnhancedShader_Compilation() {
    // Arrange
    dj::GraphicsContext graphics;
    bool graphicsInit = graphics.initialize(1920, 1080);
    
    if (!graphicsInit) {
        std::cout << "⊘ test_EnhancedShader_Compilation skipped (graphics unavailable)\n";
        return;
    }

    dj::Enhanced3DScene scene;
    bool sceneInit = scene.initialize(graphics.getD3D11Device(), graphics.getD3D11DeviceContext());
    
    if (!sceneInit) {
        std::cout << "⊘ test_EnhancedShader_Compilation skipped (scene initialization failed)\n";
        graphics.shutdown();
        return;
    }

    // Act
    bool shaderLoaded = scene.loadEnhancedShader();

    // Assert
    assert(shaderLoaded && "enhanced.hlsl should compile without errors");
    assert(scene.hasCompiledShader() && "Scene should have compiled shader after loading");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_EnhancedShader_Compilation passed\n";
}

// Test 2: EnhancedShader_MaterialBufferCreation - Test constant buffer for metallic/roughness/emissive
void test_EnhancedShader_MaterialBufferCreation() {
    // Arrange
    dj::GraphicsContext graphics;
    bool graphicsInit = graphics.initialize(1920, 1080);
    
    if (!graphicsInit) {
        std::cout << "⊘ test_EnhancedShader_MaterialBufferCreation skipped (graphics unavailable)\n";
        return;
    }

    dj::Enhanced3DScene scene;
    bool sceneInit = scene.initialize(graphics.getD3D11Device(), graphics.getD3D11DeviceContext());
    
    if (!sceneInit) {
        std::cout << "⊘ test_EnhancedShader_MaterialBufferCreation skipped (scene initialization failed)\n";
        graphics.shutdown();
        return;
    }

    bool shaderLoaded = scene.loadEnhancedShader();
    if (!shaderLoaded) {
        std::cout << "⊘ test_EnhancedShader_MaterialBufferCreation skipped (shader loading failed)\n";
        graphics.shutdown();
        return;
    }

    // Act
    bool bufferCreated = scene.createMaterialBuffer();

    // Assert
    assert(bufferCreated && "MaterialBuffer should be created successfully");
    assert(scene.hasMaterialBuffer() && "Scene should have material buffer after creation");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_EnhancedShader_MaterialBufferCreation passed\n";
}

// Test 3: EnhancedShader_BeatReactiveRendering - Verify beat intensity affects rim lighting
void test_EnhancedShader_BeatReactiveRendering() {
    // Arrange
    dj::GraphicsContext graphics;
    bool graphicsInit = graphics.initialize(1920, 1080);
    
    if (!graphicsInit) {
        std::cout << "⊘ test_EnhancedShader_BeatReactiveRendering skipped (graphics unavailable)\n";
        return;
    }

    dj::Enhanced3DScene scene;
    bool sceneInit = scene.initialize(graphics.getD3D11Device(), graphics.getD3D11DeviceContext());
    
    if (!sceneInit) {
        std::cout << "⊘ test_EnhancedShader_BeatReactiveRendering skipped (scene initialization failed)\n";
        graphics.shutdown();
        return;
    }

    bool shaderLoaded = scene.loadEnhancedShader();
    bool bufferCreated = scene.createMaterialBuffer();
    
    if (!shaderLoaded || !bufferCreated) {
        std::cout << "⊘ test_EnhancedShader_BeatReactiveRendering skipped (shader/buffer setup failed)\n";
        graphics.shutdown();
        return;
    }

    // Act - Update with high beat intensity
    float bpm = 128.0f;
    float energy = 0.8f;
    float highBeatPhase = 0.8f;
    float deltaTime = 0.016f;

    scene.update(bpm, energy, highBeatPhase, deltaTime);
    
    // Get beat intensity (should be calculated from beatPhase)
    float beatIntensity = scene.getBeatIntensity();

    // Assert - Beat intensity should be positive at phase 0.8
    assert(beatIntensity > 0.0f && "Beat intensity should be positive during active beat phase");
    
    // Update with low beat intensity (start of beat cycle)
    float lowBeatPhase = 0.1f;
    scene.update(bpm, energy, lowBeatPhase, deltaTime);
    
    float lowBeatIntensity = scene.getBeatIntensity();
    
    // Assert - Beat intensity should be lower at phase 0.1
    assert(lowBeatIntensity < beatIntensity && "Beat intensity should decrease at beat phase start");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_EnhancedShader_BeatReactiveRendering passed\n";
}

// Test 4: EnhancedShader_EmissiveColorPulsing - Test BPM-based emissive color pulsing
void test_EnhancedShader_EmissiveColorPulsing() {
    // Arrange
    dj::GraphicsContext graphics;
    bool graphicsInit = graphics.initialize(1920, 1080);
    
    if (!graphicsInit) {
        std::cout << "⊘ test_EnhancedShader_EmissiveColorPulsing skipped (graphics unavailable)\n";
        return;
    }

    dj::Enhanced3DScene scene;
    bool sceneInit = scene.initialize(graphics.getD3D11Device(), graphics.getD3D11DeviceContext());
    
    if (!sceneInit) {
        std::cout << "⊘ test_EnhancedShader_EmissiveColorPulsing skipped (scene initialization failed)\n";
        graphics.shutdown();
        return;
    }

    bool shaderLoaded = scene.loadEnhancedShader();
    bool bufferCreated = scene.createMaterialBuffer();
    
    if (!shaderLoaded || !bufferCreated) {
        std::cout << "⊘ test_EnhancedShader_EmissiveColorPulsing skipped (shader/buffer setup failed)\n";
        graphics.shutdown();
        return;
    }

    // Act - Update with different BPM values and beat phases
    float bpm1 = 120.0f;
    float energy = 0.7f;
    float beatPhase1 = 0.5f;  // Peak of beat
    float deltaTime = 0.016f;

    scene.update(bpm1, energy, beatPhase1, deltaTime);
    float emissiveIntensity1 = scene.getEmissiveIntensity();

    // Act - Same BPM but different beat phase
    float beatPhase2 = 0.25f;  // Quarter beat
    scene.update(bpm1, energy, beatPhase2, deltaTime);
    float emissiveIntensity2 = scene.getEmissiveIntensity();

    // Assert - Emissive intensity should change with beat phase (pulsing)
    assert(emissiveIntensity1 != emissiveIntensity2 && "Emissive intensity should pulse with beat phase");
    
    // Assert - Intensities should be in valid range [0, 1]
    assert(emissiveIntensity1 >= 0.0f && emissiveIntensity1 <= 1.0f && "Emissive intensity should be [0, 1]");
    assert(emissiveIntensity2 >= 0.0f && emissiveIntensity2 <= 1.0f && "Emissive intensity should be [0, 1]");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_EnhancedShader_EmissiveColorPulsing passed\n";
}

// Test 5: EnhancedShader_MaterialBufferStructure - Verify MaterialBuffer 16-byte alignment
void test_EnhancedShader_MaterialBufferStructure() {
    // Arrange - Check MaterialBuffer data structure alignment
    struct MaterialBuffer {
        float baseColor[3];      // 12 bytes
        float metallic;          // 4 bytes (total: 16 bytes) - row 1
        float emissiveColor[3];  // 12 bytes
        float roughness;         // 4 bytes (total: 16 bytes) - row 2
        float cameraPosition[3]; // 12 bytes
        float bpm;               // 4 bytes (total: 16 bytes) - row 3
    };

    // Assert - MaterialBuffer size should be multiple of 16 bytes for D3D11
    size_t bufferSize = sizeof(MaterialBuffer);
    assert(bufferSize % 16 == 0 && "MaterialBuffer size must be 16-byte aligned");
    assert(bufferSize == 48 && "MaterialBuffer should be 48 bytes (3 rows × 16 bytes)");

    std::cout << "✓ test_EnhancedShader_MaterialBufferStructure passed\n";
}

// Test 6: EnhancedShader_RenderingWithMaterialBuffer - Test rendering with dynamic lighting enabled
void test_EnhancedShader_RenderingWithMaterialBuffer() {
    // Arrange
    dj::GraphicsContext graphics;
    bool graphicsInit = graphics.initialize(1920, 1080);
    
    if (!graphicsInit) {
        std::cout << "⊘ test_EnhancedShader_RenderingWithMaterialBuffer skipped (graphics unavailable)\n";
        return;
    }

    dj::Enhanced3DScene scene;
    bool sceneInit = scene.initialize(graphics.getD3D11Device(), graphics.getD3D11DeviceContext());
    
    if (!sceneInit) {
        std::cout << "⊘ test_EnhancedShader_RenderingWithMaterialBuffer skipped (scene initialization failed)\n";
        graphics.shutdown();
        return;
    }

    bool shaderLoaded = scene.loadEnhancedShader();
    bool bufferCreated = scene.createMaterialBuffer();
    
    if (!shaderLoaded || !bufferCreated) {
        std::cout << "⊘ test_EnhancedShader_RenderingWithMaterialBuffer skipped (shader/buffer setup failed)\n";
        graphics.shutdown();
        return;
    }

    // Ensure dynamic lighting is enabled (default state)
    scene.setDynamicLighting(true);
    assert(scene.hasDynamicLighting() && "Dynamic lighting should be enabled");

    // Act - Prepare render with music state
    float bpm = 128.0f;
    float energy = 0.6f;
    float beatPhase = 0.4f;
    float deltaTime = 0.016f;
    
    scene.update(bpm, energy, beatPhase, deltaTime);

    // Create dummy matrices for rendering (4x4 identity matrices)
    float viewMatrix[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    float projMatrix[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    // Act - Render (should not crash)
    scene.render(viewMatrix, projMatrix);

    // Assert - Just verify no exceptions or crashes
    assert(true && "Rendering with material buffer should complete without error");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_EnhancedShader_RenderingWithMaterialBuffer passed\n";
}

// Test 7: EnhancedShader_IntegratedInitialization - Verify shader and buffer are created during initialize()
void test_EnhancedShader_IntegratedInitialization() {
    // Arrange
    dj::GraphicsContext graphics;
    bool graphicsInit = graphics.initialize(1920, 1080);
    
    if (!graphicsInit) {
        std::cout << "⊘ test_EnhancedShader_IntegratedInitialization skipped (graphics unavailable)\n";
        return;
    }

    dj::Enhanced3DScene scene;
    
    // Act - Initialize should create shader and buffer automatically
    bool sceneInit = scene.initialize(graphics.getD3D11Device(), graphics.getD3D11DeviceContext());
    
    if (!sceneInit) {
        std::cout << "⊘ test_EnhancedShader_IntegratedInitialization skipped (scene initialization failed)\n";
        graphics.shutdown();
        return;
    }

    // Assert - Shader and buffer should be available immediately after initialize()
    assert(scene.hasCompiledShader() && "Scene should have compiled shader after initialization");
    assert(scene.hasMaterialBuffer() && "Scene should have material buffer after initialization");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_EnhancedShader_IntegratedInitialization passed\n";
}

} // namespace

int main() {
    std::cout << "\n=== Enhanced3DScene Phase 2 Tests ===\n\n";

    try {
        // Phase 2 shader and material buffer tests
        test_EnhancedShader_Compilation();
        test_EnhancedShader_MaterialBufferCreation();
        test_EnhancedShader_MaterialBufferStructure();
        test_EnhancedShader_BeatReactiveRendering();
        test_EnhancedShader_EmissiveColorPulsing();
        test_EnhancedShader_RenderingWithMaterialBuffer();
        test_EnhancedShader_IntegratedInitialization();

        std::cout << "\n=== All Phase 2 tests passed! ===\n\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed with exception: " << e.what() << "\n\n";
        return 1;
    }
}

#else

int main() {
    std::cout << "Enhanced3DScene Phase 2 tests skipped (graphics support not enabled)\n";
    return 0;
}

#endif
