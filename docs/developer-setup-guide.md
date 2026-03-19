# DJ-ROOFRAT Developer Setup Guide

## Prerequisites

Before setting up DJ-ROOFRAT, ensure you have the following installed:

### Required
- **Visual Studio 2022** with C++ workload
- **CMake** 3.20 or later
- **Git** (for version control)

### Optional
- **PortAudio** (realtime audio output support; app works without it using generated test tones)
- **libsndfile** (audio file loading support; app works without it using generated test tones)
- **LAME** (MP3 export support; app works without it)
- **SQLite3** (database support; the project includes fallback mechanisms if not found)

## Clone the Repository

```bash
git clone https://github.com/yourusername/DJ-ROOFRAT.git
cd DJ-ROOFRAT
```

## Configure the Project

From the repository root, configure the CMake build for Visual Studio 2022 x64:

```bash
cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64
```

This command:
- `-S .` points to the current directory as the source
- `-B build-vs` creates the build output in the `build-vs` directory
- `-G "Visual Studio 17 2022"` specifies the Visual Studio 2022 generator
- `-A x64` specifies 64-bit architecture

**Important:** Always use `build-vs` as your build directory. Do not mix CMake generators in the same directory.

## Build the Project

### Debug Build
```bash
cmake --build build-vs --config Debug
```

Output: `build-vs/Debug/DJ-ROOFRAT.exe`

### Release Build
```bash
cmake --build build-vs --config Release
```

Output: `build-vs/Release/DJ-ROOFRAT.exe`

## Run the Application

### Basic Execution
```bash
build-vs/Debug/DJ-ROOFRAT.exe
```

### Run Without Audio Output
Use the `--no-audio` flag to run without audio processing (useful for headless environments or continuous integration):

```bash
build-vs/Debug/DJ-ROOFRAT.exe --no-audio
```

### Run With Audio File Arguments
You can optionally specify track files as arguments:

```bash
build-vs/Debug/DJ-ROOFRAT.exe --no-audio path/to/track1.wav path/to/track2.wav
```

## Optional Audio Dependencies

The project is configured to work without optional audio backends, using generated test tones by default. To enable optional audio features, reconfigure with CMake flags:

### Enable PortAudio (Realtime Audio Output)
```bash
cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64 -D DJ_SIM_ENABLE_AUDIO_BACKEND=ON
cmake --build build-vs --config Debug
```

### Enable libsndfile (Audio File Loading)
Automatically enabled when `DJ_SIM_ENABLE_AUDIO_BACKEND=ON` and libsndfile is found on the system.

### Enable LAME (MP3 Export)
Automatically enabled when `DJ_SIM_ENABLE_AUDIO_BACKEND=ON` and LAME is found on the system.

### Disable Audio Backend Features
```bash
cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64 -D DJ_SIM_ENABLE_AUDIO_BACKEND=OFF
cmake --build build-vs --config Debug
```

### Enable DirectX 11 Graphics (Experimental)
```bash
cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64 -D DJROOFRAT_ENABLE_GRAPHICS=ON
cmake --build build-vs --config Debug
```

## Running Tests

DJ-ROOFRAT includes a comprehensive test suite. All test executables are built to `build-vs/Debug/` and can be run independently:

### Integration Smoke Tests
- **SmokeTest_main.exe** – Basic smoke test validating core functionality
  ```bash
  build-vs/Debug/SmokeTest_main.exe
  ```

### End-to-End Integration Tests
- **E2E_LoadMixRecord_test.exe** – Tests loading tracks, mixing, and recording
  ```bash
  build-vs/Debug/E2E_LoadMixRecord_test.exe
  ```

- **E2E_AutoSync_test.exe** – Tests automatic sync features and BPM matching
  ```bash
  build-vs/Debug/E2E_AutoSync_test.exe
  ```

- **E2E_MIDIEffects_test.exe** – Tests MIDI controller integration and effects
  ```bash
  build-vs/Debug/E2E_MIDIEffects_test.exe
  ```

- **E2E_BassClashDetection_test.exe** – Tests bass clash detection algorithm
  ```bash
  build-vs/Debug/E2E_BassClashDetection_test.exe
  ```

- **E2E_SessionRoundTrip_test.exe** – Tests session save/load functionality
  ```bash
  build-vs/Debug/E2E_SessionRoundTrip_test.exe
  ```

### Unit Tests
Various phase-specific unit tests are also available in `build-vs/Debug/`, such as:
- `InputMapper_Phase3_test.exe` – Input mapping validation

Run all tests using a test script:
```bash
.\run_tests.bat
```

## Windows Application Control Note

Freshly-built test executables may be blocked by Windows Device Guard or Application Control (WDAC) policies on some machines. This is a **machine-level security restriction** and not a code defect. Common affected test executables include `Recorder_Phase14_test.exe` and occasionally others.

### Workarounds
1. **Run with Administrative Privileges**
   ```powershell
   Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process
   .\build-vs\Debug\SmokeTest_main.exe
   ```

2. **Submit for Exemption**
   Contact your IT department to whitelist the build directory or executable, or submit the executable signature to an exemption process.

3. **Run via cmd.exe with /c**
   ```bash
   cmd /c "build-vs\Debug\SmokeTest_main.exe"
   ```

If you encounter errors like "cannot execute shell script," verify that:
- Windows Defender is not quarantining the executable
- No Group Policy restrictions exist for unsigned executables
- The executable has appropriate file permissions

## CMake Configuration Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `DJ_SIM_ENABLE_AUDIO_BACKEND` | BOOL | ON | Enable optional PortAudio, libsndfile, and LAME support |
| `DJROOFRAT_ENABLE_GRAPHICS` | BOOL | OFF | Enable DirectX 11 graphics rendering (experimental) |
| `DJ_SIM_USE_PORTAUDIO` | BOOL | (auto) | Enables if PortAudio found and `DJ_SIM_ENABLE_AUDIO_BACKEND` is ON |
| `DJ_SIM_USE_SNDFILE` | BOOL | (auto) | Enables if libsndfile found and `DJ_SIM_ENABLE_AUDIO_BACKEND` is ON |
| `DJ_SIM_USE_LAME` | BOOL | (auto) | Enables if LAME found and `DJ_SIM_ENABLE_AUDIO_BACKEND` is ON |
| `DJ_SIM_USE_SQLITE3` | BOOL | ON | Enable SQLite3 support for session persistence |

To set an option during configuration:
```bash
cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64 -D OPTION_NAME=VALUE
```

## Troubleshooting

### "Cannot mix generators in the same build directory"

**Error:** CMake complains about mixed generators or fails after a configuration attempt with a different generator.

**Solution:** Delete the `build-vs` directory and reconfigure:
```bash
Remove-Item -Recurse -Force build-vs
cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64
```

### "SQLite3 not found"

**Error:** CMake warning or build failure related to SQLite3.

**Solution:**
- Install SQLite3 via vcpkg package manager
- Or install via system package manager (e.g., `chocolatey`, `winget`)
- Or download pre-built binaries and place in system PATH
- The project includes fallback mechanisms; the app runs with reduced functionality

### "Test executable blocked by Windows security"

**Error:** Test executable fails to run with "cannot execute shell script" or similar Windows security message.

**Solution:** See the **Windows Application Control Note** section above. Run with admin privileges, submit for exemption, or use workaround commands.

### "Cannot find PortAudio/libsndfile/LAME"

**Error:** Build succeeds but optional audio features don't work; metadata indicates libraries aren't linked.

**Solution:**
- Install libraries via vcpkg: `vcpkg install portaudio libsndfile lame`
- Ensure libraries are in your PATH or installed to system directories
- The app works without these libraries using generated test tones
- To verify what's enabled, check CMake output for "DJ_SIM_USE_*" flags

### "Build fails with linker errors"

**Error:** Linker cannot resolve symbols.

**Solution:**
1. Verify all dependencies are properly installed
2. Try a clean rebuild:
   ```bash
   Remove-Item -Recurse -Force build-vs
   cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64
   cmake --build build-vs --config Debug
   ```
3. Check that you're using Visual Studio 2022 (not older versions)
4. Ensure C++20 standard is supported by your compiler

## Development Tips

- **Fast iteration:** Use `cmake --build build-vs --config Debug` for incremental builds
- **Minimal test:** Run `build-vs/Debug/SmokeTest_main.exe` to quickly validate core functionality
- **Headless testing:** Use `--no-audio` flag when testing without audio hardware
- **CMake reconfigure:** Not needed unless CMakeLists.txt or dependencies change; but a clean reconfigure can resolve odd build issues
- **File monitoring:** Use your IDE's integrated test runner or PowerShell script `run_tests.bat` to execute the test suite
