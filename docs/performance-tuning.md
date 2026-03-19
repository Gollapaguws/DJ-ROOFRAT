# Performance Tuning Guide for DJ-ROOFRAT

## Overview

DJ-ROOFRAT runs a real-time audio processing loop targeting 30 FPS (33ms frame budget). Three components drive most CPU cost:

1. **FFT/Spectrum Analysis** — 1024-sample FFT window per frame for frequency-domain visualization and analysis
2. **BPM Detection** — Autocorrelation-based tempo estimation (runs once on track load)
3. **Main Rendering Loop** — Deck playback, mixing, effect chains, and terminal HUD updates

Understanding these bottlenecks and how to profile them is essential for maintaining responsive real-time performance.

---

## Frame Budget Math

The frame budget constraint is fundamental to real-time audio systems:

$$\text{Frame Budget} = \frac{1}{\text{Frame Rate}} \text{ (in seconds)}$$

**Example calculation:**
- Target frame rate: 30 FPS
- Frame budget: $\frac{1}{30} \approx 0.0333$ seconds = **33 ms**
- Sample rate: 44100 Hz
- Samples per frame: 512
- Audio epoch: $\frac{512}{44100} \approx 0.0116$ seconds = **11.6 ms**

This means each frame has ~33 ms wall-clock time to:
- Advance sample playback by 512 samples
- Run FFT analysis on the spectrum window
- Process all effects and mixing
- Render terminal output
- Handle keyboard input

**Safe buffer sizing rule:** Always ensure your frame budget comfortably exceeds the sum of all component timings. A good safety margin is **2–3× your target component time**. For example, if FFT takes 15 ms, aim for a 30–45 ms frame budget.

---

## Profiling with `std::chrono`

Use `std::chrono::high_resolution_clock` to measure any code section at microsecond precision:

```cpp
#include <chrono>

// Example: Profile a code block
auto t0 = std::chrono::high_resolution_clock::now();

// ... code to profile ...

auto t1 = std::chrono::high_resolution_clock::now();
auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
auto milliseconds = microseconds / 1000.0;

std::cerr << "Elapsed: " << milliseconds << " ms\n";
```

**Integration pattern:** For recurring profiling, store timings in a circular buffer and print percentiles in the HUD or log:

```cpp
std::vector<double> timings;
timings.push_back(milliseconds);
if (timings.size() > 1000) timings.erase(timings.begin());

// Print max, median, p95 for this component
```

This reveals whether a code section is consistently slow (bad algorithmic design) or sporadically slow (cache misses, memory pressure, OS jitter).

---

## Established Benchmark Targets

The following targets are derived from profiling on a mid-range Intel i5/Ryzen 5 system:

| Component | Target | Notes |
|-----------|--------|-------|
| FFT (SpectrumAnalyzer, 1024-sample window) | < 20 ms | Cooley-Tukey O(N log N); ~3× speedup vs. naive DFT |
| BPM Detection (5-second clip) | < 100 ms | Runs once on track load, not per-frame |
| Main loop iteration | < 33 ms | 30 FPS target (includes all processing) |
| Beat grid generation (3-minute track) | < 50 ms | Runs once at track load; onset detection + clustering |
| Session save/load | < 5 ms | JSON serialization (PortAudio/file I/O not included) |
| Spectrum display (20 bands) | < 2 ms | Logarithmic band aggregation + terminal render |
| Effect chain (one deck, 3 effects) | < 8 ms | Per-sample filter processing; FIR/IIR low order |

**Acceptance criteria:** Your profiling measurements should be within these targets for the main release build. Deviations warrant investigation—either optimize or adjust the frame rate.

---

## Top 3 Hotspots (from Phase 2 Profiling)

### 1. `SpectrumAnalyzer::processSamples()` — FFT Engine

**Why it's hot:** Runs every frame on a 1024-sample window.

**Original bottleneck:** Naive DFT (O(N²)) computed full complex exponentials per frequency bin.

**Optimization applied:** Cooley-Tukey FFT (O(N log N)), ~3× speedup. Hann window coefficients pre-computed in constructor.

**Profiling target:** < 20 ms per frame

**Further optimization opportunities:**
- Use SSE/AVX intrinsics if profiling still shows this is the bottleneck (unlikely on modern CPUs)
- Consider fixed-point arithmetic if you profile severe memory pressure

### 2. `BPMDetector::estimate()` — Autocorrelation Tap Detection

**Why it's hot (potentially):** Autocorrelation of a 5-second envelope at full resolution can be O(N²).

**Mitigation applied:** Runs only once on track load, not per-frame. Result cached in Track metadata.

**Profiling target:** < 100 ms (total, not per-frame)

**Current strategy:** Avoid re-profiling BPM after load. For real-time BPM swing detection (beat variance), use a lightweight tap-tempo detector instead.

### 3. `Deck::nextFrame()` — Full Sample Production

**Why it's hot:** Every frame, must:
- Load sample(s) via interpolation
- Apply per-deck EQ filters
- Generate spectrum window for analysis
- Mix with crossfader

**Optimizations in place:**
- Sample interpolation uses linear (not cubic); trade-off between quality and speed
- EQ filter order kept low (typically 2–3 poles per band)
- No dynamic allocations in the hot path
- Spectrum buffer pre-allocated, reused across frames

**Profiling target:** < 33 ms for the entire frame (all decks + mix)

---

## Heap Allocation in Audio Loop: The Anti-Pattern

Dynamic memory allocation (via `malloc`, `new`, or `std::vector` constructor) anywhere in the hot path causes **audio glitches and latency spikes** because:

1. Heap allocator can block other system threads (lock contention)
2. GC or fragmentation can cause unpredictable stalls
3. Cache misses spike on newly-allocated memory

### Anti-Pattern (❌ BAD):

```cpp
void Deck::nextFrame() {
    // WRONG: allocates new buffer every frame
    std::vector<float> temp(1024);
    std::vector<float> eqOutput(1024);
    
    // ... fill buffers, process ...
}
```

**Result:** Each frame triggers heap allocation penalty; latency jitter audible to listener.

### Fixed Pattern (✅ GOOD):

```cpp
class Deck {
    std::vector<float> tempBuffer;      // allocated once in constructor
    std::vector<float> eqOutputBuffer;  // pre-allocated
    
public:
    Deck() : tempBuffer(1024), eqOutputBuffer(1024) {}
    
    void nextFrame() {
        // reuse pre-allocated buffers, no new/delete
        // ... process ...
    }
};
```

**Result:** All allocations happen before the hot loop; frame timing is predictable.

### Rule of Thumb:

- **Constructor/Setup:** Safe to allocate, initialize, resize containers
- **Hot path (nextFrame, audio callback):** No new/delete/allocate; only reuse existing memory

Verify your code with a static analyzer or code review. VS Code's IntelliSense and clang-tidy can flag `new`/`delete` in suspicious contexts.

---

## FFT Interpretation Guide

`SpectrumAnalyzer::getFrequencyBands()` returns a 20-element float array covering 20 Hz–20 kHz on a logarithmic scale.

### Band Layout (Frequency Zones):

| Bands | Frequency Range | Musical Role | Typical Content |
|-------|-----------------|--------------|-----------------|
| 0–2 | 20–125 Hz | **Bass** | Kick drum, sub bass, low rumble |
| 3–5 | 125–500 Hz | **Low-mids** (boxiness) | Bass guitar body, drum toms, muddiness |
| 6–10 | 500–4 kHz | **Mids** (presence) | Vocals, snare, lead synths |
| 11–15 | 4–12 kHz | **High-mids/Presence** | Sibilance, cymbal edge, air |
| 16–19 | 12–20 kHz | **Air/Brilliance** | Shimmer, high-frequency detail |

### Reading Spectrum Values:

- **0.0–0.3:** Quiet or absent in this band
- **0.3–0.7:** Moderate energy; song has presence here
- **\> 0.7:** Strong energy; dominant band for that track's character

### Bass Clash Detection:

A common mixing challenge is **simultaneous heavy bass on both decks**, which muddies the mix. Detect the clash:

```cpp
float deckA_Bass = getFrequencyBands(DECK_A)[0] + 
                   getFrequencyBands(DECK_A)[1] + 
                   getFrequencyBands(DECK_A)[2];
float deckB_Bass = getFrequencyBands(DECK_B)[0] + 
                   getFrequencyBands(DECK_B)[1] + 
                   getFrequencyBands(DECK_B)[2];

if (deckA_Bass > 0.7 && deckB_Bass > 0.7) {
    // Coaching message: "Bass clash! Lower deck B bass or high-pass filter."
}
```

High bass clash is often a sign the DJ should isolate one deck's low end or EQ one down.

---

## Benchmark Runner Pattern

DJ-ROOFRAT includes a `tests/benchmarks/` harness for isolated timing of components. Example structure:

```cpp
// tests/benchmarks/BenchmarkFFT.cpp
#include <chrono>
#include "audio/SpectrumAnalyzer.h"

int main() {
    SpectrumAnalyzer analyzer(44100);
    std::vector<float> samples(1024);
    
    // Warm-up
    for (int i = 0; i < 100; ++i) {
        analyzer.processSamples(samples);
    }
    
    // Timed runs
    std::vector<double> times;
    for (int i = 0; i < 1000; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        analyzer.processSamples(samples);
        auto t1 = std::chrono::high_resolution_clock::now();
        times.push_back(
            std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() / 1000.0
        );
    }
    
    // Report percentiles
    std::sort(times.begin(), times.end());
    std::cerr << "FFT Benchmark (ms):\n"
              << "  Min:  " << times[0] << "\n"
              << "  p50:  " << times[500] << "\n"
              << "  p95:  " << times[950] << "\n"
              << "  Max:  " << times[999] << "\n";
    
    return (times[950] < 20.0) ? 0 : 1;  // Pass if p95 < 20ms
}
```

**Key practices:**
1. Warm up the CPU cache (100+ iterations before timing)
2. Run 1000+ iterations for statistical significance
3. Report percentiles, not just average (median and p95 reveal worst-case behavior)
4. Use `Release` build for benchmarking (see below)

---

## Build Configuration Notes

### Debug vs. Release

| Config | Purpose | Overhead | When to Use |
|--------|---------|----------|------------|
| **Debug** | Development, debugging with breakpoints, assert checks | 20–40% slower, iterator checks, extra validation | During development; NOT for benchmarking |
| **Release** | Production, benchmark target measurement, profiling | Optimized; asserts disabled; inlining enabled | Benchmarking, final performance validation |

### Configure and Build for Release:

```powershell
# Configure
cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64

# Build Release configuration
cmake --build build-vs --config Release

# Run benchmarks on Release builds
.\build-vs\Release\BenchmarkFFT.exe
.\build-vs\Release\BenchmarkBPMDetector.exe
```

**Never benchmark Debug builds.** You will get inflated timings that don't reflect the production experience.

---

## Optimization Workflow

1. **Establish a baseline** — Run benchmarks on Release build; record current timings
2. **Profile with chrono** — Identify which component(s) exceed target
3. **Minimize allocations** — Ensure hot paths use pre-allocated buffers
4. **Consider algorithmic improvements** — O(N²) → O(N log N), etc.
5. **Re-benchmark** — Confirm improvement, check for regressions in adjacent components
6. **Document trade-offs** — Comment why you chose algorithm, quality level, or buffer size

---

## Example: Profiling a Custom Effect

Suppose you add a new reverb effect and want to ensure it stays within budget:

```cpp
// audio/CustomReverb.cpp
class CustomReverb {
    std::vector<float> delayLine;  // pre-allocate
    
public:
    CustomReverb(int sampleRate)
        : delayLine(sampleRate / 2)  // 0.5 sec buffer
    {
    }
    
    void process(const float* in, float* out, int samples) {
        auto t0 = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < samples; ++i) {
            // reverb processing
            out[i] = in[i];  // simplified
        }
        
        auto t1 = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() / 1000.0;
        
        // Log or assert if exceeds budget
        if (ms > 5.0) {
            std::cerr << "WARNING: CustomReverb took " << ms << " ms (budget: 5 ms)\n";
        }
    }
};
```

Integrate this into your benchmark harness and run nightly or pre-commit checks.

---

## Further Reading

- **Frame Rate & Audio:** See [docs/architecture-diagram.md](docs/architecture-diagram.md) for the main loop structure.
- **Real-Time Audio Design:** [cycfi/q](https://github.com/cycfi/Q) library demonstrates real-time effect design patterns.
- **FFT Optimization:** Danielson-Lanczos algorithm (Cooley-Tukey variant) used in DJ-ROOFRAT; see `audio/FFTEngine.cpp` for implementation.

---

**Last Updated:** Phase 4.5 (March 2026)
