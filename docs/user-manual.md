# DJ-ROOFRAT User Manual

## Introduction

**DJ-ROOFRAT** is a terminal-based DJ simulator that puts you behind the decks with real-time mixing, live sound design, and a dynamic career mode driven by crowd energy. Control two synchronized audio decks, apply effects and EQ, beatmatch tracks using a visual HUD, and progress through increasingly challenging gigs as you refine your mixing skills.

The game is designed for Windows and runs in a console window. Whether you're learning to DJ or just want a fun terminal experience, DJ-ROOFRAT combines the tactile feel of professional DJ hardware with accessibility and instant feedback.

### System Requirements
- **Operating System:** Windows 10 or later
- **Runtime:** Visual Studio 2022 x64 runtime or later
- **Audio:** (Optional) PortAudio installation for live audio; the app includes a test-tone fallback for headless operation

---

## Getting Started

### Launching the Application

The basic launch command is:
```
build-vs/Debug/DJ-ROOFRAT.exe --no-audio
```

#### Command-Line Options

| Option | Description |
|--------|-------------|
| `--no-audio` | Run without audio output (recommended for development/headless testing). Generates test tones instead. |
| `[trackA.wav]` | Path to an audio file to load in Deck A at startup. |
| `[trackB.wav]` | Path to an audio file to load in Deck B at startup. |

#### Examples
```
# Headless mode with no audio
build-vs/Debug/DJ-ROOFRAT.exe --no-audio

# Load two tracks
build-vs/Debug/DJ-ROOFRAT.exe track1.wav track2.wav

# Headless with tracks
build-vs/Debug/DJ-ROOFRAT.exe --no-audio track1.wav track2.wav
```

### Understanding the HUD

When DJ-ROOFRAT starts, the console displays:

- **Deck Status Bars** — Two rows showing playback progress for Deck A and Deck B. Each bar fills as the track plays.
- **BPM Display** — Current tempo for each deck, updated in real-time as you nudge or sync.
- **Crossfader Position** — Visual indicator showing mix position between Deck A (left) and Deck B (right).
- **Crowd Energy Meter** — Horizontal bar (0.0–1.0) reflecting your mix quality. Smooth beatmatching and harmonic choices raise energy.
- **Spectrum Analyzer** (optional) — Frequency visualization. Toggle with `9`.
- **Coaching Overlay** (optional) — Coaching tips displayed over the HUD. Toggle with `Shift+C`.

The HUD updates every frame to reflect your input in real-time.

---

## Tutorial 1 — Load & Play

This tutorial teaches you how to launch a track and start playback.

**Step 1:** Launch DJ-ROOFRAT with an audio file:
```
build-vs/Debug/DJ-ROOFRAT.exe track.wav
```
Or launch headless first:
```
build-vs/Debug/DJ-ROOFRAT.exe --no-audio
```

**Step 2:** You will see the HUD. Press `A` to **play Deck A**.  
→ The progress bar for Deck A should begin advancing.  
→ If you supplied a track file, you hear audio (or a test tone in headless mode).

**Step 3:** Watch the **BPM counter** update in the Deck A display as the track plays.

**Step 4:** Press `A` again to **pause Deck A**.  
→ The progress bar stops advancing.

**Step 5:** Press `A` once more to **resume playback**.

**Tip:** If no track is loaded, Deck A will play with a generated test tone so you can still explore the app without audio files.

---

## Tutorial 2 — Beatmatch & Crossfade

This tutorial teaches you how to sync two decks and blend them smoothly.

**Step 1:** Load two tracks (or launch in headless mode):
```
build-vs/Debug/DJ-ROOFRAT.exe track1.wav track2.wav
```

**Step 2:** Press `A` to **play Deck A**.  
→ Watch the Deck A progress bar and BPM counter.

**Step 3:** Press `B` to **play Deck B**.  
→ Now both decks are playing.

**Step 4:** **Manually beatmatch** by nudging Deck B's tempo to match Deck A:
- Press `I` to increase Deck B tempo.
- Press `K` to decrease Deck B tempo.
- Watch both BPM counters. The goal is to get them **as close as possible** (ideally within ±0.5 BPM).

**Step 5:** (Optional) **Enable Sync Lock** for semi-automatic tempo locking:
- Press `Shift+S` to lock Deck A sync.
- Press `Shift+D` to lock Deck B sync.
- Both decks will now maintain sync automatically.

**Step 6:** **Crossfade** from Deck A to Deck B:
- Press `\` to **center the crossfader**.  
→ You now hear a balanced blend of both decks.
- Text commands `xf left` and `xf right` let you move the crossfader in text mode.

**Step 7:** Watch the **Crowd Energy meter** increase as your beatmatch quality improves.  
→ Smooth, gapless transitions and harmonic mixing (using the Camelot wheel) boost energy.

**Tip:** Use the coaching overlay (`Shift+C`) for real-time mixing tips.

---

## Tutorial 3 — Record & Export

This tutorial teaches you how to capture your mix and save it as a file.

**Step 1:** Perform a mix (following Tutorial 2 above, or your own session).

**Step 2:** Press `S` to **start recording**.  
→ The HUD will show a recording indicator.

**Step 3:** Perform your mix: play, beatmatch, crossfade, apply effects, or any other actions.

**Step 4:** Press `S` again to **stop recording**.  
→ The mix is now captured in memory.

**Step 5:** (Optional) Press `Shift+N` to **set a custom filename** for the recording.  
→ Type a name (e.g., `my_mix_v1`) and press Enter.

**Step 6:** Press `Shift+V` to **save the recording** to a file.  
→ The file is saved to disk with a `.wav` extension.  
→ A confirmation message appears in the HUD.

**Tip:** Recordings are lossless `.wav` files. You can open them in any audio player or DJ software afterward.

---

## Career Progression

### Tiers & Crowd Energy

DJ-ROOFRAT features a **career tier system** that progresses based on crowd energy. Your current tier is displayed on the main HUD.

- **Tiers** range from 1 to N, with each tier representing increased difficulty and higher crowd expectations.
- **Crowd Energy** is a normalized value from 0.0 (crowd is unimpressed) to 1.0 (crowd is ecstatic).
- **Tier Advancement** occurs when crowd energy exceeds a threshold for your current tier. Reach the energy target, and you advance to the next tier with higher expectations.

### What Raises Crowd Energy?

1. **Precise Beatmatching** — Keeping both decks' BPM within 0.5 of each other significantly boosts energy per frame of mix time.
2. **Harmonic Mixing** — Using the Camelot wheel (Key/key detection), mix tracks that are harmonically compatible. The system automatically awards energy bonuses when you mix harmonically adjacent tracks.
3. **Smooth Transitions** — Gradual crossfades and gapless playback reward sustained energy. Abrupt, jarring transitions reduce energy.
4. **Quality Mixing Technique** — Proper use of EQ (Low/Mid/High adjustment), isolators, and effects to sculpt your sound boosts energy and engagement.

### Achievement System & Leaderboards

- **Achievements** unlock as you hit milestones (e.g., "First Tier 3 Gig", "Harmonic Master", "Perfect Beatmatch").
- **Leaderboards** track your best career progress across sessions. Beat your own records and compete for top positions.

---

## Effects & Sound Design

### Effect Chain

DJ-ROOFRAT includes a comprehensive chain of audio effects that you can apply to shape your mix. The available effects are:

| Effect | Purpose | Typical Use |
|--------|---------|-------------|
| **Reverb** | Adds spatial ambience and depth | Psychedelic builds, wide soundscapes |
| **Delay** | Repeats the signal at timed intervals | Rhythmic echoes, dub-style sweeps |
| **Flanger** | Sweeping frequency modulation | Jet-plane swooshes, dynamic motion |
| **Phaser** | Peak-and-trough comb filtering | Spacey, swirly textures |
| **Bitcrusher** | Reduces bit depth for lo-fi grit | Breakdown glitches, retro sound design |
| **Ring Modulator** | Amplitude modulation spectral shift | Alien tones, industrial textures |
| **Auto Filter** | Automated low-pass or band-pass sweep | Buildup climaxes, filter sweeps |

Each effect can be controlled in real-time during a mix. Consult the [Keyboard Reference](keyboard-reference.md) for specific key bindings.

### EQ Controls

Every deck has a **3-band EQ** (Low, Mid, High) plus a **Filter** parameter:

**Deck A EQ:**
| Control | Keys |
|---------|------|
| Low | `Q` (down), `W` (up) |
| Mid | `E` (down), `R` (up) |
| High | `T` (down), `Y` (up) |
| Filter | `U` (down), `P` (up) |

**Deck B EQ:**
| Control | Keys |
|---------|------|
| Low | `D` (down), `F` (up) |
| Mid | `G` (down), `H` (up) |
| High | `J` (down), `N` (up) |
| Filter | `M` (down), `,` (up) |

### Isolators

**Isolators** are on/off switches that completely remove a frequency band (Low, Mid, or High) from a deck. This is useful for dramatic breakdowns or accentuating complementary frequencies in the mix.

**Isolator Keys (Shift+):**
| Control | Key |
|---------|-----|
| Isolate Low A (kill/restore) | `Shift+Q` |
| Isolate Mid A | `Shift+W` |
| Isolate High A | `Shift+T` |
| Isolate Mid B | `Shift+F` |
| Isolate High B | `Shift+G` |

Press the same key again to restore the band.

---

## Advanced Features

### Beat Grid Editing

The **beat grid** is the framework that aligns cue points and beat jumps with the track's actual beat structure.

| Key | Action |
|-----|--------|
| `-` | Nudge Beat Grid **Left** (−10 ms) — Move grid earlier |
| `=` | Nudge Beat Grid **Right** (+10 ms) — Move grid later |
| `Shift+−` | Adjust **First Beat** Left (−10 ms) — Fine-tune grid start point |
| `Shift+=` | Adjust **First Beat** Right (+10 ms) — Fine-tune grid start point |

Use beat grid editing to correct timing if a track's beat is slightly misaligned, ensuring cue points and beat jumps stay in sync with the music.

### Coaching Overlay

Press `Shift+C` to toggle the **coaching overlay**. This displays real-time mixing tips and feedback on your performance:
- Suggests beatmatch targets
- Alerts you to crowd energy changes
- Recommends harmonic mixing moves based on detected key signatures
- Displays your current tier and progress toward the next tier

### Energy Curve Display

Press `Shift+E` to toggle the **energy curve graph**, which visualizes how crowd energy has changed over your current session. This helps you understand which mixing moves were most effective.

### Session Save & Load

DJ-ROOFRAT automatically saves your session every 120 seconds to prevent data loss. You can also:

| Text Command | Action |
|-----|---------|
| `save recording` / `record save` | Save your current recording |
| `set recording filename` | Set a custom filename |

Saved sessions preserve:
- Current track positions and BPM
- All deck parameters (EQ, effects, sync state)
- Crowd energy history
- Career tier progress

Type `quit` or `exit` at any time to gracefully close the app and save the session.

### Battle Mode (Player 2 Controls)

DJ-ROOFRAT supports a **2-player competitive mode** where Player 2 controls Deck B using arrow keys and the numeric keypad, while Player 1 uses the standard QWERTY layout for Deck A.

**Player 2 Keymap:**

| Control | Keys |
|---------|------|
| Tempo B Up | Arrow Up |
| Tempo B Down | Arrow Down |
| Crossfade Left | Arrow Left |
| Crossfade Right | Arrow Right |
| Reset Tempo B | Numpad 0 |
| Play/Pause Deck B | Numpad 1 |
| Play Deck B | Numpad 2 |
| Pause Deck B | Numpad 3 |
| Crossfade Center | Numpad 5 |
| Crossfade Left | Numpad 7 |
| Crossfade Right | Numpad 9 |

Both players' scores are tracked independently, and the winner is determined by final crowd energy at the end of a session.

---

## Keyboard Reference

For a complete, organized list of all keyboard commands, see [keyboard-reference.md](keyboard-reference.md).

### Quick-Start Cheat Sheet

These are the **10 most important keys** for a new DJ:

| Key | Action |
|-----|--------|
| `A` | Play/Pause Deck A |
| `B` | Play/Pause Deck B |
| `Z`/`X` | Nudge Deck A Tempo Up/Down |
| `I`/`K` | Nudge Deck B Tempo Up/Down |
| `\` | Crossfade Center |
| `S` | Toggle Recording (start/stop) |
| `Shift+V` | Save Recording to file |
| `1`–`3` (Deck A) or `4`–`6` (Deck B) | Set Cue Points |
| `Shift+1`–`Shift+3` or `Shift+4`–`Shift+6` | Jump to Cue Points |
| Type `quit` | Quit Application |

---

## Troubleshooting

### No Audio Output

**Problem:** I launched DJ-ROOFRAT but hear no sound.

**Solution:**
- Run with `--no-audio` flag if PortAudio is not installed:
  ```
  build-vs/Debug/DJ-ROOFRAT.exe --no-audio
  ```
- If you have audio hardware configured and want live output, ensure PortAudio is installed and properly linked. See `developer-setup-guide.md` for build instructions.
- Test tones will play in headless mode to verify the app is working.

### Test EXE Blocked by Windows App Control

**Problem:** Windows SmartScreen or Windows Defender blocks the `.exe` from running.

**Solution:**
- Click **"More Info"** on the warning dialog, then **"Run Anyway"**.
- Alternatively, see the Windows App Control note in `developer-setup-guide.md` for build signing options.

### Session Not Loading

**Problem:** I saved a session, but it won't load on restart.

**Solution:**
- Ensure the session file path has **no backslashes** (`\`). Use forward slashes (`/`) in config files and text entry prompts.
- Verify the file is in the correct directory (`docs/sessions/` by default).
- If the file is corrupted, delete it and start a new session.

### High CPU Usage / Performance Issues

**Problem:** The app is using excessive CPU or runs slowly.

**Solution:**
- Disable the spectrum analyzer (`9` to toggle) and energy curve display (`Shift+E` to toggle) to reduce rendering overhead.
- Close other applications running in the terminal to free up resources.
- Ensure your terminal window is appropriately sized; very large terminal windows increase pixel drawing cost.

### Recording Silence or Garbled Output

**Problem:** The saved recording is silent or sounds distorted.

**Solution:**
- Ensure both decks are **playing** during recording. A silent deck will contribute silence to the mix.
- Check that EQ and effects are not over-compressed (e.g., all isolators on, extreme EQ cuts). Start with neutral settings.
- Save the recording with a clearly named filename (use `Shift+N`) and verify the file exists on disk before playback testing.

---

## For More Information

- **Build & Development:** See `developer-setup-guide.md` for build instructions, CMake setup, and architecture overview.
- **Gameplay Mechanics:** See `gameplay/` folder documentation for career progression and scoring details.
- **Audio Processing:** See `audio/` folder headers and `README.md` for signal flow and effect chain documentation.

---

**Happy Mixing!**
