## Plan: Polish & Optimization Phase

Following 41 feature-complete phases, focus shifts to integration testing, performance profiling, critical bug fixes, and developer/user documentation to achieve production-ready quality.

**Phases: 4 phases**

---

## Phase 1: Integration Testing & Smoke Tests
**Objective:** Add end-to-end integration tests validating full workflows (load track → mix → record → save session) and main application smoke test

**Files/Functions to Modify/Create:**
- `tests/integration/` (new directory)
- `tests/integration/SmokeTest_main.cpp` (new)
- `tests/integration/E2E_LoadMixRecord_test.cpp` (new)
- `tests/integration/E2E_MIDIEffects_test.cpp` (new)
- `tests/integration/E2E_SessionRoundTrip_test.cpp` (new)
- `CMakeLists.txt` (add integration test targets)

**Tests to Write:**
- `test_SmokeTest_AppStartup` - Main loop initializes without crashes
- `test_SmokeTest_TestTonePlayback` - Audio pipeline generates output
- `test_SmokeTest_KeyboardInput` - Input commands parsed and executed
- `test_E2E_LoadMixRecord_FullWorkflow` - Load MP3 → crossfade → record → export WAV
- `test_E2E_MIDIEffects_Pipeline` - MIDI CC → effect chain → audio output
- `test_E2E_SessionRoundTrip_SaveLoad` - Save session → restart → load → verify state
- `test_E2E_AutoSync_VisualIndicators` - Enable sync → verify HUD renders correctly
- `test_E2E_BattleMode_StateTransitions` - Start battle → play turn → submit score → declare winner

**Steps:**
1. Create `tests/integration/` directory structure
2. Write smoke test for main application startup (initialize all systems, no crashes)
3. Implement E2E workflow test: load track from file → play → crossfade → record → verify output file
4. Add MIDI integration test: simulate MIDI CC messages → verify effect chain applies correctly
5. Create session persistence round-trip test: save session state → reload → compare all fields
6. Add auto-sync visual integration test: trigger sync → verify CoachingHUD/SyncIndicator render
7. Implement battle mode state machine test: full battle workflow from idle to winner announcement
8. Update CMakeLists.txt with integration test targets and run all tests to confirm PASS

---

## Phase 2: Performance Profiling & Hotspot Optimization
**Objective:** Profile actual runtime performance, identify top 3 bottlenecks, and optimize critical paths (target: FFT <20ms, BPM detection <100ms)

**Files/Functions to Modify/Create:**
- `docs/performance-profiling-guide.md` (new - profiling instructions)
- `tests/benchmarks/` (new directory)
- `tests/benchmarks/Benchmark_SpectrumAnalyzer.cpp` (new)
- `tests/benchmarks/Benchmark_BPMDetector.cpp` (new)
- `audio/SpectrumAnalyzer.cpp` (optimize DFT → FFT algorithm)
- `audio/BPMDetector.cpp` (optimize autocorrelation)
- `audio/EnhancedRecording_Phase29_test.cpp` (fix vector allocations in loop)

**Tests to Write:**
- `benchmark_SpectrumAnalyzer_1024Samples` - Measure DFT performance with chrono timing
- `benchmark_BPMDetector_5SecondClip` - Measure BPM detection latency
- `benchmark_BeatGridGeneration_OnsetDetection` - Measure spectral flux performance
- `benchmark_MainLoop_FrameTime` - Measure main loop iteration time (target <16ms for 60 FPS)
- `test_SpectrumAnalyzer_FFT_Correctness` - Verify FFT optimization produces identical results
- `test_BPMDetector_OptimizedAccuracy` - Verify optimized BPM detection maintains ±0.5 BPM accuracy
- `test_Recording_NoAllocationsInLoop` - Verify no heap allocations per audio frame
- `test_BeatGridEditor_MutexContention` - Measure lock contention under concurrent access

**Steps:**
1. Create `tests/benchmarks/` and add chrono-based timing harness
2. Benchmark current SpectrumAnalyzer (measure baseline: expected ~67ms/1024 samples)
3. Profile BPMDetector (measure baseline: expected ~200ms/5s clip)
4. Identify heap allocations in audio frame loop (EnhancedRecording_Phase29_test.cpp line 121-142)
5. Optimize: Replace naive DFT with FFT algorithm (target <20ms for 1024 samples, 3× speedup)
6. Optimize: Cache envelope data in BPMDetector to avoid recomputation
7. Fix: Pre-allocate recording buffers outside loop to eliminate per-frame malloc
8. Re-benchmark optimized code and document results in performance-profiling-guide.md

---

## Phase 3: Critical Bug Fixes & Integration Gaps
**Objective:** Resolve spectrum analyzer integration gap, complete JSON session import, and fix Windows security-blocked tests

**Files/Functions to Modify/Create:**
- `src/main.cpp` (integrate spectrum analyzer for bass clash detection)
- `audio/Deck.h` (add getSpectrumAnalyzer() method)
- `audio/Deck.cpp` (expose SpectrumAnalyzer instance)
- `core/SessionManager.cpp` (complete JSON import implementation)
- `audio/BeatGrid_Phase32_test.cpp` (investigate Windows security test blocks)
- `audio/EnhancedRecording.cpp` (add file naming UI function stub)

**Tests to Write:**
- `test_Deck_GetSpectrumAnalyzer` - Verify Deck exposes analyzer instance
- `test_MainLoop_BassClashDetection` - Verify warning appears when EQ low+low conflict
- `test_SessionManager_ImportRoundTrip` - Save → import → verify all fields restored
- `test_SessionManager_MalformedJSON` - Import invalid JSON → graceful error handling
- `test_BeatGrid_WindowsSecurity_Workaround` - Attempt to run blocked tests with admin rights
- `test_Recording_CustomFilename` - Verify recorder accepts user filename parameter
- `test_Recording_FilenameCollision` - Verify auto-increment on duplicate names

**Steps:**
1. Add `SpectrumAnalyzer* getSpectrumAnalyzer()` to Deck class
2. Integrate spectrum analyzer in main.cpp bass clash detection (currently disabled at line 1740)
3. Complete SessionManager::loadSession() JSON deserialization (currently partial)
4. Add error handling for malformed JSON (return std::nullopt with error message)
5. Investigate Windows security policy blocking BeatGrid tests (check file permissions/antivirus)
6. Add setOutputFilename() method to Recorder class for user-specified names
7. Implement auto-increment logic (e.g., recording_001.wav, recording_002.wav on collision)
8. Run all tests to confirm fixes don't introduce regressions

---

## Phase 4: Documentation & Developer Onboarding
**Objective:** Create API documentation, architecture diagram, user manual, and developer setup guide for external contributors and end users

**Files/Functions to Modify/Create:**
- `docs/api-reference.md` (new - Doxygen-style API docs)
- `docs/architecture-diagram.md` (new - system component diagram)
- `docs/user-manual.md` (new - beginner-friendly guide)
- `docs/developer-setup-guide.md` (new - build from source instructions)
- `docs/performance-tuning.md` (new - profiling and optimization guide)
- `docs/data-formats.md` (new - JSON schemas for sessions/presets/leaderboards)
- `audio/*.h` (add Doxygen /** */ comments to public APIs)
- `README.md` (update with links to new documentation)

**Tests to Write:**
- (No new tests - documentation phase)

**Steps:**
1. Generate API reference: Document all public classes/methods in audio/, gameplay/, visuals/, core/
2. Create architecture diagram: Show system layers (audio → mixer → effects → renderer → output)
3. Write user manual: Getting started, keyboard controls, workflow tutorials (load track, beatmatch, record)
4. Write developer setup guide: CMake configuration, vcpkg dependencies, Visual Studio setup
5. Document performance tuning: How to profile with chrono, optimize audio paths, interpret FFT benchmarks
6. Create JSON schema specs: Document SessionState, PresetConfig, BattleState data formats
7. Add Doxygen comments to 20 most-used classes (Deck, Mixer, AudioClip, CrowdStateMachine, etc.)
8. Update README with table of contents linking to new docs/ files

---

## Open Questions

1. **Integration test scope:** Should we test every arc combination, or focus on critical workflows (load/mix/record)?
   - Option A: Full matrix (MIDI × Effects × Graphics × Recording) = ~50+ tests
   - Option B: Critical flows only (load/play, record, autosync) = ~8-10 tests
   - **Recommendation:** Option B for v1.0, Option A for future releases

2. **Performance targets:** What frame time budget should we enforce?
   - Option A: 60 FPS (16.67ms/frame) - smooth visuals
   - Option B: 30 FPS (33.33ms/frame) - acceptable for terminal UI
   - **Recommendation:** Option B (30 FPS) since DirectX visuals are optional

3. **FFT optimization approach:** Which algorithm for spectrum analyzer?
   - Option A: Cooley-Tukey FFT (O(N log N), complex but industry standard)
   - Option B: Radix-2 FFT (simpler, requires power-of-2 samples)
   - Option C: FFTW library integration (fastest, external dependency)
   - **Recommendation:** Option B (Radix-2 FFT) for balance of speed and simplicity

4. **Windows-blocked tests resolution:** How to handle security policy issues?
   - Option A: Require admin rights to run tests (friction for CI/CD)
   - Option B: Mark tests as skipped with clear error message
   - Option C: Mock file system operations to bypass security checks
   - **Recommendation:** Option B for now, Option C if it becomes blocker

5. **Documentation format:** What level of detail for API docs?
   - Option A: Full Doxygen generation with HTML output (comprehensive, time-intensive)
   - Option B: Markdown comments in code + simple docs/ folder (lightweight, maintainable)
   - **Recommendation:** Option B (Markdown), upgrade to Doxygen if community requests

---

## Success Criteria

**Phase 1 Complete When:**
- ✅ 8 integration tests passing (smoke test + 7 E2E workflows)
- ✅ Main application startup validated (no crashes)
- ✅ Full workflow tested: load MP3 → mix → record → save session → reload

**Phase 2 Complete When:**
- ✅ Performance benchmarks established for top 3 hotspots
- ✅ SpectrumAnalyzer optimized to <20ms/frame (3× speedup from 67ms baseline)
- ✅ BPMDetector optimized to <100ms for 5-second clip
- ✅ No heap allocations in audio frame loop (verified with profiling tools)

**Phase 3 Complete When:**
- ✅ Spectrum analyzer integrated in main.cpp (bass clash detection functional)
- ✅ SessionManager JSON import complete (round-trip save/load verified)
- ✅ Recorder supports custom filenames with collision handling
- ✅ All previously passing tests still pass (no regressions)

**Phase 4 Complete When:**
- ✅ docs/ folder contains 6 new documentation files
- ✅ API reference covers 20+ core classes with usage examples
- ✅ Architecture diagram shows system component relationships
- ✅ User manual provides beginner-friendly walkthrough
- ✅ Developer setup guide enables new contributor to build from source in <30 minutes

---

## Estimated Timeline

- **Phase 1 (Integration Testing):** 3-4 days (8 test files, ~1,200 lines)
- **Phase 2 (Performance Optimization):** 4-5 days (profiling + FFT algorithm + benchmarks)
- **Phase 3 (Critical Fixes):** 2-3 days (spectrum integration + JSON import + recorder naming)
- **Phase 4 (Documentation):** 3-4 days (6 docs + Doxygen comments + diagrams)

**Total:** 12-16 days for production-ready polish

---

## Dependencies

- No external dependencies on unfinished features
- Phase 2 benefits from Phase 1 (integration tests reveal real-world hotspots)
- Phase 3 can run parallel to Phase 2 (independent bug fixes)
- Phase 4 should come last (documents finalized system state)

---

## Risk Assessment

**Low Risk:**
- Integration testing (additive, doesn't modify existing code)
- Documentation (no code changes, pure writing)

**Medium Risk:**
- FFT optimization (algorithm swap could introduce frequency domain errors - mitigate with correctness tests)
- JSON import completion (deserialization bugs could corrupt sessions - mitigate with malformed input tests)

**High Risk:**
- Spectrum analyzer integration in main.cpp (touches 1,800-line event loop - mitigate with smoke tests before merging)