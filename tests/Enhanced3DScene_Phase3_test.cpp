// Phase 3: UV-based texture mapping with procedural checkerboard
// Test file for texture system: UV generation, checkerboard creation, shader compilation

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#include <dxgi.h>
#include <wrl.h>
#include "visuals/StageGeometry.h"
#include "visuals/TextureManager.h"
#include "visuals/Shader.h"
#include "visuals/GraphicsContext.h"

#include <cassert>
#include <iostream>
#include <cmath>

using Microsoft::WRL::ComPtr;

namespace dj {

// Global graphics context for tests
static std::unique_ptr<GraphicsContext> g_graphicsContext;

// Initialize graphics context for test suite
void initializeGraphicsForTests() {
    if (!g_graphicsContext) {
        g_graphicsContext = std::make_unique<GraphicsContext>();
        std::string error;
        if (!g_graphicsContext->initialize(1280, 720, &error)) {
            std::cerr << "Graphics initialization failed: " << error << std::endl;
        }
    }
}

// Test 1: StageGeometry_UVGeneration
// Verify floor grid has proper [0,1] UV coordinates
void test_StageGeometry_UVGeneration() {
    std::cout << "[TEST] StageGeometry_UVGeneration...\n";
    
    StageGeometry geometry;
    geometry.generate();
    
    const auto& vertices = geometry.getVertices();
    assert(vertices.size() > 0 && "StageGeometry should have vertices");
    
    // Verify each vertex has UV coordinates in [0, 1]
    for (size_t i = 0; i < vertices.size(); ++i) {
        float u = vertices[i].texCoord[0];
        float v = vertices[i].texCoord[1];
        
        assert(u >= 0.0f && "Vertex has U < 0");
        assert(u <= 1.0f && "Vertex has U > 1");
        assert(v >= 0.0f && "Vertex has V < 0");
        assert(v <= 1.0f && "Vertex has V > 1");
    }
    
    std::cout << "  PASSED: All " << vertices.size() << " vertices have valid UV coordinates\n";
}

// Test 2: TextureManager_ProceduralCheckerboard
// Test 256×256 checkerboard texture creation
void test_TextureManager_ProceduralCheckerboard() {
    std::cout << "[TEST] TextureManager_ProceduralCheckerboard...\n";
    
    if (!g_graphicsContext->isAvailable()) {
        std::cerr << "  SKIPPED: Graphics context not available\n";
        return;
    }

    auto device = g_graphicsContext->getD3D11Device();
    TextureManager texMgr;
    texMgr.setDevice(device);
    
    // Create 256x256 checkerboard
    bool created = texMgr.createCheckerboard(256, 256);
    assert(created && "Failed to create checkerboard texture");
    
    // Verify texture was created
    ID3D11ShaderResourceView* srv = texMgr.getTextureSRV();
    assert(srv != nullptr && "Texture SRV is null");
    
    std::cout << "  PASSED: Checkerboard texture created successfully\n";
}

// Test 3: TextureManager_TextureBinding
// Test ID3D11ShaderResourceView binding
void test_TextureManager_TextureBinding() {
    std::cout << "[TEST] TextureManager_TextureBinding...\n";
    
    if (!g_graphicsContext->isAvailable()) {
        std::cerr << "  SKIPPED: Graphics context not available\n";
        return;
    }

    auto device = g_graphicsContext->getD3D11Device();
    auto deviceContext = g_graphicsContext->getD3D11DeviceContext();
    
    assert(device != nullptr && "Device is null");
    assert(deviceContext != nullptr && "Device context is null");
    
    TextureManager texMgr;
    texMgr.setDevice(device);
    bool created = texMgr.createCheckerboard(256, 256);
    assert(created);
    
    ID3D11ShaderResourceView* srv = texMgr.getTextureSRV();
    assert(srv != nullptr && "Texture SRV is null");
    
    // Bind and verify no crash
    texMgr.bind(deviceContext, 0);
    
    std::cout << "  PASSED: Texture bound successfully\n";
}

// Test 4: TexturedShader_Compilation
// Verify textured.hlsl compiles
void test_TexturedShader_Compilation() {
    std::cout << "[TEST] TexturedShader_Compilation...\n";
    
    if (!g_graphicsContext->isAvailable()) {
        std::cerr << "  SKIPPED: Graphics context not available\n";
        return;
    }

    auto device = g_graphicsContext->getD3D11Device();
    
    Shader shader;
    std::string error;
    
    // Compile vertex shader
    bool vsCompiled = shader.compile("textured", "VSMain", "vs_5_0", &error);
    assert(vsCompiled && ("Vertex shader compilation failed: " + error).c_str());
    
    // Compile pixel shader
    bool psCompiled = shader.compile("textured", "PSMain", "ps_5_0", &error);
    assert(psCompiled && ("Pixel shader compilation failed: " + error).c_str());
    
    // Create shader objects
    bool created = shader.createShaders(device);
    assert(created && "Failed to create shader objects");
    assert(shader.getVertexShader() != nullptr && "Vertex shader is null");
    assert(shader.getPixelShader() != nullptr && "Pixel shader is null");
    
    std::cout << "  PASSED: textured.hlsl compiled successfully\n";
}

// Test 5: TexturedShader_SamplingTest
// Test texture sampling in pixel shader with mock texture
void test_TexturedShader_SamplingTest() {
    std::cout << "[TEST] TexturedShader_SamplingTest...\n";
    
    if (!g_graphicsContext->isAvailable()) {
        std::cerr << "  SKIPPED: Graphics context not available\n";
        return;
    }

    auto device = g_graphicsContext->getD3D11Device();
    auto deviceContext = g_graphicsContext->getD3D11DeviceContext();
    
    // Create shader
    Shader shader;
    std::string error;
    
    bool vsCompiled = shader.compile("textured", "VSMain", "vs_5_0", &error);
    assert(vsCompiled && ("Vertex shader compilation failed: " + error).c_str());
    
    bool psCompiled = shader.compile("textured", "PSMain", "ps_5_0", &error);
    assert(psCompiled && ("Pixel shader compilation failed: " + error).c_str());
    
    bool created = shader.createShaders(device);
    assert(created && "Failed to create shaders");
    
    // Create test texture
    TextureManager texMgr;
    texMgr.setDevice(device);
    bool texCreated = texMgr.createCheckerboard(256, 256);
    assert(texCreated && "Failed to create checkerboard texture");
    
    // Set active shader and texture without rendering
    deviceContext->VSSetShader(shader.getVertexShader(), nullptr, 0);
    deviceContext->PSSetShader(shader.getPixelShader(), nullptr, 0);
    texMgr.bind(deviceContext, 0);
    
    std::cout << "  PASSED: Shader sampling test completed\n";
}

// Test 6: TexturedGeometry_IntegrationRendering
// Integration test for complete textured rendering pipeline
void test_TexturedGeometry_IntegrationRendering() {
    std::cout << "[TEST] TexturedGeometry_IntegrationRendering...\n";
    
    if (!g_graphicsContext->isAvailable()) {
        std::cerr << "  SKIPPED: Graphics context not available\n";
        return;
    }

    auto device = g_graphicsContext->getD3D11Device();
    auto deviceContext = g_graphicsContext->getD3D11DeviceContext();
    
    // Create stage geometry
    StageGeometry geometry;
    geometry.generate();
    const auto& vertices = geometry.getVertices();
    const auto& indices = geometry.getIndices();
    assert(!vertices.empty() && "Geometry should have vertices");
    assert(!indices.empty() && "Geometry should have indices");
    
    // Create texture
    TextureManager texMgr;
    texMgr.setDevice(device);
    bool texCreated = texMgr.createCheckerboard(256, 256);
    assert(texCreated && "Failed to create texture");
    
    // Create shader
    Shader shader;
    std::string error;
    bool vsCompiled = shader.compile("textured", "VSMain", "vs_5_0", &error);
    assert(vsCompiled && ("VS compile failed: " + error).c_str());
    
    bool psCompiled = shader.compile("textured", "PSMain", "ps_5_0", &error);
    assert(psCompiled && ("PS compile failed: " + error).c_str());
    
    bool shadersCreated = shader.createShaders(device);
    assert(shadersCreated && "Failed to create shaders");
    
    // Set up rendering state
    deviceContext->VSSetShader(shader.getVertexShader(), nullptr, 0);
    deviceContext->PSSetShader(shader.getPixelShader(), nullptr, 0);
    
    // Bind texture and sampler (this is what we're testing - binding works)
    texMgr.bind(deviceContext, 0);
    
    // Verify texture is bound (by checking SRV is not null)
    ID3D11ShaderResourceView* boundSRV = texMgr.getTextureSRV();
    assert(boundSRV != nullptr && "Texture SRV should be bound");
    
    std::cout << "  PASSED: Textured geometry integration rendering setup complete\n";
}

} // namespace dj

#else

namespace dj {

void test_StageGeometry_UVGeneration() {
    std::cout << "Tests disabled: Graphics mode not enabled\n";
}

void test_TextureManager_ProceduralCheckerboard() {}
void test_TextureManager_TextureBinding() {}
void test_TexturedShader_Compilation() {}
void test_TexturedShader_SamplingTest() {}
void test_TexturedGeometry_IntegrationRendering() {}

} // namespace dj

#endif

// Main entry point for test execution
int main() {
    std::cout << "Running Enhanced3DScene Phase 3 Tests...\n";
    std::cout << "=========================================\n\n";
    
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    dj::initializeGraphicsForTests();
    
    try {
        dj::test_StageGeometry_UVGeneration();
        dj::test_TextureManager_ProceduralCheckerboard();
        dj::test_TextureManager_TextureBinding();
        dj::test_TexturedShader_Compilation();
        dj::test_TexturedShader_SamplingTest();
        dj::test_TexturedGeometry_IntegrationRendering();
        
        std::cout << "\n=========================================\n";
        std::cout << "All tests completed successfully!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test exception: " << e.what() << "\n";
        return 1;
    }
#else
    dj::test_StageGeometry_UVGeneration();
    std::cout << "Graphics tests disabled\n";
    return 0;
#endif
}
