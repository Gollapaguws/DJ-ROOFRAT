## Plan: Arc IV - Complete Visual System

Transform stub visual system into full DirectX 11 GPU-accelerated 3D stage visualization with photorealistic lighting, volumetric laser effects, dynamic crowd rendering, and particle systems synchronized to live DJ performance.

**Phases: 3**

---

## Phase 18: DirectX 11 Core Rendering Pipeline

**Objective:** Replace GraphicsContext stub with full DirectX 11 rendering pipeline including device initialization, shader compilation, vertex/index buffers, camera system with beat-reactive movement, and procedurally generated 3D stage geometry.

**Files/Functions to Modify/Create:**
- [visuals/GraphicsContext.h](visuals/GraphicsContext.h) - Real D3D11 device/context members
- [visuals/GraphicsContext.cpp](visuals/GraphicsContext.cpp) - Full D3D11 initialization
- visuals/Shader.h/cpp - HLSL shader compilation wrapper
- visuals/VertexBuffer.h/cpp - D3D11 buffer management
- visuals/IndexBuffer.h/cpp - Index buffer wrapper
- visuals/Camera.h/cpp - Perspective projection + beat-reactive zoom/pan
- visuals/StageGeometry.h/cpp - Procedural floor/walls/booth generation
- shaders/basic.hlsl - Vertex/pixel shader with Phong lighting
- visuals/GraphicsContext_Phase18_test.cpp - D3D11 initialization tests

**Tests to Write:**
- test_GraphicsContext_D3D11Initialization - Device creation succeeds
- test_GraphicsContext_SwapChainCreation - Swap chain configured correctly
- test_Shader_HLSLCompilation - Shader compiles from source
- test_VertexBuffer_Creation - Buffer created and bound
- test_Camera_PerspectiveProjection - Matrix calculation correct
- test_Camera_BeatReactiveZoom - Zoom on BPM trigger
- test_StageGeometry_ProceduralGeneration - Floor/walls created
- test_GraphicsContext_RenderLoop - 60 FPS achieved at 1920×1080

**Steps:**
1. **Write failing D3D11 device test** - Test that GraphicsContext creates ID3D11Device and ID3D11DeviceContext
2. **Implement D3D11 device creation** - Use Windows SDK to create device, swap chain, render target views
3. **Run device test** - Verify test passes
4. **Write failing shader compilation test** - Test that Shader class compiles HLSL source
5. **Implement Shader wrapper** - Use D3DCompile to compile HLSL, manage ID3D11VertexShader/PixelShader
6. **Run shader test** - Verify compilation works
7. **Write failing buffer test** - Test VertexBuffer creation and binding
8. **Implement VertexBuffer/IndexBuffer** - Wrap ID3D11Buffer with RAII, bind to device context
9. **Run buffer test** - Verify buffers work
10. **Write failing camera projection test** - Test perspective matrix generation
11. **Implement Camera class** - Generate view/projection matrices, beat-reactive zoom
12. **Run camera test** - Verify matrix math correct
13. **Write failing stage geometry test** - Test procedural floor/wall generation
14. **Implement StageGeometry** - Generate vertex data for floor grid, walls, DJ booth
15. **Run geometry test** - Verify vertices created
16. **Write failing render loop test** - Test full rendering pipeline at 60 FPS
17. **Integrate into GraphicsContext::renderFrame()** - Bind shaders, buffers, draw geometry
18. **Run render loop test** - Verify 60 FPS target met
19. **Create basic.hlsl shader** - Vertex shader (WVP transform), pixel shader (Phong lighting)
20. **Build and verify all tests pass**

---

## Phase 19: Advanced Lighting & Shaders

**Objective:** Implement photorealistic lighting with multiple light sources, volumetric laser beam rendering with additive blending, post-processing pipeline (bloom, color grading), and beat-synchronized strobe effects.

**Files/Functions to Modify/Create:**
- [visuals/LightingRig.h](visuals/LightingRig.h) - Expand from 32-bar intensities to multi-light system
- [visuals/LightingRig.cpp](visuals/LightingRig.cpp) - Spot lights, point lights, color control
- [visuals/LaserController.h](visuals/LaserController.h) - Volumetric rendering data
- [visuals/LaserController.cpp](visuals/LaserController.cpp) - Beam geometry generation
- visuals/RenderTarget.h/cpp - Off-screen render target wrapper
- visuals/PostProcessor.h/cpp - Multi-pass rendering pipeline
- shaders/lighting.hlsl - Phong/PBR lighting model
- shaders/laser.hlsl - Volumetric beam with additive blending
- shaders/bloom.hlsl - Gaussian bloom pass
- shaders/colorgrade.hlsl - Mood-based color LUT
- visuals/Lighting_Phase19_test.cpp - Lighting and post-processing tests

**Tests to Write:**
- test_LightingRig_MultiLightSources - Multiple lights created
- test_LightingRig_SpotLightDirection - Spot light direction correct
- test_LaserController_VolumetricBeam - Beam geometry generated
- test_RenderTarget_OffscreenBuffer - Off-screen target creation
- test_PostProcessor_BloomPass - Bloom applied correctly
- test_PostProcessor_ColorGradeMood - Color LUT changes with mood
- test_Lighting_StrobeSync - Strobes trigger on beat
- test_Lighting_PerformanceNoRegression - 60 FPS maintained

**Steps:**
1. **Write failing multi-light test** - Test LightingRig creates spot/point lights
2. **Expand LightingRig** - Add light types (spot, point), color, position
3. **Run multi-light test** - Verify lights created
4. **Write failing lighting shader test** - Test lighting.hlsl compiles and applies Phong
5. **Implement lighting.hlsl** - Multi-light Phong shader
6. **Run lighting shader test** - Verify lighting works
7. **Write failing laser beam test** - Test volumetric beam geometry
8. **Implement laser beam rendering** - Generate beam quad strip, additive blending
9. **Run laser beam test** - Verify beam visible
10. **Write failing laser shader test** - Test laser.hlsl volumetric effect
11. **Implement laser.hlsl** - Glow/additive blend shader
12. **Run laser shader test** - Verify laser glow
13. **Write failing render target test** - Test off-screen buffer creation
14. **Implement RenderTarget** - Wrap ID3D11RenderTargetView
15. **Run render target test** - Verify off-screen rendering
16. **Write failing bloom test** - Test Gaussian blur applied
17. **Implement bloom pass** - Two-pass Gaussian blur (horizontal/vertical)
18. **Run bloom test** - Verify bloom visible
19. **Write failing color grade test** - Test mood → LUT mapping
20. **Implement colorgrade.hlsl** - Mood-based color lookup
21. **Run color grade test** - Verify colors change
22. **Write failing strobe test** - Test beat-synced strobes
23. **Implement strobe logic** - Trigger on BPM beat
24. **Run strobe test** - Verify strobes sync
25. **Write performance test** - Verify 60 FPS maintained
26. **Optimize if needed** - Profile and fix bottlenecks
27. **Build and verify all tests pass**

---

## Phase 20: Particle Systems & Crowd Enhancement

**Objective:** Implement GPU particle system with compute shaders for confetti/smoke effects, Level-of-Detail crowd rendering with instancing for 500+ members, crowd animations synchronized to BPM, and pyrotechnics triggered by energy peaks.

**Files/Functions to Modify/Create:**
- visuals/ParticleSystem.h/cpp - GPU particle physics and rendering
- visuals/ComputeShader.h/cpp - Compute shader wrapper
- [visuals/CrowdRenderer.h](visuals/CrowdRenderer.h) - LOD system integration
- [visuals/CrowdRenderer.cpp](visuals/CrowdRenderer.cpp) - Instanced rendering
- visuals/CrowdMesh.h/cpp - Procedural crowd model generation (LOD0/1/2)
- visuals/CrowdAnimator.h/cpp - Idle/jump/wave/dance animations
- shaders/particles.hlsl - Compute shader for particle physics
- shaders/crowd.hlsl - Instanced crowd vertex/pixel shader
- visuals/Particles_Phase20_test.cpp - Particle and crowd tests

**Tests to Write:**
- test_ParticleSystem_Emission - Particles emit on trigger
- test_ParticleSystem_Physics - Gravity/wind applied correctly
- test_ParticleSystem_10KParticles - 10K particles at 60 FPS
- test_CrowdRenderer_InstancedDraw - 500+ instances rendered
- test_CrowdRenderer_LODSwitching - Distance-based LOD changes
- test_CrowdAnimator_BPMSync - Animations sync to BPM
- test_CrowdAnimator_IdleJumpWave - Animation states correct
- test_Pyrotechnics_EnergyPeakTrigger - Confetti on energy peak
- test_Performance_FullScene - Full scene at 60 FPS

**Steps:**
1. **Write failing particle emission test** - Test particles created
2. **Implement ParticleSystem basics** - Particle struct, emission logic
3. **Run particle emission test** - Verify particles created
4. **Write failing particle physics test** - Test gravity/wind applied
5. **Implement compute shader wrapper** - ComputeShader class for D3D11
6. **Implement particles.hlsl** - Compute shader for physics
7. **Run particle physics test** - Verify physics works
8. **Write failing 10K particle test** - Test performance target
9. **Optimize particle system** - GPU-based updates, efficient dispatch
10. **Run 10K particle test** - Verify 60 FPS achieved
11. **Write failing crowd instancing test** - Test 500+ instances
12. **Implement instanced rendering** - Per-instance buffers for CrowdRenderer
13. **Run crowd instancing test** - Verify instances drawn
14. **Write failing LOD test** - Test LOD switching
15. **Implement LOD system** - Generate LOD0/1/2 meshes, distance-based switching
16. **Run LOD test** - Verify LOD changes
17. **Write failing animation sync test** - Test BPM-synced animations
18. **Implement CrowdAnimator** - Idle/jump/wave/dance keyframes
19. **Run animation sync test** - Verify sync works
20. **Write failing pyrotechnics test** - Test energy peak trigger
21. **Integrate with CareerProgression** - Hook energy peaks to particle bursts
22. **Run pyrotechnics test** - Verify confetti triggers
23. **Write failing full scene test** - Test entire Arc IV at 60 FPS
24. **Optimize full scene** - Profile and fix bottlenecks
25. **Run full scene test** - Verify performance
26. **Build and verify all tests pass**

---

## Open Questions

1. **Window Creation:** Should GraphicsContext create its own HWND window, or should main.cpp create the window and pass the handle? (Recommend: GraphicsContext creates window for encapsulation)

2. **Shader Hot-Reloading:** Should shaders be recompiled at runtime when HLSL files change (for development), or only compiled at build time? (Recommend: Runtime compilation in dev builds, pre-compiled in release)

3. **Camera Control:** Auto-only camera (beat-reactive), or add manual override for debugging/director mode? (Recommend: Auto by default, 'C' key toggles manual mode)

4. **Particle Budget:** 10K particles sufficient, or should we implement dynamic scaling based on GPU performance? (Recommend: Fixed 10K initially, add scaling in future)

5. **LOD Distance Thresholds:** Fixed camera distance thresholds for LOD switching, or adaptive based on crowd count? (Recommend: Fixed initially - LOD0: <5 units, LOD1: 5-15 units, LOD2: >15 units)

6. **Thread Synchronization:** Simple state copy (audio thread → render thread), or mutex-protected shared state? (Recommend: Simple copy once per frame, acceptable latency)

7. **Fullscreen Mode:** Windowed-only, or support exclusive fullscreen? (Recommend: Windowed initially, add fullscreen toggle later)

8. **Texture Resources:** Load textures from files (stb_image.h), or procedural only? (Recommend: Procedural initially for floor/walls, add texture loading if time permits)