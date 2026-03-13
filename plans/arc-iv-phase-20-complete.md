## Phase 20 Complete: Particle Systems & Crowd Enhancement

GPU particle system with 10K-particle ring-buffer emission, compute shader physics, and procedural instanced crowd rendering with three LOD levels and BPM-synchronized animations.

**Files created/changed:**
- visuals/ParticleSystem.h — GPU particle structure, emit/physics/render API (ring-buffer slot tracking)
- visuals/ParticleSystem.cpp — Ring-buffer emission, UpdateSubresource upload, compute shader dispatch
- visuals/ComputeShader.h — Compute shader RAII wrapper
- visuals/ComputeShader.cpp — D3D11 compute shader compile and dispatch
- visuals/CrowdMesh.h — Procedural LOD mesh generation (3 levels)
- visuals/CrowdMesh.cpp — LOD0 (121 verts), LOD1 (65 verts), LOD2 (4 verts silhouette)
- visuals/CrowdRenderer.h — InstanceData/float3 structs, per-instance LOD rendering API
- visuals/CrowdRenderer.cpp — Instanced draw loop with per-instance LOD selection
- visuals/CrowdAnimator.h — AnimationType enum, BPM-synced animation state machine
- visuals/CrowdAnimator.cpp — update(bpm, dt) driving animationTime_ at 2×beatsPerSecond
- shaders/particles.hlsl — Compute shader: gravity + wind + lifetime integration
- shaders/crowd.hlsl — Instanced VS/PS: animation displacement, instance color Phong
- visuals/Particles_Phase20_test.cpp — 9 tests (test assertion fixed: BPMSync uses 5 frames to avoid wrap)
- CMakeLists.txt — Added CrowdMesh.cpp/CrowdAnimator.cpp to all targets using CrowdRenderer.cpp; added VinylSimulator.cpp/ScratchDetector.cpp to all targets using Deck.cpp; fixed DJROOFRAT_ENABLE_GRAPHICS flags

**Functions created/changed:**
- ParticleSystem::initialize() — D3D11 structured buffer + SRV + UAV + constant buffer setup
- ParticleSystem::emitParticles() — Ring-buffer emission (cursor wraps to reuse dead slots)
- ParticleSystem::updatePhysics() — UpdateSubresource upload + compute shader dispatch
- ParticleSystem::reset() — Resets emissionCursor_ alongside activeParticleCount_
- CrowdMesh::generateLOD() — Creates GPU vertex/index buffers per LOD level
- CrowdRenderer::initialize() — Generates all LOD meshes, creates DYNAMIC instance buffer
- CrowdRenderer::renderInstanced() — Maps instance buffer, per-instance LOD-selected draw
- CrowdRenderer::calculateLODLevel() — Distance-based LOD selection (0-5m=LOD0, 5-15m=LOD1, >15m=LOD2)
- CrowdAnimator::update() — animSpeed = (bpm/60)*2; animationTime_ wraps [0,1]
- CrowdAnimator::getInstanceData() — Packages animation state into InstanceData for renderer

**Tests created/changed:**
- test_ParticleSystem_Emission — Emits 100 particles, verifies count = 100
- test_ComputeShader_Creation — Verifies ComputeShader helper creates without crash
- test_ParticleSystem_Physics — Emits particles, calls updatePhysics, checks position valid
- test_ParticleSystem_10KParticles — Emits 10000 particles, checks count = 10000, perf > 100 FPS
- test_CrowdMesh_LODGeneration — LOD0 121 verts, LOD1 65 verts, LOD2 4 verts
- test_CrowdRenderer_InstancedDraw — Adds 10 instances, getInstanceCount() = 10
- test_CrowdRenderer_LODSwitching — Near 2m = LOD0, mid 10m = LOD1, far 20m = LOD2
- test_CrowdAnimator_BPMSync — 1+5 updates at 120 BPM, animState2 > animState1 (no wrap)
- test_CrowdAnimator_IdleJumpWave — AnimationType switching, state advances per type

**Review Status:** APPROVED (3 critical bugs fixed after initial review)
- Fixed: Map() on USAGE_DEFAULT structured buffer → UpdateSubresource
- Fixed: Particle slot leak → ring-buffer emission cursor
- Fixed: Missing crowd.hlsl shader file → created with instanced animation shader
- False positive: Per-instance draw loop is correct (StartInstanceLocation=i reads unique data)

**Git Commit Message:**
```
feat: add GPU particle system and instanced crowd rendering

- ParticleSystem: ring-buffer emission, compute shader dispatch, 10K particles
- CrowdMesh: procedural LOD0/1/2 geometry (121/65/4 verts)
- CrowdRenderer: per-instance LOD selection, D3D11 instanced draw calls
- CrowdAnimator: BPM-synced animation state (Idle/Jump/Wave/Dance)
- shaders/crowd.hlsl: instanced vertex+pixel shader with animation displacement
- shaders/particles.hlsl: compute shader for GPU-side particle physics
- Fix CMake: add Phase 17/20 source deps to older test targets
```
