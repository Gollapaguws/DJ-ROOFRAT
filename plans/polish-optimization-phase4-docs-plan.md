## Plan: Documentation & Developer Onboarding

Create the full developer and user documentation suite for DJ-ROOFRAT. The `docs/` directory does not yet exist; ~90% of public headers lack Doxygen coverage. This plan creates 6 documentation files, annotates 20 key headers with Doxygen comments, and updates README.md to reflect the current 41-phase feature set.

**Phases: 5 phases**

---

1. **Phase 1: docs/ Foundation — Developer Setup & Architecture**
    - **Objective:** Create `docs/` with the two highest-priority files a developer needs: a step-by-step build-from-source guide and an architecture overview explaining system layers and data flow.
    - **Files/Functions to Modify/Create:**
      - `docs/developer-setup-guide.md` (new)
      - `docs/architecture-diagram.md` (new)
    - **Tests to Write:** None (documentation phase)
    - **Steps:**
        1. Create `docs/developer-setup-guide.md`: prerequisites (VS 2022, CMake 3.20+, SQLite3 optional), clone, configure (`cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64`), build (`cmake --build build-vs --config Debug`), run (`build-vs/Debug/DJ-ROOFRAT.exe --no-audio`), optional PortAudio/libsndfile/LAME, Windows App Control note for blocked test executables, per-target test commands.
        2. Create `docs/architecture-diagram.md`: Mermaid block diagram of system layers (Input→Commands→Deck/Mixer/Effects→Analysis→Recorder/SessionManager→Visuals/CrowdAI→Gameplay), plus data-flow narrative for each layer.

2. **Phase 2: User Manual**
    - **Objective:** Write a beginner-friendly walkthrough covering getting started, all keyboard controls (60+ commands), and three hands-on workflow tutorials.
    - **Files/Functions to Modify/Create:**
      - `docs/user-manual.md` (new)
      - `docs/keyboard-reference.md` (new)
    - **Tests to Write:** None
    - **Steps:**
        1. Write getting-started section: launch options, `--no-audio` flag, HUD layout description.
        2. Create full keyboard reference table in `docs/keyboard-reference.md` (sourced from `input/InputMapper.h` 60+ InputCommand enums), organized by category: Deck A, Deck B, Effects, BPM/Sync, Recording, Session, UI toggles.
        3. Write three workflow tutorials in `user-manual.md`: "Load & Play", "Beatmatch & Crossfade", "Record & Export".
        4. Add career progression section: tiers, crowd energy, scoring system overview.

3. **Phase 3: API Reference & Data Formats**
    - **Objective:** Document 20 core classes with purpose, public interface, and usage snippets. Document JSON schemas for all persistent data types.
    - **Files/Functions to Modify/Create:**
      - `docs/api-reference.md` (new)
      - `docs/data-formats.md` (new)
    - **Tests to Write:** None
    - **Steps:**
        1. For each of 20 core classes (Deck, Mixer, AudioClip, SpectrumAnalyzer, FFTEngine, BPMDetector, EffectChain, Recorder, BeatGrid, PhaseAligner, SyncController, TransitionCoach, MixQualityAnalyzer, CrowdStateMachine, CareerProgression, SessionManager, InputMapper, WaveformRenderer, CamelotAnalyzer, EnergyCurve): write class heading, purpose summary, constructor signature, public method table, and minimal usage code snippet.
        2. Document SessionState/DeckState JSON schema: field name, C++ type, JSON type, range/default, description.
        3. Document PresetConfig schema (effect parameters, default values) and BattleState if present.

4. **Phase 4: Doxygen Comments on Top 20 Headers**
    - **Objective:** Add `/** @brief ... @param ... @return ... */` Doxygen blocks to all public constructors and methods in 20 headers. `PhaseAligner.h` and `TransitionCoach.h` already have coverage and are skipped.
    - **Files/Functions to Modify/Create:**
      - `audio/Deck.h`
      - `audio/Mixer.h`
      - `audio/AudioClip.h`
      - `audio/SpectrumAnalyzer.h`
      - `audio/BPMDetector.h`
      - `audio/Recorder.h`
      - `audio/BeatGrid.h`
      - `audio/SyncController.h`
      - `audio/CamelotAnalyzer.h`
      - `audio/EnergyAnalyzer.h`
      - `audio/FFTEngine.h`
      - `audio/TrackLoader.h`
      - `audio/EffectChain.h`
      - `gameplay/MixQualityAnalyzer.h`
      - `gameplay/EnergyCurve.h`
      - `gameplay/GameModes.h`
      - `crowdAI/CrowdStateMachine.h`
      - `core/SessionManager.h`
      - `input/InputMapper.h`
      - `visuals/WaveformRenderer.h`
    - **Tests to Write:** None
    - **Steps:**
        1. Read each header; prepend `/** @brief ... */` to class declaration.
        2. Add `/** @brief ... @param name desc @return desc */` before each public method and constructor.
        3. Follow `audio/PhaseAligner.h` style as the canonical template.
        4. Verify no compile errors after adding comments (build `dj_roofrat` target).

5. **Phase 5: Performance Tuning Guide + README Overhaul**
    - **Objective:** Write `docs/performance-tuning.md` based on the Phase 2 benchmark results. Update README.md: remove stale "Next Steps", update feature list to reflect 41 phases, add docs table of contents, fix build commands.
    - **Files/Functions to Modify/Create:**
      - `docs/performance-tuning.md` (new)
      - `README.md` (update)
    - **Tests to Write:** None
    - **Steps:**
        1. Write `docs/performance-tuning.md`: chrono-based profiling pattern, benchmark targets (FFT <20ms/1024 samples, BPM <100ms/5s clip, main loop <33ms), how to interpret spectrum output, audio frame budget math (sample rate / buffer size).
        2. Remove stale "Next Steps" section from README.
        3. Update README feature list to reflect full 41-phase implementation (crowd AI, transition coach, vinyl simulation, beat jumping, battle mode, achievements, leaderboards, autosave, etc.).
        4. Add "Documentation" section to README with a table linking to all 6 docs/ files.
        5. Update README build/run instructions to match current canonical CMake commands.

---

**Open Questions:**
1. **Doxygen HTML output:** Generate HTML docs from comments or keep pure Markdown? Recommend: Markdown-only (no extra tooling).
2. **API reference structure:** One big `api-reference.md` or per-module files? Recommend: single file for simplicity.
3. **Architecture diagram format:** Mermaid (renders on GitHub) or ASCII art? Recommend: Mermaid.
