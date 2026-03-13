# Arc VI Feasibility Research Report
**Date:** March 13, 2026  
**Status:** Research Complete

---

## Executive Summary

Arc VI (Multiplayer & Social Features) is **achievable but requires significant scope refinement**. The existing plan optimistically assumes split-screen rendering and cloud backend with minimal analysis. This research identifies critical gaps in Graphics rendering, external dependencies, and deferred Arc V integration that should be resolved before Phase 24 begins.

**Recommendation:** Proceed with Arc VI but adopt simplified Phase 24 (terminal-only hotseat battles) and defer cloud features to Phase 26+.

---

## Phase 24 Findings: Local Multiplayer (DJ Battles)

### Question 1: How complex is split-screen rendering with DirectX 11?

**Status:** ⚠️ **FEASIBLE BUT NON-TRIVIAL**

#### Current Graphics Context Analysis

The existing `GraphicsContext` (Phase 7-18) supports:
- **Single renderFrame() output:** One unified 1280x720 or 1920x1080 viewport
- **Single render target:** One `ID3D11RenderTargetView` per frame
- **Single camera:** One `Camera` object managing perspective
- **Single viewport:** `context_->RSSetViewports(1, &viewport)` - hardcoded to 1 viewport

Key code from `GraphicsContext.cpp` (line 182):
```cpp
D3D11_VIEWPORT viewport = {};
viewport.TopLeftX = 0.0f;
viewport.TopLeftY = 0.0f;
viewport.Width = (float)width_;
viewport.Height = (float)height_;
context_->RSSetViewports(1, &viewport);  // Only 1 viewport
```

#### Split-Screen Feasibility Analysis

**What would be required:**

1. **Dual Viewports:** Modify GraphicsContext to manage 2 viewports simultaneously
   - Left half: Player 1 deck (0, 0, width/2, height)
   - Right half: Player 2 deck (width/2, 0, width/2, height)
   - Shared bottom: Spectator crowd effects

2. **Dual Cameras:** Create 2 separate `Camera` objects with different perspectives
   - Currently 1 `Camera` per GraphicsContext
   - Would need camera swapping or parallel camera updates

3. **Multiple Render Passes:** 
   - Stage geometry has 1 render target view
   - Would need to render P1 deck → render target 1, P2 deck → render target 2, composite crowd
   - Requires `RenderTarget` class (exists for bloom/effects) but not currently used for split-screen

4. **Shader Modifications:**
   - Current shaders assume single deck/crossfader input
   - HLSL would need to branch based on viewport position or use instanced rendering

**Estimated Complexity:**
- **Code changes:** ~500-800 lines in GraphicsContext
- **New visual components:** SplitScreenRenderer (~200-300 lines)
- **Difficulty:** Medium-High (DirectX 11 concepts, but straightforward logic)
- **Risk:** Moderate (requires refactoring GraphicsContext; potential for regressions)

#### Alternative: Terminal-Only Battles (RECOMMENDED)

**Hotseat Battle Mode - No Split-Screen Graphics**

Instead of DirectX 11 split-screen:
- Keep terminal-based output (match Arc W design)
- Show P1 and P2 decks sequentially or side-by-side in ASCII
- Render waveform for each player using existing `WaveformRenderer`
- Display battle status (P1 score, P2 score, time remaining) in terminal

Example terminal layout:
```
BATTLE MODE: Standard Format (Best of 3 rounds)
=============================================================================
Player 1 Score: 87/100  |  Player 2 Score: 92/100  |  JUDGE: AI
Round 1/3: 45 seconds remaining
-----
P1 DECK [████████░░░░░░] | P2 DECK [██████░░░░░░░░░░]
BPM: 124.0 / Crossfade: 0.3 | BPM: 128.5 / Crossfade: -0.2
[FEEDBACK: Good energy!]   | [FEEDBACK: Smooth transition]
=============================================================================
```

**Benefits:**
- ✅ Uses existing WaveformRenderer (Phase 4)
- ✅ Terminal-based, matches Arc V design pattern  
- ✅ Zero graphics changes needed
- ✅ Can be implemented in ~400-500 lines
- ✅ Maintains focus on DJ mechanics, not graphics

**Drawbacks:**
- ❌ Less visually impressive than split-screen
- ❌ No spectator crowd visualization
- ❌ Requires sequential or ASCII side-by-side rendering

**Recommendation:** Start Phase 24 with terminal-only battles. If Phase 24 succeeds and graphics are deemed priority, add split-screen rendering as Phase 24.5 enhancement.

---

### Question 2: Can we implement battles without split-screen?

**Status:** ✅ **YES - STRONGLY RECOMMENDED**

#### Minimal Viable Battle Mode (Terminal-Only)

**Core Requirements:**
- 2 player decks (independent state, separate InputCommand mappings)
- Battle state machine (Pre-Battle → P1 Turn → P2 Turn → Scoring → Winner)
- Judge AI scoring algorithm (1-100 scale)
- Round tracking (Best of N format)

**Files to create (matches existing pattern):**
- `multiplayer/BattleMode.h/cpp` - State machine, round logic
- `multiplayer/BattleRules.h/cpp` - Scoring rules (Quick/Standard/Tournament)
- `multiplayer/Judge.h/cpp` - Scoring algorithm
- `input/InputMapper.cpp` - Extend with Player 1/Player 2 command separation
- Tests: 7-9 test files

**What's NOT needed initially:**
- Split-screen rendering
- RenderTarget multi-pass
- SpectatorCrowd voting mechanics
- Complex graphics

**Complexity:** Low-Medium
- Reuses existing Deck, Mixer, CrowdStateMachine, ScoringSystem
- State machine logic ~200-300 lines
- Judge algorithm ~150-200 lines
- Input splitting ~50-100 lines

**Estimated effort:** 3-4 implementation tasks (vs. 6-7 for split-screen version)

---

### Question 3: What's the minimal viable battle mode?

**Recommendation: Terminal Hotseat Battle**

**Specification:**

1. **Format:** Alternating turns or simultaneous play
   - Turn-based: P1 plays deck for 30s, then P2 plays
   - Simultaneous: Both play together; crowd chooses winner
   - Configurable via `BattleRules`

2. **Judging:** 5-component scoring (100 points max)
   - Beatmatching quality (20 pts)
   - Transition smoothness (25 pts)
   - Energy management (25 pts)
   - Creativity/effect usage (20 pts)
   - Crowd energy response (10 pts)

3. **Battle Duration:** 2-3 rounds, ~3 minutes total

4. **Input Mapping:** Player 1 uses QWERTY keys (existing), Player 2 uses alternate keys (WASD + modifiers)

5. **Terminal UI:** ASCII waveforms + status line per player

**Tests to write:**
- `test_BattleMode_TwoPlayers` - Player independence
- `test_Judge_ScoringConsistency` - Rational point distribution
- `test_BattleMode_RoundProgression` - Round transitions
- `test_InputMapper_DualPlayer` - P1/P2 key isolation
- `test_BattleRules_Formats` - Quick/Standard/Tournament formats

---

## Phase 25 Findings: Configuration & State Management

### Question 1: External Dependency Conflicts

**Status:** ⚠️ **POTENTIAL CONFLICT**

#### The nlohmann/json Decision

**Plan states:** "Implement ConfigManager with JSON backend (nlohmann/json)"

**Arc V reality:** All 3 phases (21-23) use **manual JSON serialization**
- ProgressTracker (Phase 21): ostringstream for write, istringstream for parse
- Leaderboard (Phase 22): Line-by-line manual parsing
- AchievementSystem (Phase 23): Manual JSON construction

**Code examples:**

ProgressTracker saving (manual):
```cpp
file << "{\n";
file << "  \"version\": 1,\n";
file << "  \"completed_lessons\": [\n";
for (std::size_t i = 0; i < completedLessons_.size(); ++i) {
    file << "    " << completedLessons_[i];
    if (i < completedLessons_.size() - 1) file << ",";
    file << "\n";
}
file << "  ]\n}\n";
```

**Why Arc V chose manual serialization:**
- Zero external dependencies (matches project philosophy)
- C++20 std library only requirement
- Small, predictable JSON structures
- Learned from fixing 3 npos-related JSON bugs (phases 21-22)

#### Options for Phase 25

**Option A: Continue Manual JSON** ✅ **RECOMMENDED**
- Pros: Consistent with Arc V, zero dependencies, proven pattern
- Cons: More boilerplate for complex ConfigManager structures
- ConfigManager would need separate read/write functions per config type
- Estimated effort: Same as nlohmann/json approach

**Option B: Adopt nlohmann/json**
- Pros: Less boilerplate, better for complex nested configs
- Cons: Introduces external dependency (breaks project philosophy)
- CMakeLists.txt impact: Add `find_package(nlohmann_json)` or `FetchContent`
- Risk: Deviates from established Arc V pattern

**Option C: Hybrid Approach**
- Use nlohmann/json ONLY for Phase 25 ConfigManager
- Keep Arc V systems on manual serialization
- Allows gradual migration without breaking existing code

**Recommendation:** **Option A - Continue Manual JSON**
- Phase 25 configs are similar complexity to Leaderboard (~500-800 JSON lines)
- Matches established pattern from Arc V
- Maintains zero external dependencies
- Follows project philosophy: "prefer standard library types"

---

### Question 2: Does project use any external HTTP libraries?

**Status:** ✅ **NO - Not currently used**

#### Current CMakeLists.txt Analysis

**External dependencies currently handled:**
1. SQLite3 - Optional, via find_package/pkg-config
2. PortAudio - Optional, for real-time audio
3. libsndfile - Optional, for audio file I/O
4. LAME - Optional, for MP3 encoding
5. DirectX (d3d11, dxgi, d3dcompiler) - Windows built-in

**HTTP libraries NOT present:**
- ❌ libcurl
- ❌ WinHTTP (though it's built into Windows)
- ❌ Boost.ASIO
- ❌ Beast (Boost.Beast)

#### Available Options for Phase 26

**Option 1: WinHTTP** (Windows built-in)
- Pros: No external dependency, Windows-native
- Cons: Windows-only, older API, less modern C++ ergonomics
- Code: `#include <winhttp.h>`, link against `winhttp.lib`
- CMakeLists.txt: `target_link_libraries(...PRIVATE winhttp)`

**Option 2: libcurl** (vendored or system)
- Pros: Cross-platform, mature, widely used
- Cons: External dependency (breaks philosophy), needs packaging
- CMakeLists.txt: `find_package(CURL)` or `FetchContent_Declare(curl ...)`
- Effort: ~30 lines in CMakeLists

**Option 3: Custom REST client** (manual HTTP over Winsock2)
- Pros: Zero dependencies, educational
- Cons: Complex, error-prone, slow development
- Not recommended

**Recommendation for Phase 26:** **Defer HTTP library decision**
- Phase 26 is cloud backend (far future)
- WinHTTP is viable for Windows target
- Can decide later based on cloud architecture needs

---

## Phase 26 Findings: Cloud Integration & Sharing

### Question 1: Is cloud backend realistic for local-first DJ game?

**Status:** ⚠️ **YES, BUT QUESTIONABLE VALUE**

#### Cloud Backend Trade-offs

**Value propositions:**
1. ✅ Online leaderboards (compare scores globally)
2. ✅ Set sharing (upload/download recorded DJ sets)
3. ✅ Remote battles (play against distant players)
4. ✅ Live streaming (audience watching live performances)

**Costs & Complexity:**
1. ❌ Requires external cloud infrastructure (not just the app)
2. ❌ Authentication system (OAuth2/JWT handling)
3. ❌ Backend API development (separate service)
4. ❌ Data persistence on server (database, storage)
5. ❌ Network reliability handling (fallback to local-only if offline)
6. ❌ Privacy/security considerations
7. ❌ Maintenance burden

#### Realistic Assessment

**Current project state:**
- DJ-ROOFRAT is a single-player offline game
- Runs in terminal with optional DirectX graphics
- No existing network/auth infrastructure

**Phase 26 scope realism:**
- Designing REST API: 1-2 phase tasks
- Implementing CloudAPI client: 1 phase task
- Authentication layer: 0.5-1 phase task
- SetUploader with multipart: 1 phase task
- OnlineLeaderboard: 1 phase task
- MatchmakingService: 1-2 phase tasks
- StreamingService (RTMP): 2+ phase tasks (complex)

**Total: 7-10 phase tasks** (vs typical 3-phase arc)

---

### Question 2: Can we defer cloud and focus on local multiplayer + config?

**Status:** ✅ **YES - STRONGLY RECOMMENDED**

#### Alternative Arc VI Proposal

**Revised Phases:**

**Phase 24: Local Multiplayer (DJ Battles)** ✅  
- Terminal-only hotseat battles (2 players, turn-based)
- Judge AI scoring
- Battle state machine
- Tests: 7-9
- ~1 week effort

**Phase 25: Configuration & State Management** ✅
- ConfigManager (manual JSON)
- DeckConfig, MIDIConfig, SessionState
- PresetManager (user presets library)
- AutoSaveManager (periodic saves)
- Tests: 7
- ~1-1.5 weeks effort

**Phase 26: DEFERRED - Cloud Integration (optional DLC)**
- Can be implemented later if demand exists
- Keep as separate module (doesn't block Phases 24-25)
- Mark as "Phase 26+" for future consideration

#### Value Proposition of Local-Only

**Phase 24-25 deliver:**
- 2-player competitive mode (high entertainment value)
- Save/load all settings (quality-of-life)
- Presets library (user customization)
- No external dependencies or infrastructure needs

**This is sufficient for v1.0 of DJ-ROOFRAT.**

---

## Arc V Integration Assessment

### Current Status: Tutorial, Mission, Career NOT Integrated

**What exists (completed & tested):**
- ✅ TutorialSystem with 2 lessons (beatmatching + EQ)
- ✅ MissionSystem with 3 mission types
- ✅ Leaderboard with JSON persistence
- ✅ CareerProgression with 5 venues, 7 effects, 5 achievements
- ✅ All tests passing (17/17 in Arc V)

**What's missing (deferred):**
- ❌ main.cpp integration (--tutorial flag)
- ❌ main.cpp integration (--mission flag)  
- ❌ Terminal UI for lesson hints
- ❌ Terminal UI for mission timers
- ❌ Career feedback in main loop
- ❌ Achievement notifications
- ❌ Effect unlock feedback in main loop

#### Integration Effort Assessment

**Should Arc V integration be completed before Phase 24?**

**Arguments FOR completing now:**
1. ✅ Adds concrete game modes to main executable
2. ✅ Arc V tests would have real integration coverage
3. ✅ Players can actually use tutorial/mission/career features
4. ✅ ~3-4 days effort (straightforward terminal output)

**Arguments AGAINST (proceed to Arc VI):**
1. ✅ Arc V functionality is complete and tested in isolation
2. ✅ Phase 24-25 are high-value (multiplayer + save/load)
3. ✅ Arc V can be integrated as Phase 27 "Polish & Integration"
4. ✅ Deferring doesn't break anything

#### Recommendation: **Complete Arc V Integration BEFORE Phase 24**

**Rationale:**
- Main.cpp is the "entry point" to all game modes
- Adding --tutorial, --mission, --career flags is quick (~200-300 lines)
- Validates that Arc V systems actually work in the running application
- Creates testing harness for Phase 24 (can test battles within career)

**Integration checklist:**
1. Add `--tutorial` flag to main.cpp → launch TutorialSystem loop
2. Add `--mission` flag to main.cpp → launch MissionSystem loop
3. Add `--career` flag to main.cpp → launch CareerProgression in existing main loop
4. Terminal UI: Print lesson/mission/career feedback each frame
5. Test: Run each mode for 30+ seconds, verify no crashes

**Estimated effort:** 1-2 tasks (1-2 days)

---

## Implementation Alternatives Summary

### Phase 24: Local Multiplayer

| Approach | Complexity | Effort | Graphics | Feasibility |
|----------|-----------|--------|----------|------------|
| **Terminal Hotseat** | Low-Medium | 1 week | ASCII | ✅ HIGH |
| Split-screen 1280x720 | High | 2 weeks | DirectX | ⚠️ MEDIUM |
| Split-screen with crowd | Very High | 3+ weeks | DirectX | ⚠️ MEDIUM |

**Recommendation:** Start with Terminal Hotseat (Phase 24a), add split-screen as Phase 24b if time permits.

### Phase 25: Configuration

| Approach | Simplicity | Dependencies | Compatibility |
|----------|-----------|--------------|---------------|
| **Manual JSON** | Medium | None | ✅ Matches Arc V |
| nlohmann/json | Low | External lib | ⚠️ Breaks pattern |
| Hybrid | Medium | Optional | ✅ Flexible |

**Recommendation:** Manual JSON (Option A), consistent with Arc V.

### Phase 26: Cloud

| Approach | Value | Effort | Infrastructure |
|----------|-------|--------|-----------------|
| **Defer** | Future | 0 | None | ✅ RECOMMENDED |
| WinHTTP | Moderate | 2-3 weeks | Custom backend |
| libcurl | Moderate | 2-3 weeks + External lib | Custom backend |

**Recommendation:** Defer to Phase 26+ or optional DLC module.

---

## Critical Gaps & Recommendations

### 1. Arc V Integration (BLOCKER?)

**Finding:** Tutorial, Mission, Career exist but aren't accessible to players.

**Recommendation:** Complete Arc V integration BEFORE Phase 24
- Add --tutorial, --mission, --career flags to main.cpp
- Integrate career feedback into main loop
- ~1-2 tasks, high-value validation

**Timeline:** Weeks 1-2 of Arc VI planning

---

### 2. Graphics Complexity (SCOPE CREEP?)

**Finding:** Phase 24 plan assumes split-screen rendering, which requires significant GraphicsContext refactoring.

**Recommendation:** Start with terminal-only battles (Phase 24a)
- Uses existing WaveformRenderer, zero graphics changes
- ~1 week effort vs. 2+ weeks for split-screen
- Can be enhanced later if desired

**Timeline:** Phase 24 core development (1 week)

---

### 3. External Dependencies Philosophy (CONSISTENCY)

**Finding:** Phase 25 plan mentions nlohmann/json, but Arc V never used it.

**Recommendation:** Continue manual JSON serialization in Phase 25
- Consistent with Arc V pattern (ProgressTracker, Leaderboard, AchievementSystem)
- Zero external dependencies
- Proven pattern with established bug fixes (npos checks)

**Timeline:** Planning stage (update Phase 25 spec)

---

### 4. Cloud Backend Feasibility (FUTURE WORK)

**Finding:** Phase 26 requires custom REST API backend service (not pictured in plan).

**Recommendation:** 
- Option A: Defer cloud to Phase 26+ (recommended)
- Option B: Make cloud optional, focus Phase 24-25 on local features
- Option C: If cloud is priority, move to separate project/service repo

**Timeline:** Revisit after Phase 25 completion

---

## Revised Arc VI Plan

### Recommended Phasing

**Arc VI - Multiplayer & Social Features (Phases 24-26)**

**Phase 24: Local Multiplayer - Hotseat Battles (TERMINAL)**
- Terminal-only 2-player DJ battles
- Judge AI scoring (5-component system)
- Battle state machine (turn-based or simultaneous)
- No graphics changes required
- Extends InputMapper for Player 1/Player 2
- Tests: 7-9
- **Effort: ~1 week**

**Phase 25: Configuration & State Management**
- ConfigManager with manual JSON (consistent with Arc V)
- DeckConfig, MIDIConfig, SessionState
- PresetManager for user presets
- AutoSaveManager for periodic saves
- Tests: 7
- **Effort: ~1-1.5 weeks**

**Phase 26: Cloud Integration (OPTIONAL/DEFERRED)**
- Can be implemented as separate module
- Consider as v1.1+ or optional DLC
- Requires custom backend service
- **Decision: Defer until Phases 24-25 stabilize**

**Phase 27: Polish & Integration (NEW)**
- Complete Arc V integration (tutorial/mission/career modes)
- Main loop feedback for career progression
- Achievement notifications in terminal
- Tests: Arc V integration tests
- **Effort: ~3-4 days**

---

## Blockers & Prerequisites

### Before Phase 24 Starts

**Required:**
1. ✅ Arc V integration (main.cpp tutorial/mission/career flags)
2. ✅ InputMapper extended for Player 1/Player 2 key separation
3. ✅ BattleMode architecture design (TDD: write tests first)

**Not required:**
- ❌ Graphics split-screen (defer to Phase 24b)
- ❌ Cloud backend (defer to Phase 26+)
- ❌ nlohmann/json (continue manual JSON)

### Before Phase 25 Starts

**Required:**
1. ✅ Phase 24 complete and tested
2. ✅ ConfigManager design review

**Not required:**
- ❌ Cloud infrastructure
- ❌ External HTTP library

### Before Phase 26 Starts

**Required:**
1. ✅ Phases 24-25 complete
2. ✅ Cloud backend architecture design
3. ✅ Backend service implementation plan (outside DJ-app scope)

---

## Recommendations

### 1. Arc VI Plan Refinement ⭐⭐⭐

**Action:** Update `arc-vi-multiplayer-social-plan.md` to:
- Phase 24: Terminal hotseat battles (not split-screen)
- Phase 25: Manual JSON ConfigManager (not nlohmann)
- Phase 26: Mark as "optional/deferred cloud features"
- Phase 27 (NEW): Arc V integration polish

**Rationale:** Currently the plan is optimistic about graphics and dependencies.

---

### 2. Arc V Integration Priority ⭐⭐⭐

**Action:** Before starting Phase 24, complete Arc V integration:
- Add --tutorial, --mission, --career flags to main.cpp
- Print lesson status, mission timer, career feedback in terminal
- ~1-2 tasks, validates Arc V actually works
- Creates foundation for Phase 24 battle integrations

**Rationale:** No point building multiplayer without playable single-player modes.

---

### 3. InputMapper Dual-Player Extension ⭐⭐

**Action:** Before Phase 24, design Player 1/Player 2 key separation:
- P1: QWERTY keys (existing)
- P2: WASD + modifier keys (new)
- Consider USB game controller support (future)

**Rationale:** Core requirement for hotseat battles.

---

### 4. Graphics Deferral Decision ⭐⭐

**Action:** Explicitly decide: Does Phase 24 need split-screen graphics?
- **Option A (Recommended):** Terminal hotseat, add graphics later (Phase 24b)
- **Option B:** Accept 2-week graphics work, do split-screen now
- **Option C:** Defer graphics entirely, focus on game mechanics

**Rationale:** Biggest technical complexity in whole plan.

---

### 5. Cloud Backend Strategy ⭐

**Action:** Decide: Is cloud backend part of DJ-ROOFRAT MVP?
- **Option A (Recommended):** Defer to Phase 26+, mark as optional
- **Option B:** Make separate backend service repo
- **Option C:** Planned but not implemented in v1.0

**Rationale:** Adds infrastructure complexity; not critical for local-first game.

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|-----------|
| Split-screen graphics complexity | HIGH | HIGH | Use terminal battles instead |
| External dependency bloat | MEDIUM | MEDIUM | Continue manual JSON pattern |
| Arc V integration delay | LOW | LOW | Complete before Phase 24 |
| Cloud backend scope creep | HIGH | HIGH | Defer to Phase 26+ explicitly |
| Input mapping conflicts (P1/P2) | LOW | MEDIUM | Design InputMapper extension early |

---

## Conclusion

**Arc VI is feasible and valuable.** With recommended scope refinement:

1. ✅ **Phase 24 (Terminal Battles):** 1 week, high-value multiplayer
2. ✅ **Phase 25 (Config):** 1-1.5 weeks, excellent quality-of-life
3. ✅ **Phase 27 (Arc V Integration):** 3-4 days, completes single-player experience
4. ⏹️ **Phase 26 (Cloud):** Defer to v1.1 or optional DLC

**Total revised effort: 2.5-3 weeks** for core local multiplayer + config (vs. 4+ weeks if including cloud + split-screen graphics).

**Next step:** Convene with conductor to refine plan, confirm graphics priorities, and approve Arc V integration first before Phase 24 kicks off.

---

## Appendix: File Structure References

### Current Art (Arc V - Complete)
- ✅ `gameplay/TutorialSystem.h/cpp`
- ✅ `gameplay/MissionSystem.h/cpp`
- ✅ `gameplay/Leaderboard.h/cpp`
- ✅ `gameplay/AchievementSystem.h/cpp`
- ✅ `gameplay/CareerProgression.*` (expanded in GameModes)

### Proposed Art (Arc VI - Phases 24-25)
- 📋 `multiplayer/BattleMode.h/cpp`
- 📋 `multiplayer/Judge.h/cpp`
- 📋 `multiplayer/BattleRules.h/cpp`
- 📋 `core/ConfigManager.h/cpp`
- 📋 `core/DeckConfig.h/cpp`
- 📋 `core/PresetManager.h/cpp`
- 📋 `core/AutoSaveManager.h/cpp`

### Not recommended for Phase 24 (defer to 24b or later)
- ❌ `visuals/SplitScreenRenderer.h/cpp`
- ❌ `multiplayer/SpectatorCrowd.h/cpp` (only if split-screen)

### Deferred (Phase 26+)
- ⏹️ `cloud/CloudAPI.h/cpp`
- ⏹️ `cloud/SetUploader.h/cpp`
- ⏹️ `cloud/OnlineLeaderboard.h/cpp`
- ⏹️ `cloud/MatchmakingService.h/cpp`
- ⏹️ `cloud/Authentication.h/cpp`

