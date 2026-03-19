# DJ-ROOFRAT

A C++20 terminal DJ simulator with dual CDJ-style decks, real-time spectrum analysis, crowd AI, beat-sync, effects, and career progression — built across 41 implementation phases.

## Features

- **Dual Deck Playback** — Independent CDJ-style decks with sample-accurate playback, slip mode, vinyl simulation, and scratch control
- **Real-Time Spectrum Analysis** — FFT-based 20-band spectrum analyzer (20Hz–20kHz) with bass clash detection
- **BPM Detection & Beat Grid** — Autocorrelation BPM estimation, manual beat grid editor with nudge/snap tools
- **Phase Alignment & Auto-Sync** — Beat-synchronizes two decks with smooth BPM matching and phase lock (< 10ms drift)
- **3-Band EQ + Effects** — Per-deck low/mid/high EQ, isolators, filter sweeps, plus Reverb, Delay, Flanger, Phaser, Bitcrusher, Ring Modulator, Auto-Filter effect chains
- **Mixing & Transition Coach** — Equal-power crossfader, AI transition suggestions with phrase detection and harmonic compatibility (Camelot Wheel)
- **Crowd AI** — State machine crowd mood system (Unimpressed → Hyped) driven by BPM match, transition smoothness, and track energy
- **Career Progression** — Tier-based career with venue unlocks, reputation system, achievements, leaderboards
- **Recording & Export** — Ring-buffer recorder with WAV export and custom filename support
- **Session Autosave** — JSON session persistence with auto-save every 120 seconds
- **Battle Mode** — 2-player mode with separate keyboard zones (Player 2: arrow keys + numpad)
- **Terminal HUD** — ASCII waveform visualization, spectrum display, beat grid, coaching overlay, energy curve, sync indicators

## Requirements

- Windows 10/11
- Visual Studio 2022 (C++ workload)
- CMake 3.20+

Optional (for real audio I/O and file loading):
- PortAudio — realtime audio playback
- libsndfile — WAV/MP3 file loading
- LAME — MP3 export

Without optional dependencies, the app runs with generated test tones in silent mode.

## Build

```powershell
cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64
cmake --build build-vs --config Debug
```

## Run

```powershell
# No audio output (headless / CI)
.\build-vs\Debug\DJ-ROOFRAT.exe --no-audio

# With optional track files
.\build-vs\Debug\DJ-ROOFRAT.exe path\to\deckA.wav path\to\deckB.wav
```

## Keyboard Controls (Quick Reference)

| Key | Action |
|-----|--------|
| `A` | Play/Pause Deck A |
| `B` | Play/Pause Deck B |
| `Z` / `X` | Tempo nudge Deck A up/down |
| `I` / `K` | Tempo nudge Deck B up/down |
| `\` | Crossfade center |
| `Q`/`W` `E`/`R` `T`/`Y` | Deck A Low/Mid/High EQ |
| `D`/`F` `G`/`H` `J`/`N` | Deck B Low/Mid/High EQ |
| `[` / `]` | Beat jump Deck A −4 / +4 beats |
| `;` / `'` | Beat jump Deck B −4 / +4 beats |
| `Shift+S` / `Shift+D` | Toggle sync lock Deck A / B |
| `S` | Toggle recording |
| `Shift+V` | Save recording |
| `9` | Toggle spectrum display |
| `Shift+C` | Toggle coaching overlay |

See [docs/keyboard-reference.md](docs/keyboard-reference.md) for the complete keyboard reference.

## Documentation

| Document | Description |
|----------|-------------|
| [docs/developer-setup-guide.md](docs/developer-setup-guide.md) | Build from source, CMake options, troubleshooting |
| [docs/architecture-diagram.md](docs/architecture-diagram.md) | System architecture, module layers, data flows |
| [docs/user-manual.md](docs/user-manual.md) | Getting started, tutorials, career progression |
| [docs/keyboard-reference.md](docs/keyboard-reference.md) | Complete keyboard binding reference |
| [docs/api-reference.md](docs/api-reference.md) | API docs for 20+ core classes |
| [docs/data-formats.md](docs/data-formats.md) | JSON session schema, file format specs |
| [docs/performance-tuning.md](docs/performance-tuning.md) | Profiling guide, benchmark targets, optimization tips |

## Project Structure

```
audio/       Deck playback, BPM, spectrum, sync, effects, recorder
crowdAI/     Crowd state machine driven by mix quality metrics
gameplay/    Scoring, career, transition coaching, energy curve
input/       Keyboard command mapping (InputMapper)
visuals/     Terminal waveform, spectrum, beat grid, HUD rendering
core/        Session persistence (JSON save/load)
src/         Main runtime loop and module wiring (main.cpp)
tests/       Unit tests (per phase) and integration end-to-end tests
docs/        Developer and user documentation
plans/       Implementation phase plans and completion records
```

## Running Tests

Integration tests are in `build-vs/Debug/`:

```powershell
.\build-vs\Debug\SmokeTest_main.exe
.\build-vs\Debug\E2E_LoadMixRecord_test.exe
.\build-vs\Debug\E2E_AutoSync_test.exe
.\build-vs\Debug\E2E_MIDIEffects_test.exe
.\build-vs\Debug\E2E_BassClashDetection_test.exe
.\build-vs\Debug\E2E_SessionRoundTrip_test.exe
.\build-vs\Debug\InputMapper_Phase3_test.exe
```

> **Note:** On systems with Windows Application Control (WDAC) enabled, freshly-built executables may be blocked at launch. This is a machine-level security restriction, not a code issue. See [docs/developer-setup-guide.md](docs/developer-setup-guide.md) for workarounds.
