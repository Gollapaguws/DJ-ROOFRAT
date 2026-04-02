## Plan: Fix 3D Controller Visibility in Graphics Window

The 3D DJ controller renders but is invisible due to missing DirectX 11 pipeline state. Research identified the root cause: the input layout is never bound to the GPU pipeline before drawing, causing the vertex buffer to be unparseable. This plan will systematically fix the pipeline setup, verify matrix transformations, and improve error handling.

**Phases: 5**

1. **Phase 1: Fix Input Layout Binding (CRITICAL)**
    - **Objective:** Bind the input layout to the DirectX 11 pipeline before rendering the controller so the GPU can parse vertex data
    - **Files/Functions to Modify/Create:**
        - [visuals/Enhanced3DScene.h](visuals/Enhanced3DScene.h) - Add `inputLayout_` member to store the input layout
        - [visuals/Enhanced3DScene.cpp](visuals/Enhanced3DScene.cpp) - `createControllerGeometry()` to create input layout, `renderController()` to bind it
    - **Tests to Write:**
        - `test_createControllerGeometry_CreatesInputLayout` - verify input layout is created alongside vertex buffer
        - `test_renderController_BindsInputLayout` - verify IASetInputLayout is called before DrawIndexed
    - **Steps:**
        1. Write tests that verify input layout creation and binding (tests should fail)
        2. Add `Microsoft::WRL::ComPtr<ID3D11InputLayout> controllerInputLayout_;` member to Enhanced3DScene class
        3. In `createControllerGeometry()`, create input layout from vertex description and shader bytecode
        4. In `renderController()`, call `context->IASetInputLayout(controllerInputLayout_.Get())` before DrawIndexed
        5. Run tests to confirm they pass
        6. Build and run app to verify controller becomes visible

2. **Phase 2: Verify Matrix Transformations**
    - **Objective:** Ensure matrix layout (row-major vs column-major) is correct and transformations place controller in view frustum
    - **Files/Functions to Modify/Create:**
        - [visuals/Enhanced3DScene.cpp](visuals/Enhanced3DScene.cpp) - `renderController()` matrix setup
        - [visuals/Camera.cpp](visuals/Camera.cpp) - verify view/projection matrix calculation
        - [shaders/enhanced.hlsl](shaders/enhanced.hlsl) - verify matrix multiplication order
    - **Tests to Write:**
        - `test_controllerWorldMatrix_IsCorrect` - verify world matrix has correct scale and translation
        - `test_cameraViewMatrix_LooksAtController` - verify camera can see controller at Y=-5
        - `test_matrixMultiplication_OrderIsCorrect` - verify World→View→Projection order
    - **Steps:**
        1. Write tests for matrix calculations (tests should fail if matrices are incorrect)
        2. Verify Camera view matrix: position (0, -2, 8), looking at (0, -5, 0)
        3. Adjust controller world matrix if needed: scale and position within view frustum
        4. Verify shader uses correct multiplication: `mul(position, World)` then `mul(result, View)` then `mul(result, Projection)`
        5. Run tests to confirm matrices are correct
        6. Build and verify controller is positioned correctly on screen

3. **Phase 3: Add Shader Compilation Error Reporting**
    - **Objective:** Report shader compilation errors to help debug rendering issues
    - **Files/Functions to Modify/Create:**
        - [visuals/Shader.cpp](visuals/Shader.cpp) - `loadFromFile()` to check for errors
        - [visuals/GraphicsContext.cpp](visuals/GraphicsContext.cpp) - initialize() to report shader errors
    - **Tests to Write:**
        - `test_shaderCompilation_ReportsErrors` - verify error messages are logged
        - `test_shaderCompilation_FailsGracefully` - verify failed compilation doesn't crash
    - **Steps:**
        1. Write tests for shader error handling (tests should fail without error checking)
        2. In Shader::loadFromFile(), check D3DCompileFromFile HRESULT
        3. If FAILED(hr), get error blob and output to console/log
        4. Return false on compilation failure
        5. Run tests to verify error reporting works
        6. Build and verify no shader errors are reported

4. **Phase 4: Fix Material Buffer Field Mismatch**
    - **Objective:** Align Material constant buffer structure with shader expectations
    - **Files/Functions to Modify/Create:**
        - [visuals/GraphicsContext.h](visuals/GraphicsContext.h) - MaterialData struct
        - [shaders/enhanced.hlsl](shaders/enhanced.hlsl) - MaterialBuffer cbuffer
    - **Tests to Write:**
        - `test_materialBuffer_MatchesShader` - verify struct size and layout match
        - `test_materialBuffer_AlignmentIsCorrect` - verify 16-byte alignment
    - **Steps:**
        1. Write tests for buffer layout matching (tests should fail if mismatched)
        2. Read MaterialBuffer from enhanced.hlsl to get expected fields
        3. Update MaterialData struct in GraphicsContext.h to match exactly
        4. Ensure BeatIntensity field is present (not just padding)
        5. Run tests to verify structures match
        6. Build and verify no rendering artifacts from buffer mismatch

5. **Phase 5: Optimize Constant Buffer Creation**
    - **Objective:** Reuse constant buffers instead of creating new ones every frame for better performance
    - **Files/Functions to Modify/Create:**
        - [visuals/Enhanced3DScene.h](visuals/Enhanced3DScene.h) - Add `controllerConstantBuffer_` member
        - [visuals/Enhanced3DScene.cpp](visuals/Enhanced3DScene.cpp) - `createControllerGeometry()` to create buffer, `renderController()` to update it
    - **Tests to Write:**
        - `test_constantBuffer_CreatedOnce` - verify buffer is created during initialization
        - `test_constantBuffer_UpdatedPerFrame` - verify buffer data is updated, not recreated
    - **Steps:**
        1. Write tests for buffer reuse (tests should fail with current per-frame creation)
        2. Add `Microsoft::WRL::ComPtr<ID3D11Buffer> controllerConstantBuffer_;` to Enhanced3DScene
        3. In `createControllerGeometry()`, create constant buffer with D3D11_USAGE_DYNAMIC
        4. In `renderController()`, use Map/Unmap to update buffer data instead of CreateBuffer
        5. Run tests to verify buffer is reused
        6. Build and verify performance improvement (measure frame time)

**Open Questions:**
1. Should we add RenderDoc/PIX integration for easier graphics debugging in the future?
2. Should we add a wireframe rendering mode for diagnostic purposes?
3. Do we want to adjust the controller's initial position/scale after it becomes visible?
4. Should we add vertex/pixel shader debug output capabilities for future debugging?
