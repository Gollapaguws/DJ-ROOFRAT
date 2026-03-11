## Phase 9 Complete: Real Audio File Support

Added MP3 decoding support via dr_mp3 header-only library, enabling playback of real music files (WAV, FLAC, MP3) alongside existing test tone generation.

**Files created/changed:**
- vendor/dr_mp3.h
- audio/TrackLoader_Phase9_test.cpp
- audio/TrackLoader.cpp
- CMakeLists.txt

**Functions created/changed:**
- TrackLoader::isMp3File() - Case-insensitive .mp3 extension detection
- TrackLoader::loadMp3File() - MP3 decoding via dr_mp3 with mono→stereo conversion
- TrackLoader::loadFile() - Extended with MP3 format support, preserves WAV/FLAC via libsndfile

**Tests created/changed:**
- test_TrackLoader_InvalidFile - Verifies error handling for non-existent files
- test_TrackLoader_CorruptFile - Validates rejection of malformed audio data
- test_TrackLoader_EmptyFile - Tests zero-byte file handling
- test_TrackLoader_AudioClipStructure - Confirms AudioClip API compliance
- test_TrackLoader_FormatDetection - Verifies .mp3 extension triggers MP3 decoder
- test_TrackLoader_FunctionSignature - Tests multiple overload variants
- test_TrackLoader_OptionalReturn - Validates std::optional semantics
- test_TrackLoader_RepeatedFailedLoads - Stress tests error path stability

**Review Status:** APPROVED

**Code Quality Notes:**
- Fixed critical bug: drmp3 struct fields captured to local variables before uninit
- Memory-safe RAII pattern with proper cleanup in all error paths
- C++20 compliance: std::optional, std::clamp, const-correctness
- Comprehensive error messages for debugging
- Existing libsndfile integration preserved without regression
- Mono→stereo conversion and sample clamping consistent across formats

**Git Commit Message:**
```
feat: Add MP3 file support via dr_mp3

- Integrate dr_mp3 v0.7.3 header-only library for MP3 decoding
- Extend TrackLoader with isMp3File() and loadMp3File() helpers
- Preserve existing WAV/FLAC support via libsndfile
- Add 8 comprehensive tests covering error handling and API semantics
- Fix drmp3 struct access-after-uninit bug with local variable capture
- Maintain C++20 standards with std::optional and proper RAII
```
