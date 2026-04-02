## Plan: DirectX 11 to OpenGL Migration

Migrate DJ-ROOFRAT from DirectX 11 to OpenGL for cross-platform compatibility and to resolve 3D controller visibility issues through proper constant buffer alignment and vertex attribute binding.

**Phases: 7 phases**

---

### **Phase 1: Infrastructure Setup and Dependencies**
- **Objective:** Add OpenGL dependencies (GLAD, GLFW, GLM) and configure CMakeLists.txt for OpenGL build
- **Files/Functions to Modify/Create:**
  - [CMakeLists.txt](CMakeLists.txt) — Remove d3d11/dxgi/d3dcompiler, add GLFW/GLAD/GLM via FetchContent or find_package
  - [vcpkg.json](vcpkg.json) — Add glfw3, glad, glm if using vcpkg
  - Create [vendor/glad/](vendor/glad/) — GLAD loader (generated from https://glad.dav1d.de/)
- **Tests to Write:**
  - `tests/GLContext_Phase1_test.cpp` — Test GLFW window creation succeeds
  - `tests/GLContext_Phase1_test.cpp` — Test GLAD loads OpenGL 3.3+ successfully
  - `tests/GLContext_Phase1_test.cpp` — Test glGetString(GL_VERSION) returns valid version
- **Steps:**
  1. Write test for GLFW window creation (expect failure: library not linked)
  2. Add GLFW to CMakeLists.txt via FetchContent or vcpkg
  3. Run test to verify GLFW window creation succeeds
  4. Write test for GLAD loader initialization (expect failure: no loader)
  5. Generate GLAD loader for OpenGL 3.3 Core, add to vendor/glad/
  6. Add GLAD source to CMakeLists.txt
  7. Run test to verify GLAD loads successfully
  8. Write test for GLM matrix operations (basic mat4 multiply)
  9. Add GLM to CMakeLists.txt
  10. Run test to verify GLM works
  11. Build project to confirm all dependencies resolve

---

### **Phase 2: Core Buffer System (VBO/EBO/UBO)**
- **Objective:** Port [VertexBuffer](visuals/VertexBuffer.h), [IndexBuffer](visuals/IndexBuffer.h) classes from DirectX buffers to OpenGL buffers (VBO/EBO/UBO)
- **Files/Functions to Modify/Create:**
  - [visuals/VertexBuffer.cpp](visuals/VertexBuffer.cpp) — Replace ID3D11Buffer with GLuint vbo_, use glGenBuffers/glBindBuffer/glBufferData
  - [visuals/VertexBuffer.h](visuals/VertexBuffer.h) — Replace ComPtr<ID3D11Buffer> with GLuint
  - [visuals/IndexBuffer.cpp](visuals/IndexBuffer.cpp) — Replace ID3D11Buffer with GLuint ebo_
  - [visuals/IndexBuffer.h](visuals/IndexBuffer.h) — Replace ComPtr with GLuint
  - Create/modify constant buffer wrapper for UBO management (may rename to UniformBuffer.cpp/h)
- **Tests to Write:**
  - `tests/VertexBuffer_Phase2_test.cpp` — Test VBO creation with vertex data
  - `tests/VertexBuffer_Phase2_test.cpp` — Test VBO update (glBufferSubData)
  - `tests/IndexBuffer_Phase2_test.cpp` — Test EBO creation with index data
  - `tests/IndexBuffer_Phase2_test.cpp` — Test EBO binding and query
- **Steps:**
  1. Write test for VertexBuffer::create() expecting GL VBO (fails: still D3D11)
  2. Replace VertexBuffer D3D11 implementation with glGenBuffers/glBindBuffer/glBufferData
  3. Run test to verify VBO creation succeeds
  4. Write test for VertexBuffer::update() with glBufferSubData
  5. Implement update() using glBufferSubData
  6. Run test to confirm buffer updates work
  7. Write test for IndexBuffer::create() expecting GL EBO
  8. Replace IndexBuffer D3D11 implementation with GL EBO
  9. Run test to verify EBO creation succeeds
  10. Lint and format all modified files

---

### **Phase 3: Shader System and GLSL Conversion**
- **Objective:** Port [Shader.cpp](visuals/Shader.cpp) to compile GLSL instead of HLSL, convert core shaders (basic, textured, enhanced) to GLSL
- **Files/Functions to Modify/Create:**
  - [visuals/Shader.cpp](visuals/Shader.cpp) — Replace D3DCompile with glCreateShader/glShaderSource/glCompileShader/glLinkProgram
  - [visuals/Shader.h](visuals/Shader.h) — Replace ID3D11VertexShader/ID3D11PixelShader with GLuint shaderProgram_
  - [shaders/basic.glsl](shaders/basic.glsl) — Convert basic.hlsl (VS/PS) to GLSL #version 330 core
  - [shaders/textured.glsl](shaders/textured.glsl) — Convert textured.hlsl to GLSL
  - [shaders/enhanced.glsl](shaders/enhanced.glsl) — Convert enhanced.hlsl (row_major matrices) to GLSL
- **Tests to Write:**
  - `tests/Shader_Phase3_test.cpp` — Test GLSL vertex shader compilation succeeds
  - `tests/Shader_Phase3_test.cpp` — Test GLSL fragment shader compilation succeeds
  - `tests/Shader_Phase3_test.cpp` — Test shader program linking succeeds
  - `tests/Shader_Phase3_test.cpp` — Test uniform location query (glGetUniformLocation)
- **Steps:**
  1. Write test for Shader::loadVertexShader() with GLSL string (fails: no GL implementation)
  2. Implement Shader class GL version: glCreateShader, glShaderSource, glCompileShader, check compile errors
  3. Run test to verify vertex shader compilation succeeds
  4. Write test for fragment shader compilation
  5. Implement fragment shader compilation in Shader class
  6. Run test to verify fragment shader compiles
  7. Write test for shader program linking
  8. Implement glLinkProgram and error checking
  9. Run test to verify program links successfully
  10. Convert basic.hlsl to basic.glsl (cbuffer → uniform block, Texture2D → sampler2D, float3 → vec3)
  11. Write test loading basic.glsl and querying uniform locations
  12. Run test to confirm basic.glsl compiles and links
  13. Convert enhanced.hlsl to enhanced.glsl (row_major → layout(row_major))
  14. Convert textured.hlsl to textured.glsl
  15. Test each shader individually
  16. Lint and format all shader files

---

### **Phase 4: GraphicsContext and Window Management**
- **Objective:** Replace DirectX device/context initialization in [GraphicsContext](visuals/GraphicsContext.h) with OpenGL context via GLFW
- **Files/Functions to Modify/Create:**
  - [visuals/GraphicsContext.cpp](visuals/GraphicsContext.cpp) — Replace D3D11CreateDevice/CreateSwapChain with glfwCreateWindow/glfwMakeContextCurrent/gladLoadGLLoader
  - [visuals/GraphicsContext.h](visuals/GraphicsContext.h) — Replace ID3D11Device/ID3D11DeviceContext/IDXGISwapChain1 with GLFWwindow*
  - [src/main.cpp](src/main.cpp) — Update initialization calls, replace swapChain->Present() with glfwSwapBuffers()
- **Tests to Write:**
  - `tests/GraphicsContext_Phase4_test.cpp` — Test GraphicsContext::initialize() creates valid GLFW window
  - `tests/GraphicsContext_Phase4_test.cpp` — Test OpenGL context is current after initialize()
  - `tests/GraphicsContext_Phase4_test.cpp` — Test getDevice() returns valid GLFWwindow pointer (or adapt interface)
- **Steps:**
  1. Write test for GraphicsContext::initialize() expecting GLFWwindow creation (fails: still D3D11)
  2. Replace D3D11CreateDevice with glfwInit() + glfwWindowHint() + glfwCreateWindow()
  3. Replace swap chain creation with glfwSetWindowUserPointer for context management
  4. Call glfwMakeContextCurrent() and gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)
  5. Run test to verify GLFW window and GL context creation succeeds
  6. Write test for context validity (glGetString(GL_VERSION) != nullptr)
  7. Run test to confirm GL context is valid
  8. Update main.cpp to replace swapChain->Present() with glfwSwapBuffers(window)
  9. Add glfwPollEvents() to main loop
  10. Build and run application to verify window appears (blank screen expected at this phase)
  11. Lint and format modified files

---

### **Phase 5: Texture System and Framebuffer Objects**
- **Objective:** Port [TextureManager](visuals/TextureManager.h), [RenderTarget](visuals/RenderTarget.h), [ShadowMap](visuals/ShadowMap.h) to OpenGL textures and FBOs
- **Files/Functions to Modify/Create:**
  - [visuals/TextureManager.cpp](visuals/TextureManager.cpp) — Replace ID3D11Texture2D/ID3D11ShaderResourceView with GLuint texture_, use glGenTextures/glBindTexture/glTexImage2D
  - [visuals/TextureManager.h](visuals/TextureManager.h) — Replace ComPtr with GLuint, sampler states with GL sampler objects
  - [visuals/RenderTarget.cpp](visuals/RenderTarget.cpp) — Replace ID3D11RenderTargetView/ID3D11DepthStencilView with GLuint fbo_, colorAttachment_, depthAttachment_
  - [visuals/RenderTarget.h](visuals/RenderTarget.h) — Replace D3D11 types with GL types
  - [visuals/ShadowMap.cpp](visuals/ShadowMap.cpp) — Use GL_DEPTH_COMPONENT texture, framebuffer with GL_DEPTH_ATTACHMENT
  - [visuals/ShadowMap.h](visuals/ShadowMap.h) — Replace D3D11 types
- **Tests to Write:**
  - `tests/TextureManager_Phase5_test.cpp` — Test texture creation from raw RGBA data
  - `tests/TextureManager_Phase5_test.cpp` — Test sampler state binding (glBindSampler)
  - `tests/RenderTarget_Phase5_test.cpp` — Test FBO creation with color + depth attachments
  - `tests/RenderTarget_Phase5_test.cpp` — Test FBO completeness (glCheckFramebufferStatus)
  - `tests/ShadowMap_Phase5_test.cpp` — Test depth texture FBO creation
- **Steps:**
  1. Write test for TextureManager::create() expecting GL texture (fails: D3D11 implementation)
  2. Replace D3D11 texture creation with glGenTextures/glBindTexture/glTexImage2D/glTexParameteri
  3. Run test to verify texture creation succeeds
  4. Write test for sampler state binding
  5. Implement GL sampler objects (glGenSamplers, glSamplerParameteri)
  6. Run test to confirm sampler binding works
  7. Write test for RenderTarget FBO creation (fails: D3D11)
  8. Replace RenderTarget with glGenFramebuffers/glBindFramebuffer/glFramebufferTexture2D
  9. Run test to verify FBO creation and completeness check passes
  10. Write test for ShadowMap depth texture FBO
  11. Implement ShadowMap using GL_DEPTH_COMPONENT texture + FBO
  12. Run test to verify shadow map FBO is complete
  13. Lint and format all modified files

---

### **Phase 6: Enhanced3DScene OpenGL Rendering Pipeline**
- **Objective:** Port [Enhanced3DScene](visuals/Enhanced3DScene.h) rendering methods to use OpenGL draw calls, VAO binding, and uniform updates
- **Files/Functions to Modify/Create:**
  - [visuals/Enhanced3DScene.cpp](visuals/Enhanced3DScene.cpp) — Replace IASetInputLayout/VSSetShader/PSSetShader/DrawIndexed with glBindVertexArray/glUseProgram/glDrawElements
  - [visuals/Enhanced3DScene.h](visuals/Enhanced3DScene.h) — Add GLuint controllerVAO_, replace ID3D11InputLayout
  - [visuals/Enhanced3DScene.cpp](visuals/Enhanced3DScene.cpp)::createControllerGeometry() — Create VAO, bind VBO/EBO, set vertex attribute pointers (glVertexAttribPointer, glEnableVertexAttribArray)
  - [visuals/Enhanced3DScene.cpp](visuals/Enhanced3DScene.cpp)::renderController() — Bind VAO, use shader program, set uniforms (glUniformMatrix4fv), draw with glDrawElements
- **Tests to Write:**
  - `tests/Enhanced3DScene_Phase6_test.cpp` — Test VAO creation for controller geometry
  - `tests/Enhanced3DScene_Phase6_test.cpp` — Test vertex attribute pointer setup matches Vertex struct
  - `tests/Enhanced3DScene_Phase6_test.cpp` — Test renderController() binds correct VAO and shader
  - `tests/Enhanced3DScene_Phase6_test.cpp` — Test uniform updates (matrix, light data)
- **Steps:**
  1. Write test for createControllerGeometry() expecting VAO creation (fails: no GL implementation)
  2. Implement VAO creation in createControllerGeometry(): glGenVertexArrays, glBindVertexArray
  3. Bind VBO/EBO to VAO
  4. Set vertex attribute pointers matching Vertex struct (position, color, normal, texCoord)
  5. glEnableVertexAttribArray for each attribute
  6. Run test to verify VAO created and attributes bound correctly
  7. Write test for renderController() expecting glUseProgram and glDrawElements calls
  8. Implement renderController() with glBindVertexArray, glUseProgram, glUniformMatrix4fv for matrices
  9. Replace DrawIndexed() with glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0)
  10. Run test to verify rendering pipeline executes without errors
  11. Add glEnable(GL_DEPTH_TEST), glCullFace(GL_BACK) to ensure depth testing and culling
  12. Update viewport with glViewport() matching render target dimensions
  13. Build and run application to verify 3D controller geometry is now visible
  14. Lint and format modified files

---

### **Phase 7: Advanced Features and ImGui Integration**
- **Objective:** Port remaining features (compute shaders, post-processing, instanced rendering) and switch ImGui to OpenGL3 backend
- **Files/Functions to Modify/Create:**
  - [visuals/ComputeShader.cpp](visuals/ComputeShader.cpp) — Replace ID3D11ComputeShader with GLuint computeProgram_, RWStructuredBuffer → SSBO
  - [visuals/ParticleSystem.cpp](visuals/ParticleSystem.cpp) — Update particle buffer to use SSBO (GL_SHADER_STORAGE_BUFFER)
  - [visuals/PostProcessor.cpp](visuals/PostProcessor.cpp) — Update multi-pass rendering to use FBOs and texture ping-pong
  - [visuals/CrowdRenderer.cpp](visuals/CrowdRenderer.cpp) — Replace DrawIndexedInstanced with glDrawElementsInstanced
  - [src/main.cpp](src/main.cpp) — Replace imgui_impl_dx11 includes/calls with imgui_impl_opengl3
  - [shaders/particles.glsl](shaders/particles.glsl) — Convert particles.hlsl compute shader to GLSL #version 430 (SSBO support)
  - [shaders/lighting.glsl](shaders/lighting.glsl) — Convert lighting.hlsl (shadow mapping) to GLSL
  - [shaders/bloom.glsl](shaders/bloom.glsl), [shaders/colorgrade.glsl](shaders/colorgrade.glsl), etc. — Convert remaining post-process shaders
- **Tests to Write:**
  - `tests/ComputeShader_Phase7_test.cpp` — Test compute shader compilation and dispatch
  - `tests/ParticleSystem_Phase7_test.cpp` — Test SSBO creation and compute shader particle updates
  - `tests/PostProcessor_Phase7_test.cpp` — Test bloom pass renders to intermediate FBO
  - `tests/CrowdRenderer_Phase7_test.cpp` — Test instanced rendering with glDrawElementsInstanced
  - `tests/ImGuiOpenGL_Phase7_test.cpp` — Test ImGui OpenGL3 backend initialization and frame rendering
- **Steps:**
  1. Write test for ComputeShader compilation (GLSL #version 430) (fails: no implementation)
  2. Implement ComputeShader using glCreateShader(GL_COMPUTE_SHADER), glDispatchCompute
  3. Run test to verify compute shader compiles and dispatches
  4. Write test for ParticleSystem SSBO creation
  5. Replace particle buffer with GL_SHADER_STORAGE_BUFFER, bind with glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo)
  6. Run test to verify SSBO created and compute shader can read/write
  7. Convert particles.hlsl to particles.glsl (RWStructuredBuffer → layout(std430) buffer)
  8. Test particle system updates particles correctly via compute shader
  9. Write test for PostProcessor bloom pass FBO rendering
  10. Implement PostProcessor using FBO ping-pong for multi-pass effects
  11. Run test to verify bloom renders to intermediate texture
  12. Convert bloom.hlsl, colorgrade.hlsl, and other post-process shaders to GLSL
  13. Write test for CrowdRenderer instanced draw
  14. Replace DrawIndexedInstanced with glDrawElementsInstanced
  15. Run test to verify instanced rendering works
  16. Update main.cpp: remove `#include <imgui_impl_dx11.h>`, add `#include <imgui_impl_opengl3.h>`
  17. Replace ImGui_ImplDX11_Init() with ImGui_ImplOpenGL3_Init("#version 330")
  18. Replace ImGui_ImplDX11_NewFrame() with ImGui_ImplOpenGL3_NewFrame()
  19. Replace ImGui_ImplDX11_RenderDrawData() with ImGui_ImplOpenGL3_RenderDrawData()
  20. Build and run application to verify ImGui renders alongside 3D scene
  21. Test all features (controller, crowd, particles, post-processing) working together
  22. Lint and format all modified files
  23. Run full test suite to ensure all tests pass

---

## **Open Questions**

1. **OpenGL Version Target**: Should we target OpenGL 3.3 Core (widest compatibility) or OpenGL 4.6 (latest features, compute shaders guaranteed)? Compute shaders require 4.3+.
   - **Recommendation**: OpenGL 4.3 for compute shader support (particles), fallback to 3.3 for non-compute features.

2. **Windowing Library**: Prefer GLFW (cross-platform, modern) or keep Windows-specific window creation (HWND)?
   - **Recommendation**: GLFW for cross-platform support and easier context management.

3. **Extension Loader**: Use GLAD (modern, customizable) or GLEW (mature, widely used)?
   - **Recommendation**: GLAD for cleaner integration and smaller footprint.

4. **Matrix Library**: Use GLM (C++ header-only, GLSL-like syntax) or keep custom matrix implementation?
   - **Recommendation**: GLM to match GLSL types (mat4, vec3) and avoid manual debugging.

5. **Shader Format**: Store GLSL as separate .vert/.frag files or combined .glsl with preprocessor directives?
   - **Recommendation**: Separate .vert/.frag for clarity; alternatively use #ifdef VERTEX/FRAGMENT in single .glsl.

6. **Constant Buffer Packing**: Use std140 layout (guaranteed packing) or std430 (tighter packing, compute only)?
   - **Recommendation**: std140 for uniform blocks (vertex/fragment shaders), std430 for SSBOs (compute shaders).
