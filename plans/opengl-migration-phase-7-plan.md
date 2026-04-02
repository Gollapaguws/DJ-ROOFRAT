## Plan: Phase 7 - Advanced Features Integration

Complete the OpenGL migration by integrating ImGui OpenGL3 backend and porting essential GLSL shaders for particles and post-processing effects. FBOs and compute shaders are already implemented from previous work.

**Phases: 3 sub-phases**

### 1. **Phase 7A: ImGui OpenGL3 Backend Integration**
- **Objective:** Switch ImGui rendering from D3D11 to OpenGL3 backend when DJROOFRAT_OPENGL_MIGRATION is defined
- **Files/Functions to Modify/Create:**
  - CMakeLists.txt: Switch ImGui backend sources from D3D11 to OpenGL3
  - src/main.cpp: Add OpenGL3 ImGui initialization path
  - Modify ImGui setup to use ImGui_ImplOpenGL3_* functions
- **Tests to Write:**
  - ImGui_Phase7A_test.cpp:
    - Test 1: ImGui OpenGL3 backend initializes successfully
    - Test 2: ImGui renders simple window with OpenGL3
    - Test 3: ImGui handles input events correctly
    - Test 4: ImGui cleanup works without leaks
    - Test 5: Multiple frames can be rendered
- **Steps:**
  1. Write failing tests for ImGui OpenGL3 initialization
  2. Modify CMakeLists.txt to link imgui_impl_opengl3.cpp when migration enabled
  3. Add conditional ImGui initialization in src/main.cpp using OpenGL3 backend
  4. Replace ImGui_ImplDX11_* calls with ImGui_ImplOpenGL3_* equivalents
  5. Run tests and verify all 5 pass
  6. Verify main DJ-ROOFRAT application UI renders correctly

### 2. **Phase 7B: Particle Physics GLSL Shader**
- **Objective:** Port particle compute shader from HLSL to GLSL for GPU particle effects (confetti, smoke)
- **Files/Functions to Modify/Create:**
  - shaders/particles.glsl: Port from particles.hlsl with GLSL 430 compute syntax
  - Verify ParticleSystem.cpp can load GLSL version when DJROOFRAT_OPENGL_MIGRATION defined
- **Tests to Write:**
  - ParticleShader_Phase7B_test.cpp:
    - Test 1: Compile particles.glsl successfully
    - Test 2: Dispatch compute shader with particle data
    - Test 3: Verify particle positions update correctly
    - Test 4: Ensure no OpenGL errors during dispatch
- **Steps:**
  1. Write failing tests for GLSL particle shader compilation
  2. Create shaders/particles.glsl with compute shader layout
  3. Port particle update logic from HLSL to GLSL syntax
  4. Update ParticleSystem to load .glsl version when migration enabled
  5. Run tests and verify particle shader works
  6. Test in main app to see visual particle effects

### 3. **Phase 7C: Post-Processing GLSL Shaders**
- **Objective:** Port bloom and color grading shaders from HLSL to GLSL for post-processing pipeline
- **Files/Functions to Modify/Create:**
  - shaders/bloom.frag: Bloom/blur fragment shader (GLSL 430)
  - shaders/colorgrade.frag: Color grading fragment shader (GLSL 430)
  - shaders/postprocess.vert: Simple fullscreen quad vertex shader
  - Verify PostProcessor.cpp can use GLSL shaders when migration enabled
- **Tests to Write:**
  - PostProcessShaders_Phase7C_test.cpp:
    - Test 1: Compile bloom.frag successfully
    - Test 2: Compile colorgrade.frag successfully
    - Test 3: Compile postprocess.vert successfully
    - Test 4: Render to FBO and apply bloom effect
    - Test 5: Apply color grading to rendered texture
    - Test 6: Chain bloom + color grading effects
- **Steps:**
  1. Write failing tests for GLSL post-processing shader compilation
  2. Create shaders/postprocess.vert for fullscreen quad rendering
  3. Create shaders/bloom.frag with gaussian blur logic
  4. Create shaders/colorgrade.frag with exposure/saturation/contrast controls
  5. Update PostProcessor to load GLSL shaders when migration enabled
  6. Run tests and verify all 6 pass
  7. Test visual effects in main application

**Open Questions:**
1. Should we create tests for ParticleSystem and PostProcessor OpenGL integration, or trust existing tests + visual validation?
2. Do we need to port ALL particle shader variants, or just the core compute shader?
3. Should ImGui OpenGL3 tests be standalone or integrated into existing Graphics tests?

**Dependencies:**
- Phase 7A depends on nothing (can start immediately)
- Phase 7B depends on ComputeShader class (already complete)
- Phase 7C depends on Framebuffer class (already complete)
- All phases use existing GLAD functions (no new GL functions needed)

**Success Criteria:**
- All new tests pass (15 tests across 3 phases)
- DJ-ROOFRAT.exe runs with OpenGL backend, showing:
  - Working ImGui UI
  - Particle effects rendering
  - Post-processing effects visible
- No visual regressions compared to D3D11 version
