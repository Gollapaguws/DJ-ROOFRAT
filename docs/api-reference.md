# DJ-ROOFRAT API Reference

This document provides a comprehensive reference for the 20 core classes in DJ-ROOFRAT. All classes are in the `dj` namespace unless otherwise noted.

---

## AudioClip

**Purpose:** Stores raw audio samples with metadata. Provides frame-based access and static factory methods for generating test tones.

**Header:** `audio/AudioClip.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `bool empty() const` | Returns true if the clip has no samples. |
| `std::size_t frameCount() const` | Returns the number of audio frames (samples per channel). |
| `std::array<float, 2> frameAt(double frameIndex) const` | Gets stereo samples at the given frame index (interpolated). |
| `const TrackMetadata* metadata() const` | Returns pointer to associated track metadata. |
| `static AudioClip generateTestTone(float frequencyHz, float seconds, int sampleRate)` | Generates a sine-wave test tone at specified frequency and duration. |

**Properties:**
- `int sampleRate` — Sample rate in Hz (default: 44100)
- `int channels` — Number of channels (default: 2)
- `std::vector<float> samples` — Interleaved audio samples

**Usage Example:**
```cpp
// Load a test tone
AudioClip clip = AudioClip::generateTestTone(440.0f, 2.0f, 44100);

// Check if loaded
if (!clip.empty()) {
    std::size_t frames = clip.frameCount();
    auto stereoSample = clip.frameAt(1000.0);  // Get frame 1000
}
```

---

## TrackLoader

**Purpose:** Static utility for loading audio files from disk into `AudioClip` objects.

**Header:** `audio/TrackLoader.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `static std::optional<AudioClip> loadFile(const std::string& path, std::string* errorOut)` | Loads an audio file; returns `std::nullopt` on failure and writes error message if `errorOut` is provided. |

**Usage Example:**
```cpp
std::string error;
auto clip = TrackLoader::loadFile("path/to/track.mp3", &error);
if (!clip) {
    std::cerr << "Failed to load: " << error << std::endl;
} else {
    std::cout << "Loaded " << clip->frameCount() << " frames." << std::endl;
}
```

---

## Deck

**Purpose:** Represents a DJ deck with playback control, effects, cueing, looping, sync, and beat warping.

**Header:** `audio/Deck.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `bool loadClip(const AudioClip& clip)` | Loads an `AudioClip` into the deck. Returns false if clip is empty. |
| `bool loadFromFile(const std::string& path, std::string* errorOut)` | Loads audio file directly; returns false on error. |
| `bool hasClip() const` | Returns true if a clip is currently loaded. |
| `const AudioClip* clip() const` | Returns pointer to loaded clip or `nullptr`. |
| `void play()` | Starts playback. |
| `void pause()` | Pauses playback. |
| `void stop()` | Stops playback and resets position. |
| `bool isPlaying() const` | Returns true if deck is playing. |
| `void setOutputSampleRate(int sampleRate)` | Sets resampling target (e.g., for output device). |
| `void setTempoPercent(float percent)` | Sets tempo stretch (100.0 = original speed, 50.0 = half speed, 150.0 = 1.5x). |
| `float tempoPercent() const` | Gets current tempo percent. |
| `void setLoopBeats(int beats)` | Sets loop point length in beats for the loaded track. |
| `int loopBeats() const` | Gets current loop length in beats. |
| `void setTrim(float trim)` | Sets deck output trim/gain. |
| `void setEQ(float low, float mid, float high)` | Sets 3-band EQ gains (typically -12 to +12 dB). |
| `void setEQFrequencies(float lowMid, float midHigh)` | Adjusts crossover frequencies between EQ bands. |
| `void setFilter(float cutoff)` | Sets low-pass filter cutoff frequency (Hz). |
| `void setIsolatorMode(bool low, bool mid, bool high)` | Enables isolator mode for selected bands (kills those frequencies). |
| `bool isIsolatorEnabled(int band) const` | Returns true if isolator is active on specified band. |
| `void setFilterOrder(int order)` | Sets filter slope (typically 12, 24, 48 dB/octave). |
| `int getFilterOrder() const` | Gets current filter slope setting. |
| `void setEffectChain(std::shared_ptr<EffectChain> effectChain)` | Chains effects (reverb, delay, etc.). |
| `void setEffectSendLevel(float sendLevel)` | Sets wet signal level sent to effect chain (0.0–1.0). |
| `void setCue(std::size_t frame)` | Sets cue point (main bank). |
| `void jumpToCue()` | Jumps to cue point in main bank. |
| `void setCue(std::size_t frame, int bank)` | Sets cue point in specified bank (0–3). |
| `void jumpToCue(int bank)` | Jumps to cue point in specified bank. |
| `void setActiveCueBank(int bank)` | Activates cue bank (0–3). |
| `void configureLoop(std::size_t startFrame, std::size_t endFrame, bool enabled)` | Configure loop region and toggle loop mode. |
| `void configureLoop(std::size_t startFrame, std::size_t endFrame, bool enabled, float bpm)` | Configure loop with tempo context. |
| `void setSlipMode(bool enabled)` | Enables slip mode (cue without stopping playback). |
| `void setVinylMode(bool enabled)` | Enables vinyl mode (physical-platter-like scratching). |
| `void setScratchVelocity(float velocity)` | Sets scratch speed multiplier. |
| `void setTempoRampEnabled(bool enabled)` | Enables tempo ramping for smooth pitch transitions. |
| `void setTargetTempo(float percent)` | Sets target tempo for ramp (100 = original). |
| `void setTempoRampRate(float rate)` | Sets ramp speed (seconds per percentage point). |
| `void alignPhaseWithDeck(const Deck& targetDeck, double bpmA, double bpmB)` | Aligns phase and beat offset with another deck. |
| `void setAutoSyncTarget(Deck* target)` | Enables sync to target deck. |
| `void disableAutoSync()` | Disables sync. |
| `bool isSyncEnabled() const` | Returns true if sync is active. |
| `void beatJump(int beats)` | Jumps playhead forward/backward by N beats. |
| `SyncState getSyncState() const` | Returns current sync state (Off, Initializing, Locked, Drifting). |
| `double getPhaseOffset() const` | Returns phase offset relative to sync target (-0.5 to +0.5 beat). |
| `void setWarp(float warpPercent)` | Sets time-stretch warp (0.5x to 2.0x speed without pitch change). |
| `float getWarp() const` | Gets current warp setting. |
| `void clearWarp()` | Resets warp to 1.0x (normal). |
| `std::array<float, 2> nextFrame()` | Returns next stereo frame and advances playhead. |
| `float recentEnergy() const` | Returns RMS energy of recent samples (0.0–1.0). |
| `std::size_t currentFrame() const` | Returns current playback position in frames. |
| `float getBPM() const` | Returns detected or configured BPM. |
| `const SpectrumAnalyzer* getSpectrumAnalyzer() const` | Returns pointer to frequency spectrum analyzer. |

**Usage Example:**
```cpp
Deck deckA;
if (deckA.loadFromFile("track_a.mp3")) {
    deckA.setTempoPercent(110.0f);  // Speed up 10%
    deckA.setEQ(2.0f, 0.0f, -1.5f);  // Low boost, high cut
    deckA.play();
    
    for (int i = 0; i < 1024; ++i) {
        auto frame = deckA.nextFrame();  // Get output
    }
}
```

---

## Mixer

**Purpose:** Blends output from two decks using a crossfader, manages equal-power mixing, and exports metrics like smoothness and energy.

**Header:** `audio/Mixer.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `void setCrossfader(float value)` | Sets crossfader position (-1.0 = full A, 0.0 = center, 1.0 = full B). |
| `float crossfader() const` | Gets current crossfader position. |
| `void setDeckTrim(float a, float b)` | Sets per-deck output trim/gain. |
| `void setMasterGain(float gain)` | Sets overall mixer output gain. |
| `std::pair<float, float> crossfadeGains() const` | Returns current equal-power crossfade gains for decks A and B. |
| `std::vector<float> mixBlock(Deck& deckA, Deck& deckB, std::size_t frames, MixMetrics& metrics)` | Mixes N frames from both decks; populates metrics with RMS, smoothness, and energy levels. Returns interleaved stereo. |

**MixMetrics Structure:**
- `float rms` — Root-mean-square level of mixed output (0.0–1.0)
- `float transitionSmoothness` — Smoothness metric for crossfade transition (0.0–1.0)
- `float deckAEnergy` — Energy of deck A samples (0.0–1.0)
- `float deckBEnergy` — Energy of deck B samples (0.0–1.0)

**Usage Example:**
```cpp
Mixer mixer;
mixer.setCrossfader(-0.5f);  // Favor deck A
mixer.setMasterGain(0.9f);

MixMetrics metrics;
auto output = mixer.mixBlock(deckA, deckB, 2048, metrics);
std::cout << "Mix RMS: " << metrics.rms << ", Smoothness: " << metrics.transitionSmoothness << std::endl;
```

---

## EffectChain

**Purpose:** Manages a chain of audio effects (reverb, delay) in serial or parallel mode.

**Header:** `audio/EffectChain.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `EffectChain()` | Constructs an empty effect chain. |
| `void setMode(Mode mode)` | Sets chain mode: `Serial` (effects in series) or `Parallel` (effects blended). |
| `Mode mode() const` | Returns current chain mode. |
| `void addEffect(std::shared_ptr<Reverb> reverb, float wetDryMix)` | Adds a reverb effect with wet/dry mix (0.0 = dry, 1.0 = wet). |
| `void addEffect(std::shared_ptr<Delay> delay, float wetDryMix)` | Adds a delay effect with wet/dry mix. |
| `std::array<float, 2> process(const std::array<float, 2>& input)` | Processes a stereo frame through the chain. |
| `void clear()` | Removes all effects. |

**Usage Example:**
```cpp
auto effectChain = std::make_shared<EffectChain>();
effectChain->setMode(EffectChain::Mode::Serial);

auto reverb = std::make_shared<Reverb>();
effectChain->addEffect(reverb, 0.35f);  // 35% wet reverb

deck.setEffectChain(effectChain);
deck.setEffectSendLevel(0.5f);
```

---

## SpectrumAnalyzer

**Purpose:** Performs real-time FFT-based frequency analysis of audio, returning 20 log-spaced frequency bands (20 Hz–20 kHz) and full spectrum data.

**Header:** `audio/SpectrumAnalyzer.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `SpectrumAnalyzer(int sampleRate, int windowSize)` | Constructs analyzer with specified sample rate and FFT window size (default: 1024). |
| `void processSamples(const float* samples, std::size_t count)` | Feeds audio samples into the analysis engine. |
| `std::vector<float> getFullSpectrum() const` | Returns raw spectrum data (N/2+1 bins, 0.0–1.0 normalized). |
| `std::array<float, NUM_BANDS> getFrequencyBands() const` | Returns 20 log-spaced frequency bands for visual display. |
| `int getWindowSize() const` | Returns FFT window size. |
| `int getSampleRate() const` | Returns sample rate. |

**Constants:**
- `NUM_BANDS = 20` — Number of output frequency bands

**Usage Example:**
```cpp
SpectrumAnalyzer analyzer(44100, 2048);
analyzer.processSamples(audioBuffer, 2048);

auto bands = analyzer.getFrequencyBands();
// bands[0] ~= 20 Hz, bands[19] ~= 20 kHz
for (int i = 0; i < 20; ++i) {
    std::cout << "Band " << i << ": " << bands[i] << std::endl;
}
```

---

## FFTEngine

**Purpose:** Low-level FFT computation engine supporting windowing and power-spectrum calculation.

**Header:** `audio/FFTEngine.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `FFTEngine(int size, WindowFunction window)` | Constructs FFT engine; size must be power of 2 (default window: Hann). |
| `void compute(std::vector<std::complex<double>>& data)` | In-place FFT computation. |
| `std::vector<float> computePowerSpectrum(const float* samples, size_t count)` | Returns power spectrum (magnitude-squared). |
| `int getSize() const` | Returns FFT size. |
| `static bool isPowerOfTwo(int n)` | Helper to validate FFT size. |

**Window Functions:**
- `None`, `Hann`, `Hamming`, `Blackman` — Supported window types for spectral analysis

**Usage Example:**
```cpp
FFTEngine fft(2048, FFTEngine::WindowFunction::Hann);
std::vector<float> spectrum = fft.computePowerSpectrum(audioSamples, 2048);
// spectrum.size() = 1025 (2048/2 + 1)
```

---

## BPMDetector

**Purpose:** Static utility for estimating the BPM of an audio clip using onset detection and autocorrelation.

**Header:** `audio/BPMDetector.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `static std::optional<float> estimate(const AudioClip& clip, float minBpm, float maxBpm)` | Detects BPM in clip; returns `std::nullopt` if detection fails. Search range is [minBpm, maxBpm] (default: 70–180). |

**Usage Example:**
```cpp
auto bpm = BPMDetector::estimate(clip, 80.0f, 140.0f);
if (bpm) {
    std::cout << "Detected BPM: " << *bpm << std::endl;
} else {
    std::cout << "BPM detection failed." << std::endl;
}
```

---

## EnergyAnalyzer

**Purpose:** Tracks real-time RMS energy, peak levels, dynamic range, and categorizes energy into 1–10 ratings for crowd feedback.

**Header:** `audio/EnergyAnalyzer.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `EnergyAnalyzer()` | Constructs analyzer with default settings. |
| `void processSamples(const float* samples, size_t count)` | Feeds audio samples. |
| `float getRMS() const` | Returns RMS energy (0.0–1.0). |
| `float getPeak() const` | Returns peak level (0.0–1.0). |
| `float getDynamicRange() const` | Returns dynamic range in dB. |
| `int getEnergyRating() const` | Returns 1–10 energy rating. |
| `std::string getEnergyLabel() const` | Returns descriptive label (e.g., "Ambient", "Peak Hour"). |
| `void reset()` | Clears history. |

**Energy Labels:**
- 1–2: "Ambient"
- 3–4: "Chill"
- 5–6: "Moderate"
- 7–8: "Upbeat"
- 9–10: "Peak Hour"

**Usage Example:**
```cpp
EnergyAnalyzer energy;
energy.processSamples(buffer, 1024);

std::cout << "Energy: " << energy.getEnergyLabel() 
          << " (Rating: " << energy.getEnergyRating() << ")" << std::endl;
```

---

## BeatGrid

**Purpose:** Manages beat and bar markers for a track; supports nudging, onset snapping, and phrase detection.

**Header:** `audio/BeatGrid.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `BeatGrid()` | Constructs empty beat grid. |
| `std::vector<BeatMarker> getBeats() const` | Returns all beat markers. |
| `void generateFromBPM(double bpm, double firstBeatOffset, double trackDuration)` | Auto-generates grid from BPM and track duration. |
| `void nudgeBeat(std::size_t beatIndex, double offsetMs)` | Adjusts timing of single beat. |
| `void nudgeAll(double offsetMs)` | Applies offset to all beats. |
| `void snapToOnset(std::size_t beatIndex, const std::vector<double>& onsets)` | Aligns beat to nearest onset in time-domain. |
| `void markPhrases(int barsPerPhrase)` | Marks phrase boundaries (default: 16 bars). |
| `double getBPM() const` | Returns grid BPM. |
| `double getFirstBeatOffset() const` | Returns offset of first beat (seconds). |
| `BeatGridData toData() const` | Exports grid to persistent format. |
| `void fromData(const BeatGridData& data, double trackDuration)` | Loads grid from persistent format. |

**BeatMarker Structure:**
- `double timestamp` — Position in seconds from track start
- `int barPosition` — 1–4 (beat within bar)
- `bool isDownbeat` — First beat of bar
- `bool isPhraseStart` — First beat of phrase

**Usage Example:**
```cpp
BeatGrid grid;
grid.generateFromBPM(128.0, 0.1, 300.0);
grid.markPhrases(16);

auto beats = grid.getBeats();
std::cout << "Total beats: " << beats.size() << std::endl;
```

---

## PhaseAligner

**Purpose:** Calculates phase offset between two synchronized decks to detect/correct timing drift.

**Header:** `audio/PhaseAligner.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `double calculatePhaseOffset(const Deck& deckA, const Deck& deckB, double bpmA, double bpmB) const` | Returns phase offset in time (seconds). |
| `double calculatePhaseOffsetBeats(const Deck& deckA, const Deck& deckB, double bpmA, double bpmB) const` | Returns phase offset normalized to [-0.5, +0.5] beats. |

**Return Value Range:** [-0.5, +0.5] — Negative means deckB leads deckA; positive means deckA leads deckB.

**Usage Example:**
```cpp
PhaseAligner aligner;
double offset = aligner.calculatePhaseOffsetBeats(deckA, deckB, 128.0, 128.0);
if (std::abs(offset) > 0.1) {
    std::cout << "Decks are drifting by " << offset << " beats." << std::endl;
}
```

---

## SyncController

**Purpose:** Manages tempo and phase synchronization between a sync deck and a target deck, maintaining lock across BPM variations.

**Header:** `audio/SyncController.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `void update(Deck& syncDeck, const Deck& targetDeck, double bpmA, double bpmB)` | Updates sync deck's tempo and phase toward target. |
| `void enableSync(Deck& syncDeck, Deck* targetDeck)` | Initiates sync between two decks. |
| `void disableSync()` | Disables sync mode. |
| `bool isLocked() const` | Returns true if sync is locked (phase drift < threshold). |
| `SyncState getState() const` | Returns current sync state. |

**SyncState Enum:**
- `Off` — Sync disabled
- `Initializing` — Sync starting, searching for lock
- `Locked` — In sync, within tolerance
- `Drifting` — Tempo matched but phase unstable

**Usage Example:**
```cpp
SyncController sync;
sync.enableSync(deckB, &deckA);

while (shouldUpdate) {
    sync.update(deckB, deckA, deckA.getBPM(), deckB.getBPM());
    if (sync.isLocked()) {
        std::cout << "Decks are synchronized!" << std::endl;
    }
}
```

---

## CamelotAnalyzer

**Purpose:** Maps musical keys to Camelot notation and computes harmonic compatibility between tracks.

**Header:** `audio/CamelotAnalyzer.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `std::string keyToCamelot(const std::string& musicalKey) const` | Converts "C minor" → "5A". |
| `float getCompatibilityScore(const std::string& from, const std::string& to) const` | Returns score 0.0–1.0 (1.0 = identical, 0.9 = relative, 0.8 = adjacent, 0.3 = distant). |
| `int getEnergyDirection(const std::string& from, const std::string& to) const` | Returns +1 (energy up), 0 (neutral), −1 (energy down). |
| `bool isCompatible(const std::string& from, const std::string& to, float threshold)` | Returns true if compatibility score ≥ threshold (default: 0.7). |
| `std::vector<std::string> getCompatibleKeys(const std::string& current) const` | Returns Camelot wheel neighbors of specified key. |

**Compatibility Scoring:**
- 1.0 → Same key
- 0.9 → Relative key (major ↔ minor)
- 0.8 → Adjacent on Camelot wheel (±1 position)
- 0.3 → Distant (>2 steps)

**Usage Example:**
```cpp
CamelotAnalyzer camelot;
std::string keyA = "D minor", keyB = "A minor";
float score = camelot.getCompatibilityScore(keyA, keyB);

if (camelot.isCompatible(keyA, keyB)) {
    std::cout << "Great harmonic match!" << std::endl;
}
```

---

## Recorder

**Purpose:** Records audio frames to memory buffer and exports to file via optional export filename.

**Header:** `audio/Recorder.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `Recorder(std::size_t sampleRate, std::size_t channels, std::size_t capacitySeconds)` | Constructs recorder with specified sample rate, channel count, and buffer capacity. |
| `bool start()` | Begins recording. |
| `void stop()` | Stops recording. |
| `void pause()` | Pauses recording (resume to restart). |
| `void resume()` | Resumes paused recording. |
| `bool isRecording() const` | Returns true if recording is active. |
| `bool isPaused() const` | Returns true if recording is paused. |
| `void submitFrames(const float* data, std::size_t numFrames)` | Feeds audio frames into buffer. |
| `std::vector<float> getRecordedData() const` | Exports recorded samples as interleaved stereo. |
| `void clear()` | Clears buffer. |
| `float getDuration() const` | Returns recording duration in seconds. |
| `void setExportFilename(const std::string& filename)` | Sets filename for export (e.g., "my_mix.wav"). |
| `std::string getExportFilename() const` | Returns currently set export filename. |
| `std::string getExportFilenameOrDefault(const std::string& fallbackFilename) const` | Returns export filename or uses fallback if not set. |

**Usage Example:**
```cpp
Recorder recorder(44100, 2, 300);  // 5 minutes capacity
recorder.setExportFilename("my_session.wav");
recorder.start();

// ... record audio ...
while (shouldRecord) {
    recorder.submitFrames(mixOutput, 1024);
}

recorder.stop();
auto data = recorder.getRecordedData();
```

---

## TransitionCoach

**Purpose:** Analyzes tracks and crowd state to suggest optimal transition timing and coaching cues.

**Header:** `gameplay/TransitionCoach.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `std::vector<double> detectPhrases(const BeatGrid& grid) const` | Returns list of phrase start timestamps. |
| `Suggestion suggestNextTransition(const Deck& outgoing, const Deck& incoming, const EnergyCurve& energyCurve, const CamelotAnalyzer& camelot) const` | Analyzes both tracks and energy curve; returns transition advice. |
| `double calculateCountdown(double currentPos, double nextPhrase, double bpm) const` | Calculates time (seconds) until next phrase. |

**Suggestion Structure:**
- `double timestamp` — When to mix (seconds from track start)
- `std::string reason` — Coaching message
- `float confidence` — 0.0–1.0
- `bool urgency` — true if <8 beats remain
- `float energyDelta` — Energy change %
- `float harmonicScore` — 0.0–1.0

**Usage Example:**
```cpp
TransitionCoach coach;
auto suggestion = coach.suggestNextTransition(outgoing, incoming, energy, camelot);
std::cout << "Suggestion: " << suggestion.reason 
          << " (Confidence: " << suggestion.confidence << ")" << std::endl;
```

---

## MixQualityAnalyzer

**Purpose:** Evaluates overall mix quality across beatmatching, EQ balance, transitions, and harmonic alignment; detects bass clashes.

**Header:** `gameplay/MixQualityAnalyzer.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `float analyzeMix(const Deck* deckA, const Deck* deckB, const Mixer* mixer)` | Performs comprehensive mix analysis; returns 0–100 score. |
| `MixQualityComponents getComponentScores() const` | Returns breakdown of all scoring components. |
| `float getOverallScore() const` | Returns total mix score (0–100). |
| `bool hasBassClash() const` | Returns true if bass ranges conflict. |
| `void setAnalysisContext(float bpmA, float bpmB, float transitionSmoothness, const std::string& keyA, const std::string& keyB)` | Provides analysis context. |
| `void setBassClashState(bool clashDetected)` | Manually flag bass clash. |
| `bool detectBassClash(const float* spectrumA, const float* spectrumB, size_t numBins) const` | Detects bass conflict between spectrum data. |
| `float scoreBeatmatch(float bpmA, float bpmB) const` | Scores tempo matching (0–25). |
| `float scoreHarmonic(const std::string& keyA, const std::string& keyB) const` | Scores harmonic compatibility (0–25). |

**MixQualityComponents Structure:**
- `float beatmatchScore` — 0–25
- `float eqBalanceScore` — 0–25
- `float transitionScore` — 0–25
- `float harmonicScore` — 0–25

**Usage Example:**
```cpp
MixQualityAnalyzer analyzer;
float score = analyzer.analyzeMix(&deckA, &deckB, &mixer);
auto components = analyzer.getComponentScores();

std::cout << "Overall: " << score << "/100\n"
          << "Beatmatch: " << components.beatmatchScore << "/25\n"
          << "Harmonic: " << components.harmonicScore << "/25" << std::endl;
```

---

## EnergyCurve

**Purpose:** Tracks energy changes over time; provides statistics for coaching and mix analysis.

**Header:** `gameplay/EnergyCurve.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `EnergyCurve(double timeWindow, double sampleRate)` | Constructs curve with specified time window and sample rate. |
| `void addSample(float energy, double timestamp, float rms)` | Adds energy sample with RMS level. |
| `std::vector<EnergySample> getCurve() const` | Returns all samples. |
| `float getAverage() const` | Returns average energy across samples. |
| `float getPeak() const` | Returns maximum energy in window. |
| `float getDip() const` | Returns minimum energy in window. |

**EnergySample Structure:**
- `float energy` — Energy level (0.0–1.0)
- `double timestamp` — Time (seconds)
- `float rms` — RMS level (0.0–1.0)

**Usage Example:**
```cpp
EnergyCurve curve(30.0, 44100.0);
curve.addSample(0.6f, 0.0, 0.5f);
curve.addSample(0.8f, 0.5, 0.65f);

std::cout << "Average: " << curve.getAverage() 
          << ", Peak: " << curve.getPeak() << std::endl;
```

---

## CareerProgression

**Purpose:** Manages career tier progression, venue unlocking, and reputation tracking across play sessions.

**Header:** `gameplay/GameModes.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `void reset()` | Resets tier to 1 and clears reputation. |
| `void update(float crowdEnergy)` | Updates tier progress based on crowd energy (0.0–1.0). |
| `int tier() const` | Returns current career tier (1–10+). |
| `std::string currentVenueName() const` | Returns name of current venue tier. |
| `float reputation() const` | Returns reputation level (0.0–1.0). |
| `int peakTier() const` | Returns highest tier reached. |
| `bool isVenueUnlocked(int venueTier) const` | Returns true if venue tier is unlocked. |

**Tier Progression:** Monotonic (tier never decreases; unlocked venues remain available).

**Usage Example:**
```cpp
CareerProgression career;
career.update(0.8f);  // High crowd energy
std::cout << "Tier: " << career.tier() 
          << ", Venue: " << career.currentVenueName() << std::endl;
```

---

## ScoringSystem

**Purpose:** Calculates play session score based on mix quality and transition timing.

**Header:** `gameplay/GameModes.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `void reset()` | Resets score to 0. |
| `int update(float crowdEnergy, float transitionSmoothness)` | Updates score; returns points awarded this frame. |
| `int score() const` | Returns current accumulated score. |

**Usage Example:**
```cpp
ScoringSystem scoring;
scoring.reset();

while (playing) {
    int points = scoring.update(crowdEnergy, smoothness);
    std::cout << "Session score: " << scoring.score() << std::endl;
}
```

---

## CrowdStateMachine

**Purpose:** Models crowd mood based on mix quality, BPM, and energy; generates reactive feedback messages.

**Header:** `crowdAI/CrowdStateMachine.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `CrowdStateMachine(CrowdPersonality personality)` | Constructs crowd with specified personality (default: Default). |
| `CrowdOutput update(float bpm, float transitionSmoothness, float trackEnergy, float beatmatchDelta)` | Updates crowd response and returns output. |

**CrowdMood Enum:**
- `Unimpressed` — Low engagement
- `Calm` — Steady, low-energy
- `Grooving` — Active engagement
- `Hyped` — Maximum energy

**CrowdPersonality Enum:**
- `Default`, `Rave`, `Jazz`, `EDM` — Personality profiles affecting reaction thresholds

**CrowdOutput Structure:**
- `float energyMeter` — Crowd energy level (0.0–1.0)
- `std::string reaction` — Feedback message
- `CrowdMood mood` — Current mood

**Usage Example:**
```cpp
CrowdStateMachine crowd(CrowdPersonality::EDM);
auto output = crowd.update(128.0f, 0.85f, 0.7f, 0.02f);

std::cout << "Crowd: " << output.reaction 
          << " (" << static_cast<int>(output.energyMeter * 100) << "%)" << std::endl;
```

---

## SessionManager

**Purpose:** Saves and loads session state (track positions, EQ, crossfader, tier) to/from JSON; manages auto-save.

**Header:** `core/SessionState.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `SessionManager()` | Constructs manager with auto-save disabled. |
| `bool saveSession(const std::string& filePath, const SessionState& state)` | Writes session to JSON file. Returns false on I/O error. |
| `std::optional<SessionState> loadSession(const std::string& filePath)` | Loads session from JSON file. Returns `std::nullopt` on error. |
| `void enableAutoSave(bool enabled)` | Enables or disables periodic auto-saves. |
| `bool isAutoSaveEnabled() const` | Returns true if auto-save is active. |
| `void setAutoSaveInterval(int seconds)` | Sets interval between auto-saves (default: 120). |

**SessionState Structure:**
- `DeckState deckA, deckB` — Per-deck state (track path, position, EQ, tempo)
- `float crossfader` — -1.0 (A) to 1.0 (B)
- `int currentCareerTier` — Career tier
- `float crowdEnergy` — Last crowd energy value
- `std::string venueId` — Current venue ID

**DeckState Structure:**
- `std::string trackPath` — File path to loaded track
- `double playbackPosition` — Playhead position (seconds)
- `float tempoBend` — Tempo adjustment (0.5–2.0)
- `bool isPlaying` — Playback state
- `float lowGain, midGain, highGain` — EQ gains (dB)

**Usage Example:**
```cpp
SessionManager manager;
manager.enableAutoSave(true);
manager.setAutoSaveInterval(60);

SessionState state;
state.deckA.trackPath = "path/to/track.mp3";
state.crossfader = 0.0f;

bool success = manager.saveSession("session.json", state);
if (success) {
    auto loaded = manager.loadSession("session.json");
    if (loaded) {
        std::cout << "Session restored." << std::endl;
    }
}
```

---

## WaveformRenderer

**Purpose:** Renders a terminal-based ASCII waveform display for visual feedback.

**Header:** `visuals/WaveformRenderer.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `WaveformRenderer(std::size_t width, std::size_t height)` | Constructs renderer with specified dimensions (default: 72×13 characters). |
| `std::string render(const std::vector<float>& interleavedStereo) const` | Renders stereo samples to ASCII string. |

**Usage Example:**
```cpp
WaveformRenderer waveform(80, 16);
auto display = waveform.render(stereoSamples);
std::cout << display << std::endl;
```

---

## InputMapper

**Purpose:** Translates keyboard input to structured command enum values; supports single-player and two-player key maps.

**Header:** `input/InputMapper.h`

**Public API:**

| Method Signature | Description |
|---|---|
| `static InputCommand parse(const std::string& rawCommand)` | Parses a command string or key. |
| `static InputCommand parseKey(char key)` | Parses single-player keyboard input. |
| `static InputCommand parseKeyPlayer1(char key)` | Parses player 1 (QWEASD/ZXCVBN) input. |
| `static InputCommand parseKeyPlayer2(char key)` | Parses player 2 (IJKLUOP/MHNM) input. |

**Note:** `InputCommand` enum has 160+ members. See [keyboard-reference.md](keyboard-reference.md) for complete hotkey mapping.

**Usage Example:**
```cpp
auto cmd1 = InputMapper::parseKey('a');       // Single-player
auto cmd2 = InputMapper::parseKeyPlayer1('q'); // Player 1
auto cmd3 = InputMapper::parseKeyPlayer2('i'); // Player 2
```

---

## Appendix: Namespace

All classes are in the `dj` namespace unless otherwise noted in this document.

```cpp
using namespace dj;
// or: auto deck = dj::Deck();
```
