## Plan: Enhanced Music-Reactive 3D Visuals

Enhance DJ-ROOFRAT's existing 3D graphics system with advanced visual effects including PBR materials, texture mapping, shadow mapping, and integrated tunnel effects for immersive music visualization.

**Phases: 5**

### Phase 1: Integrate Enhanced3DScene Framework
- **Objective:** Wire the existing Enhanced3DScene framework into the rendering pipeline and add comprehensive test coverage
- **Files/Functions to Modify/Create:**
  - visuals/Enhanced3DScene.cpp - Implement initialize(), update(), render() methods
  - visuals/GraphicsContext.cpp - Integrate Enhanced3DScene into renderFrame()
  - tests/Enhanced3DScene_Phase1_test.cpp - Test initialization and basic rendering
  - CMakeLists.txt - Add Enhanced3DScene.cpp to build
- **Tests to Write:**
  - `Enhanced3DScene_Initialization` - Verify D3D11 device/context setup
  - `Enhanced3DScene_UpdateWithMusicData` - Verify BPM/energy/beatPhase state updates
  - `Enhanced3DScene_FeatureToggling` - Test enable/disable flags for effects
  - `Enhanced3DScene_IntegrationWithGraphicsContext` - Verify renderFrame() calls Enhanced3DScene::render()
- **Steps:**
  1. Write failing tests for Enhanced3DScene initialization with mock D3D11 device
  2. Implement initialize() method to store device/context pointers and validate
  3. Run tests to see them pass
  4. Write failing tests for update() method with various BPM/energy values
  5. Implement update() to store music state and detect beats
  6. Run tests to confirm passing
  7. Write failing test for render() stub integration
  8. Add Enhanced3DScene member to GraphicsContext and call in renderFrame()
  9. Run tests and verify integration works

### Phase 2: Complete Enhanced PBR Shader
- **Objective:** Finalize the enhanced.hlsl shader with proper MaterialBuffer constant buffer and integrate into rendering pipeline
- **Files/Functions to Modify/Create:**
  - shaders/enhanced.hlsl - Verify/complete PBR implementation (metallic/roughness)
  - visuals/Enhanced3DScene.cpp - Load and compile enhanced.hlsl shader
  - visuals/GraphicsContext.cpp - Create MaterialBuffer constant buffer
  - tests/Enhanced3DScene_Phase1_test.cpp - Add shader compilation and material buffer tests
- **Tests to Write:**
  - `EnhancedShader_Compilation` - Verify enhanced.hlsl compiles without errors
  - `EnhancedShader_MaterialBufferCreation` - Test constant buffer for metallic/roughness/emissive
  - `EnhancedShader_BeatReactiveRendering` - Verify beat intensity affects rim lighting
  - `EnhancedShader_EmissiveColorPulsing` - Test BPM-based emissive color pulsing
- **Steps:**
  1. Write failing test for enhanced.hlsl shader compilation
  2. Add shader loading code in Enhanced3DScene::initialize() with D3DCompile
  3. Run test to see it pass
  4. Write failing test for MaterialBuffer constant buffer creation
  5. Create ID3D11Buffer for MaterialBuffer in GraphicsContext with BaseColor/Metallic/Roughness/Emissive fields
  6. Run test to confirm buffer creation works
  7. Write failing test for beat-reactive rendering (mock beat phase input)
  8. Implement shader parameter updates in Enhanced3DScene::render() to populate MaterialBuffer
  9. Run all tests and verify shader receives correct material data

### Phase 3: Add Texture Mapping System
- **Objective:** Implement UV-based texture mapping for stage geometry with procedural checkerboard pattern
- **Files/Functions to Modify/Create:**
  - visuals/StageGeometry.cpp - Add UV coordinates to vertex data
  - visuals/VertexBuffer.h - Extend vertex structure with TEXCOORD semantic
  - visuals/TextureManager.h/cpp - Texture loading and management class
  - shaders/textured.hlsl - Texture sampling shader
  - tests/Enhanced3DScene_Phase1_test.cpp - Add texture creation and sampling tests
- **Tests to Write:**
  - `StageGeometry_UVGeneration` - Verify floor grid has proper [0,1] UV coordinates
  - `TextureManager_ProceduralCheckerboard` - Test 256×256 checkerboard texture creation
  - `TextureManager_TextureBinding` - Test ID3D11ShaderResourceView binding
  - `TexturedShader_Compilation` - Verify textured.hlsl compiles
  - `TexturedShader_SamplingTest` - Test texture sampling in pixel shader with mock texture
- **Steps:**
  1. Write failing test for StageGeometry UV coordinate generation
  2. Modify StageGeometry::generate() to add TEXCOORD0 semantic with UV [0,1] per vertex
  3. Run test to see UV data present in vertex buffer
  4. Write failing test for TextureManager class with createCheckerboard() method
  5. Implement TextureManager with D3D11 texture creation and SRV wrapper
  6. Run test to confirm 256×256 RGBA texture created
  7. Write failing test for textured.hlsl shader compilation
  8. Create textured.hlsl with VSMain (pass through UVs) and PSMain (sample texture with SamplerState)
  9. Run test and verify shader compiles
  10. Write failing integration test for textured rendering
  11. Bind texture SRV and sampler state in Enhanced3DScene::render() before draw call
  12. Run all tests to confirm textured rendering works

### Phase 4: Implement Tunnel Effect
- **Objective:** Create procedural tunnel geometry with per-vertex scrolling UVs for music-reactive visual effect
- **Files/Functions to Modify/Create:**
  - visuals/Enhanced3DScene.cpp - Implement createTunnelGeometry() and renderTunnel()
  - visuals/TunnelGeometry.h/cpp - Procedural tunnel mesh generation (cylindrical)
  - shaders/tunnel.hlsl - Scrolling texture with distortion based on beat
  - tests/Enhanced3DScene_Phase1_test.cpp - Add tunnel generation and rendering tests
- **Tests to Write:**
  - `TunnelGeometry_CylindricalMeshGeneration` - Verify 32-segment x 16-ring cylinder (513 verts)
  - `TunnelGeometry_ProperNormals` - Test normals point inward for inside-facing tunnel
  - `TunnelEffect_ScrollingUVs` - Verify UV offset increments with time
  - `TunnelEffect_BeatDistortion` - Test beat intensity affects radial distortion amplitude
  - `TunnelShader_Compilation` - Verify tunnel.hlsl compiles
- **Steps:**
  1. Write failing test for TunnelGeometry::generate() with 32 segments, 16 rings
  2. Implement procedural cylindrical mesh with position/normal/UV generation
  3. Run test to confirm 513 vertices generated
  4. Write failing test for inward-facing normals (negative radial direction)
  5. Fix normal generation to point toward cylinder axis
  6. Run test to see normals correct
  7. Write failing test for tunnel.hlsl shader compilation with scrolling UV offset constant
  8. Create tunnel.hlsl with VSMain (apply UV scroll) and PSMain (sample texture with distortion)
  9. Run test to confirm compilation
  10. Write failing integration test for renderTunnel() method
  11. Implement Enhanced3DScene::renderTunnel() with constant buffer update for scroll speed
  12. Run all tests and verify tunnel renders with scrolling texture

### Phase 5: Add Basic Shadow Mapping
- **Objective:** Implement single directional light shadow mapping with depth texture pass
- **Files/Functions to Modify/Create:**
  - visuals/Enhanced3DScene.cpp - Add shadow depth pass before main rendering
  - visuals/ShadowMap.h/cpp - Shadow depth texture and DSV management
  - shaders/shadowdepth.hlsl - Depth-only vertex/pixel shader
  - shaders/lighting.hlsl - Add shadow lookup with PCF sampling
  - tests/Enhanced3DScene_Phase1_test.cpp - Add shadow map tests
- **Tests to Write:**
  - `ShadowMap_DepthTextureCreation` - Verify 1024×1024 D24_UNORM_S8_UINT texture
  - `ShadowMap_DepthStencilView` - Test DSV creation for shadow depth pass
  - `ShadowMap_ShaderResourceView` - Test SRV for sampling in lighting shader
  - `ShadowDepthShader_Compilation` - Verify shadowdepth.hlsl compiles
  - `ShadowMapping_LightSpaceMatrix` - Test orthographic projection from light's perspective
  - `ShadowMapping_PCFFiltering` - Verify 2×2 PCF reduces aliasing
- **Steps:**
  1. Write failing test for ShadowMap class with 1024×1024 depth texture
  2. Implement ShadowMap::create() with D3D11_TEXTURE2D_DESC for depth (D24_UNORM_S8_UINT format)
  3. Run test to see depth texture created successfully
  4. Write failing test for shadowdepth.hlsl shader compilation
  5. Create shadowdepth.hlsl with VSMain (transform to light space) and PSMain (empty, depth written automatically)
  6. Run test to confirm shader compiles
  7. Write failing test for depth pass rendering (draw stage geometry to shadow map)
  8. Implement Enhanced3DScene shadow depth pass: clear DSV, bind shadow shader, draw geometry
  9. Run test to verify depth values written to shadow texture
  10. Write failing test for shadow lookup in lighting.hlsl with SamplerComparisonState
  11. Modify lighting.hlsl PSMain to sample shadow map with PCF (2×2 tap) and attenuate light
  12. Run all tests and confirm shadows render with soft edges

**Resolved Design Decisions (Recommended Options):**
1. **Shader priority**: User toggle between enhanced.hlsl (PBR) and textured.hlsl - GraphicsContext offers shader selection API
2. **Tunnel activation**: Manual toggle with 'T' key - gives user control over when tunnel effect is active
3. **Shadow resolution**: 1024×1024 depth map - balances quality and performance for target 60 FPS
4. **Texture asset source**: Procedural textures only - keeps dependencies minimal, fits DJ theme
5. **Build integration**: Add new shaders to CMakeLists.txt SHADER_FILES - follows existing pattern from Phase 18-20
