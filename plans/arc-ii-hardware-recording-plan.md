## Plan: Arc II - Hardware Integration & Recording (Phases 12-14)

Add MIDI controller support for tactile hardware control and implement performance recording/export to capture DJ sets for sharing and review.

**Phases: 3 phases**

### **Phase 12: MIDI Controller Foundation**
- **Objective:** Detect and receive input from MIDI controllers via Windows MIDI API
- **Files/Functions to Modify/Create:**
  - input/MIDIDevice.h: Device enumeration and info struct
  - input/MIDIDevice.cpp: Windows midiInXXX API wrapper
  - input/MIDIMessage.h: MIDI message types (NoteOn, ControlChange, PitchBend)
  - input/MIDIMessage.cpp: Message parsing from raw MIDI bytes
  - input/MIDIController.h: Controller class with event callback system
  - input/MIDIController.cpp: Device opening, message handling, callback dispatch
  - CMakeLists.txt: Link winmm.lib (Windows Multimedia API)
- **Tests to Write:**
  - test_MIDIDevice_Enumeration: Verify device detection (requires test MIDI device or mock)
  - test_MIDIMessage_Parsing: Validate NoteOn/CC/PitchBend parsing from bytes
  - test_MIDIController_Connection: Test device open/close lifecycle
  - test_MIDIController_Callback: Verify event dispatch to registered handlers
  - test_MIDIController_MultiDevice: Test multiple simultaneous controllers
- **Steps:**
  1. Write tests for MIDI device detection and message parsing (tests fail)
  2. Implement MIDIDevice enumeration using midiInGetNumDevs/midiInGetDevCaps
  3. Create MIDIMessage parser for standard MIDI protocol
  4. Implement MIDIController with midiInOpen and callback handler
  5. Add event subscription system for message dispatch
  6. Run tests with virtual MIDI device or mock (tests pass)
  7. Lint/format

### **Phase 13: MIDI Control Mapping System**
- **Objective:** Map MIDI controls to deck functions with visual learn mode and presets
- **Files/Functions to Modify/Create:**
  - input/MIDIMapping.h: Mapping definition (MIDI CC → DJ function)
  - input/MIDIMapping.cpp: Serialization, curve application (linear, logarithmic, etc.)
  - input/MIDILearnMode.h: Visual MIDI learn interface
  - input/MIDILearnMode.cpp: Capture next MIDI input and assign to function
  - input/ControllerProfiles.h: Preset mappings for popular controllers
  - input/ControllerProfiles.cpp: Pioneer DDJ, Numark, Traktor S2/S4 presets
  - src/main.cpp: Integrate MIDI input into control loop
  - CMakeLists.txt: No changes needed
- **Tests to Write:**
  - test_MIDIMapping_LinearCurve: Verify linear CC value → parameter mapping
  - test_MIDIMapping_LogarithmicCurve: Test log curve for faders (crossfader, volume)
  - test_MIDIMapping_Serialization: Validate save/load of mapping configs
  - test_MIDILearnMode_Capture: Test assignment of next MIDI input to function
  - test_ControllerProfiles_PioneerDDJ: Verify preset mapping loads correctly
  - test_MIDIMapping_Jogwheel: Test high-resolution jogwheel with acceleration
- **Steps:**
  1. Write tests for mapping, curves, and learn mode (tests fail)
  2. Implement MIDIMapping with sensitivity curves
  3. Create MIDILearnMode for visual mapping assignment
  4. Add controller profiles for 3-5 popular models
  5. Implement jogwheel acceleration curves
  6. Integrate MIDI input handling in main control loop
  7. Run tests to verify mapping accuracy and latency <1ms (tests pass)
  8. Lint/format

### **Phase 14: Performance Recording & Export**
- **Objective:** Capture mixer output and export DJ sets as WAV/MP3 files
- **Files/Functions to Modify/Create:**
  - audio/Recorder.h: Recording interface with start/stop/pause
  - audio/Recorder.cpp: Capture mixer output to ring buffer
  - audio/WAVExporter.h: WAV file writing interface
  - audio/WAVExporter.cpp: RIFF/WAV format encoding (16/24/32-bit)
  - audio/MP3Exporter.h: MP3 encoding interface
  - audio/MP3Exporter.cpp: LAME library integration for MP3 export
  - audio/SessionMetadata.h: Track list, timestamps, transition markers
  - audio/SessionMetadata.cpp: JSON serialization for set metadata
  - src/main.cpp: Add record toggle, display recording indicator
  - CMakeLists.txt: Link LAME library (vendored or system)
- **Tests to Write:**
  - test_Recorder_CaptureStart: Verify recording starts without clicks
  - test_Recorder_RingBuffer: Test buffer overflow handling
  - test_WAVExporter_16bit: Validate 16-bit WAV export correctness
  - test_WAVExporter_24bit: Test 24-bit WAV export
  - test_MP3Exporter_CBR: Verify constant bitrate MP3 encoding (320kbps)
  - test_MP3Exporter_VBR: Test variable bitrate encoding (V0)
  - test_SessionMetadata_Serialization: Validate JSON export/import
- **Steps:**
  1. Write tests for recording and export (tests fail)
  2. Implement Recorder with ring buffer capturing mixer output
  3. Create WAVExporter with RIFF format encoding
  4. Integrate LAME for MP3 export (vendor lame.lib)
  5. Implement SessionMetadata with track/timestamp logging
  6. Add record button and visual indicator in UI
  7. Run tests to verify export quality and metadata accuracy (tests pass)
  8. Lint/format

**Open Questions:**
1. MIDI learn mode: Dedicated UI or overlay on main screen? **Overlay (simpler integration)**
2. Jogwheel resolution: Standard 7-bit or high-res 14-bit? **Support both, auto-detect**
3. Recording buffer size: Fixed or dynamic? **Dynamic with 60-second ring buffer (configurable)**
4. MP3 encoder: LAME or alternative (Shine, libmp3lame)? **LAME (best quality, widely available)**
5. Auto-detect transitions for metadata? **Yes, using energy/crossfader analysis**

**Dependencies:**
- **Windows MIDI API** (winmm.lib, built-in)
- **LAME** (vendor static library)
- **Arc I** incomplete but not blocking (can record test tones initially)
