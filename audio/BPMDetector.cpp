#include "audio/BPMDetector.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace dj {

std::optional<float> BPMDetector::estimate(const AudioClip& clip, float minBpm, float maxBpm) {
    if (clip.empty() || minBpm <= 0.0f || maxBpm <= minBpm) {
        return std::nullopt;
    }

    constexpr int envelopeRate = 200;
    const std::size_t step = static_cast<std::size_t>(std::max(1, clip.sampleRate / envelopeRate));
    const std::size_t frames = clip.frameCount();

    if (frames < step * 400U) {
        return std::nullopt;
    }

    std::vector<float> envelope;
    envelope.reserve(frames / step);

    for (std::size_t i = 0; i < frames; i += step) {
        const auto sample = clip.frameAt(static_cast<double>(i));
        envelope.push_back(std::abs((sample[0] + sample[1]) * 0.5f));
    }

    if (envelope.size() < 200U) {
        return std::nullopt;
    }

    float smoothed = envelope.front();
    for (float& value : envelope) {
        smoothed = (smoothed * 0.85f) + (value * 0.15f);
        value = smoothed;
    }

    float mean = 0.0f;
    for (const float value : envelope) {
        mean += value;
    }
    mean /= static_cast<float>(envelope.size());

    for (float& value : envelope) {
        value -= mean;
    }

    const std::size_t minLag = static_cast<std::size_t>((60.0f * envelopeRate) / maxBpm);
    const std::size_t maxLag = static_cast<std::size_t>((60.0f * envelopeRate) / minBpm);
    if (maxLag >= envelope.size()) {
        return std::nullopt;
    }

    float bestCorrelation = -1.0f;
    std::size_t bestLag = minLag;

    for (std::size_t lag = minLag; lag <= maxLag; ++lag) {
        float corr = 0.0f;
        for (std::size_t i = 0; (i + lag) < envelope.size(); ++i) {
            corr += envelope[i] * envelope[i + lag];
        }

        if (corr > bestCorrelation) {
            bestCorrelation = corr;
            bestLag = lag;
        }
    }

    if (bestCorrelation <= 0.0f || bestLag == 0U) {
        return std::nullopt;
    }

    const float bpm = (60.0f * envelopeRate) / static_cast<float>(bestLag);
    return bpm;
}

} // namespace dj
