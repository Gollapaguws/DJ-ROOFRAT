## Plan: Arc IX - Performance Optimization & UI Enhancement

Integrate Arc VIII analysis features into live DJ workflow with real-time visualization and interactive controls. Replace naive O(N²) DFT with Cooley-Tukey FFT for 60 FPS spectrum display, add beat grid nudge editor, energy curve visualization, and performance metrics overlay.

**Phases (4 phases, 32 tests total)**

1. **Phase 34: Cooley-Tukey FFT Implementation**
    - **Objective:** Replace naive DFT with fast O(N log N) FFT for 50-100× performance improvement
    - **Files/Functions to Modify/Create:**
        - [audio/SpectrumAnalyzer.cpp](audio/SpectrumAnalyzer.cpp): Replace `computeFFT()` with Cooley-Tukey radix-2 algorithm
        - [audio/FFTEngine.h](audio/FFTEngine.h) (NEW): Standalone FFT engine with Hann/Hamming/Blackman windows
        - [audio/FFTEngine.cpp](audio/FFTEngine.cpp) (NEW): Radix-2 decimation-in-time implementation
        - [audio/FFT_Phase34_test.cpp](audio/FFT_Phase34_test.cpp) (NEW): Comprehensive FFT correctness tests
    - **Tests to Write:**
        - test_FFT_PowerOfTwo (128, 256, 512, 1024 samples)
        - test_FFT_MatchesNaiveDFT (verify correctness against known implementation)
        - test_FFT_WindowFunctions (Hann, Hamming, Blackman spectral leakage)
        - test_FFT_Performance (measure speedup: target 50× faster than naive DFT)
        - test_FFT_DCComponent (zero-frequency bin handling)
        - test_FFT_NyquistFrequency (fs/2 boundary condition)
        - test_FFT_PhaseAccuracy (preserve phase information)
        - test_SpectrumAnalyzer_FFTIntegration (drop-in replacement for naive DFT)
    - **Steps:**
        1. Write all 8 tests against naive DFT baseline (establish correctness criteria)
        2. Implement FFTEngine.h with radix-2 Cooley-Tukey algorithm
        3. Add bit-reversal permutation for input reordering
        4. Implement iterative butterfly operations (log N stages)
        5. Add window function support (Hann, Hamming, Blackman)
        6. Integrate into SpectrumAnalyzer::computeFFT() with feature flag
        7. Run tests - verify correctness matches naive DFT within 0.1% tolerance
        8. Benchmark performance - confirm 50-100× speedup for N=1024

2. **Phase 35: Real-time Spectrum Display Integration**
    - **Objective:** Add keyboard-toggleable spectrum visualization overlay with 60 FPS refresh rate
    - **Files/Functions to Modify/Create:**
        - [src/main.cpp](src/main.cpp): Add spectrum display mode (S key toggle), render loop integration
        - [visuals/SpectrumDisplay.h](visuals/SpectrumDisplay.h) (NEW): Layered terminal UI (waveform + spectrum bars)
        - [visuals/SpectrumDisplay.cpp](visuals/SpectrumDisplay.cpp) (NEW): Multi-deck spectrum comparison view
        - [audio/SpectrumAnalyzer.cpp](audio/SpectrumAnalyzer.cpp): Add real-time mode with double-buffering
        - [visuals/SpectrumDisplay_Phase35_test.cpp](visuals/SpectrumDisplay_Phase35_test.cpp) (NEW): UI rendering tests
    - **Tests to Write:**
        - test_SpectrumDisplay_ToggleVisibility (S key shows/hides spectrum)
        - test_SpectrumDisplay_DualDeckMode (deck A left, deck B right split-screen)
        - test_SpectrumDisplay_HistoryTrace (fade out previous frames for motion effect)
        - test_SpectrumDisplay_PeakHold (hold peak values for 500ms)
        - test_SpectrumDisplay_ColorCoding (bass=red, mid=green, treble=blue ASCII colors)
        - test_SpectrumDisplay_60FPSUpdate (verify frame timing <= 16.7ms)
        - test_SpectrumDisplay_ThreadSafety (audio thread writes, render thread reads)
        - test_SpectrumAnalyzer_RealTimeMode (double-buffer swap without blocking)
    - **Steps:**
        1. Write all 8 tests for UI rendering and real-time constraints
        2. Create SpectrumDisplay class with terminal split-screen layout
        3. Add double-buffering to SpectrumAnalyzer (swap buffers atomically)
        4. Integrate into main.cpp render loop (current FPS: ~120 Hz, target: 60 Hz stable)
        5. Add S key toggle for spectrum overlay (default: OFF)
        6. Implement peak hold and history trace for visual feedback
        7. Add ASCII color coding for frequency bands (red/green/blue)
        8. Benchmark frame timing - ensure <= 16.7ms per frame (60 FPS)

3. **Phase 36: Beat Grid Nudge Editor**
    - **Objective:** Interactive beat grid alignment with J/K keys for ±10ms nudge, visual beat markers on waveform
    - **Files/Functions to Modify/Create:**
        - [src/main.cpp](src/main.cpp): Add J/K key handlers for beat nudging, Shift+J/K for first-beat offset
        - [visuals/BeatGridOverlay.h](visuals/BeatGridOverlay.h) (NEW): Waveform + beat marker renderer
        - [visuals/BeatGridOverlay.cpp](visuals/BeatGridOverlay.cpp) (NEW): Animated nudge feedback (highlight changed beat)
        - [audio/BeatGrid.cpp](audio/BeatGrid.cpp): Add getCurrentBeatIndex(), getNextBeatTimestamp() for live tracking
        - [audio/Deck.cpp](audio/Deck.cpp): Link deck playback position to beat grid overlay
        - [visuals/BeatGridOverlay_Phase36_test.cpp](visuals/BeatGridOverlay_Phase36_test.cpp) (NEW): Interactive editor tests
    - **Tests to Write:**
        - test_BeatGridEditor_NudgeBeat (J/K keys shift current beat by ±10ms)
        - test_BeatGridEditor_NudgeFirstBeat (Shift+J/K adjust global offset)
        - test_BeatGridEditor_LiveTracking (highlight current beat during playback)
        - test_BeatGridEditor_VisualFeedback (flash nudged beat marker for 1 second)
        - test_BeatGridEditor_PersistChanges (save nudges to TrackMetadata)
        - test_BeatGridEditor_UndoRedo (Ctrl+Z, Ctrl+Y for nudge history)
        - test_BeatGridOverlay_Rendering (ASCII | markers at beat positions)
        - test_BeatGrid_CurrentBeatIndex (track playback position → beat index mapping)
    - **Steps:**
        1. Write all 8 tests for interactive beat grid editing
        2. Extend BeatGrid with getCurrentBeatIndex(double timestamp)
        3. Create BeatGridOverlay for waveform + beat marker rendering
        4. Add J/K key handlers in main.cpp (nudge current beat ±10ms)
        5. Add Shift+J/K handlers (nudge first beat offset)
        6. Implement visual feedback (highlight nudged beat for 1s)
        7. Add undo/redo stack for nudge operations (Ctrl+Z, Ctrl+Y)
        8. Integrate with Deck playback (live beat tracking indicator)

4. **Phase 37: Energy Curve & Performance Metrics Display**
    - **Objective:** Visualize track energy rating, set energy flow curve, and live performance metrics (mix quality score)
    - **Files/Functions to Modify/Create:**
        - [src/main.cpp](src/main.cpp): Add energy display mode (E key toggle), performance metrics HUD
        - [visuals/EnergyCurveDisplay.h](visuals/EnergyCurveDisplay.h) (NEW): ASCII line chart for set energy progression
        - [visuals/EnergyCurveDisplay.cpp](visuals/EnergyCurveDisplay.cpp) (NEW): Real-time energy curve with 30-minute history
        - [visuals/PerformanceMetrics.h](visuals/PerformanceMetrics.h) (NEW): Mix quality scoring (timing, EQ, transitions)
        - [visuals/PerformanceMetrics.cpp](visuals/PerformanceMetrics.cpp) (NEW): Calculate score based on crowd response model
        - [gameplay/CrowdAI.cpp](gameplay/CrowdAI.cpp): Expose getEnergyLevel(), getTransitionScore() for metrics
        - [visuals/EnergyDisplay_Phase37_test.cpp](visuals/EnergyDisplay_Phase37_test.cpp) (NEW): Energy visualization tests
    - **Tests to Write:**
        - test_EnergyCurveDisplay_RealTimeUpdates (energy curve updates every 10 seconds)
        - test_EnergyCurveDisplay_30MinHistory (retain last 1800 samples @ 1 sample/10s)
        - test_EnergyCurveDisplay_EnergyPeaks (mark peak hour sections in curve)
        - test_PerformanceMetrics_MixQualityScore (calculate 0-100 score from transitions)
        - test_PerformanceMetrics_TimingAccuracy (penalize off-beat transitions)
        - test_PerformanceMetrics_FrequencyClash (detect overlapping bass frequencies)
        - test_PerformanceMetrics_KeyCompatibility (bonus for Camelot-compatible mixes)
        - test_EnergyCurveDisplay_ToggleVisibility (E key shows/hides energy curve)
    - **Steps:**
        1. Write all 8 tests for energy visualization and performance scoring
        2. Create EnergyCurveDisplay with ASCII line chart (30-minute history buffer)
        3. Implement PerformanceMetrics scoring system (0-100 scale)
        4. Add timing accuracy detection (δt from beat grid)
        5. Add frequency clash detection (simultaneous bass peaks)
        6. Add Camelot compatibility bonus (Arc VIII-31 integration)
        7. Integrate CrowdAI energy level feedback into curve display
        8. Add E key toggle in main.cpp (default: OFF)

---

## Open Questions (1-5 questions, ~5-25 words each)

1. **FFT radix support?** Radix-2 only (powers of 2: 512, 1024, 2048) / Radix-4 / Mixed-radix (support arbitrary sizes)
2. **Spectrum display layout?** Horizontal bars (current Arc VIII design) / Vertical bars (traditional DJ software) / Both with toggle
3. **Beat grid undo stack size?** 10 operations / 50 operations / Unlimited with memory limit
4. **Energy curve sampling rate?** 10 seconds/sample (180 samples for 30 min) / 30 seconds/sample (60 samples) / User configurable
5. **Performance metrics visibility?** Always-on HUD overlay / Toggle with hotkey / End-of-set summary only

---

## Implementation Strategy

### Performance Targets
- **Phase 34 FFT**: 50-100× speedup over naive DFT (target: <1ms for N=1024 @ 44.1kHz)
- **Phase 35 Spectrum**: 60 FPS stable (16.7ms frame budget, FFT + render + display)
- **Phase 36 Beat Grid**: <5ms nudge latency (instant visual feedback)
- **Phase 37 Energy Curve**: <100µs update cost (negligible overhead)

### Testing Strategy
- **Phase 34**: Correctness first (match naive DFT within 0.1%), then performance benchmarks
- **Phase 35**: Real-time constraints (timing tests, thread safety, no dropped frames)
- **Phase 36**: User interaction (keyboard input → visual feedback → persistence)
- **Phase 37**: Algorithmic accuracy (energy curve smoothing, metrics calculation)

### Integration Approach
- **Backward compatible**: Naive DFT remains available as fallback (compile-time flag)
- **Progressive enhancement**: Each phase independently useful (spectrum → beat grid → energy curve)
- **Minimal disruption**: All changes behind keyboard toggles (default: current UI)
- **Zero external dependencies**: Pure C++20 std::complex<double>, <cmath>, <chrono>

### Code Architecture
- **FFTEngine**: Standalone, reusable FFT implementation (used by SpectrumAnalyzer, future phases)
- **Display classes**: Separate concerns (SpectrumDisplay, BeatGridOverlay, EnergyCurveDisplay)
- **main.cpp integration**: Keyboard toggles, render loop hooks, minimal coupling
- **Thread safety**: Double-buffering for audio → render data flow

---

## Arc IX Scope Summary

| Metric | Value |
|--------|-------|
| **Phases** | 4 (34-37) |
| **Tests** | 32 (8 per phase) |
| **New Files** | ~16 (headers + implementations + tests) |
| **Modified Files** | 4 (SpectrumAnalyzer.cpp, Deck.cpp, BeatGrid.cpp, main.cpp) |
| **Est. Lines of Code** | ~4,000 (implementation + tests) |
| **Performance Gain** | 50-100× FFT speedup, 60 FPS UI |
| **User Value** | Industry-standard spectrum + interactive controls |

---

## Dependencies & Risks

### Dependencies
- **Arc VIII complete** ✅ (SpectrumAnalyzer, BeatGrid, EnergyAnalyzer, Camelot all exist)
- **Current UI framework** ✅ (terminal rendering with ANSI codes already working)
- **CrowdAI system** ✅ (exists from Arc V gameplay)

### Risks & Mitigations
1. **FFT complexity** 🟡 Medium risk
   - *Mitigation*: Test against naive DFT baseline, extensive unit tests for bit-reversal and butterfly ops
2. **60 FPS stability** 🟡 Medium risk
   - *Mitigation*: Profile render loop, use double-buffering, fallback to 30 FPS if needed
3. **Thread safety** 🟢 Low risk
   - *Mitigation*: Audio thread writes, render thread reads, atomic buffer swap
4. **UI complexity in terminal** 🟡 Medium risk
   - *Mitigation*: Start simple (ASCII only), progressive enhancement, keyboard toggles

---

## Alternatives Considered

### Alternative 1: Loop Sampler Arc (Arc IX-3)
- **Pros**: Creative performance tool, no dependencies, medium complexity
- **Cons**: Doesn't integrate Arc VIII, doesn't remove FFT bottleneck
- **Decision**: Consider for Arc X (can develop in parallel)

### Alternative 2: Advanced Dynamics Arc (Arc IX-2)
- **Pros**: Professional mixing features (sidechain, multiband compression)
- **Cons**: High complexity, requires DSP expertise, less immediate user value
- **Decision**: Defer to Arc XI after FFT and UI foundation

### Alternative 3: Auto-Sync & Coaching Arc (Arc IX-4)
- **Pros**: Beginner-friendly, AI-powered recommendations
- **Cons**: Depends on FFT for phase detection (Arc IX-1 prerequisite)
- **Decision**: Natural follow-up to Arc IX-1 (consider for Arc X)

---

## Success Criteria

**Phase 34 Success:**
- ✅ All 8 FFT tests passing (correctness, performance, integration)
- ✅ 50× minimum speedup over naive DFT for N=1024
- ✅ SpectrumAnalyzer maintains API compatibility (drop-in FFT replacement)

**Phase 35 Success:**
- ✅ 60 FPS spectrum display with S key toggle
- ✅ Dual-deck comparison view (deck A vs deck B split-screen)
- ✅ Frame timing stable <= 16.7ms (no dropped frames)

**Phase 36 Success:**
- ✅ Interactive beat nudging with J/K keys (±10ms precision)
- ✅ Visual feedback (highlight nudged beat, undo/redo support)
- ✅ Changes persist to TrackMetadata.beatGrid

**Phase 37 Success:**
- ✅ Energy curve displays 30-minute set history
- ✅ Performance metrics calculate mix quality score (0-100)
- ✅ E key toggles energy curve overlay

**Arc IX Success:**
- ✅ 32/32 tests passing
- ✅ FFT enables real-time spectrum (60 FPS)
- ✅ Arc VIII features fully integrated into live workflow
- ✅ Performance metrics provide actionable feedback to DJs

---

## Post-Arc IX Roadmap

**Arc X Candidates (ranked by priority):**
1. **Auto-Sync & Coaching** - Phase detection, auto-beatmatching, transition suggestions (depends on FFT from Arc IX-1)
2. **Loop Sampler** - 4-bank loop capture, BPM-sync playback, instant repeat (independent, can start now)
3. **Advanced Dynamics** - Sidechain compression, multiband compression, frequency-selective FX (professional mixing)
4. **Web Remote Control** - Browser-based DJ controller, mobile app integration (accessibility)
5. **VST/VST3 Plugin Support** - Third-party effects integration (extensibility)
