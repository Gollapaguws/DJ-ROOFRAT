## Plan: Complete Graphics Integration

The graphics backend (DirectX 11, 40+ visual components) is fully implemented and the rendering loop is active. However, many advanced visualization features exist but are not displayed in the UI or actively rendered. This plan adds comprehensive ImGui dashboard panels and ensures all 3D visual effects are visible to the user.

**Phases: 6**

1. **Phase 1: Add Missing ImGui Visualization Panels**
    - **Objective:** Create UI panels for BeatGridRenderer, EnergyHistogram, CamelotWheel, CoachingHUD, and SyncIndicator
    - **Files/Functions to Modify/Create:**
        - [src/main.cpp](src/main.cpp) - Add panel initialization and rendering code in ImGui block
        - Utilize existing [visuals/BeatGridRenderer.h](visuals/BeatGridRenderer.h)
        - Utilize existing [visuals/EnergyHistogram.h](visuals/EnergyHistogram.h)
        - Utilize existing [visuals/CamelotWheel.h](visuals/CamelotWheel.h)
        - Utilize existing [visuals/CoachingHUD.h](visuals/CoachingHUD.h)
        - Utilize existing [visuals/SyncIndicator.h](visuals/SyncIndicator.h)
    - **Tests to Write:**
        - `test_phase1_imgui_panels.cpp` - Verify each panel initializes and renders without crash
        - `test_phase1_panel_toggles.cpp` - Verify keyboard toggles show/hide panels
    - **Steps:**
        1. Write failing tests to verify panel existence and toggle functionality
        2. Initialize BeatGridRenderer, EnergyHistogram, CamelotWheel instances in main.cpp
        3. Add ImGui windows for each visualization (toggleable with keyboard)
        4. Wire CoachingHUD and SyncIndicator data display
        5. Run tests to verify panels render and toggle correctly

2. **Phase 2: Integrate 3D Crowd Visualization**
    - **Objective:** Display CrowdRenderer, CrowdAnimator with BPM-synced animations in the 3D scene
    - **Files/Functions to Modify/Create:**
        - [visuals/GraphicsContext.cpp](visuals/GraphicsContext.cpp) `renderFrame()` - Add crowd rendering call
        - Utilize existing [visuals/CrowdRenderer.h](visuals/CrowdRenderer.h)
        - Utilize existing [visuals/CrowdAnimator.h](visuals/CrowdAnimator.h)
        - [visuals/Enhanced3DScene.cpp](visuals/Enhanced3DScene.cpp) - Integrate crowd mesh into scene
    - **Tests to Write:**
        - `test_phase2_crowd_render.cpp` - Verify crowd renders at different LODs
        - `test_phase2_crowd_animation.cpp` - Verify crowd animates to BPM changes
    - **Steps:**
        1. Write failing tests for crowd visibility and animation sync
        2. Add CrowdRenderer and CrowdAnimator initialization to GraphicsContext or Enhanced3DScene
        3. Update renderFrame() to call CrowdRenderer with BPM and mood parameters
        4. Verify crowd animations (idle/jump/wave/dance) trigger based on energy/mood
        5. Run tests to confirm crowd renders and animates properly

3. **Phase 3: Enable Stage Lighting and Laser Effects**
    - **Objective:** Ensure LightingRig and LaserController are actively rendering synchronized to BPM
    - **Files/Functions to Modify/Create:**
        - [visuals/GraphicsContext.cpp](visuals/GraphicsContext.cpp) `renderFrame()` - Ensure lighting and lasers render
        - [visuals/LightingRig.cpp](visuals/LightingRig.cpp) - Verify update() integrates with render
        - [visuals/LaserController.cpp](visuals/LaserController.cpp) - Add to render pipeline
    - **Tests to Write:**
        - `test_phase3_lighting_sync.cpp` - Verify lights flash on beat
        - `test_phase3_laser_beams.cpp` - Verify laser beams render with correct intensity
    - **Steps:**
        1. Write failing tests for lighting flash timing and laser beam visibility
        2. Confirm LightingRig.update() is called with correct BPM each frame (already at line 1825)
        3. Add LaserController rendering to GraphicsContext or Enhanced3DScene
        4. Verify lighting patterns sync to beat grid
        5. Run tests to confirm lighting and lasers are visible and beat-synced

4. **Phase 4: Add Particle System and Post-Processing Effects**
    - **Objective:** Enable ParticleSystem bursts on energy peaks and PostProcessor effects (bloom, color grading)
    - **Files/Functions to Modify/Create:**
        - [visuals/GraphicsContext.cpp](visuals/GraphicsContext.cpp) `renderFrame()` - Add particle and post-process calls
        - [visuals/ParticleSystem.cpp](visuals/ParticleSystem.cpp) - Wire to energy peaks
        - [visuals/PostProcessor.cpp](visuals/PostProcessor.cpp) - Apply bloom and color grading
    - **Tests to Write:**
        - `test_phase4_particles.cpp` - Verify particles emit on energy threshold
        - `test_phase4_postprocess.cpp` - Verify bloom/color effects apply correctly
    - **Steps:**
        1. Write failing tests for particle emission and post-process effect visibility
        2. Add ParticleSystem initialization and update/render to GraphicsContext
        3. Trigger particle bursts when crowdOut.energyMeter crosses thresholds
        4. Integrate PostProcessor bloom and color grading into render pipeline
        5. Run tests to confirm particles and effects are visible

5. **Phase 5: Add Performance and Career UI Panels**
    - **Objective:** Create ImGui panels for FPS/latency metrics, career progression, achievements, and library browser
    - **Files/Functions to Modify/Create:**
        - [src/main.cpp](src/main.cpp) - Add PerformancePanel, CareerPanel, LibraryPanel ImGui windows
        - Utilize [gameplay/CareerProgression.h](gameplay/CareerProgression.h)
        - Utilize [gameplay/AchievementSystem.h](gameplay/AchievementSystem.h)
        - Utilize [library/TrackBrowser.h](library/TrackBrowser.h)
    - **Tests to Write:**
        - `test_phase5_performance_panel.cpp` - Verify FPS, frame time, latency display
        - `test_phase5_career_panel.cpp` - Verify tier, reputation, unlocks display
        - `test_phase5_library_panel.cpp` - Verify track list browsing
    - **Steps:**
        1. Write failing tests for panel data accuracy
        2. Create PerformancePanel showing FPS, frame time, audio latency, block count
        3. Create CareerPanel showing tier, reputation, unlocks, achievements
        4. Create LibraryPanel with track list from TrackBrowser (read-only for now)
        5. Run tests to verify all data displays correctly

6. **Phase 6: Add Graphics Settings and Hotkey Toggles**
    - **Objective:** Implement UI for graphics quality settings and keyboard shortcuts to toggle 3D effects
    - **Files/Functions to Modify/Create:**
        - [src/main.cpp](src/main.cpp) - Add SettingsPanel and hotkey handlers
        - [visuals/GraphicsContext.h](visuals/GraphicsContext.h) - Add enable/disable methods for effects
        - [input/InputCommand.h](input/InputCommand.h) - Add toggle commands
    - **Tests to Write:**
        - `test_phase6_settings_panel.cpp` - Verify quality settings apply
        - `test_phase6_hotkey_toggles.cpp` - Verify keyboard shortcuts toggle effects
    - **Steps:**
        1. Write failing tests for settings application and hotkey response
        2. Add SettingsPanel with checkboxes/sliders for crowd, lasers, particles, shadows, post-processing
        3. Define InputCommands for toggling 3D effects (e.g., ToggleCrowd, ToggleLasers)
        4. Wire hotkeys to enable/disable rendering of each effect
        5. Run tests to confirm settings and hotkeys work correctly

**Open Questions:**
1. Should the 3D scene (stage, crowd, lasers) be visible by default or require user activation?
2. Should there be a camera control system for users to fly around the 3D environment, or fixed cinematic angles?
3. Should waveform and spectrum visualizations also render in 3D space on virtual "screens" in the venue?
4. Should the library panel support drag-and-drop track loading onto decks?
5. Should post-processing effects (bloom, color grading) be adjustable via UI sliders or preset-based?
