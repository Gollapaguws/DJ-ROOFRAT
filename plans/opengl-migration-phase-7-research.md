# Phase 7 Research: OpenGL Migration - Advanced Features

**Date:** April 2, 2026  
**Status:** Research Complete  
**Context:** Phases 1-6 complete. Phase 6 (Enhanced3DScene) successfully migrated DirectX 11 3D rendering to OpenGL 4.3 Core.  
**Target:** OpenGL 4.3 Core Profile, Custom GLAD loader (~55+ functions loaded), Final migration phase

---

## Executive Summary

Phase 7 is the **FINAL** phase of the OpenGL migration. Significant infrastructure is **ALREADY IMPLEMENTED**:

| Feature | Status | Completeness |
|---------|--------|--------------|
| Framebuffer Objects (FBOs) | ✅ IMPLEMENTED | 100% - Ready for post-processing |
| Compute Shaders | ✅ IMPLEMENTED | 100% - OpenGL path exists with tests |
| ImGui OpenGL3 Backend | ❌ NOT STARTED | 0% - Currently D3D11 only |
| Post-Processing + FBO Integration | ⚠️ PARTIAL | D3D11 exists, needs OpenGL glue |
| GLSL Post-Processing Shaders | ❌ NOT STARTED | 0% - Only HLSL versions exist |
| GLAD Requirements | ✅ COMPLETE | 100% - All needed functions loaded |

**Key Finding:** Much of Phase 7 is **already built**. Work focus is on:
1. **ImGui OpenGL3/GLFW integration** (HIGH PRIORITY - UI rendering)
2. **PostProcessor OpenGL implementation** (MEDIUM - polish feature)
3. **Shader migration from HLSL to GLSL** (MEDIUM - compatibility)

---

## 1. ImGui Integration Status

### Current Implementation (D3D11 Path)
- **Backend Files Missing:** `imgui_impl_opengl3.cpp`, `imgui_impl_glfw.cpp`
- **Current Setup:** Only `imgui_impl_dx11.cpp` + `imgui_impl_win32.cpp` compiled
- **Initialization Code:** `src/main.cpp` lines 1016-1030:
  ```cpp
  ImGui_ImplWin32_Init(windowHandle);
  ImGui_ImplDX11_Init(device, deviceContext);
  ```

### What Needs OpenGL3 Integration
1. **CMakeLists.txt Changes Required:**
   - Change ImGui library compilation from D3D11/Win32 backends to **OpenGL3/GLFW** backends
   - Current: Lines 47-59 build only D3D11 backends
   - Needed: Switch to build from `${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp` and `imgui_impl_glfw.cpp`

2. **Initialization Changes in src/main.cpp:**
   - Replace `ImGui_ImplWin32_Init()` + `ImGui_ImplDX11_Init()` with:
     ```cpp
     ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
     ImGui_ImplOpenGL3_Init("#version 430 core");
     ```
   - Use `graphics.getGLFWWindow()` instead of raw window handle

3. **Frame Rendering Changes:**
   - Replace current D3D11 frame sequence with:
     ```cpp
     ImGui_ImplOpenGL3_NewFrame();
     ImGui_ImplGlfw_NewFrame();
     ImGui::NewFrame();
     // ... UI code ...
     ImGui::Render();
     ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
     ```

4. **Key Integration Points:**
   - GraphicsContext already has `getGLFWWindow()` method → ready for GLFW integration
   - No D3D11 device/context needed for OpenGL path
   - GLFW window context already created for OpenGL rendering

### Existing Infrastructure Supporting ImGui OpenGL3
✅ **GLFW window** - Already initialized in GraphicsContext  
✅ **OpenGL context** - Already active from Enhanced3DScene  
✅ **GLAD loader** - All ImGui functions already loaded  

---

## 2. Post-Processing Existing Code

### Current D3D11 Implementation (COMPLETE)
**Files:** `visuals/PostProcessor.h/cpp`

**Effects Implemented:**
1. **Bloom Gaussian Blur** (Two-pass separable)
   - Horizontal + Vertical blur passes
   - 5-tap Gaussian kernel with weights: [0.227, 0.195, 0.122, 0.054, 0.016]
   - Shader: `shaders/bloom.hlsl` (HLSL version)
   - Controllable intensity [0, 1]

2. **Color Grading LUT-Based**
   - 4 mood-based color profiles:
     - Unimpressed (1.0, 1.0, 1.0) - neutral
     - Calm (0.8, 0.9, 1.2) - blue tint
     - Grooving (0.9, 1.1, 0.9) - green tint
     - Hyped (1.2, 0.8, 0.8) - red tint
   - Shader: `shaders/colorgrade.hlsl` (HLSL version)

**Render Target Infrastructure:**
- `visuals/RenderTarget.h/cpp` - D3D11 RenderTargetView wrapping
- Used for bloom intermediate passes and post-processing

### Available in D3D11 (Needs Migration)
- Fullscreen quad geometry generation (`createFullscreenQuad()`)
- Constant buffer setup for shader parameters
- Sampler state configuration for texture filtering
- Multi-pass rendering pipeline

### OpenGL Equivalent Infrastructure (ALREADY BUILT)
✅ **Framebuffer Class** (`visuals/Framebuffer.h/cpp`)
- Methods: `bind()`, `unbind()`, `attachColorTexture()`, `attachDepthTexture()`, `setDrawBuffers()`, `isComplete()`
- Fully tested in `tests/Framebuffer_Phase7_test.cpp` (8 tests passing)
- Supports Multiple Render Targets (MRT) for advanced effects

✅ **Texture Class** (`visuals/Texture.h/cpp`)  
- OpenGL texture creation, binding, mipmap generation
- Methods: `create()`, `loadFromData()`, `bind()`, `generateMipmaps()`, `resize()`

✅ **Shader Class** (`visuals/Shader.h/cpp`)
- OpenGL shader compilation and linking
- Methods: `compile()`, `link()`, `use()`, `getUniformLocation()`

### What's Missing for OpenGL Post-Processing
❌ PostProcessor OpenGL implementation branch (only D3D11 exists)  
❌ GLSL versions of bloom.glsl, colorgrade.glsl (only HLSL exists)  
❌ OpenGL fullscreen quad mesh generation  

---

## 3. Compute Shader Usage (GPU Effects)

### Current Status: ALREADY IMPLEMENTED FOR OPENGL
✅ **OpenGL Compute Shader Path Exists** in `visuals/ComputeShader.h/cpp`

**OpenGL Methods Available:**
- `bool compileSource(const char* source, std::string* errorOut)` - Compile GLSL compute
- `void use()` - Use compute program
- `void bindSSBO(GLuint ssbo, GLuint bindingPoint)` - Bind storage buffer
- `void dispatch(uint32_t groupCountX/Y/Z)` - Launch compute threads

**Tests:** `tests/ComputeShader_Phase7_test.cpp` - 8 comprehensive tests
- Compute shader compilation
- SSBO creation and binding
- Workgroup dispatching
- Memory barrier synchronization
- Result read-back
- Multiple SSBO bindings
- Uniform setting
- Error handling

**Current Usage:**
- `ParticleSystem` uses compute shaders for GPU particle physics
- `particles.hlsl` (D3D11) - needs GLSL compute shader equivalent `particles.glsl`
- Particle count: 10,000+ particles at 60 FPS
- Compute dispatch: `ceil(particleCount / 256)` thread groups

### Particle Physics Compute Shader (CRITICAL FEATURE)
**File:** `shaders/particles.hlsl` (D3D11 version - needs GLSL port)
**Purpose:** GPU-accelerated particle updating
- Apply gravity and wind forces
- Update velocity and position
- Handle particle lifetime decay
- Support confetti burst triggering

**GLSL Equivalent Needed:** `shaders/particles.glsl`
- GLSL 4.3 compute shader syntax
- Same physics calculations
- Shader storage buffer bindings

### GPU Particle Effects Criticality
🔴 **HIGH PRIORITY** - Particle effects are:
- Used for confetti bursts during transitions
- Used for smoke/environment effects
- Directly visible in gameplay
- Performance-critical (10k+ particles)
- **Cannot be skipped** - affects visual quality significantly

---

## 4. GLAD Requirements Analysis

### Successfully Loaded Functions (~55+ total)

**Core Rendering (Already Loaded):**
- glClear, glClearColor, glGetString, glGetIntegerv, glViewport
- glEnable, glDisable (state management)
- glCreateShader, glDeleteShader, glShaderSource, glCompileShader
- glCreateProgram, glDeleteProgram, glLinkProgram, glUseProgram
- glGetShaderiv, glGetShaderInfoLog, glGetProgramiv, glGetProgramInfoLog

**Buffer Management (Already Loaded):**
- glGenBuffers, glDeleteBuffers, glBindBuffer, glBufferData, glBufferSubData
- glMapBuffer, glUnmapBuffer, glMapBufferRange
- glBindBufferBase (for SSBO and compute shader bindings)

**Vertex Attributes (Already Loaded):**
- glGenVertexArrays, glDeleteVertexArrays, glBindVertexArray
- glVertexAttribPointer, glEnableVertexAttribArray, glDisableVertexAttribArray
- glGetVertexAttribiv

**Textures (Already Loaded):**
- glGenTextures, glDeleteTextures, glBindTexture, glTexImage2D, glTexSubImage2D
- glTexParameteri, glTexParameterf, glActiveTexture
- glGenerateMipmap

**Framebuffer Objects (Already Loaded):**
- ✅ glGenFramebuffers, glDeleteFramebuffers, glBindFramebuffer
- ✅ glFramebufferTexture2D, glCheckFramebufferStatus
- ✅ glDrawBuffers, glBlitFramebuffer (for post-processing)

**Compute Shaders (Already Loaded):**
- ✅ glDispatchCompute, glMemoryBarrier, glBindBufferBase

**Error Queries (Already Loaded):**
- glGetError, glIsEnabled, glGetFloatv

### GLAD Status Summary
✨ **COMPLETE** - All functions needed for Phase 7 are already in GLAD loader
- No additional GLAD extensions needed
- Vendor/glad/include/glad/glad.h complete
- Vendor/glad/src/glad.c complete with all function pointers initialized

---

## 5. File Structure Analysis

### Existing Files (Ready for Use)

**OpenGL Rendering Infrastructure (Fully Implemented):**
```
visuals/
├── Framebuffer.h/cpp ✅ (GLuint FBO wrapper with RAII)
├── ComputeShader.h/cpp ✅ (OpenGL compute path + D3D11 path)
├── Texture.h/cpp ✅ (GLuint texture wrapper)
├── Shader.h/cpp ✅ (OpenGL shader compilation + D3D11 path)
├── Enhanced3DScene.h/cpp ✅ (Phase 6: VAO/VBO rendering)
└── Camera.h/cpp ✅ (3D view matrix management)
```

**Test Files (Comprehensive Coverage):**
```
tests/
├── Framebuffer_Phase7_test.cpp ✅ (8 FBO tests)
├── ComputeShader_Phase7_test.cpp ✅ (8 compute tests)
└── Enhanced3DScene_Phase6_test.cpp ✅ (12 3D rendering tests)
```

**Shaders (D3D11 Only - Need GLSL Ports):**
```
shaders/
├── basic3d.vert/frag ✅ (Phase 6 - already GLSL)
├── bloom.hlsl ❌ (HLSL - needs GLSL port: bloom.frag)
├── colorgrade.hlsl ❌ (HLSL - needs GLSL port: colorgrade.frag)
└── particles.hlsl ❌ (HLSL - needs GLSL port: particles.glsl compute)
```

**Post-Processing (D3D11 Only):**
```
visuals/
├── PostProcessor.h/cpp ❌ (D3D11-only, needs OpenGL path)
└── RenderTarget.h/cpp ❌ (D3D11-specific)
```

**ImGui Integration:**
```
src/
└── main.cpp (lines 1016-1030, 2172-2530) ❌ (D3D11 backend currently)
CMakeLists.txt (lines 47-59) ❌ (ImGui built with D3D11/Win32 only)
```

### Files That Need Creation/Modification

| File | Type | Purpose | Priority |
|------|------|---------|----------|
| `shaders/bloom.frag` | CREATE | GLSL bloom post-processing | MEDIUM |
| `shaders/colorgrade.frag` | CREATE | GLSL color grading LUT | MEDIUM |
| `shaders/particles.glsl` | CREATE | GLSL compute for particle physics | HIGH |
| `visuals/PostProcessor.cpp` | MODIFY | Add OpenGL `#elif` branch | MEDIUM |
| `src/main.cpp` | MODIFY | Add OpenGL `#elif` ImGui init path | HIGH |
| `CMakeLists.txt` | MODIFY | Build OpenGL3/GLFW ImGui backends | HIGH |

---

## 6. Implementation Scope Recommendations

### 🟢 HIGH PRIORITY (MUST INCLUDE)

**1. ImGui OpenGL3/GLFW Integration**
- CMakeLists.txt: Switch ImGui library to OpenGL3/GLFW backends
- src/main.cpp: Add alternate initialization and frame sequence
- Scope: ~50 lines CMake changes + ~100 lines C++ changes
- Impact: UI rendering works with OpenGL (critical for usability)
- Tests: Rendering should not crash, UI elements visible

**2. Compute Shader GLSL Particle Physics**
- Create `shaders/particles.glsl` (port from particles.hlsl)
- Ensure ComputeShader::compileSource() can load and compile it
- Scope: ~80 lines GLSL
- Impact: Particle effects render correctly in OpenGL
- Tests: ParticleSystem_Phase20_test.cpp should pass with OpenGL

**3. End-to-End Pipeline Test**
- Create test showing Enhanced3DScene + ImGui UI + particle effects rendering
- Verify no GL errors in any feature
- Scope: ~200 lines test code
- Impact: Validates complete Phase 7 integration

### 🟡 MEDIUM PRIORITY (SHOULD INCLUDE)

**4. Post-Processing GLSL Shaders**
- Create `shaders/bloom.frag` (port from bloom.hlsl)
- Create `shaders/colorgrade.frag` (port from colorgrade.hlsl)
- Scope: ~150 lines GLSL combined
- Impact: Bloom and color grading effects available
- Tests: PostProcessor_test.cpp verifies effect application

**5. PostProcessor OpenGL Implementation Branch**
- Add `#elif defined(DJROOFRAT_OPENGL_MIGRATION)` branch to PostProcessor.cpp
- Use Framebuffer class instead of RenderTarget
- Use new GLSL shaders
- Scope: ~300 lines C++
- Impact: Professional post-processing effects work
- Tests: Bloom and color grade effects render without artifacts

### 🔴 OPTIONAL (PHASE 8 OR LATER)

**6. Advanced Post-Processing Enhancements**
- Temporal antialiasing (TAA)
- Screen-space reflections (SSR)
- Advanced color correction
- Scope: Significant
- Can be deferred - core Phase 7 functional without these

---

## 7. GLSL Shader Porting Strategy

### Bloom: HLSL → GLSL Conversion

**HLSL Structure (bloom.hlsl):**
```hlsl
Texture2D SourceTexture : register(t0);
SamplerState LinearSampler : register(s0);

cbuffer BloomConstants : register(b0) {
    float2 TextureSize;
    float2 BlurDirection;
    float Intensity;
    float3 Padding;
};

struct VS_INPUT { float3 Position : POSITION; float2 TexCoord : TEXCOORD0; };
struct PS_INPUT { float4 Position : SV_POSITION; float2 TexCoord : TEXCOORD0; };

PS_INPUT VSMain(VS_INPUT input) { ... }
float4 PSMain(PS_INPUT input) : SV_TARGET { ... }
```

**GLSL Equivalent (bloom.frag):**
```glsl
#version 430 core

layout(binding = 0) uniform sampler2D SourceTexture;

layout(std140, binding = 0) uniform BloomConstants {
    vec2 TextureSize;
    vec2 BlurDirection;
    float Intensity;
};

in vec2 TexCoord;
out vec4 FragColor;

void main() { ... }
```

**Key Conversion Rules:**
- `Texture2D` → `sampler2D` with layout binding
- `SamplerState` → Merged into sampler declaration
- `cbuffer` → `uniform` block with `std140` layout
- `register(tx)` → `layout(binding = x)`
- `SV_POSITION` → `gl_Position` (VS out)
- `SV_TARGET` → Output fragment out

### Color Grade: HLSL → GLSL Conversion

**Same pattern as bloom:**
- Input texture + sampler
- Constant buffer with mood value [0-3]
- Fragment shader applies color LUT based on mood
- Scope: ~40 lines GLSL

### Particles Compute: HLSL → GLSL Conversion

**HLSL Compute Signature:**
```hlsl
struct Particle { float3 position; float lifetime; float3 velocity; float initialLife; float4 color; float size; float3 padding; };
RWStructuredBuffer<Particle> Particles : register(u0);
cbuffer ParticleConstants : register(b0) { float3 Gravity; float DeltaTime; float3 WindForce; int ParticleCount; };
[numthreads(256, 1, 1)] void CSMain(uint3 DTid : SV_DispatchThreadID) { ... }
```

**GLSL Compute Equivalent:**
```glsl
#version 430 core
layout(local_size_x = 256) in;

struct Particle { vec3 position; float lifetime; vec3 velocity; float initialLife; vec4 color; float size; vec3 padding; };
layout(std430, binding = 0) buffer ParticleBuffer { Particle particles[]; };

layout(std140, binding = 0) uniform ParticleConstants {
    vec3 Gravity;
    float DeltaTime;
    vec3 WindForce;
    int ParticleCount;
};

void main() { uint idx = gl_GlobalInvocationID.x; if (idx >= ParticleCount) return; ... }
```

---

## 8. Testing Strategy

### Existing Tests (Already Passing)
✅ `tests/Framebuffer_Phase7_test.cpp` - 8 tests, verifies FBO operations  
✅ `tests/ComputeShader_Phase7_test.cpp` - 8 tests, verifies compute dispatch  
✅ `tests/Enhanced3DScene_Phase6_test.cpp` - 12 tests, verifies 3D rendering  

### New Tests Needed for Phase 7

**1. ImGui OpenGL3 Integration Tests**
- Test ImGui initialization with GLFW/OpenGL3
- Test UI element rendering (buttons, panels, text)
- Test frame loop sequence without crashes
- Expected: 4-5 tests

**2. Post-Processing Pipeline Tests**
- Test Framebuffer binding for bloom passes
- Test bloom effect renders without artifacts
- Test color grading applies correct mood LUT
- Test multiple render targets (MRT) work
- Expected: 4-5 tests

**3. Compute Shader Particle Tests**
- Test GLSL particle shader compilation
- Test particle dispatch with 10k particles
- Test particle physics (gravity, wind) calculation
- Test particle lifetime decay
- Expected: 4-5 tests

**4. End-to-End Integration Test**
- Initialize graphics with OpenGL pipeline
- Create UI with ImGui/OpenGL3
- Trigger particle effects
- Apply post-processing to scene
- Render complete frame
- Expected: 1 comprehensive integration test

**5. Backward Compatibility Tests**
- Ensure D3D11 path still works (conditional compilation)
- Run all existing graphics tests with D3D11
- Expected: Regression tests (existing suite runs unchanged)

### Test Execution Plan
1. Run all existing Phase 6 tests - verify no regressions
2. Run new Phase 7 unit tests - verify each feature
3. Run integration test - verify complete pipeline
4. Manual testing - visual validation of effects
5. Performance testing - ensure 60 FPS maintained

---

## 9. Shader Compilation Strategy

### Shader File Organization
```
shaders/
├── basic3d.vert ✅ (existing, GLSL)
├── basic3d.frag ✅ (existing, GLSL)
├── bloom.frag (NEW - GLSL fragment)
├── colorgrade.frag (NEW - GLSL fragment)
└── particles.glsl (NEW - GLSL compute)
```

### Shader Compilation in Code

**Existing Approach (Phase 6):**
- `Shader::compile(vertexSrc, fragmentSrc, "vertex"|"fragment")`
- Shader files loaded and compiled at runtime
- GLSL source files stored as plain text in `shaders/` directory

**For Post-Processing Shaders:**
Same approach - use `Shader::compile()` with bloom/colorgrade source files

**For Compute Shader:**
Already supported via `ComputeShader::compileSource()`
- Reads `particles.glsl` if available
- Fallback to runtime string if file not found (for testing)

### Build System Integration
CMakeLists.txt doesn't need to compile shaders (they're runtime-loaded)
- Ensure shader files are copied to build output directory
- Or embed as strings in header files (optional optimization)

---

## 10. Recommended Phase 7 Architecture

### Complete OpenGL Pipeline
```
User Input (Keyboard/MIDI)
    ↓
ImGui UI Rendering (OpenGL3/GLFW)
    ↓
Application Logic
    ↓
3D Scene Rendering (Enhanced3DScene - VAO/VBO)
    ↓
GPU Particle Update (ComputeShader - particles.glsl)
    ↓
Post-Processing (Framebuffer + bloom/colorgrade GLSL)
    ↓
ImGui UI Overlay
    ↓
SwapBuffers (GLFW)
```

### Feature Maturity
- **Tier 1 (Core):** Enhanced3DScene, ComputeShader, ImGui OpenGL3
- **Tier 2 (Polish):** Post-Processing (bloom, color grading)
- **Tier 3 (Advanced):** TAA, SSR (phase 8+)

---

## 11. Deliverables Summary

### Code Changes Required
| Category | Count | LOC | Priority |
|----------|-------|-----|----------|
| GLSL Shaders (new) | 3 files | ~270 lines | HIGH |
| C++ Implementation | 1-2 files | ~300 lines | HIGH |
| CMakeLists.txt | 1 file | ~30 lines | HIGH |
| Tests | 10-12 files | ~1200 lines | HIGH |
| Documentation | 1 file | This doc | HIGH |

### Files Created
1. `shaders/bloom.frag`
2. `shaders/colorgrade.frag`
3. `shaders/particles.glsl`
4. `tests/PostProcessor_Phase7_test.cpp` (if OpenGL impl)
5. `tests/ImGuiOpenGL3_Phase7_test.cpp` (integration)
6. `tests/EndToEnd_Phase7_test.cpp` (full pipeline)

### Files Modified
1. `CMakeLists.txt` → Add OpenGL3/GLFW ImGui backend
2. `src/main.cpp` → Add OpenGL ImGui initialization path
3. `visuals/PostProcessor.cpp` → Add OpenGL branch with Framebuffer
4. `visuals/PostProcessor.h` → Add OpenGL conditional compilation

### Functions Implemented
- ImGui OpenGL3 backend initialization (`ImGui_ImplOpenGL3_Init`)
- ImGui GLFW platform backend initialization (`ImGui_ImplGlfw_InitForOpenGL`)
- PostProcessor OpenGL methods (if included)
- GLSL shader compilation helpers (if needed)

---

## 12. Risk Assessment & Mitigation

### Technical Risks
| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|-----------|
| HLSL→GLSL shader translation errors | MEDIUM | HIGH | Port conservatively, test each effect independently |
| ImGui OpenGL3 initialization bugs | LOW | HIGH | Use tested ImGui examples as reference |
| Performance regression in particle compute | LOW | MEDIUM | Profile before/after with 10k particles |
| Framebuffer completeness issues | LOW | MEDIUM | Verify FBO status before rendering |

### Regression Prevention
- Keep D3D11 path intact with `#if/#elif/#else` conditional compilation
- All existing tests must still pass with D3D11 build
- Use feature flags to toggle between backends
- Run full test suite on both paths

---

## 13. Success Criteria

✅ **Phase 7 Complete When:**
1. ImGui renders correctly with OpenGL3/GLFW backend
2. All existing Phase 6 3D rendering continues to work
3. Particle effects render with GPU compute shader
4. Application compiles and runs without graphics errors
5. All Phase 7 tests pass
6. 30+ FPS minimum maintained with all features active
7. D3D11 path still functional (backward compatibility)

**Optional Enhancements:**
- Bloom and color grading effects render correctly
- Post-processing pipeline documented
- Performance optimizations applied

---

## 14. Next Steps (Implementation Phase)

1. **Immediate (Week 1):**
   - Implement ImGui OpenGL3/GLFW CMakeLists changes
   - Create src/main.cpp OpenGL ImGui path
   - Create particles.glsl compute shader
   - Write integration tests

2. **Short-term (Week 2):**
   - Port bloom.frag and colorgrade.frag
   - Implement PostProcessor OpenGL branch
   - Pass all Phase 7 tests

3. **Validation (Week 3):**
   - Performance testing
   - Visual validation
   - D3D11 regression testing
   - Documentation

---

## Appendix A: GLAD Function Reference

### Complete List of Loaded Functions (55+)

**State Management (8):**
glClear, glClearColor, glViewport, glEnable, glDisable, glGetError, glIsEnabled, glGetIntegerv

**Shader Compilation (10):**
glCreateShader, glDeleteShader, glShaderSource, glCompileShader, glGetShaderiv, glGetShaderInfoLog, glCreateProgram, glDeleteProgram, glLinkProgram, glUseProgram

**Uniform/Attribute Setup (8):**
glGetUniformLocation, glUniform1f, glUniform2f, glUniform3f, glUniform4f, glUniformMatrix4fv, glGetAttribLocation, glVertexAttribPointer

**Buffer Operations (10):**
glGenBuffers, glDeleteBuffers, glBindBuffer, glBufferData, glBufferSubData, glMapBuffer, glUnmapBuffer, glMapBufferRange, glBindBufferBase, glGetBufferParameteriv

**Vertex Arrays (5):**
glGenVertexArrays, glDeleteVertexArrays, glBindVertexArray, glEnableVertexAttribArray, glDisableVertexAttribArray

**Texture Operations (10):**
glGenTextures, glDeleteTextures, glBindTexture, glTexImage2D, glTexSubImage2D, glTexParameteri, glTexParameterf, glActiveTexture, glGenerateMipmap, glGetTexParameteriv

**Framebuffer Objects (6):**
glGenFramebuffers, glDeleteFramebuffers, glBindFramebuffer, glFramebufferTexture2D, glCheckFramebufferStatus, glDrawBuffers

**Drawing (2):**
glBlitFramebuffer, (DrawIndexed implemented via VAO + glDrawElements from core)

**Compute Shaders (3):**
glDispatchCompute, glMemoryBarrier, glBindBufferBase (also used for buffer binding)

**Queries (7):**
glGetFloatv, glGetVertexAttribiv, glGetProgramiv, glGetProgramInfoLog, glGetShaderiv, glGetShaderInfoLog, glGetError

---

## Appendix B: D3D11 Reference Implementation (PostProcessor)

### Key Methods to Replicate in OpenGL

```cpp
// D3D11 version has:
- bool initialize(ID3D11Device*, ID3D11DeviceContext*, int width, int height)
- bool applyBloom(float intensity)
- bool applyColorGrade(int mood)
- RenderTarget* getBloomTarget()
- RenderTarget* getTempTarget()

// Private helpers:
- bool createFullscreenQuad()
- bool updateBloomConstantBuffer(...)
- bool updateColorGradeConstantBuffer(int mood)
```

### OpenGL Equivalent Pattern
```cpp
#elif defined(DJROOFRAT_OPENGL_MIGRATION)
bool PostProcessor::initialize(int width, int height) {
    // Create Framebuffer objects instead of RenderTarget
    // Create Texture objects for attachments
    // Compile GLSL shaders
}

bool PostProcessor::applyBloom(float intensity) {
    // Bind framebuffers
    // Render to bloom texture with bloom.frag
    // Apply blitting/downsampling if needed
}
```

---

**Research Complete: April 2, 2026**  
**Next Phase: Implementation Planning & Development**
