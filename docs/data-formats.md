# DJ-ROOFRAT Data Formats Reference

This document describes the persistent data formats used by DJ-ROOFRAT for saving sessions, beat grids, EQ presets, and recordings.

---

## Section 1: Session File Format (JSON)

Session files are saved by `SessionManager::saveSession()` in JSON format. They capture the complete state of both decks, mixing parameters, career progress, and crowd state for later restoration.

### Session JSON Schema

| Field Path | JSON Type | C++ Type | Default | Range/Notes | Description |
|---|---|---|---|---|---|
| `deckA` | Object | `DeckState` | — | — | State of deck A |
| `deckA.trackPath` | String | `std::string` | `""` | Valid file path | Path to loaded track file |
| `deckA.playbackPosition` | Number | `double` | `0.0` | ≥ 0 | Current playhead position in seconds |
| `deckA.tempoBend` | Number | `float` | `0.0` | 0.5–2.0 | Tempo adjustment multiplier |
| `deckA.isPlaying` | Boolean | `bool` | `false` | — | Current playback state |
| `deckA.lowGain` | Number | `float` | `1.0` | -∞ to +∞ (dB) | Low-band EQ gain |
| `deckA.midGain` | Number | `float` | `1.0` | -∞ to +∞ (dB) | Mid-band EQ gain |
| `deckA.highGain` | Number | `float` | `1.0` | -∞ to +∞ (dB) | High-band EQ gain |
| `deckB` | Object | `DeckState` | — | — | State of deck B (same structure as `deckA`) |
| `deckB.trackPath` | String | `std::string` | `""` | Valid file path | Path to loaded track file |
| `deckB.playbackPosition` | Number | `double` | `0.0` | ≥ 0 | Current playhead position in seconds |
| `deckB.tempoBend` | Number | `float` | `0.0` | 0.5–2.0 | Tempo adjustment multiplier |
| `deckB.isPlaying` | Boolean | `bool` | `false` | — | Current playback state |
| `deckB.lowGain` | Number | `float` | `1.0` | -∞ to +∞ (dB) | Low-band EQ gain |
| `deckB.midGain` | Number | `float` | `1.0` | -∞ to +∞ (dB) | Mid-band EQ gain |
| `deckB.highGain` | Number | `float` | `1.0` | -∞ to +∞ (dB) | High-band EQ gain |
| `crossfader` | Number | `float` | `0.0` | -1.0 to 1.0 | -1.0 = full A, 0.0 = center, 1.0 = full B |
| `currentCareerTier` | Integer | `int` | `1` | 1–10+ | Current career progression tier |
| `crowdEnergy` | Number | `float` | `0.5` | 0.0–1.0 | Last-recorded crowd energy level |
| `venueId` | String | `std::string` | `""` | Valid venue ID | Identifier of current venue |

### Complete Example Session JSON

```json
{
  "deckA": {
    "trackPath": "C:\\Music\\DeepHouse_A_128bpm.mp3",
    "playbackPosition": 45.5,
    "tempoBend": 1.05,
    "isPlaying": true,
    "lowGain": 2.5,
    "midGain": 0.0,
    "highGain": -1.2
  },
  "deckB": {
    "trackPath": "C:\\Music\\TechHouse_B_126bpm.mp3",
    "playbackPosition": 32.1,
    "tempoBend": 0.98,
    "isPlaying": false,
    "lowGain": 0.0,
    "midGain": 1.5,
    "highGain": 0.8
  },
  "crossfader": 0.0,
  "currentCareerTier": 3,
  "crowdEnergy": 0.72,
  "venueId": "warehouse_chicago"
}
```

### Loading and Saving

- **Save:** `SessionManager::saveSession(filePath, state)` → JSON file
- **Load:** `SessionManager::loadSession(filePath)` → `SessionState` object (or `std::nullopt` on error)
- **Extension:** `.json`
- **Encoding:** UTF-8

---

## Section 2: DeckState Fields

The `DeckState` structure represents the complete state of a single deck.

### DeckState Field Reference

| Field | Type | Default | Range | Description |
|---|---|---|---|---|
| `trackPath` | `std::string` | `""` | Valid filesystem path | Full or relative path to the loaded audio file; empty if no track is loaded |
| `playbackPosition` | `double` | `0.0` | ≥ 0 seconds | Playback position from the start of the track |
| `tempoBend` | `float` | `0.0` | 0.5–2.0 | Tempo adjustment factor (1.0 = original speed, 0.5 = half speed, 2.0 = double) |
| `isPlaying` | `bool` | `false` | — | True if the deck is currently playing; false if stopped or paused |
| `lowGain` | `float` | `1.0` | -∞ to +∞ dB (typically -12 to +12) | Gain applied to the low-frequency EQ band |
| `midGain` | `float` | `1.0` | -∞ to +∞ dB (typically -12 to +12) | Gain applied to the mid-frequency EQ band |
| `highGain` | `float` | `1.0` | -∞ to +∞ dB (typically -12 to +12) | Gain applied to the high-frequency EQ band |

### DeckState Usage in Session Restoration

When loading a session:

1. Both `deckA` and `deckB` are restored in parallel
2. Each deck loads its `trackPath` via `Deck::loadFromFile()`
3. Playback position is restored via `Deck::currentFrame()`
4. EQ settings are restored via `Deck::setEQ(lowGain, midGain, highGain)`
5. Tempo is restored via `Deck::setTempoPercent(tempoBend * 100.0f)`
6. Playback state `isPlaying` is used to determine whether to call `play()` or `stop()`

---

## Section 3: BeatGridData (Per-Track Metadata)

Beat grid data is embedded within track metadata files and persisted per track (not per session).

### BeatGridData Structure

| Field | Type | Description |
|---|---|---|
| `bpm` | `double` | Detected or user-entered BPM for the track |
| `firstBeatOffset` | `double` | Time offset in seconds from track start to first beat; allows compensation for lead-in audio |
| `nudges` | Array of `double` | Per-beat timing adjustments in milliseconds; allows precise beat correction via `BeatGrid::nudgeBeat()` |

### Conceptual Example

```json
{
  "bpm": 128.4,
  "firstBeatOffset": 0.156,
  "nudges": [
    0.0,
    -2.5,
    1.2,
    0.8,
    -0.4,
    ...
  ]
}
```

### Storage Location

- Beat grid data is typically stored within a `.metadata` sidecar file alongside the audio track, or embedded in a `TrackMetadata` database.
- When a track is loaded, `BeatGrid::fromData()` restores the grid if metadata is available.
- User adjustments via `BeatGrid::nudgeBeat()` and `BeatGrid::markPhrases()` are persisted to the metadata file.

### Persistence Workflow

1. **Auto-detection:** `BPMDetector::estimate()` → `BeatGrid::generateFromBPM()`
2. **User refinement:** `BeatGrid::nudgeBeat()` or `BeatGrid::snapToOnset()`
3. **Export:** `BeatGrid::toData()` → metadata file
4. **Restoration:** `BeatGrid::fromData()` ← metadata file

---

## Section 4: PresetConfig (EQ Presets)

EQ presets are accessible via hotkeys (F1–F12 in Phase 27) and store per-deck EQ settings.

### EQ Preset Structure

| Field | Type | Default | Range | Description |
|---|---|---|---|---|
| `presetIndex` | `int` | 1–12 | — | Hotkey number (F1 = preset 1, F12 = preset 12) |
| `deck` | `char` | `'A'` or `'B'` | — | Deck this preset applies to |
| `lowGain` | `float` | `0.0` | -12 to +12 dB | Gain for low frequencies |
| `midGain` | `float` | `0.0` | -12 to +12 dB | Gain for mid frequencies |
| `highGain` | `float` | `0.0` | -12 to +12 dB | Gain for high frequencies |

### Conceptual Example Preset List

```json
[
  {
    "presetIndex": 1,
    "deck": "A",
    "lowGain": 3.0,
    "midGain": -1.0,
    "highGain": 0.0,
    "name": "BassBoost"
  },
  {
    "presetIndex": 2,
    "deck": "B",
    "lowGain": 0.0,
    "midGain": 2.5,
    "highGain": -2.0,
    "name": "MidIsolate"
  },
  {
    "presetIndex": 3,
    "deck": "A",
    "lowGain": -12.0,
    "midGain": 0.0,
    "highGain": 0.0,
    "name": "HighPassFilter"
  }
]
```

### Loading and Selection

- **Hotkey action:** User presses F1–F12 (e.g., F5)
- **Command parsed:** `InputMapper::parseKey()` → Preset hotkey
- **Preset loaded:** `Deck::setEQ(lowGain, midGain, highGain)` applies saved gains
- **Storage location:** Typically `config/eq-presets.json` or embedded in main config

### Preset Persistence

Presets are user-defined and typically stored in a configuration file separate from session state, allowing them to be reused across multiple sessions.

---

## Section 5: File Naming Conventions

### Session Files

- **Naming Pattern:** User-specified or timestamped
- **Example Paths:**
  - `sessions/my_session_march19.json`
  - `backups/auto_save_20260319_143022.json` (auto-save with timestamp)
- **Extension:** `.json`
- **Encoding:** UTF-8

### Recording Exports

- **Naming Pattern Option 1:** User-specified via `Recorder::setExportFilename()`
  - Example: `my_mix.wav`, `session_edit_v2.wav`
- **Naming Pattern Option 2:** Auto-generated default timestamp
  - Pattern: `recording_YYYYMMDD_HHMMSS.wav`
  - Example: `recording_20260319_143022.wav` (March 19, 2026 @ 14:30:22)
- **Selection:** User-provided filename is preferred; fallback to timestamp if not set via `Recorder::getExportFilenameOrDefault(fallback)`
- **Extension:** `.wav`
- **Encoding:** PCM (standard WAV format)

### Beat Grid & Metadata Files

- **Naming Pattern:** Sidecar approach
  - Audio file: `track_name.mp3`
  - Metadata file: `track_name.metadata` or `track_name.json`
- **Extension:** `.metadata` or `.json` (project-dependent)
- **Storage:** Same directory as audio file or centralized metadata cache

### EQ Preset Files

- **Naming Pattern:** Centralized config
  - Path: `config/eq-presets.json` or `presets/eq-presets.json`
- **Extension:** `.json`
- **Encoding:** UTF-8

### Auto-Save Backups

- **Directory:** Configurable; typically `backups/` or `sessions/autosave/`
- **Naming Pattern:** `auto_save_YYYYMMDD_HHMMSS.json`
- **Retention:** User-defined (e.g., keep last 10 auto-saves)
- **Interval:** Configurable via `SessionManager::setAutoSaveInterval()` (default: 120 seconds)

---

## Appendix: File Format Summary Table

| Format | File Extension | Purpose | Primary Accessor | Persistence |
|---|---|---|---|---|
| Session (JSON) | `.json` | Save/restore deck state, career tier, crowd energy | `SessionManager` | User-specified or auto-save |
| Beat Grid (JSON/Metadata) | `.metadata`, `.json` | Store BPM, beat offsets, nudge history | `BeatGrid` | Per-track sidecar or database |
| EQ Preset (JSON) | `.json` | Store and recall 3-band EQ settings | Hotkey system (F1–F12) | `config/` or preset directory |
| Recording (WAV) | `.wav` | Export mixed audio | `Recorder` | User-specified or timestamp-based filename |
| Track Metadata | `.metadata` | Embed key, energy, camelot info | `TrackMetadata` | Track-specific sidecar file |

---

## Notes

- All JSON files use UTF-8 encoding with Unix-style line endings (`\n`) for cross-platform compatibility.
- File paths in JSON use forward slashes (`/`) or escaped backslashes (`\\`) for Windows paths.
- Floating-point values are stored as decimal numbers (e.g., `1.05`, not `1+5e-2`).
- Timestamps use ISO 8601 format where applicable (e.g., `2026-03-19T14:30:22Z` for UTC).
- Auto-saved files include a timestamp to prevent overwriting and enable session recovery workflows.
