# DJ-ROOFRAT Simulator

C++ prototype for a DJ simulator with CDJ-style dual decks, mixer logic, crowd feedback, and visualization scaffolding.

## Implemented First Milestone

- Two independent decks that can load tracks (via `libsndfile`) or generated fallback tones.
- Simultaneous playback of Deck A and Deck B.
- Equal-power crossfade between decks.
- Crowd energy meter with simple reaction state machine.
- Basic waveform visualization in terminal output.

## Engine Foundations Included

- Deck controls: tempo adjustment, cue, loop config, slip mode toggles, vinyl mode flag.
- EQ and filter hooks: three-band EQ and a deck low-pass blend filter.
- BPM detection via envelope autocorrelation.
- Optional PortAudio playback wrapper.
- Gameplay scaffolding for `Career` and `Sandbox` progression/score loops.
- Input command parser for keyboard control extensions.

## Project Structure

```
/audio
/visuals
/crowdAI
/gameplay
/input
/assets
/src
```

## Build (Windows)

```powershell
cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64
cmake --build build-vs --config Debug
```

Run:

```powershell
.\build-vs\Debug\DJ-ROOFRAT.exe --no-audio
```

Optional track args:

```powershell
.\build-vs\Debug\DJ-ROOFRAT.exe path\\to\\deckA.wav path\\to\\deckB.wav
```

## Live Controls (Windows Console)

During playback, use keyboard controls for manual beatmatching and transitions:

- `[` crossfade left
- `\\` center crossfader
- `]` crossfade right
- `a` toggle play/pause Deck A
- `b` toggle play/pause Deck B
- `i` tempo nudge Deck B up (+0.25%)
- `k` tempo nudge Deck B down (-0.25%)
- `o` reset Deck B tempo to 0%
- `1` toggle Deck A 16-beat loop
- `2` set Deck A cue at current frame
- `3` jump Deck A to cue
- `l` toggle Deck B 16-beat loop
- `c` set Deck B cue at current frame
- `v` jump Deck B to cue
- Deck A EQ/Filter: `q/w` low -/+ , `e/r` mid -/+ , `t/y` high -/+ , `u/p` filter -/+
- Deck B EQ/Filter: `d/f` low -/+ , `g/h` mid -/+ , `j/n` high -/+ , `m/,` filter -/+
- `x` exit set early

This keeps beatmatching manual (no auto-sync). A target tempo offset hint is shown at startup.

## Optional Dependencies

- `PortAudio` for realtime playback
- `libsndfile` for audio file decoding

If dependencies are missing, the simulator still runs in silent mode using generated test tones.

## Next Steps

- Expand EQ/filter behavior and per-band isolator UX.
- Add loop controls and tempo/BPM interaction UI.
- Prototype crowd AI feedback loop tuning.
- Add OpenGL/DirectX stage prototype and lighting toggles.
