## Phase 5 Complete: Add Basic Shadow Mapping

Implemented single directional-light shadow mapping with depth-pass infrastructure, depth texture resources, and PCF-ready sampling integration. Shadow resources are created and bound correctly, including comparison sampler support required by `SamplerComparisonState`, and all Phase 5 tests pass.

**Files created/changed:**
- CMakeLists.txt
- visuals/ShadowMap.h
- visuals/ShadowMap.cpp
- visuals/Enhanced3DScene.h
- visuals/Enhanced3DScene.cpp
- shaders/shadowdepth.hlsl
- shaders/lighting.hlsl
- tests/Enhanced3DScene_Phase5_test.cpp

**Functions created/changed:**
- ShadowMap::initialize
- ShadowMap::createDepthTexture
- ShadowMap::createDepthStencilView
- ShadowMap::createShaderResourceView
- ShadowMap::setupRenderState
- ShadowMap::bindForDepthPass
- ShadowMap::unbindDepthPass
- ShadowMap::getComparisonSampler
- Enhanced3DScene::createShadowResources
- Enhanced3DScene::renderShadowDepthPass
- Enhanced3DScene::render (shadow SRV + comparison sampler binding)

**Tests created/changed:**
- ShadowMap_DepthTextureCreation
- ShadowMap_DepthStencilView
- ShadowMap_ShaderResourceView
- ShadowDepthShader_Compilation
- ShadowMapping_LightSpaceMatrix
- ShadowMapping_PCFFiltering

**Review Status:** APPROVED

**Git Commit Message:**
feat: add basic shadow mapping pipeline

- implement ShadowMap depth texture, DSV, SRV, and depth-pass binding flow
- add shadowdepth shader and integrate shadow depth pass in Enhanced3DScene
- add comparison sampler setup/binding for SamplerComparisonState PCF sampling
- update lighting shader with 2x2 PCF shadow lookup path
- add Phase 5 test coverage for resource creation, shader compile, matrices, and PCF
- fix CMake target source linkage for Enhanced3DScene test dependencies
