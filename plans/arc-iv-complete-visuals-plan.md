## Plan: Arc IV - Complete Visual System (Phases 18-20)

Transform stub DirectX 11 implementation into full GPU-accelerated 3D stage visualization with realistic lighting, volumetric lasers, and dynamic crowd rendering.

**Phases: 3 phases**

### **Phase 18: DirectX 11 Core Rendering Pipeline**
- **Objective:** Implement actual D3D11 rendering with shaders, buffers, and 3D stage geometry
- **Files/Functions to Modify/Create:**
  - visuals/GraphicsContext.h: Update with real D3D11 members (device, context, swapchain)
  - visuals/GraphicsContext.cpp: Implement full D3D11 initialization (was stub)
  - visuals/Shader.h: Shader compilation and management interface
  - visuals/Shader.cpp: HLSL compilation from source or bytecode
  - visuals/VertexBuffer.h: Vertex/index buffer wrapper
  - visuals/VertexBuffer.cpp: D3D11 buffer creation and binding
  - visuals/Camera.h: 3D camera with projection/view matrices
  - visuals/Camera.cpp: Beat-reactive camera movement implementation
  - visuals/StageGeometry.h: 3D stage model (floor, walls, DJ booth)
  - visuals/StageGeometry.cpp: Procedural geometry generation
  - shaders/basic.hlsl: Vertex/pixel shader for simple lit geometry
- **Tests to Write:**
  - test_GraphicsContext_RealInitialization: Verify D3D11 device creation
  - test_Shader_CompileHLSL: Test shader compilation from HLSL source
  - test_VertexBuffer_Creation: Validate buffer creation and binding
  - test_Camera_Projection: Verify perspective matrix calculation
  - test_Camera_BeatSync: Test camera zoom/pan on beat
  - test_StageGeometry_Generation: Validate procedural mesh creation
- **Steps:**
  1. Write tests for D3D11 initialization and rendering (tests fail where stub was)
  2. Implement full GraphicsContext with D3D11 device/swapchain/rendertarget
  3. Create Shader manager for HLSL compilation
  4. Implement VertexBuffer/IndexBuffer wrappers
  5. Build Camera with perspective projection and beat-reactive movement
  6. Generate procedural StageGeometry (floor grid, walls, DJ booth)
  7. Run tests to verify rendering at 60 FPS 1920×1080 (tests pass)
  8. Lint/format

### **Phase 19: Advanced Lighting & Shader Effects**
- **Objective:** Photorealistic lighting rig and volumetric laser rendering with post-processing
- **Files/Functions to Modify/Create:**
  - visuals/LightingRig.h: Update with actual light positions and colors
  - visuals/LightingRig.cpp: Expand to control multiple light types (spots, points)
  - visuals/LaserController.h: Update for volumetric rendering
  - visuals/LaserController.cpp: Generate laser beam geometry with glow
  - shaders/lighting.hlsl: Phong/PBR lighting shader
  - shaders/laser.hlsl: Volumetric laser beam shader with alpha blending
  - visuals/PostProcessor.h: Post-processing pipeline interface
  - visuals/PostProcessor.cpp: Bloom, blur, color grading implementation
  - visuals/RenderTarget.h: Off-screen render target wrapper
  - visuals/RenderTarget.cpp: D3D11 texture creation for post-processing
  - shaders/bloom.hlsl: Gaussian bloom shader
  - shaders/colorgrade.hlsl: Mood-based color grading LUT
- **Tests to Write:**
  - test_LightingRig_MultiLight: Verify multiple light sources render correctly
  - test_LaserController_VolumetricRender: Test laser beam visibility and glow
  - test_PostProcessor_Bloom: Validate bloom effect intensity
  - test_PostProcessor_ColorGrade: Test mood → color mapping
  - test_Shader_AlphaBlending: Verify laser transparency with stage geometry
  - test_RenderTarget_ChainedPasses: Test multi-pass rendering pipeline
- **Steps:**
  1. Write tests for advanced lighting and shaders (tests fail)
  2. Expand LightingRig with spot/point lights (moving heads, pars, strobes)
  3. Implement volumetric laser rendering with additive blending
  4. Create lighting.hlsl with Phong or PBR model
  5. Build post-processing pipeline (bloom, blur, color grading)
  6. Implement mood-based color grading (blue=chill, red=energetic)
  7. Run tests to verify visual quality and performance >60 FPS (tests pass)
  8. Lint/format

### **Phase 20: Particle Systems & Enhanced Crowd Rendering**
- **Objective:** GPU particle systems and detailed crowd with LOD and individual animations
- **Files/Functions to Modify/Create:**
  - visuals/ParticleSystem.h: GPU particle emitter interface
  - visuals/ParticleSystem.cpp: Compute shader particle simulation
  - visuals/CrowdRenderer.h: Update for 3D crowd with LOD
  - visuals/CrowdRenderer.cpp: Instanced rendering of crowd models
  - visuals/CrowdMember.h: Individual crowd character state
  - visuals/CrowdMember.cpp: Animation (idle, jump, wave, dance)
  - shaders/particles.hlsl: Compute shader for particle physics
  - shaders/crowd.hlsl: Instanced rendering shader with LOD
  - visuals/PyroController.h: Pyrotechnics for peak moments
  - visuals/PyroController.cpp: Firework/confetti burst effects
  - models/crowd_lod0.obj: High-detail crowd member model (close)
  - models/crowd_lod1.obj: Medium-detail model (mid)
  - models/crowd_lod2.obj: Low-detail silhouette (far)
- **Tests to Write:**
  - test_ParticleSystem_Emission: Verify particle spawn rate control
  - test_ParticleSystem_Physics: Validate gravity, wind, collision
  - test_CrowdRenderer_LODSwitch: Test distance-based LOD transitions
  - test_CrowdMember_Animation: Verify jump/wave animation timing
  - test_PyroController_Burst: Test pyro effect on energy peaks
  - test_CrowdRenderer_InstancedPerformance: Verify >500 crowd at 60 FPS
- **Steps:**
  1. Write tests for particles and enhanced crowd (tests fail)
  2. Implement GPU ParticleSystem with compute shaders
  3. Create 3-LOD crowd member models (procedural or imported)
  4. Implement instanced crowd rendering with LOD distance switching
  5. Add crowd member animations (idle, jump, wave, dance)
  6. Build PyroController for confetti/firework bursts
  7. Run tests to verify 500+ crowd members at 60 FPS (tests pass)
  8. Lint/format

**Open Questions:**
1. Rendering API: DirectX 11 only or add Vulkan/OpenGL? **DX11 only (Windows focus, can add later)**
2. Shader language: HLSL or GLSL (via spirv-cross)? **HLSL (native DX11)**
3. Crowd models: Procedural or artist-created? **Procedural initially (stick figures → low-poly)**
4. Particle count limit: 10K or 50K particles? **10K (sufficient for confetti/smoke)**
5. Camera control: Auto only or manual override? **Auto with manual override (free camera mode)**

**Dependencies:**
- **DirectX 11 SDK** (Windows SDK 10+)
- **Arc I Phase 10** recommended (BPM detection for beat-reactive camera)
- **HLSL shader compiler** (d3dcompiler.lib, included with Windows SDK)
