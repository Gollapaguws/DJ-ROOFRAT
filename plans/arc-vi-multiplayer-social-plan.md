## Plan: Arc VI - Multiplayer & Social Features (Phases 24-26)

Enable competitive local multiplayer, comprehensive configuration management, and cloud integration for set sharing and online leaderboards.

**Phases: 3 phases**

### **Phase 24: Local Multiplayer (DJ Battles)**
- **Objective:** Split-screen DJ battles with competitive scoring and spectator crowd
- **Files/Functions to Modify/Create:**
  - multiplayer/BattleMode.h: Battle framework (rounds, judging, scoring)
  - multiplayer/BattleMode.cpp: Turn-based and simultaneous battle logic
  - multiplayer/BattleRules.h: Rule set definitions (time limits, judging criteria)
  - multiplayer/BattleRules.cpp: Standard/freestyle/technical rule implementations
  - multiplayer/Judge.h: Performance judging interface
  - multiplayer/Judge.cpp: Automated judging algorithm (transitions, energy, creativity)
  - multiplayer/SpectatorCrowd.h: Voting and reaction system
  - multiplayer/SpectatorCrowd.cpp: Crowd voting mechanics and visual feedback
  - visuals/SplitScreenRenderer.h: Dual-deck visualization
  - visuals/SplitScreenRenderer.cpp: Split-screen rendering with shared crowd view
  - input/InputMapper.h: Extend with Player 1/Player 2 key mappings
  - input/InputMapper.cpp: Dual controller support
  - src/main.cpp: Add battle mode, split-screen UI
- **Tests to Write:**
  - test_BattleMode_TurnBased: Verify turn switching logic
  - test_BattleMode_Simultaneous: Test concurrent player performance
  - test_Judge_TransitionScoring: Validate transition quality grading
  - test_Judge_EnergyManagement: Test energy maintenance scoring
  - test_SpectatorCrowd_Voting: Verify crowd vote distribution
  - test_SplitScreenRenderer_Layout: Test dual-deck view rendering
  - test_InputMapper_DualPlayer: Verify independent player inputs
- **Steps:**
  1. Write tests for battle mode and split-screen (tests fail)
  2. Implement BattleMode with turn-based and simultaneous modes
  3. Create BattleRules for different competition formats
  4. Build Judge with automated scoring algorithm
  5. Implement SpectatorCrowd with voting mechanics
  6. Create SplitScreenRenderer for dual visualization
  7. Extend InputMapper for Player 1/Player 2 key separation
  8. Run tests to verify battle balance and fairness (tests pass)
  9. Lint/format

### **Phase 25: Configuration & State Management**
- **Objective:** Comprehensive save/load system for all user preferences and session state
- **Files/Functions to Modify/Create:**
  - core/ConfigManager.h: Central configuration management
  - core/ConfigManager.cpp: JSON/INI parsing and serialization
  - core/DeckConfig.h: Deck configuration struct (EQ curves, cue points, loops)
  - core/DeckConfig.cpp: Deck state serialization
  - core/MIDIConfig.h: MIDI mapping configuration
  - core/MIDIConfig.cpp: MIDI mapping save/load
  - core/SessionState.h: Complete session state (tracks, positions, settings)
  - core/SessionState.cpp: Session serialization/deserialization
  - core/PresetManager.h: User preset management
  - core/PresetManager.cpp: Save/load/organize presets
  - core/AutoSaveManager.h: Periodic auto-save interface
  - core/AutoSaveManager.cpp: Background auto-save with configurable interval
  - src/main.cpp: Integrate save/load menu, auto-save hooks
- **Tests to Write:**
  - test_ConfigManager_JSONSerialization: Verify JSON save/load correctness
  - test_DeckConfig_CuePointPersistence: Test cue point save/restore
  - test_MIDIConfig_MappingSave: Validate MIDI mapping persistence
  - test_SessionState_FullRestore: Test complete session restoration
  - test_PresetManager_Organize: Verify preset categorization and search
  - test_AutoSaveManager_Interval: Test auto-save triggers correctly
  - test_ConfigManager_BackupRecovery: Validate backup on corrupted config
- **Steps:**
  1. Write tests for configuration management (tests fail)
  2. Implement ConfigManager with JSON backend (nlohmann/json)
  3. Create DeckConfig capturing all deck state
  4. Implement MIDIConfig for mapping persistence
  5. Build SessionState for full session save/restore
  6. Create PresetManager for user preset library
  7. Implement AutoSaveManager with periodic background saves
  8. Run tests to verify data integrity and recovery (tests pass)
  9. Lint/format

### **Phase 26: Cloud Integration & Sharing**
- **Objective:** Online features for set sharing, leaderboards, and remote battles
- **Files/Functions to Modify/Create:**
  - cloud/CloudAPI.h: REST API interface definition
  - cloud/CloudAPI.cpp: HTTP client wrapper (WinHTTP or libcurl)
  - cloud/SetUploader.h: Recording upload interface
  - cloud/SetUploader.cpp: Multipart upload with progress tracking
  - cloud/SetBrowser.h: Online set browsing and downloading
  - cloud/SetBrowser.cpp: Search, filter, download community sets
  - cloud/OnlineLeaderboard.h: Global leaderboard interface
  - cloud/OnlineLeaderboard.cpp: Score submission and ranking retrieval
  - cloud/MatchmakingService.h: Remote battle matchmaking
  - cloud/MatchmakingService.cpp: Player matching and lobby system
  - cloud/StreamingService.h: Live set streaming interface
  - cloud/StreamingService.cpp: RTMP or WebRTC streaming to audience
  - cloud/Authentication.h: User authentication and session management
  - cloud/Authentication.cpp: OAuth2 or JWT token handling
  - src/main.cpp: Add online menu, upload/download UI
- **Tests to Write:**
  - test_CloudAPI_Connection: Verify API endpoint connectivity
  - test_SetUploader_MultipartUpload: Test large file upload with resume
  - test_SetBrowser_Search: Validate search and filtering
  - test_OnlineLeaderboard_ScoreSubmission: Test score upload and retrieval
  - test_MatchmakingService_PlayerMatching: Verify skill-based matching
  - test_StreamingService_RTMPStream: Test live stream initialization
  - test_Authentication_TokenRefresh: Validate JWT refresh logic
- **Steps:**
  1. Write tests for cloud integration (tests fail, requires mock API or staging server)
  2. Design cloud backend API specification (REST endpoints)
  3. Implement CloudAPI with HTTP client
  4. Create SetUploader with resumable multipart upload
  5. Build SetBrowser for community set discovery
  6. Implement OnlineLeaderboard with score sync
  7. Create MatchmakingService for remote battles
  8. Add StreamingService for live audience streaming (optional, advanced)
  9. Implement Authentication with secure token management
  10. Run tests to verify cloud integration (tests pass with staging server)
  11. Lint/format

**Open Questions:**
1. Battle format: Best of 3/5/7 or single round? **Configurable (quick=1, standard=3, tournament=5)**
2. Judging: AI only or human judges (network voting)? **AI initially, human voting Phase 26**
3. Config format: JSON or TOML or INI? **JSON (better tooling, nested structures)**
4. Auto-save interval: 1 minute or 5 minutes? **Configurable, default 2 minutes**
5. Cloud backend: Custom server or BaaS (Firebase, Supabase)? **Custom REST API (more control)**
6. Streaming protocol: RTMP or WebRTC? **RTMP (simpler, mature tooling)**

**Dependencies:**
- **Arc II Phase 14** required (recording for set uploads)
- **Arc V Phase 22** required (missions for leaderboard scores)
- **HTTP library** (WinHTTP built-in or libcurl vendored)
- **JSON library** (nlohmann/json)
- **Cloud backend** (separate service implementation, Phase 26 only)
