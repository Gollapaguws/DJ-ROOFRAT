#include "audio/TrackLoader.h"

#include <algorithm>

#if DJ_SIM_USE_SNDFILE
#include <sndfile.h>
#endif

namespace dj {

std::optional<AudioClip> TrackLoader::loadFile(const std::string& path, std::string* errorOut) {
#if DJ_SIM_USE_SNDFILE
    SF_INFO info{};
    SNDFILE* handle = sf_open(path.c_str(), SFM_READ, &info);
    if (handle == nullptr) {
        if (errorOut != nullptr) {
            *errorOut = sf_strerror(nullptr);
        }
        return std::nullopt;
    }

    if (info.frames <= 0 || info.channels <= 0 || info.samplerate <= 0) {
        sf_close(handle);
        if (errorOut != nullptr) {
            *errorOut = "Unsupported or empty audio file";
        }
        return std::nullopt;
    }

    std::vector<float> decoded(static_cast<std::size_t>(info.frames) * static_cast<std::size_t>(info.channels), 0.0f);
    const sf_count_t framesRead = sf_readf_float(handle, decoded.data(), info.frames);
    sf_close(handle);

    if (framesRead <= 0) {
        if (errorOut != nullptr) {
            *errorOut = "Could not decode audio frames";
        }
        return std::nullopt;
    }

    AudioClip clip;
    clip.sampleRate = info.samplerate;
    clip.channels = 2;
    clip.samples.resize(static_cast<std::size_t>(framesRead) * 2U, 0.0f);

    const std::size_t channelCount = static_cast<std::size_t>(info.channels);
    for (sf_count_t i = 0; i < framesRead; ++i) {
        const std::size_t sourceBase = static_cast<std::size_t>(i) * channelCount;
        const float left = decoded[sourceBase];
        const float right = (channelCount > 1U) ? decoded[sourceBase + 1U] : left;

        clip.samples[static_cast<std::size_t>(i) * 2U] = std::clamp(left, -1.0f, 1.0f);
        clip.samples[(static_cast<std::size_t>(i) * 2U) + 1U] = std::clamp(right, -1.0f, 1.0f);
    }

    return clip;
#else
    (void)path;
    if (errorOut != nullptr) {
        *errorOut = "libsndfile is not linked. Reconfigure CMake with libsndfile installed.";
    }
    return std::nullopt;
#endif
}

} // namespace dj
