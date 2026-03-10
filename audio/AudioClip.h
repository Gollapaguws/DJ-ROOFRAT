#pragma once

#include <array>
#include <cstddef>
#include <vector>

namespace dj {

struct AudioClip {
    int sampleRate = 44100;
    int channels = 2;
    std::vector<float> samples;

    bool empty() const;
    std::size_t frameCount() const;
    std::array<float, 2> frameAt(double frameIndex) const;

    static AudioClip generateTestTone(float frequencyHz, float seconds, int sampleRate = 44100);
};

} // namespace dj
