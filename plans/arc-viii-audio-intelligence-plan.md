## Plan: Arc VIII - Audio Intelligence & Analysis (Phases 30-33)

Elevate DJ-ROOFRAT from simulator to professional-grade analysis tool with real-time spectrum visualization, harmonic mixing intelligence, phrase detection, and energy rating system. Close the feature gap between current implementation and industry-standard DJ software (Serato, Traktor, Rekordbox).

**Phases: 4 phases**

### **Phase 30: Real-time Spectrum Analyzer**
- **Objective:** FFT-based frequency domain visualization for informed EQ and mixing decisions
- **Files/Functions to Modify/Create:**
  - audio/SpectrumAnalyzer.h: FFT window processing interface
  - audio/SpectrumAnalyzer.cpp: FFT implementation, frequency band aggregation (20 bands, 20Hz-20kHz logarithmic)
  - visuals/SpectrumRenderer.h: Terminal bar graph or D3D11 overlay texture
  - visuals/SpectrumRenderer.cpp: ASCII bar chart (terminal mode) or shader-based visualization (graphics mode)
  - src/main.cpp: Integrate spectrum for deck A, deck B, and master mix
  - audio/SpectrumAnalyzer_Phase30_test.cpp: Comprehensive test suite
  - CMakeLists.txt: Add spectrum_analyzer_phase30_test target
- **Tests to Write:**
  - test_SpectrumAnalyzer_FFTWindow: Verify FFT processing with Hann window (512/1024/2048 sample sizes)
  - test_SpectrumAnalyzer_FrequencyBands: Validate 20-band logarithmic aggregation (20Hz-20kHz)
  - test_SpectrumAnalyzer_RealTimeUpdate: Test 30-60 FPS update rate without audio dropouts
  - test_SpectrumAnalyzer_PerDeckMode: Verify separate spectrum for deck A, deck B, master mix
  - test_SpectrumRenderer_TerminalBars: ASCII bar graph rendering (0-100% scale)
  - test_SpectrumRenderer_PeakHold: Peak markers persist for visual clarity
  - test_SpectrumAnalyzer_SilenceHandling: Zero-amplitude input produces zero spectrum
  - test_SpectrumAnalyzer_Normalization: Spectrum normalized to 0.0-1.0 range
- **Steps:**
  1. Write tests for FFT processing and band aggregation (tests fail)
  2. Implement SpectrumAnalyzer with FFT (Cooley-Tukey algorithm or std::complex-based DFT)
  3. Add Hann window function to reduce spectral leakage
  4. Aggregate FFT bins into 20 logarithmic frequency bands
  5. Implement SpectrumRenderer for terminal (ASCII bars) and graphics (D3D11 texture)
  6. Add peak hold logic (decay over time for visual persistence)
  7. Integrate into main loop: spectrum from deck A, deck B, mixed output
  8. Run tests to verify FFT accuracy and rendering (tests pass)
  9. Manual test with music tracks (verify bass/mid/treble separation)
  10. Lint/format

### **Phase 31: Harmonic Mixing Analyzer**
- **Objective:** Camelot Wheel notation and energy key compatibility scoring for seamless harmonic transitions
- **Files/Functions to Modify/Create:**
  - audio/CamelotAnalyzer.h: Camelot Wheel conversion (musical key → 1A-12A, 1B-12B)
  - audio/CamelotAnalyzer.cpp: Energy key compatibility scoring (perfect=1.0, adjacent=0.8, relative minor/major=0.9, distant=0.3)
  - library/TrackBrowser.h: Add getHarmonicMatches() method
  - library/TrackBrowser.cpp: Filter tracks by harmonic compatibility with current deck track
  - visuals/CamelotWheel.h: ASCII art Camelot Wheel renderer
  - visuals/CamelotWheel.cpp: Terminal circle diagram with current key highlighted
  - src/main.cpp: Display Camelot key notation, show harmonic matches in browser
  - audio/CamelotAnalyzer_Phase31_test.cpp: Comprehensive test suite
  - CMakeLists.txt: Add camelot_analyzer_phase31_test target
- **Tests to Write:**
  - test_CamelotAnalyzer_KeyConversion: Musical keys (C, C#, D, etc.) → Camelot (1A-12A, 1B-12B)
  - test_CamelotAnalyzer_PerfectMatch: Same Camelot key = 1.0 compatibility score
  - test_CamelotAnalyzer_AdjacentKeys: ±1 step (e.g., 5A → 6A) = 0.8 compatibility
  - test_CamelotAnalyzer_RelativeMinorMajor: A ↔ B transition (e.g., 5A → 5B) = 0.9 compatibility
  - test_TrackBrowser_HarmonicFilter: getHarmonicMatches() returns only compatible keys
  - test_CamelotWheel_Rendering: ASCII circle diagram generated correctly
  - test_CamelotAnalyzer_EnergyBoost: +1 Camelot key (e.g., 5A → 6A) = energy increase
  - test_CamelotAnalyzer_EnergyDrop: -1 Camelot key (e.g., 6A → 5A) = energy decrease
- **Steps:**
  1. Write tests for Camelot conversion and compatibility scoring (tests fail)
  2. Implement CamelotAnalyzer with musical key → Camelot mapping table
  3. Add compatibility scoring algorithm (perfect/adjacent/relative/distant rules)
  4. Extend TrackBrowser with getHarmonicMatches(currentKey) method
  5. Implement CamelotWheel ASCII renderer (circle with 12 positions, A/B layers)
  6. Integrate into main.cpp: display Camelot notation in deck status, show matches in browser
  7. Run tests to verify conversion and filtering logic (tests pass)
  8. Manual test with real tracks (verify harmonic mixing suggestions work)
  9. Lint/format

### **Phase 32: Phrase Detection & Beat Grid Editor**
- **Objective:** Automatic phrase structure analysis (verse/chorus/breakdown) and manual beat grid refinement for precise beatmatching
- **Files/Functions to Modify/Create:**
  - audio/PhraseDetector.h: Onset detection and phrase clustering
  - audio/PhraseDetector.cpp: Spectral flux onset detection, 8/16/32 bar segmentation via autocorrelation
  - audio/BeatGrid.h: Beat marker storage (grid offset, BPM, manual adjustments)
  - audio/BeatGrid.cpp: Grid calculation, nudge operations, lock/unlock, reset to auto
  - audio/TrackMetadata.h: Add phraseMarkers and beatGridData fields
  - audio/TrackMetadata.cpp: Serialize/deserialize phrase and grid data to JSON
  - visuals/WaveformRenderer.h: Add beat marker overlay rendering
  - visuals/WaveformRenderer.cpp: Draw beat grid lines on waveform display
  - src/main.cpp: Beat grid editor UI (nudge left/right, lock/unlock, save to track metadata)
  - audio/PhraseDetector_Phase32_test.cpp: Comprehensive test suite
  - CMakeLists.txt: Add phrase_detector_phase32_test target
- **Tests to Write:**
  - test_PhraseDetector_OnsetDetection: Spectral flux identifies kick/snare onsets
  - test_PhraseDetector_BarSegmentation: 8/16/32 bar phrases clustered correctly
  - test_BeatGrid_AutoCalculation: BPM → grid spacing calculation (samples per beat)
  - test_BeatGrid_ManualNudge: Grid offset adjusts by ±10ms increments
  - test_BeatGrid_LockUnlock: Locked grid persists across track reloads
  - test_TrackMetadata_GridSerialization: Beat grid saved/loaded from JSON
  - test_WaveformRenderer_BeatOverlay: Beat markers drawn at correct positions
  - test_BeatGrid_VariableTempo: Grid handles tempo changes (rare edge case)
- **Steps:**
  1. Write tests for onset detection and beat grid (tests fail)
  2. Implement PhraseDetector with spectral flux algorithm (FFT-based)
  3. Add autocorrelation-based phrase clustering (8/16/32 bar detection)
  4. Implement BeatGrid with offset/BPM storage and nudge operations
  5. Extend TrackMetadata to store phrase markers and grid data
  6. Add WaveformRenderer overlay for beat grid lines
  7. Create beat grid editor controls in main.cpp (arrow keys nudge, 'L' lock/unlock)
  8. Run tests to verify onset accuracy and grid persistence (tests pass)
  9. Manual test with tracks (verify phrases align with musical structure)
  10. Lint/format

### **Phase 33: Track Energy Rating System**
- **Objective:** Automated energy analysis (1-10 scale) for intelligent set building and collection filtering
- **Files/Functions to Modify/Create:**
  - audio/EnergyAnalyzer.h: RMS, peak, dynamic range calculation
  - audio/EnergyAnalyzer.cpp: Energy rating algorithm (low=1-3 ambient, mid=4-7 groove, high=8-10 peak-hour)
  - library/TrackLibrary.h: Add energyRating field to StoredTrack
  - library/TrackLibrary.cpp: Store energy rating in SQLite
  - library/LibraryScanner.h: Add analyzeEnergy() step
  - library/LibraryScanner.cpp: Calculate energy during folder scan
  - library/TrackBrowser.h: Add filterByEnergyRange() method
  - library/TrackBrowser.cpp: Energy range filtering (e.g., "Show tracks 7-10")
  - visuals/EnergyHistogram.h: Terminal histogram renderer
  - visuals/EnergyHistogram.cpp: ASCII bar chart showing collection energy distribution
  - src/main.cpp: Energy filter UI, histogram display
  - audio/EnergyAnalyzer_Phase33_test.cpp: Comprehensive test suite
  - CMakeLists.txt: Add energy_analyzer_phase33_test target
- **Tests to Write:**
  - test_EnergyAnalyzer_RMSCalculation: Root mean square amplitude over full track
  - test_EnergyAnalyzer_PeakDetection: Maximum sample amplitude (0.0-1.0 range)
  - test_EnergyAnalyzer_DynamicRange: Peak-to-RMS ratio indicates track dynamics
  - test_EnergyAnalyzer_EnergyRating: Low/mid/high tracks rated 1-10 correctly
  - test_LibraryScanner_EnergyStorage: Energy rating saved to SQLite
  - test_TrackBrowser_EnergyFilter: filterByEnergyRange(7, 10) returns only high-energy tracks
  - test_EnergyHistogram_Distribution: Histogram shows track count per energy level
  - test_EnergyAnalyzer_SilenceHandling: Silent track = energy rating 0
- **Steps:**
  1. Write tests for energy calculation and filtering (tests fail)
  2. Implement EnergyAnalyzer with RMS/peak/dynamic range algorithms
  3. Add energy rating logic (1-10 scale based on RMS + peak + dynamic range)
  4. Extend StoredTrack struct with energyRating field
  5. Modify LibraryScanner to calculate energy during folder scan
  6. Add TrackBrowser::filterByEnergyRange() method
  7. Implement EnergyHistogram ASCII renderer (show collection distribution)
  8. Integrate into main.cpp: energy filter controls, histogram display
  9. Run tests to verify calculation accuracy and filtering (tests pass)
  10. Manual test with real library (verify energy ratings match subjective assessment)
  11. Lint/format

**Open Questions:**
1. FFT implementation: Custom Cooley-Tukey / std::complex DFT / external library (FFTW)? → **CUSTOM DFT (C++20 std::complex, no external deps)**
2. Spectrum update rate: 30 FPS / 60 FPS / audio callback rate? → **60 FPS (balance smoothness vs CPU)**
3. Beat grid editor: Terminal UI only / D3D11 waveform overlay? → **BOTH (terminal for --no-graphics mode, D3D11 when graphics enabled)**
4. Energy rating: Automated only / allow manual override? → **AUTOMATED with manual override option (0-10 input)**
5. Camelot notation: Display only / also filter by energy boost rules? → **BOTH (show notation + filter by energy-aware compatibility)**
