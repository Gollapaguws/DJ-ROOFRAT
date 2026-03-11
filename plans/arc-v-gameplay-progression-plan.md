## Plan: Arc V - Gameplay & Progression Systems (Phases 21-23)

Build comprehensive tutorial system, mission framework, and expanded career mode to create structured learning paths and long-term engagement.

**Phases: 3 phases**

### **Phase 21: Tutorial & Training System**
- **Objective:** Interactive lessons teaching DJ techniques with skill assessment and progress tracking
- **Files/Functions to Modify/Create:**
  - gameplay/TutorialSystem.h: Lesson framework and state management
  - gameplay/TutorialSystem.cpp: Lesson sequencing, condition checking
  - gameplay/Lesson.h: Individual lesson interface (objectives, hints, validation)
  - gameplay/Lesson.cpp: Base lesson implementation
  - gameplay/lessons/BeatmatchingLesson.{h,cpp}: Teach beatmatching fundamentals
  - gameplay/lessons/EQTrainingLesson.{h,cpp}: Frequency recognition and mixing
  - gameplay/lessons/PhrasingLesson.{h,cpp}: Intro/outro identification, transition timing
  - gameplay/lessons/TransitionLesson.{h,cpp}: Crossfader and EQ transitions
  - gameplay/SkillAssessment.h: Performance analysis and grading
  - gameplay/SkillAssessment.cpp: Score calculation, feedback generation
  - gameplay/ProgressTracker.h: Save/load lesson completion state
  - gameplay/ProgressTracker.cpp: JSON-based progress persistence
  - visuals/TutorialOverlay.h: Visual guides and hints
  - visuals/TutorialOverlay.cpp: Render lesson instructions, highlight controls
  - src/main.cpp: Add tutorial mode, overlay rendering
- **Tests to Write:**
  - test_TutorialSystem_LessonSequence: Verify lessons unlock in order
  - test_BeatmatchingLesson_Validation: Test beatmatch accuracy detection
  - test_EQTrainingLesson_FrequencyID: Validate frequency identification scoring
  - test_PhrasingLesson_TimingWindow: Test transition timing tolerance
  - test_SkillAssessment_Grading: Verify grade calculation (A/B/C/D/F)
  - test_ProgressTracker_Persistence: Test save/load of completion state
  - test_TutorialOverlay_Highlighting: Verify control highlighting works
- **Steps:**
  1. Write tests for tutorial system and lessons (tests fail)
  2. Implement TutorialSystem with lesson state machine
  3. Create 4-5 core lessons (beatmatching, EQ, phrasing, transitions)
  4. Build SkillAssessment with performance metrics
  5. Implement ProgressTracker with JSON persistence
  6. Create TutorialOverlay for visual guides
  7. Run tests to verify lesson validation accuracy (tests pass)
  8. Lint/format

### **Phase 22: Mission & Challenge System**
- **Objective:** Time-based and technique-specific challenges with leaderboards
- **Files/Functions to Modify/Create:**
  - gameplay/MissionSystem.h: Mission framework (goals, rewards, timers)
  - gameplay/MissionSystem.cpp: Mission state management and validation
  - gameplay/Mission.h: Base mission interface
  - gameplay/Mission.cpp: Common mission logic
  - gameplay/missions/BeatmatchDurationMission.{h,cpp}: Maintain beatmatch for X minutes
  - gameplay/missions/PerfectTransitionMission.{h,cpp}: Execute flawless transition
  - gameplay/missions/EnergyManagementMission.{h,cpp}: Keep crowd above threshold
  - gameplay/missions/SurvivalMission.{h,cpp}: Don't drop below minimum crowd energy
  - gameplay/ChallengeRotation.h: Daily/weekly challenge selection
  - gameplay/ChallengeRotation.cpp: Seeded random challenge generation
  - gameplay/Leaderboard.h: Local and online leaderboard interface
  - gameplay/Leaderboard.cpp: Score submission and ranking
  - src/main.cpp: Add challenge mode, display objectives and timers
- **Tests to Write:**
  - test_MissionSystem_Completion: Verify mission complete detection
  - test_BeatmatchDurationMission_Timer: Test sustained beatmatch tracking
  - test_PerfectTransitionMission_Scoring: Validate transition quality grading
  - test_EnergyManagementMission_Threshold: Test crowd energy monitoring
  - test_SurvivalMission_Failure: Verify failure condition triggers correctly
  - test_ChallengeRotation_Seeding: Test deterministic daily challenge generation
  - test_Leaderboard_Ranking: Verify score sorting and ranking
- **Steps:**
  1. Write tests for mission system and challenges (tests fail)
  2. Implement MissionSystem with goal tracking
  3. Create 4-5 mission types (duration, perfect transition, energy, survival)
  4. Build ChallengeRotation with seeded random selection (date-based seed)
  5. Implement Leaderboard with local storage (JSON) and online stub
  6. Add challenge mode UI with timer and objective display
  7. Run tests to verify mission logic and leaderboard ranking (tests pass)
  8. Lint/format

### **Phase 23: Career Mode Expansion**
- **Objective:** Deep progression system with venue unlocks, equipment gating, and boss battles
- **Files/Functions to Modify/Create:**
  - gameplay/CareerProgression.h: Expand with reputation, sponsors, unlocks
  - gameplay/CareerProgression.cpp: Extended progression logic
  - gameplay/Venue.h: Venue properties (capacity, difficulty, requirements)
  - gameplay/Venue.cpp: 10+ venue definitions (basement → stadium)
  - gameplay/UnlockSystem.h: Equipment and feature unlock management
  - gameplay/UnlockSystem.cpp: Unlock conditions and state tracking
  - gameplay/ReputationSystem.h: Reputation gain/loss from performance
  - gameplay/ReputationSystem.cpp: Reputation-based booking opportunities
  - gameplay/SponsorSystem.h: Sponsorship deals and bonuses
  - gameplay/SponsorSystem.cpp: Sponsor requirements and rewards
  - gameplay/BossBattle.h: DJ vs AI opponent interface
  - gameplay/BossBattle.cpp: Adaptive AI difficulty, crowd split mechanics
  - gameplay/AchievementSystem.h: Achievement/trophy definitions
  - gameplay/AchievementSystem.cpp: Achievement tracking and unlocking
  - src/main.cpp: Integrate expanded career features
- **Tests to Write:**
  - test_Venue_Requirements: Verify reputation gates venue access
  - test_UnlockSystem_EffectGating: Test equipment unlocks by level
  - test_ReputationSystem_Performance: Validate reputation gain from good sets
  - test_SponsorSystem_Deals: Test sponsor unlock conditions
  - test_BossBattle_AI: Verify AI opponent makes intelligent choices
  - test_BossBattle_CrowdSplit: Test crowd energy competition mechanics
  - test_AchievementSystem_Triggering: Validate achievement unlock conditions
- **Steps:**
  1. Write tests for expanded career features (tests fail)
  2. Expand CareerProgression with reputation, sponsors, unlocks
  3. Create 10+ venue definitions with progressive difficulty
  4. Implement UnlockSystem gating effects/decks/visuals by progression
  5. Build ReputationSystem with performance-based reputation
  6. Create SponsorSystem with equipment bonuses
  7. Implement BossBattle with adaptive AI opponent
  8. Add AchievementSystem with 20-30 achievements
  9. Run tests to verify progression balance and AI competency (tests pass)
  10. Lint/format

**Open Questions:**
1. Tutorial voice-over: Text only or TTS/recorded audio? **Text initially, audio later**
2. Mission timer: Real-time or in-game beats/bars? **Both options (user choice)**
3. Leaderboard scope: Local only or add online? **Local Phase 22, online Phase 26**
4. AI opponent difficulty: Fixed levels or adaptive? **Adaptive (scales to player skill)**
5. Achievement rewards: Cosmetic only or gameplay bonuses? **Mix (cosmetic + minor bonuses)**

**Dependencies:**
- **Arc I** highly recommended (real music for lessons/missions)
- **Arc III Phase 15-16** for effect unlocks
- **Arc IV** for visual unlocks (optional)
- **JSON library** for progress persistence (nlohmann/json or similar)
