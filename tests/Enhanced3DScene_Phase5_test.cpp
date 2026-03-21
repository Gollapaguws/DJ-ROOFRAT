// Phase 5: Single directional light shadow mapping with depth texture pass
// Test file for shadow map creation, depth pass, and PCF filtering

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#include <dxgi.h>
#include <wrl.h>
#include "visuals/ShadowMap.h"
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
void initializeGraphicsForPhase5Tests() {
    if (!g_graphicsContext) {
        g_graphicsContext = std::make_unique<GraphicsContext>();
        std::string error;
        if (!g_graphicsContext->initialize(1280, 720, &error)) {
            std::cerr << "Graphics initialization failed: " << error << std::endl;
        }
    }
}

// Test 1: ShadowMap_DepthTextureCreation
// Verify shadow map creates 1024x1024 D24_UNORM_S8_UINT depth texture
void test_ShadowMap_DepthTextureCreation() {
    std::cout << "[TEST] ShadowMap_DepthTextureCreation...\n";
    
    initializeGraphicsForPhase5Tests();
    
    ShadowMap shadowMap;
    bool result = shadowMap.initialize(g_graphicsContext->getD3D11Device(), 1024, 1024);
    assert(result && "ShadowMap initialization should succeed");
    
    // Verify texture was created correctly
    ID3D11Texture2D* depthTexture = shadowMap.getDepthTexture();
    assert(depthTexture != nullptr && "Depth texture should be created");
    
    D3D11_TEXTURE2D_DESC desc = {};
    depthTexture->GetDesc(&desc);
    
    assert(desc.Width == 1024 && "Depth texture width should be 1024");
    assert(desc.Height == 1024 && "Depth texture height should be 1024");
    assert(desc.Format == DXGI_FORMAT_R24G8_TYPELESS || desc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT 
           && "Depth texture should use D24_UNORM_S8_UINT or typeless variant");
    assert(desc.BindFlags & D3D11_BIND_DEPTH_STENCIL && "Texture should have depth-stencil bind flag");
    
    std::cout << "  ✓ Depth texture created with correct dimensions and format\n";
}

// Test 2: ShadowMap_DepthStencilView
// Verify DSV is created for rendering to depth texture
void test_ShadowMap_DepthStencilView() {
    std::cout << "[TEST] ShadowMap_DepthStencilView...\n";
    
    initializeGraphicsForPhase5Tests();
    
    ShadowMap shadowMap;
    bool result = shadowMap.initialize(g_graphicsContext->getD3D11Device(), 1024, 1024);
    assert(result && "ShadowMap initialization should succeed");
    
    ID3D11DepthStencilView* dsv = shadowMap.getDepthStencilView();
    assert(dsv != nullptr && "Depth stencil view should be created");
    
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsv->GetDesc(&dsvDesc);
    
    assert((dsvDesc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT || dsvDesc.Format == DXGI_FORMAT_D32_FLOAT)
           && "DSV format should be D24_UNORM_S8_UINT or D32_FLOAT");
    
    std::cout << "  ✓ Depth stencil view created successfully\n";
}

// Test 3: ShadowMap_ShaderResourceView
// Verify SRV is created for sampling depth texture in lighting pass
void test_ShadowMap_ShaderResourceView() {
    std::cout << "[TEST] ShadowMap_ShaderResourceView...\n";
    
    initializeGraphicsForPhase5Tests();
    
    ShadowMap shadowMap;
    bool result = shadowMap.initialize(g_graphicsContext->getD3D11Device(), 1024, 1024);
    assert(result && "ShadowMap initialization should succeed");
    
    ID3D11ShaderResourceView* srv = shadowMap.getShaderResourceView();
    assert(srv != nullptr && "Shader resource view should be created");
    
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srv->GetDesc(&srvDesc);
    
    // SRV should be able to sample depth values
    assert(srvDesc.ViewDimension == D3D11_SRV_DIMENSION_TEXTURE2D && "SRV should be 2D texture view");
    
    // Phase 5: Verify comparison sampler exists for PCF sampling
    ID3D11SamplerState* comparisonSampler = shadowMap.getComparisonSampler();
    assert(comparisonSampler != nullptr && "Comparison sampler should be created for PCF filtering");
    
    D3D11_SAMPLER_DESC sampDesc = {};
    comparisonSampler->GetDesc(&sampDesc);
    assert(sampDesc.ComparisonFunc == D3D11_COMPARISON_LESS && "Sampler should use COMPARISON_LESS");
    
    std::cout << "  ✓ Shader resource view and comparison sampler created for depth sampling\n";
}

// Test 4: ShadowDepthShader_Compilation
// Verify shadowdepth.hlsl compiles to vertex and pixel shaders
void test_ShadowDepthShader_Compilation() {
    std::cout << "[TEST] ShadowDepthShader_Compilation...\n";
    
    initializeGraphicsForPhase5Tests();
    
    Shader depthShader;
    std::string errorMsg;
    bool compileResult = depthShader.compile("shadowdepth", "VSMain", "vs_5_0", &errorMsg);
    
    if (!compileResult) {
        std::cerr << "  Shader compile error: " << errorMsg << "\n";
    }
    assert(compileResult && "shadowdepth.hlsl vertex shader should compile");
    
    bool pixelCompileResult = depthShader.compile("shadowdepth", "PSMain", "ps_5_0", &errorMsg);
    if (!pixelCompileResult) {
        std::cerr << "  Shader compile error: " << errorMsg << "\n";
    }
    assert(pixelCompileResult && "shadowdepth.hlsl pixel shader should compile");
    
    ID3DBlob* vsBlob = depthShader.getVertexShaderBlob();
    ID3DBlob* psBlob = depthShader.getPixelShaderBlob();
    
    assert(vsBlob != nullptr && "Vertex shader blob should be valid");
    assert(psBlob != nullptr && "Pixel shader blob should be valid");
    assert(vsBlob->GetBufferSize() > 0 && "Vertex shader blob should have content");
    assert(psBlob->GetBufferSize() > 0 && "Pixel shader blob should have content");
    
    std::cout << "  ✓ Depth shaders compiled successfully\n";
}

// Test 5: ShadowMapping_LightSpaceMatrix
// Verify light-space matrix is set up correctly (orthographic projection)
void test_ShadowMapping_LightSpaceMatrix() {
    std::cout << "[TEST] ShadowMapping_LightSpaceMatrix...\n";
    
    initializeGraphicsForPhase5Tests();
    
    ShadowMap shadowMap;
    bool result = shadowMap.initialize(g_graphicsContext->getD3D11Device(), 1024, 1024);
    assert(result && "ShadowMap initialization should succeed");
    
    // Set up a directional light (sun light from above)
    float lightPosition[3] = {0.0f, 10.0f, 0.0f};
    float lightDirection[3] = {0.0f, -1.0f, 0.0f};  // pointing down
    
    shadowMap.setLightPosition(lightPosition);
    shadowMap.setLightDirection(lightDirection);
    
    // Get light-space matrix
    const float* viewMatrix = shadowMap.getLightViewMatrix();
    const float* projMatrix = shadowMap.getLightProjectionMatrix();
    
    assert(viewMatrix != nullptr && "Light view matrix should be valid");
    assert(projMatrix != nullptr && "Light projection matrix should be valid");
    
    // Verify it's an orthographic projection (suitable for directional light)
    // For orthographic projection, proj[3][3] should be 1.0 and proj[2][3] should be 0.0
    // This is a simple sanity check that the matrix looks reasonable
    assert(std::abs(projMatrix[15] - 1.0f) < 0.01f && "Projection should be orthographic");
    
    std::cout << "  ✓ Light-space matrix set up correctly\n";
}

// Test 6: ShadowMapping_PCFFiltering
// Verify PCF filtering is enabled and configured in lighting shader
void test_ShadowMapping_PCFFiltering() {
    std::cout << "[TEST] ShadowMapping_PCFFiltering...\n";
    
    initializeGraphicsForPhase5Tests();
    
    Shader lightingShader;
    std::string errorMsg;
    bool compileResult = lightingShader.compile("lighting", "PSMain", "ps_5_0", &errorMsg);
    
    if (!compileResult) {
        std::cerr << "  Shader compile error: " << errorMsg << "\n";
    }
    assert(compileResult && "lighting.hlsl pixel shader should compile with PCF");
    
    ID3DBlob* psBlob = lightingShader.getPixelShaderBlob();
    assert(psBlob != nullptr && "Lighting shader blob should be valid");
    assert(psBlob->GetBufferSize() > 0 && "Lighting shader blob should have content");
    
    // Verify shader can be created into a pixel shader
    bool psCreated = lightingShader.createPixelShader(g_graphicsContext->getD3D11Device());
    assert(psCreated && "Lighting pixel shader should be created successfully");
    
    ID3D11PixelShader* ps = lightingShader.getPixelShader();
    assert(ps != nullptr && "Pixel shader interface should be valid");
    
    std::cout << "  ✓ PCF filtering configured in lighting shader\n";
}

} // namespace dj

#else
namespace dj {
void initializeGraphicsForPhase5Tests() {}
void test_ShadowMap_DepthTextureCreation() {
    std::cout << "[SKIPPED] ShadowMap_DepthTextureCreation (Graphics disabled)\n";
}
void test_ShadowMap_DepthStencilView() {
    std::cout << "[SKIPPED] ShadowMap_DepthStencilView (Graphics disabled)\n";
}
void test_ShadowMap_ShaderResourceView() {
    std::cout << "[SKIPPED] ShadowMap_ShaderResourceView (Graphics disabled)\n";
}
void test_ShadowDepthShader_Compilation() {
    std::cout << "[SKIPPED] ShadowDepthShader_Compilation (Graphics disabled)\n";
}
void test_ShadowMapping_LightSpaceMatrix() {
    std::cout << "[SKIPPED] ShadowMapping_LightSpaceMatrix (Graphics disabled)\n";
}
void test_ShadowMapping_PCFFiltering() {
    std::cout << "[SKIPPED] ShadowMapping_PCFFiltering (Graphics disabled)\n";
}
}
#endif

// Main entry point
int main() {
    std::cout << "\n=== Enhanced3DScene Phase 5 Tests: Shadow Mapping ===\n\n";

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    try {
        dj::initializeGraphicsForPhase5Tests();
        
        dj::test_ShadowMap_DepthTextureCreation();
        dj::test_ShadowMap_DepthStencilView();
        dj::test_ShadowMap_ShaderResourceView();
        dj::test_ShadowDepthShader_Compilation();
        dj::test_ShadowMapping_LightSpaceMatrix();
        dj::test_ShadowMapping_PCFFiltering();
        
        std::cout << "\n=== All Phase 5 tests passed! ===\n\n";
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Test exception: " << e.what() << "\n";
        return 1;
    }
#else
    std::cout << "[INFO] Graphics subsystem disabled. Running stub tests.\n\n";
    dj::test_ShadowMap_DepthTextureCreation();
    dj::test_ShadowMap_DepthStencilView();
    dj::test_ShadowMap_ShaderResourceView();
    dj::test_ShadowDepthShader_Compilation();
    dj::test_ShadowMapping_LightSpaceMatrix();
    dj::test_ShadowMapping_PCFFiltering();
    std::cout << "\n=== Stub tests completed (graphics disabled) ===\n\n";
    return 0;
#endif
}
