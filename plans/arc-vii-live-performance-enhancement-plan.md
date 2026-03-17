## Plan: Arc VII - Live Performance Enhancement (Phases 26-29)

Enable production-ready DJ workflow with session persistence, instant preset recall, professional multi-track recording, and performance analytics.

**Phases: 4 phases (3 core + 1 deferred)**

### **Phase 26: Session Auto-Save & Recovery**
- **Objective:** Automatic session state persistence with timer-based auto-save and startup recovery
- **Files/Functions to Modify/Create:**
  - src/main.cpp: Auto-save timer integration, recovery prompt, state capture/restore helpers
  - core/SessionAutoSave_Phase26_test.cpp: Comprehensive test suite
  - CMakeLists.txt: Add session_autosave_phase26_test target
- **Tests to Write:**
  - test_AutoSave_TimerTrigger: Verify save occurs at configured interval
  - test_AutoSave_StateCapture: Deck A/B, mixer, career state captured
  - test_Recovery_LoadOnStartup: autosave.json restored on startup
  - test_Recovery_MissingFile: Graceful handling when no autosave
  - test_Recovery_CorruptFile: Invalid JSON returns nullopt
  - test_AutoSave_ConfigInterval: Respects ConfigManager autosaveIntervalSeconds
  - test_StateCapture_AllFields: DeckState completeness verified
  - test_StateApply_RestoreDeckState: Loaded SessionState applied correctly
- **Steps:**
  1. Write tests for auto-save timer and recovery (tests fail)
  2. Implement captureCurrentState() helper (Deck, Mixer, Career state)
  3. Implement applySessionState() helper (restore all state)
  4. Add auto-save timer in main loop (120s interval from ConfigManager)
  5. Add startup recovery prompt checking autosave.json
  6. Run tests to verify persistence and recovery (tests pass)
  7. Manual integration test with DJ-ROOFRAT executable
  8. Lint/format

### **Phase 27: Preset Hotkey System**
- **Objective:** F1-F12 hotkeys for instant EQ preset recall during live performance
- **Files/Functions to Modify/Create:**
  - input/InputMapper.h: Add LoadPresetEQ_A_1 through LoadPresetEQ_B_12 enum values
  - input/InputMapper.cpp: F-key detection with Windows scan codes (0x3B-0x46)
  - src/main.cpp: PresetManager integration, 24 command handlers, F-key help text
  - input/PresetHotkeys_Phase27_test.cpp: Comprehensive test suite
  - CMakeLists.txt: Add preset_hotkeys_phase27_test target
- **Tests to Write:**
  - test_InputMapper_FKeyParsing: F1-F12 detection returns correct enum values
  - test_InputMapper_ShiftFKeys: Shift+F1-F12 for Deck B
  - test_PresetHotkeys_LoadEQDeckA: F1 loads A_Slot1 to Deck A EQ
  - test_PresetHotkeys_LoadEQDeckB: Shift+F1 loads B_Slot1 to Deck B EQ
  - test_PresetHotkeys_MissingPreset: Graceful handling when slot empty
  - test_PresetHotkeys_SlotNaming: A_Slot1...A_Slot12 convention enforced
  - test_PresetHotkeys_MultipleRecall: Load different presets sequentially
  - test_PresetHotkeys_LiveEQUpdate: Deck EQ reflects preset instantly
- **Steps:**
  1. Write tests for F-key detection and preset loading (tests fail)
  2. Extend InputMapper with 24 new enum values (LoadPresetEQ_A/B_1-12)
  3. Implement F-key detection using Windows scan codes (0x3B-0x46)
  4. Add Shift state checking via GetKeyState(VK_SHIFT)
  5. Add getPresetSlotName() and applyEQPreset() helpers in main.cpp
  6. Add 24 switch cases calling applyEQPreset()
  7. Update printLiveControls() with F-key mappings
  8. Run tests to verify hotkey functionality (tests pass)
  9. Manual test with live preset loading
  10. Lint/format

### **Phase 29: Enhanced Recording Features**
- **Objective:** Multi-track recording with cue markers, timestamp auto-naming, and session metadata export
- **Files/Functions to Modify/Create:**
  - src/main.cpp: 3 Recorder instances (mix, deckA, deckB), timestamp export, cue tracking
  - audio/SessionMetadata.h: Add CueSet/CueJump EventType values
  - audio/SessionMetadata.cpp: Add addCueMarker() method
  - audio/EnhancedRecording_Phase29_test.cpp: Comprehensive test suite
  - CMakeLists.txt: Add enhanced_recording_phase29_test target
- **Tests to Write:**
  - test_DualRecording_SeparateDecks: Exports deckA.wav, deckB.wav, mix.wav
  - test_DualRecording_SyncedLength: All 3 files identical frame count
  - test_Recording_AutoNaming: Timestamp format "session_mix_2026-03-17_02-46-57.wav"
  - test_Recording_CueMarkers: Cue points logged in SessionMetadata
  - test_Recording_MetadataExport: JSON contains cue timestamps
  - test_Recording_BufferCapacity: Shows remaining time "9:50 / 10:00"
  - test_Recording_PauseIndicator: Visual state shows [REC] / [PAUSED]
  - test_Recording_MultiExport: 3 WAV files + 1 JSON metadata file
- **Steps:**
  1. Write tests for multi-track recording and cue markers (tests fail)
  2. Extend SessionMetadata with CueSet/CueJump event types
  3. Implement addCueMarker(timestamp, deck, bank, frame) method
  4. Add getCurrentTimestamp() helper using std::put_time
  5. Add exportRecording() helper for multi-file export
  6. Create 3 Recorder instances in main.cpp (recorderMix, recorderDeckA, recorderDeckB)
  7. Add cue marker tracking in SetCue commands
  8. Modify SaveRecording to export 3 WAV files + JSON metadata
  9. Run tests to verify multi-track export (tests pass)
  10. Manual test with live recording session
  11. Lint/format

### **Phase 28: Performance Metrics & Analysis (DEFERRED)**
- **Objective:** BPM history tracking, beatmatch quality metrics, energy curve visualization, and session analysis reports
- **Status:** DEFERRED to v1.1 (lower priority than phases 26-27-29)
- **Reason:** Medium complexity, less immediate user value, can be delivered separately
- **Planned Features:**
  - BPM history tracking across session
  - Beatmatch quality metrics (deviation over time)
  - Energy curve visualization in terminal
  - Session analysis reports with JSON export
  - Performance improvement suggestions based on metrics

**Open Questions:**
1. Auto-save interval: 30s / 60s / 120s (default) / 300s? → **DEFAULT: 120s (2 minutes) from ConfigManager**
2. Preset slot count: 8 / 12 (default) / 24? → **DEFAULT: 12 slots per deck (F1-F12 + Shift+F1-F12)**
3. F-key detection: Windows-only scan codes / cross-platform solution? → **WINDOWS-ONLY (matches existing _kbhit/_getch pattern)**
4. Phase 28 priority: Include in Arc VII / defer to v1.1? → **DEFER to v1.1 (focus on phases 26-27-29)**
