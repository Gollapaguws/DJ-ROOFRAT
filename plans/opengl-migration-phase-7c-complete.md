## Phase 7C Complete: Post-Processing GLSL Shaders

Implemented GLSL-based post-processing pipeline with bloom and color grading effects for DJ-ROOFRAT's OpenGL migration. All 7 tests passing with successful shader compilation and framebuffer rendering.

**Files created/changed:**
- shaders/postprocess.vert
- shaders/bloom.frag
- shaders/colorgrade.frag
- tests/PostProcessing_Phase7C_test.cpp
- vendor/glad/include/glad/glad.h
- vendor/glad/src/glad.c
- CMakeLists.txt

**Functions created/changed:**
- `glReadPixels` (added to GLAD loader)
- GL_RGBA8, GL_RGBA16F, GL_FLOAT, GL_UNSIGNED_BYTE constants added
- Fullscreen quad vertex shader (postprocess.vert)
- Gaussian blur bloom shader (bloom.frag)
- Color grading shader with exposure/gamma/saturation (colorgrade.frag)

**Tests created/changed:**
- Test 1: Compile postprocess.vert shader ✓
- Test 2: Compile bloom.frag shader ✓
- Test 3: Compile colorgrade.frag shader ✓
- Test 4: Link bloom program and create framebuffer ✓
- Test 5: Link color grade program and render to framebuffer ✓
- Test 6: Verify framebuffer color output ✓
- Test 7: Check for OpenGL errors ✓

**Review Status:** APPROVED

**Git Commit Message:**
```
feat: Add GLSL post-processing pipeline with bloom and color grading

- Implement postprocess.vert for fullscreen quad rendering
- Add bloom.frag with Gaussian blur and brightness threshold
- Add colorgrade.frag with exposure/gamma/saturation controls
- Extend GLAD with glReadPixels and texture format constants
- Create 7 comprehensive tests validating shader compilation and framebuffer rendering
- Configure CMake build target for Phase 7C test suite
```
