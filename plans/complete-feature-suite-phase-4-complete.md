## Phase 4 Complete: Loop Quantization, Tempo Ramping & Multi-Cue

Implemented beat-quantized loop boundaries, tempo ramp controls, and 3-bank cue hotspots per deck, then hardened behavior with a targeted fix for ramp/manual nudge interaction and stronger regression tests.

**Files created/changed:**
- audio/Deck.h
- audio/Deck.cpp
- input/InputMapper.h
- input/InputMapper.cpp
- src/main.cpp
- audio/Deck_Phase4_test.cpp
- input/InputMapper_Phase4_test.cpp
- CMakeLists.txt

**Functions created/changed:**
- Deck::configureLoop(std::size_t, std::size_t, bool, float)
- Deck::quantizeFrameToBeat(std::size_t, float) const
- Deck::setTempoRampEnabled(bool)
- Deck::setTargetTempo(float)
- Deck::setTempoRampRate(float)
- Deck::setCue(std::size_t, int)
- Deck::jumpToCue(int)
- Deck::setActiveCueBank(int)
- Deck::setTempoPercent(float) [updated to preserve manual nudges while ramping]
- InputMapper::parseKey(char) [cue bank and tempo ramp mappings]
- src/main.cpp command switch [tempo ramp toggle and multi-cue command handling]

**Tests created/changed:**
- test_Deck_LoopQuantization_SnapToNearest
- test_Deck_LoopQuantization_BoundaryAlignment
- test_Deck_TempoRamp_SmoothInterpolation
- test_Deck_TempoRamp_BoundaryRespect
- test_Deck_TempoRamp_DisableAndEnable
- test_Deck_TempoRamp_ManualNudgePreserved
- test_Deck_MultiCueHotspots_IndependentBanks
- test_Deck_MultiCueHotspots_BankBoundaries
- test_Deck_MultiCueHotspots_DefaultState
- test_InputMapper_CueASetKeys
- test_InputMapper_CueAJumpKeys
- test_InputMapper_CueBSetKeys
- test_InputMapper_TempoRampToggleKey
- test_InputMapper_ParseStringCommands

**Review Status:** APPROVED

**Git Commit Message:**
feat: add loop quantization tempo ramp and multi-cue banks

- Add BPM-based loop quantization and tempo ramp controls to Deck
- Implement three-bank cue hotspots per deck with dedicated key mappings
- Wire phase-4 commands in main runtime loop and add test targets in CMake
- Fix tempo ramp/manual nudge conflict by syncing ramp target on direct tempo set
- Strengthen phase-4 tests with explicit cue destination and clamp assertions
