## Plan: Arc V - Gameplay & Progression Systems (Phases 21-23)

Build tutorial system, mission framework, and expanded career mode to create structured learning paths and long-term engagement. Scoped to terminal-based integration without graphics overlays or AI opponents.

**Phases: 3 phases**

### **Phase 21: Tutorial & Training System**
- **Objective:** Interactive tutorial mode teaching beatmatching and EQ mixing with progress tracking
- **Files/Functions to Modify/Create:**
  - gameplay/TutorialSystem.h: Lesson framework and state management
  - gameplay/TutorialSystem.cpp: Lesson sequencing, condition checking
  - gameplay/Lesson.h: Individual lesson interface (setup, validate, teardown)
  - gameplay/Lesson.cpp: Base lesson implementation
  - gameplay/lessons/BeatmatchingLesson.{h,cpp}: Teach beatmatching fundamentals (BPM delta < 2%)
  - gameplay/lessons/EQMixingLesson.{h,cpp}: Teach EQ transition techniques
  - gameplay/ProgressTracker.h: Save/load lesson completion state
  - gameplay/ProgressTracker.cpp: JSON-based progress persistence (manual serialization)
  - src/main.cpp: Add tutorial mode (--tutorial or --mode tutorial flag), terminal output for lesson status
- **Tests to Write:**
  - test_TutorialSystem_SequenceProgression: Verify lessons unlock in order
  - test_BeatmatchingLesson_Validation: Detect when deckB tempo is within ±2% of deckA
  - test_EQMixingLesson_Transition: Verify lesson marks complete when EQ moves from preset-A to preset-B
  - test_ProgressTracker_SaveLoad: JSON save/load of completion state
  - test_TutorialSystem_HintGeneration: Verify hints emitted at correct checkpoint
- **Steps:**
  1. Write tests for tutorial system and lessons (tests fail)
  2. Implement TutorialSystem with lesson state machine
  3. Create 2 core lessons (beatmatching + EQ mixing)
  4. Implement ProgressTracker with manual JSON serialization
  5. Add tutorial mode to main.cpp with terminal status display
  6. Run tests to verify lesson validation accuracy (tests pass)
  7. Lint/format

### **Phase 22: Mission & Challenge System**
- **Objective:** Time-based and technique-specific challenges with local leaderboard
- **Files/Functions to Modify/Create:**
  - gameplay/MissionSystem.h: Mission framework (goals, timers, validation)
  - gameplay/MissionSystem.cpp: Mission state management
  - gameplay/Mission.h: Base mission interface
  - gameplay/Mission.cpp: Common mission logic
  - gameplay/missions/SustainedBeatmatchMission.{h,cpp}: Maintain beatmatch < 2% delta for 30 seconds
  - gameplay/missions/EnergySurvivalMission.{h,cpp}: Keep crowd energy > 0.5 for 60 seconds
  - gameplay/missions/TransitionMission.{h,cpp}: Execute smooth crossfade (transitionSmoothness > 0.7)
  - gameplay/Leaderboard.h: Local leaderboard interface
  - gameplay/Leaderboard.cpp: Score submission and ranking with JSON persistence
  - src/main.cpp: Add mission mode (--mission flag), display objectives and timers
- **Tests to Write:**
  - test_MissionSystem_Lifecycle: Mission state transitions (idle → running → complete/failed)
  - test_SustainedBeatmatchMission_Timer: Correctly track 30-second duration
  - test_SustainedBeatmatchMission_Failure: Fail if beatmatch delta > 2%
  - test_EnergySurvivalMission_ThresholdDetection: Fail if crowd energy drops below 0.5
  - test_TransitionMission_SmoothnessBoundary: Pass/fail on transitionSmoothness threshold
  - test_Leaderboard_PersistenceAndRanking: Save scores to JSON, reload, verify sorted by score descending
  - test_Leaderboard_MultiMissionTracking: Separate leaderboards per mission type
- **Steps:**
  1. Write tests for mission system and challenges (tests fail)
  2. Implement MissionSystem with goal tracking
  3. Create 3 mission types (sustained beatmatch, energy survival, smooth transition)
  4. Implement Leaderboard with local JSON storage (manual serialization)
  5. Add mission mode UI with terminal timer and objective display
  6. Run tests to verify mission logic and leaderboard ranking (tests pass)
  7. Lint/format

### **Phase 23: Career Mode Expansion**
- **Objective:** Deep progression system with venue unlocks, effect gating by tier, and focused achievements
- **Files/Functions to Modify/Create:**
  - gameplay/Venue.h: Venue properties (name, minReputationRequired, capacity, difficulty)
  - gameplay/Venue.cpp: 5-7 venue definitions (Basement Bar → Festival Main Stage)
  - gameplay/UnlockSystem.h: Effect unlock management by tier
  - gameplay/UnlockSystem.cpp: Unlock conditions and state tracking (audio effects only)
  - gameplay/AchievementSystem.h: Achievement/trophy definitions
  - gameplay/AchievementSystem.cpp: Achievement tracking and unlocking (5-7 focused achievements)
  - gameplay/GameModes.h: Expand CareerProgression with unlock queries
  - gameplay/GameModes.cpp: Extended progression logic (preserve monotonic tier behavior)
  - src/main.cpp: Display venue unlock progress, show locked effects
- **Tests to Write:**
  - test_Venue_ProgressionHierarchy: Verify venues unlock in reputation order (monotonic)
  - test_UnlockSystem_EffectGating: Test effects locked/unlocked by tier correctly
  - test_CareerProgression_ReputationGain: Validate reputation increases from high crowdEnergy
  - test_AchievementSystem_Triggering: Test 5-7 simple achievements unlock correctly
  - test_AchievementSystem_Persistence: Save/load achievement state from JSON
- **Steps:**
  1. Write tests for expanded career features (tests fail)
  2. Expand CareerProgression with reputation, unlocks (preserve monotonic tier)
  3. Create 5-7 venue definitions with progressive difficulty
  4. Implement UnlockSystem gating audio effects by tier (e.g., Filters at tier 2, Advanced at tier 3+)
  5. Add AchievementSystem with 5-7 achievements (e.g., "Reach Tier 3", "Complete 5 missions")
  6. Integrate career feedback into main loop terminal output
  7. Run tests to verify progression balance (tests pass)
  8. Lint/format

**Open Questions:**
1. Boss battles: Defer to Phase 24 (Arc VI)? **YES - Deferred**
2. Phase 21 lesson count: Start with 2 or 4 lessons? **2 lessons (beatmatching + EQ mixing)**
3. Mode entry: Command-line flags only or flags + menu? **Flags now (--tutorial, --mission), menu later**
4. Effect unlock scope: Audio effects only or audio + visuals? **Audio effects only in Arc V**
5. Achievement count: 5-7 focused achievements or 20-30? **5-7 focused achievements**

**Dependencies:**
- **Arc I** recommended (real music for lessons/missions)
- **Arc III Phase 15-16** for effect unlocks
- **Manual JSON serialization** for progress persistence (pattern already used in SessionMetadata/MIDIMapping)
