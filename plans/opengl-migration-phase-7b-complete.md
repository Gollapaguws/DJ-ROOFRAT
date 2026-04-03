## Phase 7B Complete: Particle GLSL Shader

Implemented GPU-accelerated particle physics system using GLSL 430 compute shaders for DJ-ROOFRAT visual effects (confetti, smoke). All tests passing with correct particle position updates from gravity/wind physics.

**Files created/changed:**
- shaders/particles.glsl
- tests/ParticleShader_Phase7B_test.cpp
- vendor/glad/include/glad/glad.h
- vendor/glad/src/glad.c
- CMakeLists.txt

**Functions created/changed:**
- `glMapBuffer` (added to GLAD loader)
- `glUnmapBuffer` (added to GLAD loader)
- Particle compute shader main() with physics integration
- GL_READ_ONLY/GL_WRITE_ONLY/GL_READ_WRITE constants added

**Tests created/changed:**
- Test 1: Compile particles.glsl compute shader ✓
- Test 2: Link compute program ✓
- Test 3: Dispatch compute and verify particle position updates ✓
- Test 4: Check for OpenGL errors ✓

**Review Status:** APPROVED

**Git Commit Message:**
```
feat: Add GLSL compute shader for GPU particle physics

- Implement particles.glsl with gravity/wind force integration
- Extend GLAD with glMapBuffer/glUnmapBuffer for buffer readback
- Add 4 comprehensive tests validating shader compilation and physics
- Configure CMake build target for Phase 7B test suite
```
