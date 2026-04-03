# Plan: Complete OpenGL Migration for DJ-ROOFRAT

**Goal:** Fully implement OpenGL 4.3 rendering backend for DJ-ROOFRAT with windowed graphics display.

**TL;DR:** Replace all D3D11-specific shader creation calls with OpenGL implementations, add full OpenGL rendering for PostProcessor (bloom/color grading), ShadowMap (depth rendering), and Enhanced3DScene (3D geometry), then build and run with graphics window.

---

## Phases (5 phases)

### Phase 1: Fix Shader Method Calls
**Objective:** Replace D3D11-specific `createShaders()` and `createPixelShader()` calls with OpenGL shader compilation

**Files/Functions to Modify:**
- `visuals/GraphicsContext.cpp::initialize()`
- `visuals/Enhanced3DScene.cpp::initialize()`
- `visuals/PostProcessor.cpp::initialize()`

**Tests to Write:**
- Verify shader compilation succeeds with OpenGL backend
- Verify no D3D11 linker errors
- Test that Shader::loadFromFiles() works in OpenGL mode

**Steps:**
1. Read existing D3D11 shader loading code
2. Replace `createShaders(device)` calls with `loadFromFiles(vspath, fspath)` + conditional compilation
3. Replace `createPixelShader(device)` calls with fragment-only compilation pattern
4. Add `#if defined(DJROOFRAT_OPENGL_MIGRATION)` guards
5. Ensure shader file paths are correct
6. Build and verify no shader-related linker errors

---

### Phase 2: Implement PostProcessor OpenGL Rendering
**Objective:** Add full OpenGL framebuffer, texture, and fullscreen quad rendering for bloom and color grading

**Files/Functions to Modify/Create:**
- `visuals/PostProcessor.h` - Add OpenGL member variables
- `visuals/PostProcessor.cpp::initialize()` - Create FBOs, textures, VAOs
- `visuals/PostProcessor.cpp::createFullscreenQuad()` - OpenGL VAO/VBO setup
- `visuals/PostProcessor.cpp::applyBloom()` - Multi-pass Gaussian blur
- `visuals/PostProcessor.cpp::applyColorGrade()` - Color grading shader pass

**Tests to Write:**
- Test framebuffer creation and binding
- Test fullscreen quad renders to screen
- Test bloom blur produces expected output
- Test color grading alters image tone

**Steps:**
1. Add OpenGL member variables (GLuint bloomFBO_, bloomTexture_, fullscreenVAO_, etc.)
2. Implement framebuffer and texture creation in `initialize()`
3. Create fullscreen quad VAO with normalized device coordinates
4. Load bloom and color grading shaders from files
5. Implement multi-pass Gaussian blur for bloom
6. Implement color grading LUT or parametric adjustment
7. Add cleanup in destructor

---

### Phase 3: Implement ShadowMap OpenGL Rendering
**Objective:** Add depth framebuffer and shadow texture rendering for dynamic shadows

**Files/Functions to Modify/Create:**
- `visuals/ShadowMap.h` - Add OpenGL member variables
- `visuals/ShadowMap.cpp::initialize()` - Create depth FBO and texture
- `visuals/ShadowMap.cpp::bindForDepthPass()` - Bind shadow framebuffer
- `visuals/ShadowMap.cpp::unbindDepthPass()` - Restore default framebuffer
- `visuals/ShadowMap.cpp::getLightViewMatrix()`, `getLightProjMatrix()` - Matrix computation

**Tests to Write:**
- Test depth framebuffer creation
- Test depth texture binding
- Test shadow matrix computation
- Test depth pass renders geometry

**Steps:**
1. Add OpenGL member variables (GLuint depthFBO_, depthTexture_, depthSampler_)
2. Create depth texture with GL_DEPTH_COMPONENT32F format
3. Attach depth texture to framebuffer
4. Setup comparison sampler for PCF shadow filtering
5. Implement light view/projection matrix computation
6. Implement framebuffer binding/unbinding
7. Add member variable guards for when graphics disabled

---

### Phase 4: Complete Enhanced3DScene OpenGL Rendering
**Objective:** Implement full 3D geometry rendering with shaders, textures, and lighting

**Files/Functions to Modify/Create:**
- `visuals/Enhanced3DScene.cpp::initializeOpenGL()` - Setup VAOs, VBOs, textures
- `visuals/Enhanced3DScene.cpp::renderOpenGL()` - Main render loop
- `visuals/Enhanced3DScene.cpp::createGeometry()` - Generate vertex data
- `visuals/Enhanced3DScene.cpp::updateUniforms()` - Upload shader uniforms

**Tests to Write:**
- Test VAO/VBO creation for tunnel geometry
- Test shader uniform updates
- Test texture binding
- Test full render pipeline executes

**Steps:**
1. Implement `initializeOpenGL()` method body
2. Create geometry buffers (VAO, VBO, EBO)
3. Load and compile all shaders (enhanced, tunnel, shadow depth)
4. Setup texture loading and binding
5. Implement uniform buffer updates (matrices, lighting)
6. Implement render loop with proper state management
7. Integrate ShadowMap depth pass
8. Integrate PostProcessor effects

---

### Phase 5: Build and Run with OpenGL Window
**Objective:** Successfully build, launch, and verify DJ-ROOFRAT with OpenGL graphics window

**Steps:**
1. Configure CMake: `cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64 -DDJROOFRAT_OPENGL_MIGRATION=ON -DDJROOFRAT_ENABLE_GRAPHICS=ON`
2. Build: `cmake --build build-vs --config Debug --target dj_roofrat`
3. Run: `build-vs\Debug\DJ-ROOFRAT.exe --no-audio`
4. Verify GLFW window opens
5. Verify ImGui interface renders
6. Test keyboard commands (deck control, EQ, mixer)
7. Verify graphics render without errors
8. Check for OpenGL errors with glGetError()
9. Test performance (should run at reasonable FPS)
10. Document any remaining issues

---

## Open Questions
None - full implementation approved.
