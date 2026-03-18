# DJ-ROOFRAT Project Guidelines

## Build And Run
- Use CMake with Visual Studio 2022 x64 generator.
- Configure: `cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64`
- Build: `cmake --build build-vs --config Debug`
- Run: `build-vs/Debug/DJ-ROOFRAT.exe --no-audio`
- Optional backends (`PortAudio`, `libsndfile`) are not required for development; app must still run with generated test tones.

## Architecture
- `audio/`: deck playback, BPM detection, EQ/filter, track loading, mixer, audio output wrappers.
- `crowdAI/`: crowd state machine driven by BPM, transition smoothness, and energy.
- `gameplay/`: scoring and career progression.
- `input/`: keyboard command mapping.
- `visuals/`: terminal waveform rendering.
- `src/main.cpp`: runtime loop and module wiring.

## C++ Conventions
- Target C++20 and prefer standard library types (`std::vector`, `std::array`, `std::optional`, `std::chrono`, `std::filesystem`).
- Keep values bounded with `std::clamp` for user/input controlled parameters.
- Prefer small, single-purpose functions and composition over inheritance.
- Maintain const-correctness and explicit, descriptive names.
- Use RAII and smart pointers for owned resources.
- Keep header/source separation clear (`.h` for interfaces, `.cpp` for implementation).

## Project-Specific Rules
- Preserve equal-power crossfade behavior in `audio/Mixer.cpp`.
- Preserve sample-accurate playback progression in `audio/Deck.cpp`.
- For new live controls: update `InputCommand`, `InputMapper::parseKey`, and the command switch in `src/main.cpp` together.
- Keep `CareerProgression` tier behavior monotonic unless explicitly asked to change game design.

## Environment Notes
- Do not mix generators in the same build directory. Use `build-vs` for Visual Studio builds.
- Windows keyboard polling is guarded with `#if defined(_WIN32)` and `_kbhit/_getch`.

## Agent Rate-Limit Resilience
- Use subagents only when the task clearly requires multi-step delegation (research/review/parallel exploration).
- Prefer direct implementation with local tools for focused single-phase changes to reduce API pressure.
- Avoid repeated immediate retries after a `rate_limited` error; wait before retrying.
- Retry delegated subagent calls at most once after cooldown. If still rate-limited, continue in direct implementation mode.
- Keep work moving by preserving progress in TODO state and switching to scoped file/grep reads.

If a retry is needed, use a short cooldown first:

```powershell
Start-Sleep -Seconds 30
```

## Imported Instruction Packs
- `.github/instructions/cpp-programming-guidelines.instructions.md` is adapted from `awesome-copilot-instructions/rules-copilot-instructions/cpp-programming-guidelines`.
- `.github/instructions/general-code-practices.instructions.md` is adapted from `awesome-copilot-instructions/rules-copilot-instructions/github`.
- `.github/instructions/general-quality-guardrails.instructions.md` is adapted from `awesome-copilot-instructions/rules-copilot-instructions/code-guidelines`.
