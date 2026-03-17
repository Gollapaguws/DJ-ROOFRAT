# Arc X: Auto-Sync & Coaching System

**Transform DJ-ROOFRAT from manual-only to assisted performance** by leveraging Arc VIII/IX analysis infrastructure to provide intelligent sync and real-time coaching without sacrificing manual control.

---

## Vision

Enable DJs of all skill levels to achieve professional-quality mixes through:
- **Automatic phase alignment** between decks (beat-matched sync)
- **Intelligent tempo matching** with smooth ramping
- **Real-time transition suggestions** based on phrase structure and energy
- **Visual coaching overlay** showing timing windows and mix quality feedback

**Philosophy:** Assist, don't replace. Sync features are **optional tools** that enhance workflow without removing manual control. Advanced DJs can toggle sync off; beginners use it as training wheels.

---

## Motivation

### Why Now?
Arc IX delivered world-class analysis (148× FFT speedup, beat grids, energy curves) but currently **only displays data**. Arc X **closes the loop** by using analysis to actively **assist performance**.

### User Value
- **Beginners:** Lower learning curve (auto-sync enables focus on song selection/energy)
- **Intermediate:** Coaching accelerates skill development (learn phrasing through visual feedback)
- **Advanced:** Workflow efficiency (sync handles tedious beatmatching, DJ focuses on creativity)

### Technical Readiness
95% of prerequisites exist:
- ✅ BPM Detection (Arc I Phase 10)
- ✅ Beat Grid (Arc VIII Phase 32, Arc IX Phase 36)
- ✅ Tempo Control with ramping (Arc I/IV)
- ✅ Onset Detection (Arc VIII Phase 32)
- ✅ Energy Analysis (Arc VIII Phase 33, Arc IX Phase 37)
- ✅ Harmonic Analysis (Arc VIII Phase 31)

**What's Missing:** Phase alignment algorithm + coaching logic (both are well-defined DSP/analysis problems).

---

## Phases (4 phases, 32 tests total)

### **Phase 38: Phase Alignment Engine**
**Goal:** Calculate and correct beat offset between decks for sample-accurate sync

#### Features
- Create `PhaseAligner` class analyzing beat grid offset
- Implement `Deck::alignPhaseWithDeck(targetDeck)` method
- Calculate phase offset in samples and beats
- Support both same-BPM and different-BPM alignment

#### Implementation Details
**Core Algorithm:**
```cpp
// Pseudocode for phase alignment
double PhaseAligner::calculateOffset(const Deck& deckA, const Deck& deckB) {
    // 1. Get current playback positions
    size_t posA = deckA.currentFrame();
    size_t posB = deckB.currentFrame();
    
    // 2. Find nearest beat positions in each beat grid
    double beatA = deckA.getBeatGrid().findNearestBeat(posA);
    double beatB = deckB.getBeatGrid().findNearestBeat(posB);
    
    // 3. Calculate offset (positive = B ahead, negative = B behind)
    double offsetBeats = beatB - beatA;
    
    // 4. Normalize to [-0.5, +0.5] range (choose nearest beat boundary)
    if (offsetBeats > 0.5) offsetBeats -= 1.0;
    if (offsetBeats < -0.5) offsetBeats += 1.0;
    
    // 5. Convert beats to samples
    double bpmA = deckA.getBPM() * (1.0 + deckA.getTempoPercent() / 100.0);
    double samplesPerBeat = (60.0 / bpmA) * deckA.sampleRate();
    return offsetBeats * samplesPerBeat;
}
```

**Alignment Correction:**
```cpp
void Deck::alignPhaseWithDeck(const Deck& target) {
    double offsetSamples = phaseAligner_.calculateOffset(*this, target);
    
    // Nudge playback position to correct phase
    // (instant jump for now, Phase 39 will add smooth ramping)
    size_t newPosition = currentFrame() - static_cast<size_t>(offsetSamples);
    seek(newPosition);
}
```

#### Files to Create
- `audio/PhaseAligner.h` (~80 lines)
- `audio/PhaseAligner.cpp` (~150 lines)
- `audio/PhaseAligner_Phase38_test.cpp` (~500 lines, 8 tests)

#### Files to Modify
- `audio/Deck.h` (add `alignPhaseWithDeck()` method)
- `audio/Deck.cpp` (implement alignment correction)
- `CMakeLists.txt` (add Phase 38 test target)

#### Tests (8)
1. `test_PhaseAligner_ZeroOffset` - Decks already in sync (offset = 0)
2. `test_PhaseAligner_PositiveOffset` - Deck B ahead by 0.25 beats
3. `test_PhaseAligner_NegativeOffset` - Deck B behind by 0.5 beats
4. `test_PhaseAligner_DifferentBPM` - 120 BPM vs 128 BPM (different beat lengths)
5. `test_PhaseAligner_NoBeatGrid` - Graceful fallback when beat grid missing
6. `test_Deck_AlignPhase` - Verify position adjustment after alignment
7. `test_PhaseAligner_LiveTracking` - Track phase drift during playback
8. `test_PhaseAligner_EdgeCases` - Loops, cue jumps, tempo changes

#### Success Criteria
- Phase offset calculation accuracy: ±5ms (imperceptible)
- Alignment works across 70-180 BPM range
- Handles beat grid edge cases (track start/end, loops)
- All 8 tests passing

---

### **Phase 39: Auto-Tempo Matching & Sync Lock**
**Goal:** Automatically match BPM and maintain phase lock during playback

#### Features
- `Deck::setAutoSyncTarget(Deck* target)` method enables sync lock
- Auto-adjust tempo to match target deck BPM
- Smooth tempo ramping (reuse existing `setTempoRampEnabled()`)
- "Sync Lock" mode maintains phase during playback
- Beat jump controls (±4 beats) during sync
- Keyboard: `S` key toggles sync, MIDI: sync button integration

#### Implementation Details
**Sync State Machine:**
```cpp
enum class SyncState {
    Off,           // Manual control
    Initializing,  // Matching BPM + aligning phase
    Locked,        // Sync active, maintaining phase
    Drifting       // Phase drifting (user intervention needed)
};

class SyncController {
public:
    void update(Deck& syncDeck, const Deck& targetDeck);
    void enableSync(Deck& syncDeck, Deck* targetDeck);
    void disableSync();
    bool isLocked() const;
    
private:
    SyncState state_ = SyncState::Off;
    Deck* targetDeck_ = nullptr;
    double lastPhaseCheck_ = 0.0;
};
```

**Auto-Tempo Algorithm:**
```cpp
void SyncController::update(Deck& syncDeck, const Deck& targetDeck) {
    if (state_ == SyncState::Off) return;
    
    // 1. Match BPM (if needed)
    float targetBPM = targetDeck.getEffectiveBPM();
    float currentBPM = syncDeck.getEffectiveBPM();
    float bpmDelta = targetBPM - currentBPM;
    
    if (std::abs(bpmDelta) > 0.1f) {
        // Calculate required tempo % adjustment
        float requiredTempo = (targetBPM / syncDeck.getBPM() - 1.0f) * 100.0f;
        syncDeck.setTargetTempo(requiredTempo);  // Ramp smoothly
        state_ = SyncState::Initializing;
        return;
    }
    
    // 2. Check phase lock (every 4 beats)
    double currentTime = syncDeck.currentTimeSeconds();
    if (currentTime - lastPhaseCheck_ >= 4.0 * (60.0 / targetBPM)) {
        double phaseOffset = phaseAligner_.calculateOffset(syncDeck, targetDeck);
        
        if (std::abs(phaseOffset) > 50.0) {  // >50 samples = drifting
            state_ = SyncState::Drifting;
            // Micro-adjust tempo to pull phase back (0.01% nudge)
            float microAdjust = (phaseOffset > 0) ? -0.01f : +0.01f;
            syncDeck.setTempoPercent(syncDeck.getTempoPercent() + microAdjust);
        } else {
            state_ = SyncState::Locked;
        }
        
        lastPhaseCheck_ = currentTime;
    }
}
```

**Beat Jump (during sync):**
```cpp
void Deck::beatJump(int beats) {
    if (!beatGrid_.isValid()) return;
    
    // Calculate target position (N beats ahead/behind)
    double currentBeat = beatGrid_.getCurrentBeat(currentFrame());
    double targetBeat = currentBeat + beats;
    auto timestamps = beatGrid_.getBeatTimestamps();
    
    // Find closest beat timestamp
    size_t targetFrame = static_cast<size_t>(targetBeat * 60.0 / bpm_ * sampleRate_);
    seek(targetFrame);
    
    // Phase re-alignment happens in next sync update cycle
}
```

#### Files to Create
- `audio/SyncController.h` (~100 lines)
- `audio/SyncController.cpp` (~200 lines)
- `audio/SyncController_Phase39_test.cpp` (~600 lines, 8 tests)

#### Files to Modify
- `audio/Deck.h` (add sync methods, beat jump)
- `audio/Deck.cpp` (implement sync state)
- `src/main.cpp` (add `S` key handler, beat jump keys)
- `input/InputMapper.h/cpp` (add ToggleSync, BeatJumpForward/Back commands)

#### Tests (8)
1. `test_AutoSync_BPMMatch` - Deck B matches Deck A BPM automatically
2. `test_AutoSync_PhaseHold` - Sync lock maintains phase for 5+ minutes
3. `test_AutoSync_SyncRelease` - Disable sync returns to manual control
4. `test_AutoSync_TempoRamp` - Smooth BPM transition (not instant jump)
5. `test_AutoSync_MIDIButton` - MIDI controller sync button triggers
6. `test_AutoSync_BeatJump` - +4/-4 beat jump during sync maintains phase
7. `test_AutoSync_DriftCorrection` - Auto-correct phase drift <0.1%
8. `test_AutoSync_DisableOnManualNudge` - Sync off if user adjusts tempo

#### Success Criteria
- BPM matching: ±0.05 BPM accuracy
- Phase lock stability: <10ms drift over 5 minutes
- Tempo ramp: 2-5 second smooth transition (no audible jumps)
- Beat jump: maintains phase alignment after jump
- All 8 tests passing

---

### **Phase 40: Transition Coaching & Phrasing**
**Goal:** Provide real-time guidance on when and how to transition between tracks

#### Features
- Phrase boundary detection (8/16/32 bar structure)
- Transition timing suggestions with visual countdown
- Energy/Camelot context integration (warn if energy clash or incompatible keys)
- Coaching HUD overlay (terminal ASCII, color-coded timing bars)
- Confidence scoring (rate suggestion quality 0-100%)
- Optional coaching mode (can be disabled for advanced users)

#### Implementation Details
**Phrase Structure Analysis:**
```cpp
class TransitionCoach {
public:
    struct Suggestion {
        double timestamp;          // When to mix (in seconds)
        std::string reason;        // "16-bar phrase end" or "Energy boost zone"
        float confidence;          // 0.0-1.0 (how confident is this suggestion?)
        bool urgency;              // True if window closing soon
    };
    
    std::vector<Suggestion> analyzeTransitions(
        const Deck& deckA, const Deck& deckB,
        const EnergyCurve& energyCurve,
        const CamelotAnalyzer& camelot);
};
```

**Phrase Detection Logic:**
```cpp
std::vector<double> TransitionCoach::detectPhrases(const BeatGrid& grid) {
    auto beats = grid.getBeatTimestamps();
    std::vector<double> phrases;
    
    // Standard DJ phrasing: every 16 or 32 beats
    for (size_t i = 0; i < beats.size(); i += 16) {
        if (i > 0 && (i % 32 == 0)) {
            // Major phrase boundary (32 bars = ~1 minute @ 128 BPM)
            phrases.push_back(beats[i]);
        } else if (i % 16 == 0) {
            // Minor phrase boundary (16 bars = ~30 seconds @ 128 BPM)
            phrases.push_back(beats[i]);
        }
    }
    
    return phrases;
}
```

**Transition Suggestion Algorithm:**
```cpp
TransitionCoach::Suggestion TransitionCoach::suggestNextTransition(
    const Deck& outgoing, const Deck& incoming,
    const EnergyCurve& curve, const CamelotAnalyzer& camelot) {
    
    Suggestion suggestion;
    
    // 1. Find next phrase boundary in outgoing track
    auto phrases = detectPhrases(outgoing.getBeatGrid());
    double currentPos = outgoing.currentTimeSeconds();
    double nextPhrase = findNextPhrase(phrases, currentPos);
    
    // 2. Check energy context
    float currentEnergy = curve.getCurrent();
    float incomingEnergy = incoming.getEnergyRating() / 10.0f;  // 1-10 → 0.1-1.0
    float energyDelta = incomingEnergy - currentEnergy;
    
    // 3. Check harmonic compatibility
    std::string keyA = outgoing.getKey();  // e.g., "8A"
    std::string keyB = incoming.getKey();  // e.g., "8B"
    float harmonicScore = camelot.getCompatibilityScore(keyA, keyB);
    
    // 4. Calculate confidence
    suggestion.timestamp = nextPhrase;
    suggestion.confidence = harmonicScore * 0.6f + (1.0f - std::abs(energyDelta)) * 0.4f;
    
    // 5. Generate coaching message
    if (harmonicScore < 0.5f) {
        suggestion.reason = "⚠ Key mismatch: " + keyA + " → " + keyB + " (consider different track)";
        suggestion.confidence *= 0.5f;  // Lower confidence for bad key match
    } else if (energyDelta > 0.3f) {
        suggestion.reason = "Energy boost zone (+" + std::to_string(int(energyDelta*100)) + "%)";
    } else if (energyDelta < -0.3f) {
        suggestion.reason = "Energy drop zone (" + std::to_string(int(energyDelta*100)) + "%)";
    } else {
        suggestion.reason = "16-bar phrase end (smooth transition)";
    }
    
    // 6. Check urgency (< 8 beats away = urgent)
    double timeToPhrase = nextPhrase - currentPos;
    double beatsToPhrase = timeToPhrase / (60.0 / outgoing.getEffectiveBPM());
    suggestion.urgency = (beatsToPhrase < 8.0);
    
    return suggestion;
}
```

**Coaching HUD (Terminal ASCII):**
```
┌─────────────────────────────────────────────────────────────────┐
│ TRANSITION COACH                                                │
├─────────────────────────────────────────────────────────────────┤
│ Suggestion: 16-bar phrase end (smooth transition)              │
│ Confidence: 85%                                                 │
│ Timing:     [████████████────────] 8 beats (5.2s)              │
│ Energy:     7.5 → 8.2 (+7% boost) ✓                            │
│ Harmonic:   8A → 8B (Relative Minor) ✓                         │
│ Quality:    89/100 (Excellent)                                 │
└─────────────────────────────────────────────────────────────────┘
```

#### Files to Create
- `gameplay/TransitionCoach.h` (~120 lines)
- `gameplay/TransitionCoach.cpp` (~300 lines)
- `visuals/CoachingHUD.h` (~60 lines)
- `visuals/CoachingHUD.cpp` (~180 lines)
- `gameplay/TransitionCoach_Phase40_test.cpp` (~700 lines, 8 tests)

#### Files to Modify
- `src/main.cpp` (integrate coaching HUD, toggle with `C` key)
- `input/InputMapper.h/cpp` (add ToggleCoaching command)

#### Tests (8)
1. `test_TransitionCoach_PhraseDetection` - Identify 16/32 bar phrases
2. `test_TransitionCoach_TransitionWindow` - Suggest "8 bars until phrase end"
3. `test_TransitionCoach_EnergyContext` - Factor in energy delta
4. `test_TransitionCoach_HarmonicCheck` - Warn if keys incompatible
5. `test_TransitionCoach_Countdown` - Display "3... 2... 1... MIX NOW!"
6. `test_TransitionCoach_ConfidenceScore` - Rate 0-100% based on context
7. `test_TransitionCoach_Urgency` - Flag urgent transitions (<8 beats)
8. `test_CoachingHUD_Render` - Terminal ASCII overlay formatting

#### Success Criteria
- Phrase detection: 90%+ accuracy (standard 16/32 bar structure)
- Suggestion relevance: 80%+ subjective quality (tested with real mixes)
- Timing accuracy: ±1 beat window
- HUD render: <1ms (negligible overhead)
- All 8 tests passing

---

### **Phase 41: Beat Jump, Warp Grid & Polish**
**Goal:** Advanced sync features and workflow integration

#### Features
- Beat jump controls (±1/4/8 beats) mapped to keyboard/MIDI
- Temporary BPM warp (micro-adjust ±0.02% without breaking sync)
- Cue jump maintains phase (hotspot jumps don't desync)
- Sync undo system (restore pre-sync state)
- Visual sync indicators (show "SYNC" badge, phase meter)
- MIDI controller integration (jogwheel, sync button, beat jump pads)
- Session state persistence (save/load sync settings)

#### Implementation Details
**Beat Jump with Configurable Size:**
```cpp
void Deck::beatJump(int beats) {
    if (!beatGrid_.isValid()) return;
    
    // Support variable jump sizes: ±1, ±4, ±8, ±16 beats
    double currentBeat = beatGrid_.getCurrentBeat(currentFrame());
    double targetBeat = std::clamp(currentBeat + beats, 0.0, beatGrid_.size());
    
    // Seek to target beat position
    double timestamp = beatGrid_.getBeatTimestamp(static_cast<int>(targetBeat));
    seek(static_cast<size_t>(timestamp * sampleRate_));
    
    // If sync enabled, trigger phase re-alignment in next update
    if (syncController_.isEnabled()) {
        syncController_.requestPhaseUpdate();
    }
}
```

**BPM Warp (Micro-Tune):**
```cpp
void Deck::applyWarp(float warpPercent) {
    // Warp is temporary BPM adjustment (±0.02% typical)
    // Does NOT break sync lock (treated as fine-tuning)
    warpAmount_ = std::clamp(warpPercent, -0.05f, +0.05f);
    
    // Effective tempo = base tempo + warp
    float effectiveTempo = tempoPercent_ + warpAmount_;
    applyTempoAdjustment(effectiveTempo);
}

void Deck::clearWarp() {
    warpAmount_ = 0.0f;
    applyTempoAdjustment(tempoPercent_);
}
```

**Visual Sync Indicator:**
```cpp
std::string renderSyncIndicator(const Deck& deck, const SyncController& sync) {
    if (!sync.isEnabled()) return "";
    
    std::ostringstream oss;
    oss << "[SYNC";
    
    switch (sync.getState()) {
    case SyncState::Initializing:
        oss << " ⟳ MATCHING"; break;
    case SyncState::Locked:
        oss << " ✓ LOCKED"; break;
    case SyncState::Drifting:
        oss << " ⚠ DRIFT"; break;
    default:
        break;
    }
    
    // Show phase meter: [====|====] (center = in phase)
    //                    ← behind | ahead →
    double phaseOffset = sync.getPhaseOffset();  // -0.5 to +0.5 beats
    int position = static_cast<int>((phaseOffset + 0.5) * 20);  // 0-20 scale
    oss << " [";
    for (int i = 0; i < 20; ++i) {
        if (i == 10) oss << "|";  // Center line
        else if (i == position) oss << "█";
        else oss << (i == 10 ? "─" : " ");
    }
    oss << "]]";
    
    return oss.str();
}
```

**Sync Undo System:**
```cpp
class SyncUndoStack {
public:
    void captureState(const Deck& deck);
    void undo(Deck& deck);
    bool canUndo() const;
    
private:
    struct SyncSnapshot {
        float tempoPercent;
        size_t playbackPosition;
        bool syncEnabled;
    };
    std::deque<SyncSnapshot> undoStack_;  // Max 10 entries
};
```

#### Files to Create
- `audio/SyncUndoStack.h` (~50 lines)
- `audio/SyncUndoStack.cpp` (~80 lines)
- `visuals/SyncIndicator.h` (~40 lines)
- `visuals/SyncIndicator.cpp` (~120 lines)
- `audio/BeatJump_Phase41_test.cpp` (~600 lines, 8 tests)

#### Files to Modify
- `audio/Deck.h` (add beatJump, warp methods)
- `audio/Deck.cpp` (implement beat jump + warp)
- `src/main.cpp` (integrate sync indicators, undo, beat jump keys)
- `input/InputMapper.h/cpp` (BeatJump+1/+4/+8, Warp+/-, SyncUndo commands)
- `audio/SessionState.h` (add sync state to auto-save)

#### Tests (8)
1. `test_BeatJump_VariableSizes` - Jump ±1/4/8/16 beats correctly
2. `test_BeatJump_MaintainsPhase` - Phase preserved after jump (when sync enabled)
3. `test_Warp_MicroTune` - ±0.02% BPM adjustment without breaking sync
4. `test_CueJump_PhasePreserve` - Cue hotspot jump doesn't desync
5. `test_SyncUndo_RestoreState` - Ctrl+Z restores pre-sync tempo/position
6. `test_SyncIndicator_Visual` - SYNC badge + phase meter rendering
7. `test_BeatJump_MIDI` - MIDI controller jogwheel triggers beat jump
8. `test_SessionState_SyncPersistence` - Save/load sync settings

#### Success Criteria
- Beat jump accuracy: lands exactly on target beat
- Warp range: ±0.05% max (prevents extreme misuse)
- Undo stack: 10 operations (matches beat grid editor pattern)
- Phase meter: visual accuracy ±0.1 beats
- MIDI latency: <5ms for beat jump response
- All 8 tests passing

---

## Integration Status

### Dependencies (All Satisfied ✅)
- **BPM Detection** (Arc I Phase 10) → Used for tempo matching
- **Beat Grid** (Arc VIII Phase 32, Arc IX Phase 36) → Used for phase alignment
- **Tempo Control** (Arc I/IV) → Used for auto-tempo adjustment
- **Onset Detection** (Arc VIII Phase 32) → Used for phrase boundary detection
- **Energy Analysis** (Arc VIII Phase 33, Arc IX Phase 37) → Used for transition coaching
- **Camelot Wheel** (Arc VIII Phase 31) → Used for harmonic coaching
- **MIDI Controllers** (Arc II Phase 13) → Used for sync button mapping
- **Session State** (Arc VII Phase 26) → Used for sync persistence

### Files to Create (13 files)
**Headers (7):**
- audio/PhaseAligner.h
- audio/SyncController.h
- audio/SyncUndoStack.h
- gameplay/TransitionCoach.h
- visuals/CoachingHUD.h
- visuals/SyncIndicator.h

**Implementations (6):**
- audio/PhaseAligner.cpp
- audio/SyncController.cpp
- audio/SyncUndoStack.cpp
- gameplay/TransitionCoach.cpp
- visuals/CoachingHUD.cpp
- visuals/SyncIndicator.cpp

**Tests (4):**
- audio/PhaseAligner_Phase38_test.cpp (8 tests)
- audio/SyncController_Phase39_test.cpp (8 tests)
- gameplay/TransitionCoach_Phase40_test.cpp (8 tests)
- audio/BeatJump_Phase41_test.cpp (8 tests)

### Files to Modify (7 files)
- audio/Deck.h/cpp (alignPhaseWithDeck, beatJump, warp methods)
- src/main.cpp (sync controls, coaching HUD, sync indicators)
- input/InputMapper.h/cpp (ToggleSync, BeatJump, ToggleCoaching, Warp commands)
- audio/SessionState.h (persist sync settings)
- CMakeLists.txt (4 new test targets)

### Keyboard Controls Added
- `S`: Toggle sync lock
- `Shift+S`: Force phase re-alignment
- `C`: Toggle coaching HUD
- `[` or `]`: Beat jump -4/+4 beats (remapped from crossfade)
- `Ctrl+[` or `Ctrl+]`: Beat jump -1/+1 beats
- `Shift+[` or `Shift+]`: Beat jump -8/+8 beats
- `-` or `=`: BPM warp -0.01%/+0.01% (overlaps with beat grid nudge - TBD conflict resolution)
- `Shift+U`: Sync undo

---

## Testing Strategy

### Test-Driven Development (TDD)
All 4 phases follow proven Arc IX TDD workflow:
1. **Write 8 tests per phase FIRST** (define success criteria)
2. **Implement minimal code to pass tests**
3. **Refactor for performance/readability**
4. **Validate integration** (run all phase tests + main app)

### Test Categories

#### Unit Tests (28 tests)
- Phase alignment math (offset calculation, normalization)
- Sync state machine (transitions: Off → Initializing → Locked → Drifting)
- Tempo matching (BPM adjustment, ramp smoothing)
- Beat jump (position calculation, phase preservation)
- Phrase detection (16/32 bar boundary identification)
- Coaching logic (confidence scoring, energy context)

#### Integration Tests (4 tests)
- MIDI controller (sync button triggers sync lock)
- Session state (save/load sync settings)
- Beat grid nudge (handle overlap with tempo warp keys)
- Main loop (coaching HUD rendering at 60 FPS)

### Performance Benchmarks
- **Phase alignment**: <1ms calculation time (sample accurate)
- **Sync update**: <0.5ms per audio block (negligible overhead)
- **Coaching analysis**: <5ms per update (1 Hz acceptable)
- **HUD render**: <1ms (60 FPS safe)

### Edge Case Coverage
- Missing beat grids (graceful fallback)
- BPM extremes (70 BPM vs 180 BPM sync)
- Loop playback during sync (maintain phase lock)
- Cue jump during sync (re-align phase)
- Tempo ramping + sync simultaneously
- Sync between decks with different sample rates

---

## User Experience

### Beginner Workflow
1. Load two tracks (auto-detected BPM/key from track metadata)
2. Press `S` on deck B → **Auto-sync to deck A** (instant beatmatch)
3. Press `C` → **Coaching HUD appears** showing "16 bars until transition"
4. Wait for countdown → **"3... 2... 1... MIX NOW!"** prompt
5. Move crossfader → **Smooth transition with perfect phase alignment**

**Learning:** Beginner sees what "in phase" feels like, learns phrase structure through visual feedback.

### Intermediate Workflow
1. Load tracks, manually beatmatch (traditional DJ skill)
2. Enable coaching (`C` key) → **See suggested transition windows**
3. Compare manual timing vs coach suggestions → **Learn optimal phrasing**
4. Use sync as **safety net** (if manual beatmatch drifts, press `S` to re-lock)

**Learning:** Intermediate DJ accelerates skill development by comparing their instincts to coach suggestions.

### Advanced Workflow
1. Manual beatmatch (sync disabled)
2. Use beat jump for phrase-aligned cue points (±4 beats lands on phrase boundary)
3. Enable sync temporarily during complex transitions (3-deck mix, layered loops)
4. Coach shows energy/harmonic warnings but DJ overrides ("I know this will work")

**Learning:** Advanced DJ uses sync as **workflow efficiency tool**, not a crutch.

---

## Professional Feature Parity

| Feature | Serato | Traktor | Rekordbox | DJ-ROOFRAT (Arc X) |
|---------|--------|---------|-----------|---------------------|
| **Auto-BPM Match** | ✅ Sync button | ✅ Sync button | ✅ Sync button | ✅ `S` key / MIDI sync |
| **Phase Alignment** | ✅ Master clock | ✅ Beatgrid sync | ✅ Beat sync | ✅ PhaseAligner class |
| **Beat Jump** | ✅ +/- 1/4/8 beats | ✅ Jogwheel | ✅ Beat jump pads | ✅ Keyboard + MIDI pads |
| **Sync Lock** | ✅ Master/slave | ✅ Tempo master | ✅ Quantize | ✅ SyncController |
| **Transition Coaching** | ⚠ Basic cue hints | ⚠ Remix deck hints | ❌ None | ✅ Full phrase analysis |
| **Energy Context** | ❌ | ❌ | ❌ | ✅ EnergyCurve integration |
| **Harmonic Warnings** | 🟡 Manual check | 🟡 Manual check | 🟡 Key display | ✅ Automatic Camelot warnings |

**Unique Advantages:**
- **Coaching system is industry-first** (no DJ software has real-time phrasing suggestions)
- **Energy + Harmonic context** (leverages Arc VIII/IX analysis for smarter suggestions)
- **Transparent phase meter** (shows exact beat offset, not just "in sync" LED)
- **Undo system** (restore pre-sync state if sync fails)

---

## Implementation Timeline

**Estimated Effort:** 1-2 weeks (using Conductor workflow)

| Phase | Focus | Estimated Days | Risk |
|-------|-------|----------------|------|
| 38: Phase Alignment | Core sync math | 1-2 days | Low |
| 39: Auto-Tempo & Sync Lock | State machine + tempo ramp | 2-3 days | Low |
| 40: Transition Coaching | Phrase analysis + HUD | 2-3 days | Medium |
| 41: Beat Jump & Polish | Workflow features | 1-2 days | Low |

**Parallel Work Opportunities:**
- Phase 38-39 focus on audio engine (PhaseAligner, SyncController)
- Phase 40-41 focus on UI/coaching (TransitionCoach, CoachingHUD)
- Can implement UI scaffolding during Phase 38-39 (reduces Phase 40 time)

**Dependencies:**
- No external libraries needed (all C++20 standard library)
- No architectural changes (extends existing Deck/BeatGrid classes)
- Can start immediately (all prerequisites exist)

---

## Success Metrics

### Phase Completion Criteria
- **Phase 38:** Phase alignment <5ms accuracy, 8/8 tests passing
- **Phase 39:** Sync lock stable for 5+ minutes, 8/8 tests passing
- **Phase 40:** Coaching suggestions 80%+ relevance, 8/8 tests passing
- **Phase 41:** Beat jump accurate to target beat, 8/8 tests passing

### Arc X Success Criteria
✅ **Functional:**
- 32/32 tests passing (100% coverage)
- All sync features working in main application
- MIDI controller integration functional

✅ **Performance:**
- Phase alignment: <1ms calculation
- Sync update: <0.5ms per block
- Coaching analysis: <5ms per update
- HUD render: <1ms (60 FPS safe)

✅ **Quality:**
- BPM matching: ±0.05 BPM accuracy
- Phase lock: <10ms drift over 5 minutes
- Phrase detection: 90%+ accuracy
- Coaching relevance: 80%+ subjective quality

✅ **User Experience:**
- Beginners can beatmatch instantly with sync button
- Coaching provides actionable timing guidance
- Advanced users can toggle all features off
- Sync never "fights" manual control (disable on user intervention)

---

## Alternatives Considered

### Alternative 1: Simple Tempo Sync Only (No Phase Alignment)
**Pros:** Easier to implement (just match BPM)  
**Cons:** Tracks still drift apart (phase misalignment destroys mix)  
**Decision:** Rejected (phase alignment is core requirement for usable sync)

### Alternative 2: Full AI-Powered Mix Automation
**Pros:** "Press play and walk away" experience  
**Cons:** Removes all DJ skill/creativity, technically infeasible  
**Decision:** Rejected (Arc X assists, doesn't replace human DJ)

### Alternative 3: Hardware-Only Sync (MIDI Controller Exclusive)
**Pros:** Mirrors professional DJ hardware workflow  
**Cons:** Excludes keyboard users, harder to test  
**Decision:** Rejected (keyboard + MIDI both supported)

### Alternative 4: Coaching as Separate App Arc
**Pros:** Could focus on coaching standalone  
**Cons:** Coaching without sync is less useful (suggestions but no auto-correction)  
**Decision:** Rejected (sync + coaching together provide complete workflow)

---

## Risks & Mitigations

### Risk 1: Phase Drift During Long Mixes 🟡 **Medium**
**Scenario:** BPM detection inaccuracies cause sync to drift over 5+ minutes  
**Mitigation:**
- Implement periodic phase checks (every 4 beats)
- Micro-adjust tempo (±0.01%) to pull phase back into alignment
- Show drift warning in sync indicator ("DRIFT" state)
- Allow manual phase nudge (Shift+S forces re-alignment)

### Risk 2: Beat Grid Edge Cases 🟡 **Medium**
**Scenario:** Tracks without beat grids, intro/outro sections, live recordings  
**Mitigation:**
- Graceful fallback: disable sync if beat grid missing
- Onset detection fallback (use spectral flux when beat grid unreliable)
- Manual override always available (disable sync instantly)

### Risk 3: Coaching False Positives 🟢 **Low**
**Scenario:** Coach suggests bad transitions (wrong phrase boundaries, key mismatch)  
**Mitigation:**
- Confidence scoring (0-100%) shows suggestion quality
- Warnings for low-confidence suggestions ("⚠ Key mismatch detected")
- Coach is **advisory only** (never forces transition)
- Can be disabled entirely (`C` key toggle)

### Risk 4: Key Mapping Conflicts 🟢 **Low**
**Scenario:** Beat grid nudge uses `-/=` keys, tempo warp also wants `-/=`  
**Mitigation:**
- Use modifier keys (Ctrl+- for warp, plain - for beat grid)
- Document key mappings clearly in help text
- MIDI controller avoids conflicts (dedicated sync/warp buttons)

### Risk 5: Real-time Performance 🟢 **Low**
**Scenario:** Sync updates cause audio glitches or dropped frames  
**Mitigation:**
- Phase alignment is one-time calculation (<1ms)
- Sync update runs per-block (512 samples = 11.6ms @ 44.1kHz)
- Coaching runs at 1 Hz (5ms budget, plenty of headroom)
- Performance benchmarks in test suite catch regressions

---

## Post-Arc X Roadmap

**Arc XI: Loop Sampler** (natural follow-up)
- Complements sync (sync-locked loops layer over beatmatched tracks)
- Reuses tempo/phase infrastructure from Arc X
- Extends multi-bank pattern (beat grid editor had 50-op undo, loop sampler has 4-bank slots)

**Arc XII: Advanced Dynamics** (completes audio processing)
- Sidechain compression benefits from Arc X (sync-locked decks enable automatic ducking)
- Master limiter ensures coached transitions don't clip
- Multiband compression for mastered output quality

**Arc XIII: Full Graphics Mode** (complete Arc IV deferred work)
- Sync lock visualized in 3D (beatgrid alignment shown as overlapping patterns)
- Coaching HUD in graphics mode (3D timing windows, energy flow visualization)
- Lighting rig reacts to sync state (strobe on phase lock, pulse on coaching countdown)

---

## Open Questions

1. **Sync Button Modifier:** Should sync be single-press toggle (current plan) or hold-to-enable (like Serato)?  
   **Recommendation:** Toggle (easier for keyboard users, MIDI can map either way)

2. **Coaching Frequency:** 1 Hz updates sufficient or need real-time (<100ms)?  
   **Recommendation:** 1 Hz (coaching is advisory, not critical path)

3. **Beat Jump Key Mapping:** Conflict with crossfade (`[` and `]` keys)?  
   **Recommendation:** Remap crossfade to `Ctrl+[/]`, beat jump gets unmodified `[/]`

4. **Sync Master/Slave:** Should one deck be designated "master" (others sync to it)?  
   **Recommendation:** Yes - deck A is default master, configurable via `Shift+S` (swap master)

5. **Coaching in Multiplayer:** Should coaching work in battle mode (Arc VI)?  
   **Recommendation:** Yes - coaching gives beginner advantage in hotseat battles (balancing mechanism)

---

## Conclusion

**Arc X: Auto-Sync & Coaching System** is the **highest-value, lowest-risk** arc available. It leverages 95% of existing infrastructure (BPM detection, beat grids, energy analysis) to deliver immediate workflow improvements for DJs of all skill levels. The 4-phase plan is well-defined, technically feasible, and delivers professional-grade features that match or exceed commercial DJ software.

**Ready to begin Phase 38 implementation.**

---

**Questions or approval needed before proceeding?**
