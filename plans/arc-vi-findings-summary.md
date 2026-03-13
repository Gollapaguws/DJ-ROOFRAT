# Arc VI - Quick Reference Findings

## Phase 24: Local Multiplayer - Split-Screen vs Terminal

### Split-Screen Graphics (Current Plan)
- **Complexity:** Medium-High (500-800 lines, DirectX 11 viewport/camera refactoring)
- **Timeline:** 2+ weeks
- **GraphicsContext changes:** Dual viewports, dual cameras, multi-pass rendering
- **Current state:** GraphicsContext supports only 1 viewport, 1 camera, 1 render target
- **Risk:** Moderate (refactoring complexity, potential regressions)

### Terminal-Only Battles (RECOMMENDED)
- **Complexity:** Low-Medium (400-500 lines)
- **Timeline:** 1 week
- **Rendering:** ASCII waveforms via existing WaveformRenderer
- **Changes needed:** Zero graphics modifications
- **Risk:** Low
- **Scope reduction:** Yes, but maintains gameplay value

**Recommendation:** Implement Phase 24a as terminal hotseat battles, defer split-screen to Phase 24b or later.

---

## Phase 25: Configuration Management

### Manual JSON (RECOMMENDED)
- **Pattern source:** Arc V (ProgressTracker, Leaderboard, AchievementSystem)
- **Effort:** Same as external library
- **Dependencies:** Zero (matches project philosophy)
- **Advantages:** Consistent with Arc V, proven pattern

### nlohmann/json (Plan mentions this)
- **Problem:** Breaks Arc V pattern of zero dependencies
- **Effort:** Same as manual JSON (roughly)
- **Risk:** Introduces external dependency

**Recommendation:** Continue Arc V's manual JSON pattern. Skip nlohmann/json.

---

## Phase 26: Cloud Integration

### Current Plan
- Requires REST API backend, HTTP library, authentication system
- ~7-10 phase tasks (vs. 3-phase typical arc)
- Includes SetUploader, OnlineLeaderboard, MatchmakingService, StreamingService

### Recommendation: DEFER
- **Why defer:** Local multiplayer (Phase 24-25) delivers full value without cloud
- **Alternative:** Cloud as optional v1.1 DLC or separate service
- **Infrastructure gap:** Backend service design not in scope of current plan
- **Priority:** Phase 24-25 are higher value for local-first game

---

## Arc V Integration (CRITICAL GAP)

### What exists
- ✅ TutorialSystem (2 lessons, 5 tests)
- ✅ MissionSystem (3 mission types, 7 tests)
- ✅ CareerProgression (5 venues, 7 effects, 5 achievements, 5 tests)
- ✅ All tests passing (17/17 in Arc V suite)

### What's missing
- ❌ NOT integrated into main.cpp
- ❌ No --tutorial flag
- ❌ No --mission flag
- ❌ No career feedback in main loop

### Recommendation: Complete BEFORE Phase 24
- Timeline: 1-2 tasks, ~3-4 days
- Impact: Validates Arc V actually works in running app
- Blocker: Phase 24 design should consider career integration

---

## External Dependencies Analysis

### Current Status
- SQLite3 (optional, audio)
- PortAudio (optional, real-time audio)
- libsndfile (optional, file I/O)
- LAME (optional, MP3)
- DirectX (Windows built-in)

### For Phase 25
- ❌ NO JSON library currently used (use manual JSON)

### For Phase 26
- WinHTTP (Windows built-in, viable)
- libcurl (external, viable but adds dependency)
- Decision deferred until Phase 26 design

---

## Key Decision Points

| Decision | Current Plan | Recommendation | Impact |
|----------|-------------|-----------------|--------|
| Phase 24 graphics | Split-screen DirectX | Terminal hotseat | 1 week vs. 2 weeks |
| Phase 25 JSON | nlohmann external lib | Manual JSON (Arc V pattern) | Zero vs. external dependency |
| Phase 26 cloud | Included in v1.0 | Defer to v1.1+ | Reduces scope 7-10 tasks |
| Arc V integration | Deferred | Complete first | 1-2 tasks, enables Phase 24 |

---

## Blockers Before Phase 24

1. ✅ Arc V integration completion (--tutorial, --mission, --career flags)
2. ✅ InputMapper design for Player 1/Player 2 key separation
3. ✅ BattleMode architecture (TDD-first design)

**NOT blockers:**
- ❌ Split-screen graphics (can defer to Phase 24b)
- ❌ Cloud backend (defer to Phase 26+)
- ❌ nlohmann/json (use manual JSON)

---

## Revised Arc VI Timeline

### Phase 24: Local Multiplayer (Terminal)
- Timeline: 1 week
- Value: 2-player competitive mode
- Terminal hotseat battles, no graphics changes

### Phase 25: Configuration & State
- Timeline: 1-1.5 weeks  
- Value: Save/load + presets
- ConfigManager (manual JSON)

### Phase 27: Arc V Integration (NEW)
- Timeline: 3-4 days
- Value: Completes single-player experience
- Adds tutorial/mission/career modes to main.cpp

### Phase 26: Cloud (DEFERRED)
- Decision: Implement as optional v1.1 DLC or separate service
- No impact on core gameplay

**Total: 2.5-3 weeks for core value** (vs. 4+ weeks if including all cloud + graphics)

---

## Go/No-Go Recommendation

✅ **GO AHEAD WITH ARC VI**  
Feasible with scope refinement. Key changes:
1. Terminal battles instead of split-screen (Phase 24a)
2. Manual JSON instead of nlohmann (Phase 25)
3. Defer cloud to Phase 26+ (Phase 26)
4. Complete Arc V integration first (Pre-Phase 24)

**Confidence:** HIGH (all components achievable, existing Arc V provides foundation)
