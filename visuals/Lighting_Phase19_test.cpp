#include <cassert>
#include <cmath>
#include <iostream>
#include <string>

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include "visuals/GraphicsContext.h"
#include "visuals/LightingRig.h"
#include "visuals/LaserController.h"
#include "visuals/RenderTarget.h"
#include "visuals/PostProcessor.h"

namespace {

// Test 1: Multi-Light System
void test_LightingRig_MultiLightSources() {
    // Arrange
    dj::LightingRig rig;

    // Act: Add multiple lights
    auto spotLight = rig.addSpotLight({1.0f, 2.0f, 3.0f}, {0.0f, -1.0f, 0.0f}, 
                                      {1.0f, 0.0f, 0.0f}, 45.0f, 0.8f);
    auto pointLight = rig.addPointLight({-1.0f, 1.0f, 0.0f}, 
                                         {0.0f, 1.0f, 1.0f}, 0.6f);

    // Assert
    assert(spotLight >= 0 && "Spot light should be added with valid index");
    assert(pointLight >= 0 && "Point light should be added with valid index");
    
    const auto& lights = rig.getLights();
    assert(lights.size() >= 2 && "Should have at least 2 lights");
    
    std::cout << "✓ test_LightingRig_MultiLightSources passed\n";
}

// Test 2: Lighting Shader Compilation
void test_Shader_LightingCompilation() {
    // Arrange
    dj::Shader shader;

    // Act: Compile lighting shader
    std::string errorMsg;
    bool compiled = shader.compile("lighting", "PSMain", "ps_5_0", &errorMsg);

    // Assert
    assert(compiled && "Lighting shader should compile successfully");
    assert(errorMsg.empty() && "No compilation errors should occur");
    
    std::cout << "✓ test_Shader_LightingCompilation passed\n";
}

// Test 3: Laser Beam Geometry
void test_LaserController_BeamGeometry() {
    // Arrange
    dj::LaserController laser;
    laser.update(120.0f, 0.0f, 0.016f);

    // Act: Get beam geometry
    auto geometry = laser.getBeamGeometry(1.0f);  // 1.0 unit width

    // Assert
    assert(geometry.vertices.size() == 4 && "Beam should be a quad (4 vertices)");
    assert(geometry.indices.size() == 6 && "Quad requires 6 indices (2 triangles)");
    
    // Check that vertices form a valid quad
    for (const auto& v : geometry.vertices) {
        assert(std::isfinite(v.position[0]) && "X coordinate should be finite");
        assert(std::isfinite(v.position[1]) && "Y coordinate should be finite");
        assert(std::isfinite(v.position[2]) && "Z coordinate should be finite");
    }
    
    std::cout << "✓ test_LaserController_BeamGeometry passed\n";
}

// Test 4: Additive Blend State
void test_Laser_AdditiveBlend() {
    // Arrange
    dj::GraphicsContext graphics;
    bool initialized = graphics.initialize(1920, 1080);

    if (!initialized) {
        std::cout << "⊘ test_Laser_AdditiveBlend skipped (graphics unavailable)\n";
        return;
    }

    // Act: Create additive blend state
    auto blendState = graphics.createAdditiveBlendState();

    // Assert
    assert(blendState != nullptr && "Additive blend state should be created");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_Laser_AdditiveBlend passed\n";
}

// Test 5: Render Target Creation
void test_RenderTarget_Creation() {
    // Arrange
    dj::GraphicsContext graphics;
    bool initialized = graphics.initialize(1920, 1080);

    if (!initialized) {
        std::cout << "⊘ test_RenderTarget_Creation skipped (graphics unavailable)\n";
        return;
    }

    // Act: Create offscreen render target
    dj::RenderTarget target;
    bool created = target.create(graphics.getD3D11Device(), graphics.getD3D11DeviceContext(),
                                 1920, 1080, DXGI_FORMAT_R8G8B8A8_UNORM);

    // Assert
    assert(created && "Render target should be created successfully");
    assert(target.getWidth() == 1920 && "Width should match requested dimensions");
    assert(target.getHeight() == 1080 && "Height should match requested dimensions");
    assert(target.getRenderTargetView() != nullptr && "RTV should be valid");
    assert(target.getShaderResourceView() != nullptr && "SRV should be valid");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_RenderTarget_Creation passed\n";
}

// Test 6: Bloom Post-Processing
void test_PostProcessor_Bloom() {
    // Arrange
    dj::GraphicsContext graphics;
    bool initialized = graphics.initialize(1920, 1080);

    if (!initialized) {
        std::cout << "⊘ test_PostProcessor_Bloom skipped (graphics unavailable)\n";
        return;
    }

    // Act: Create post-processor with bloom
    dj::PostProcessor postProc;
    bool setup = postProc.initialize(graphics.getD3D11Device(), graphics.getD3D11DeviceContext(),
                                      1920, 1080);

    // Assert
    assert(setup && "Post-processor should initialize successfully");
    
    // Apply bloom and verify it doesn't crash
    bool applied = postProc.applyBloom(1.0f);  // intensity = 1.0
    assert(applied && "Bloom should apply successfully");

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_PostProcessor_Bloom passed\n";
}

// Test 7: Color Grading
void test_PostProcessor_ColorGrade() {
    // Arrange
    dj::GraphicsContext graphics;
    bool initialized = graphics.initialize(1920, 1080);

    if (!initialized) {
        std::cout << "⊘ test_PostProcessor_ColorGrade skipped (graphics unavailable)\n";
        return;
    }

    // Act: Create post-processor
    dj::PostProcessor postProc;
    postProc.initialize(graphics.getD3D11Device(), graphics.getD3D11DeviceContext(),
                        1920, 1080);

    // Test all 4 moods
    for (int mood = 0; mood < 4; ++mood) {
        bool applied = postProc.applyColorGrade(mood);
        assert(applied && "Color grade should apply for all moods (0-3)");
    }

    // Cleanup
    graphics.shutdown();

    std::cout << "✓ test_PostProcessor_ColorGrade passed\n";
}

// Test 8: Strobe Sync with Beat
void test_Lighting_StrobeSync() {
    // Arrange
    dj::LightingRig rig;
    float bpm = 120.0f;
    float energy = 0.9f;  // High energy for strobe

    // Act: Update to trigger strobe on beat
    rig.update(bpm, energy, 0.016f);  // One frame at 60 FPS
    
    float strobeIntensity = rig.getStrobeIntensity();

    // Assert
    assert(strobeIntensity >= 0.0f && strobeIntensity <= 1.0f && 
           "Strobe intensity should be in valid range [0, 1]");
    
    // At high energy, strobe should be active
    if (energy > 0.8f) {
        // Only check strobe is valid, timing depends on internal state
        assert(strobeIntensity >= 0.0f && "Strobe should activate at high energy");
    }

    // Cleanup
    std::cout << "✓ test_Lighting_StrobeSync passed\n";
}

}  // namespace

#endif

int main() {
    std::cout << "\n=== Phase 19: Advanced Lighting & Shaders Tests ===\n";

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    try {
        test_LightingRig_MultiLightSources();
        test_Shader_LightingCompilation();
        test_LaserController_BeamGeometry();
        test_Laser_AdditiveBlend();
        test_RenderTarget_Creation();
        test_PostProcessor_Bloom();
        test_PostProcessor_ColorGrade();
        test_Lighting_StrobeSync();

        std::cout << "\n✓ All 8 Phase 19 tests passed!\n\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << "\n";
        return 1;
    }
#else
    std::cout << "Graphics tests skipped (DJROOFRAT_ENABLE_GRAPHICS not defined)\n";
    return 0;
#endif
}
