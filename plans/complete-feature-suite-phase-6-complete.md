## Phase 6 Complete: Expanded Crowd Reactions & Beatmatch Bonus

Implemented expanded mood reaction libraries with non-repetitive selection and integrated beatmatch quality into crowd energy feedback while preserving backward compatibility for existing callers.

**Files created/changed:**
- crowdAI/CrowdStateMachine.h
- crowdAI/CrowdStateMachine.cpp
- crowdAI/CrowdStateMachine_Phase6_test.cpp
- src/main.cpp
- CMakeLists.txt

**Functions created/changed:**
- CrowdStateMachine::update(float bpm, float transitionSmoothness, float trackEnergy, float beatmatchDelta)
- CrowdStateMachine::update(float bpm, float transitionSmoothness, float trackEnergy) [backward-compatible wrapper]
- selectReaction(...) [expanded to multi-reaction round-robin behavior]

**Tests created/changed:**
- test_CrowdStateMachine_ReactionVariety
- test_CrowdStateMachine_BeatmatchBonus
- test_CrowdStateMachine_BeatmatchPenalty
- test_CrowdStateMachine_BackwardCompatibility
- test_CrowdStateMachine_EnergyClamp
- test_CrowdStateMachine_BonusVsPenalty

**Verification:**
- crowd_phase6_test: build passed
- CrowdStateMachine_Phase6_test.exe: all tests passed
- crowd_phase5_test: build passed
- CrowdStateMachine_Phase5_test.exe: all tests passed
- dj_roofrat target: build passed
- Code review: APPROVED

**Review Status:** APPROVED

**Git Commit Message:**
feat: expand crowd reactions and add beatmatch bonus

- Add 4-arg crowd update API with beatmatch delta integration
- Implement beatmatch energy bonus for tight sync and penalty for poor sync
- Expand per-mood reaction sets and use round-robin selection to reduce repetition
- Preserve backward compatibility via 3-arg update wrapper
- Wire beatmatch delta from effective deck BPMs in runtime loop
- Add dedicated Phase 6 test target and comprehensive crowd behavior tests
