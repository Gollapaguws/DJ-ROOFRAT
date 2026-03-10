#include "audio/AudioClip.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace dj {

namespace {

std::array<float, 2> frameAtIndex(const AudioClip& clip, std::size_t frameIndex) {
    if (clip.channels <= 0 || clip.samples.empty()) {
        return {0.0f, 0.0f};
    }

    const std::size_t base = frameIndex * static_cast<std::size_t>(clip.channels);
    if (base >= clip.samples.size()) {
        return {0.0f, 0.0f};
    }

    if (clip.channels == 1) {
        const float mono = clip.samples[base];
        return {mono, mono};
    }

    const float left = clip.samples[base];
    const float right = clip.samples[base + 1];
    return {left, right};
}

} // namespace

bool AudioClip::empty() const {
    return channels <= 0 || samples.empty();
}

std::size_t AudioClip::frameCount() const {
    if (channels <= 0) {
        return 0;
    }
    return samples.size() / static_cast<std::size_t>(channels);
}

std::array<float, 2> AudioClip::frameAt(double frameIndex) const {
    const std::size_t frames = frameCount();
    if (frames == 0) {
        return {0.0f, 0.0f};
    }

    const double clamped = std::clamp(frameIndex, 0.0, static_cast<double>(frames - 1));
    const std::size_t i0 = static_cast<std::size_t>(clamped);
    const std::size_t i1 = std::min(i0 + 1, frames - 1);
    const float t = static_cast<float>(clamped - static_cast<double>(i0));

    const auto a = frameAtIndex(*this, i0);
    const auto b = frameAtIndex(*this, i1);

    return {
        (a[0] * (1.0f - t)) + (b[0] * t),
        (a[1] * (1.0f - t)) + (b[1] * t),
    };
}

AudioClip AudioClip::generateTestTone(float frequencyHz, float seconds, int outputSampleRate) {
    AudioClip clip;
    clip.sampleRate = std::max(8000, outputSampleRate);
    clip.channels = 2;

    const std::size_t totalFrames = static_cast<std::size_t>(std::max(0.5f, seconds) * clip.sampleRate);
    clip.samples.resize(totalFrames * 2U, 0.0f);

    for (std::size_t i = 0; i < totalFrames; ++i) {
        const float time = static_cast<float>(i) / static_cast<float>(clip.sampleRate);
        const float phase = 2.0f * std::numbers::pi_v<float> * frequencyHz * time;
        const float fundamental = std::sin(phase);
        const float harmonic = 0.4f * std::sin(phase * 2.0f);
        const float lfo = 0.2f * std::sin((2.0f * std::numbers::pi_v<float> * 0.25f) * time);

        const float left = 0.35f * (fundamental + harmonic + lfo);
        const float right = 0.35f * (fundamental + harmonic - lfo);

        clip.samples[(i * 2U)] = left;
        clip.samples[(i * 2U) + 1U] = right;
    }

    return clip;
}

} // namespace dj
