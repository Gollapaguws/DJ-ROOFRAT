## Plan: Arc I - Audio Foundation & Content (Phases 9-11)

Enable real music file playback, automatic BPM/key detection, and comprehensive track library management to transform DJ-ROOFRAT from test-tone prototype to professional DJ application.

**Phases: 3 phases**

### **Phase 9: Real Audio File Support**
- **Objective:** Load and play real music files (WAV, FLAC, MP3) with waveform analysis
- **Files/Functions to Modify/Create:**
  - audio/AudioFileLoader.h: File format detection, multi-format loading interface
  - audio/AudioFileLoader.cpp: libsndfile integration for WAV/FLAC, dr_mp3 for MP3
  - audio/AudioClip.h: Add `loadFromFile(path)` method, file metadata storage
  - audio/AudioClip.cpp: Refactor to support both file and generated audio
  - CMakeLists.txt: Add libsndfile linkage (already optional), include dr_mp3 header
- **Tests to Write:**
  - test_AudioFileLoader_WAVLoading: Verify 16/24/32-bit WAV file loading
  - test_AudioFileLoader_FLACLoading: Validate FLAC decompression
  - test_AudioFileLoader_MP3Loading: Test MP3 decoding with dr_mp3
  - test_AudioFileLoader_FormatDetection: Confirm auto-detection by extension/header
  - test_AudioClip_FilePlayback: Verify loaded files play correctly
  - test_AudioClip_StereoHandling: Test mono/stereo file conversion
- **Steps:**
  1. Write tests for multi-format loading and playback (tests fail)
  2. Create AudioFileLoader class with format detection
  3. Integrate libsndfile for WAV/FLAC (CMake find_package or vendored)
  4. Add dr_mp3.h header-only library to vendor/ directory
  5. Implement MP3 decoding via dr_mp3
  6. Update AudioClip to support file-based initialization
  7. Run tests to verify all formats load and play (tests pass)
  8. Lint/format

### **Phase 10: Track Metadata & BPM Detection**
- **Objective:** Extract metadata and automatically detect BPM/key for intelligent mixing
- **Files/Functions to Modify/Create:**
  - audio/TrackMetadata.h: Struct for title, artist, BPM, key, duration, genre
  - audio/MetadataParser.h: ID3 tag parsing interface
  - audio/MetadataParser.cpp: TagLib integration for ID3v1/ID3v2/Vorbis comments
  - audio/BPMDetector.h: Automatic BPM detection interface
  - audio/BPMDetector.cpp: Phase-vocoder or autocorrelation algorithm
  - audio/KeyDetector.h: Musical key detection interface
  - audio/KeyDetector.cpp: Krumhansl-Schmuckler key-finding algorithm
  - audio/WaveformCache.h: Downsampled waveform storage for visualization
  - audio/AudioClip.h: Add `metadata()` accessor
- **Tests to Write:**
  - test_MetadataParser_ID3v2: Verify ID3 tag extraction (title, artist, etc.)
  - test_BPMDetector_KnownTempo: Validate BPM detection on test files (120, 128, 140 BPM)
  - test_BPMDetector_VariableTempo: Test average BPM calculation
  - test_KeyDetector_MajorKeys: Validate detection of C major, G major, etc.
  - test_KeyDetector_MinorKeys: Test A minor, E minor, etc.
  - test_WaveformCache_Downsampling: Verify efficient waveform generation
- **Steps:**
  1. Write tests for metadata parsing and auto-detection (tests fail)
  2. Integrate TagLib for metadata extraction (CMake find_package)
  3. Implement BPMDetector with phase-vocoder or autocorrelation
  4. Implement KeyDetector using Krumhansl-Schmuckler algorithm
  5. Create WaveformCache for efficient visualization data
  6. Update AudioClip to store and expose metadata
  7. Run tests to verify detection accuracy >90% (tests pass)
  8. Lint/format

### **Phase 11: Track Library & Browser System**
- **Objective:** Manage music collection with database, scanning, and smart filtering
- **Files/Functions to Modify/Create:**
  - library/TrackLibrary.h: Database manager interface (CRUD operations)
  - library/TrackLibrary.cpp: SQLite integration with track table schema
  - library/LibraryScanner.h: Recursive folder scanning interface
  - library/LibraryScanner.cpp: Filesystem traversal, file filtering, progress reporting
  - library/TrackBrowser.h: Filtering and sorting interface
  - library/TrackBrowser.cpp: BPM/key/genre filtering, smart playlists
  - src/main.cpp: Add track browser UI mode (keyboard navigation)
  - CMakeLists.txt: Link SQLite3 (vendor or system)
- **Tests to Write:**
  - test_TrackLibrary_AddTrack: Verify track insertion to database
  - test_TrackLibrary_QueryByBPM: Test BPM range filtering (120-130 BPM)
  - test_TrackLibrary_QueryByKey: Validate key-based queries
  - test_LibraryScanner_RecursiveScan: Confirm multi-folder scanning
  - test_TrackBrowser_SmartPlaylist: Test compatible key/BPM recommendations
  - test_TrackBrowser_Sorting: Verify sort by BPM, artist, title
- **Steps:**
  1. Write tests for database operations and filtering (tests fail)
  2. Design SQLite schema (tracks table: id, path, title, artist, bpm, key, etc.)
  3. Implement TrackLibrary with SQLite prepared statements
  4. Create LibraryScanner with recursive folder traversal
  5. Implement TrackBrowser with filtering and smart playlist logic
  6. Add keyboard-navigable track selection in main.cpp
  7. Run tests to verify database performance <100ms for 10K tracks (tests pass)
  8. Lint/format

**Open Questions:**
1. TagLib vs simpler header-only alternative? **Recommend TagLib (industry standard, comprehensive)**
2. BPM detection algorithm: Phase-vocoder vs autocorrelation? **Start with autocorrelation (simpler), upgrade later if needed**
3. SQLite vendored or system library? **Vendor amalgamation for portability**
4. Waveform cache format: Custom binary vs PNG/image? **Custom binary (faster, smaller)**
5. Initial library scan: Foreground or background thread? **Background with progress bar**

**Dependencies:**
- **External libraries:** libsndfile, dr_mp3 (header-only), TagLib, SQLite3
- **All can be vendored** in `vendor/` directory for zero system dependencies
