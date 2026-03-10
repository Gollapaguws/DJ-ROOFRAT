## Plan: Complete DJ-ROOFRAT Feature Suite

Implement professional-grade EQ/filter controls, enhanced loop/tempo workflows, tunable crowd AI with personality presets, and DirectX 11 stage visuals to deliver a fully-featured DJ simulator.

**Phases: 8 phases**

### **Phase 1: EQ/Filter Architecture Enhancement**
- **Objective:** Parameterize frequency boundaries and add dB gain conversion for professional EQ control
- **Files/Functions to Modify/Create:**
  - audio/ThreeBandEQ.h: Add frequency parameter setters, dB conversion methods
  - audio/ThreeBandEQ.cpp: Refactor fixed 220/2200 Hz to member variables
  - audio/Deck.h: Add `setEQFrequencies()` method
- **Tests to Write:**
  - test_ThreeBandEQ_FrequencyConfiguration: Verify frequency setters update crossover points
  - test_ThreeBandEQ_dBConversion: Validate linear-to-dB and dB-to-linear conversions
  - test_Deck_FrequencyPresets: Confirm deck can apply preset frequency configurations
- **Steps:**
  1. Write test scaffolding for frequency configuration and dB conversion (tests fail)
  2. Add `setLowMidCrossover()` and `setMidHighCrossover()` to `ThreeBandEQ`
  3. Add static `gainTodB()` and `dBToGain()` helper methods
  4. Update `process()` to use member variable frequencies instead of hardcoded values
  5. Run tests to confirm frequency tuning works (tests pass)
  6. Lint/format all modified files

### **Phase 2: Isolator Mode & Filter Order Upgrade**
- **Objective:** Add per-band "kill" mode and higher-order Butterworth filter option (opt-in toggle) for pro mixer behavior
- **Files/Functions to Modify/Create:**
  - audio/Deck.h: Add `setIsolatorMode()`, `isIsolatorEnabled()` per band, `filterOrder_` member
  - audio/Deck.cpp: Replace single-pole filter with configurable Butterworth (default single-pole)
  - input/InputMapper.h: Add `IsolatorLowA/B`, `IsolatorMidA/B`, `IsolatorHighA/B`, `FilterOrderToggle` commands
  - input/InputMapper.cpp: Map new keys for isolator toggles and filter order
- **Tests to Write:**
  - test_Deck_IsolatorKillBand: Verify isolator mutes specific band completely
  - test_Deck_ButterworthFilter: Validate higher-order filter steeper rolloff
  - test_InputMapper_IsolatorKeys: Confirm key mapping for new commands
- **Steps:**
  1. Write tests for isolator enable/disable and filter order (tests fail)
  2. Add boolean flags `isolatorLow_`, `isolatorMid_`, `isolatorHigh_` to `Deck`
  3. Implement Butterworth 2nd-order filter calculation in `applyFilter()`
  4. Add `filterOrder_` member to `Deck` (1=single-pole, 2=Butterworth), default 1
  5. Update `setEQ()` to check isolator flags and force 0.0 gain when enabled
  6. Run tests to verify isolator and filter upgrades (tests pass)
  7. Lint/format all files

### **Phase 3: Loop Beat Count & Tempo A Control**
- **Objective:** Allow user-configurable loop lengths (8/16/32 beats) and symmetric tempo control for both decks
- **Files/Functions to Modify/Create:**
  - audio/Deck.h: Add `setLoopBeats(int)` method, `loopBeats_` member
  - audio/Deck.cpp: Refactor loop calculation to use beat count parameter
  - input/InputMapper.h: Add `TempoAUp`, `TempoADown`, `TempoAReset`, `LoopBeatsToggle` commands
  - input/InputMapper.cpp: Map tempo A keys (e.g., z/x/c) and loop beat selector
  - src/main.cpp: Add tempo A state tracking and loop beat cycle (8→16→32→8)
- **Tests to Write:**
  - test_Deck_LoopBeatsConfiguration: Validate 8/16/32-beat loop sizing
  - test_Deck_TempoScaling: Confirm both decks apply tempo independently
  - test_InputMapper_TempoAKeys: Verify tempo A keyboard commands
- **Steps:**
  1. Write tests for loop beat count configuration and tempo A control (tests fail)
  2. Add `loopBeats_` member to `Deck`, default 16
  3. Add `setLoopBeats(int beats)` method to `Deck`
  4. Update `configurePerformanceLoop()` in main.cpp to use `deck.loopBeats_`
  5. Add tempo A input commands and state tracking in main.cpp
  6. Run tests to confirm beat count and tempo A logic (tests pass)
  7. Lint/format

### **Phase 4: Loop Quantization, Tempo Ramping & Multi-Cue**
- **Objective:** Snap loop points to beat boundaries, add smooth tempo transitions, and implement A/B/C cue hotspot banks
- **Files/Functions to Modify/Create:**
  - audio/Deck.h: Add `quantizeToNearestBeat()`, `targetTempo_`, `tempoRampRate_`, multi-cue arrays
  - audio/Deck.cpp: Implement beat-aligned loop start/end, tempo acceleration curve, cue bank switching
  - input/InputMapper.h: Add `SetCueA1/A2/A3`, `JumpCueA1/A2/A3` (mirror for B), `TempoRampToggle`
  - input/InputMapper.cpp: Map cue bank keys
  - src/main.cpp: Add tempo ramp state and cue bank selection UI
- **Tests to Write:**
  - test_Deck_LoopQuantization: Verify loop start/end snaps to beat grid
  - test_Deck_TempoRamp: Validate smooth tempo change over time (not instant)
  - test_Deck_MultiCueHotspots: Confirm 3 independent cue points per deck
- **Steps:**
  1. Write tests for quantization, tempo ramping, and multi-cue (tests fail)
  2. Add BPM-aware `quantizeFrameToBeat()` method to `Deck`
  3. Update `configureLoop()` to quantize start/end when BPM is known
  4. Add `targetTempo_` and `tempoRampRate_` members to `Deck`
  5. Update `nextFrame()` to interpolate current tempo toward target when ramping enabled
  6. Add `cuePoints_` array (3 slots) to `Deck`, update setCue/jumpToCue to use bank index
  7. Run tests to verify quantization, ramping, and multi-cue (tests pass)
  8. Lint/format

### **Phase 5: Crowd AI Personality Presets & Hysteresis**
- **Objective:** Add configurable crowd personalities (Rave/Jazz/EDM) and mood hysteresis to reduce flickering
- **Files/Functions to Modify/Create:**
  - crowdAI/CrowdStateMachine.h: Add `CrowdPersonality` enum, constructor param, hysteresis members
  - crowdAI/CrowdStateMachine.cpp: Extract weights/thresholds to personality config, implement hysteresis
  - gameplay/GameModes.h: Add crowd personality to game mode state
  - src/main.cpp: Initialize crowd with personality selection
- **Tests to Write:**
  - test_CrowdStateMachine_RavePersonality: Verify high-energy tuning (tighter BPM weight)
  - test_CrowdStateMachine_JazzPersonality: Confirm smoothness-focused tuning
  - test_CrowdStateMachine_MoodHysteresis: Validate mood doesn't flicker on threshold boundary
- **Steps:**
  1. Write tests for personality presets and hysteresis (tests fail)
  2. Create `CrowdPersonality` enum: `Rave`, `Jazz`, `EDM`, `Default`
  3. Add personality-specific weight maps (bpm/smoothness/energy ratios)
  4. Implement hysteresis: require energy to exceed threshold + 0.05 delta before mood change up, threshold - 0.03 before mood change down
  5. Add optional personality parameter to `CrowdStateMachine` constructor
  6. Run tests to verify personality behavior and hysteresis stability (tests pass)
  7. Lint/format

### **Phase 6: Expanded Crowd Reactions & Beatmatch Bonus**
- **Objective:** Add richer reaction strings (10+ per mood) and reward tight beatmatching in crowd response
- **Files/Functions to Modify/Create:**
  - crowdAI/CrowdStateMachine.cpp: Expand `selectReaction()` with larger reaction library
  - crowdAI/CrowdStateMachine.h: Add `beatmatchDelta` input to `update()`
  - src/main.cpp: Calculate BPM delta and pass beatmatch quality to crowd
- **Tests to Write:**
  - test_CrowdStateMachine_ReactionVariety: Confirm at least 3 reactions per mood level
  - test_CrowdStateMachine_BeatmatchBonus: Validate energy boost when BPM delta < 1.0
  - test_CrowdStateMachine_BeatmatchPenalty: Confirm energy decay when BPM delta > 3.0
- **Steps:**
  1. Write tests for reaction variety and beatmatch bonus (tests fail)
  2. Replace single reaction per mood with vector of reaction strings (3-5 per mood)
  3. Add randomized or alternating reaction selection (avoid repetition)
  4. Update `update()` signature to accept `beatmatchDelta` float
  5. Apply beatmatch bonus: energy += 0.05 if delta < 1.0, energy -= 0.03 if delta > 3.0
  6. Run tests to verify expanded reactions and beatmatch integration (tests pass)
  7. Lint/format

### **Phase 7: DirectX 11 Integration & Lighting Rig Prototype**
- **Objective:** Add DirectX 11 rendering loop and prototype BPM-synced lighting visualization
- **Files/Functions to Modify/Create:**
  - CMakeLists.txt: Add `option(DJROOFRAT_ENABLE_GRAPHICS ...)`, link d3d11.lib when enabled
  - Create visuals/GraphicsContext.h and visuals/GraphicsContext.cpp: D3D11 window, device, swap chain
  - Create visuals/LightingRig.h and visuals/LightingRig.cpp: BPM-synced vertical bar array
  - src/main.cpp: Integrate graphics update per audio block, preserve ASCII fallback
- **Tests to Write:**
  - test_GraphicsContext_Initialize: Verify D3D11 init succeeds or fails gracefully
  - test_LightingRig_BPMSync: Validate lighting bars pulse at correct BPM interval
  - test_GraphicsContext_Fallback: Confirm ASCII fallback when graphics unavailable
- **Steps:**
  1. Write tests for graphics initialization and lighting animation (tests fail)
  2. Add `option(DJROOFRAT_ENABLE_GRAPHICS "Enable DirectX 11 visuals" OFF)` to CMake
  3. Add `target_link_libraries(... d3d11.lib dxgi.lib)` when graphics enabled
  4. Implement `GraphicsContext`: D3D11 device, swap chain, window creation (640×480 default)
  5. Create `LightingRig` class: array of 32 vertical bars, intensity = sin(bpm_phase) × energy
  6. Update main loop: call `graphics.render(bpm, energy, mood)` every audio block if graphics enabled
  7. Run tests to verify graphics pipeline and ASCII fallback (tests pass)
  8. Lint/format

### **Phase 8: Laser & Crowd Silhouette Visuals**
- **Objective:** Add laser beam animations (crossfader-responsive) and crowd mood silhouettes for full stage effect
- **Files/Functions to Modify/Create:**
  - Create visuals/LaserController.h and visuals/LaserController.cpp: Oscillating beam rendering
  - Create visuals/CrowdRenderer.h and visuals/CrowdRenderer.cpp: Stick figure crowd mood animation
  - visuals/GraphicsContext.cpp: Integrate laser and crowd layers into composite render
- **Tests to Write:**
  - test_LaserController_CrossfaderSync: Validate laser angle follows crossfader position
  - test_CrowdRenderer_MoodAnimation: Verify crowd silhouettes animate per mood state
  - test_GraphicsContext_CompositeRender: Confirm all visual layers render together
- **Steps:**
  1. Write tests for laser animation and crowd rendering (tests fail)
  2. Implement `LaserController`: beam origin (center stage), target angle = crossfader × 45° offset
  3. Add oscillation: laser sweeps sinusoidally at BPM / 4 rate (once per 4 beats)
  4. Implement `CrowdRenderer`: draw 50-100 stick figure silhouettes, density + vertical motion vary with mood
  5. Update `GraphicsContext::render()` to composite lighting + lasers + crowd in back-to-front layers
  6. Run tests to verify laser tracking and crowd animation (tests pass)
  7. Lint/format all new files

**Open Questions (resolved):**
1. Graphics library: **DirectX 11** (Windows-native, no external deps)
2. Filter order default: **Opt-in via keyboard toggle** to preserve CPU performance
3. Multi-cue hotspots: **Yes, A/B/C banks in Phase 4**
