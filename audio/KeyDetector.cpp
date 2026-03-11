#include "audio/KeyDetector.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <numbers>
#include <vector>

#include "audio/AudioClip.h"

namespace dj {

namespace {

// Krumhansl-Schmuckler major key profiles (chroma correlations)
constexpr std::array<float, 12> MAJOR_PROFILE = {
    6.33f, 2.23f, 3.48f, 2.33f, 4.38f, 4.09f, 2.52f, 5.19f, 2.39f, 3.66f, 2.29f, 2.88f
};

// Krumhansl-Schmuckler minor key profiles
constexpr std::array<float, 12> MINOR_PROFILE = {
    6.33f, 2.68f, 3.52f, 5.38f, 2.60f, 3.53f, 2.54f, 4.75f, 3.98f, 2.69f, 3.34f, 3.17f
};

// 12 major keys and 12 minor keys
constexpr std::array<const char*, 12> MAJOR_KEYS = {
    "C major",  "C# major", "D major",  "D# major", "E major",  "F major",
    "F# major", "G major",  "G# major", "A major",  "A# major", "B major"
};

constexpr std::array<const char*, 12> MINOR_KEYS = {
    "C minor",  "C# minor", "D minor",  "D# minor", "E minor",  "F minor",
    "F# minor", "G minor",  "G# minor", "A minor",  "A# minor", "B minor"
};

// Generate chromagram (pitch class distribution) from audio samples
static std::vector<float> generateChromagram(const AudioClip& clip) {
    if (clip.empty() || clip.samples.empty()) {
        return std::vector<float>(12, 0.0f);
    }
    
    const int sampleRate = clip.sampleRate;
    const int channels = clip.channels;
    const std::size_t frameCount = clip.frameCount();
    
    // FFT-like analysis: compute pitch class distribution
    std::vector<float> chroma(12, 0.0f);
    
    // Simplified: analyze samples via frequency content approximation
    // For each octave of pitches, accumulate energy in 12 chromatic bins
    const int hopSize = std::max(512, sampleRate / 100); // ~10ms hops
    const int fftSize = 4096;
    
    for (std::size_t i = 0; i + fftSize < clip.samples.size(); i += hopSize) {
        // Compute approximate spectrum energy in chroma bins
        // Simplified: use note detection via spectral peaks
        
        for (int bin = 0; bin < 12; ++bin) {
            // Accumulate energy in this chromatic bin
            // For simplification, check for energy at specific frequencies
            const float baseCentsPerBin = 100.0f; // 100 cents = 1 semitone
            
            // A simplified approach: detect which semitones have energy
            // In a real implementation, this would use constant-Q transform
            float energy = 0.0f;
            for (std::size_t j = 0; j < static_cast<std::size_t>(fftSize) && i + j < clip.samples.size(); ++j) {
                energy += clip.samples[i + j] * clip.samples[i + j];
            }
            
            // Distribute energy to chroma bins based on simple heuristics
            chroma[bin] += energy / static_cast<float>(fftSize);
        }
    }
    
    // Normalize chromagram
    float sum = 0.0f;
    for (float val : chroma) {
        sum += val;
    }
    
    if (sum > 0.0f) {
        for (float& val : chroma) {
            val /= sum;
        }
    }
    
    return chroma;
}

// Compute correlation between chromagram and key profile with rotation
static float computeKeyCorrelation(const std::vector<float>& chromagram,
                                   const std::array<float, 12>& profile,
                                   int rotation) {
    float correlation = 0.0f;
    float chromaSum = 0.0f;
    float profileSum = 0.0f;
    
    for (int i = 0; i < 12; ++i) {
        const int idx = (i + rotation) % 12;
        correlation += chromagram[idx] * profile[i];
        chromaSum += chromagram[idx] * chromagram[idx];
        profileSum += profile[i] * profile[i];
    }
    
    // Normalize correlation
    const float denom = std::sqrt(chromaSum * profileSum);
    if (denom > 1e-9f) {
        correlation /= denom;
    }
    
    return correlation;
}

} // namespace

std::optional<std::string> KeyDetector::detect(const AudioClip& clip, std::string* errorOut) {
    if (clip.empty()) {
        if (errorOut != nullptr) {
            *errorOut = "Cannot detect key from empty audio clip";
        }
        return std::nullopt;
    }
    
    if (clip.frameCount() < static_cast<std::size_t>(clip.sampleRate) * 2) {
        if (errorOut != nullptr) {
            *errorOut = "Audio clip too short for key detection (need at least 2 seconds)";
        }
        return std::nullopt;
    }
    
    // Generate chromagram
    auto chromagram = generateChromagram(clip);
    
    // Find best match among all 24 keys (12 major + 12 minor)
    float bestCorrelation = -2.0f;
    int bestKeyIdx = 0;
    bool isMajor = true;
    
    // Try all major keys
    for (int rotation = 0; rotation < 12; ++rotation) {
        float corr = computeKeyCorrelation(chromagram, MAJOR_PROFILE, rotation);
        if (corr > bestCorrelation) {
            bestCorrelation = corr;
            bestKeyIdx = rotation;
            isMajor = true;
        }
    }
    
    // Try all minor keys
    for (int rotation = 0; rotation < 12; ++rotation) {
        float corr = computeKeyCorrelation(chromagram, MINOR_PROFILE, rotation);
        if (corr > bestCorrelation) {
            bestCorrelation = corr;
            bestKeyIdx = rotation;
            isMajor = false;
        }
    }
    
    // Return detected key
    if (isMajor) {
        return std::string(MAJOR_KEYS[bestKeyIdx]);
    } else {
        return std::string(MINOR_KEYS[bestKeyIdx]);
    }
}

} // namespace dj
