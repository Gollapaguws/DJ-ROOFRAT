#include "audio/TrackLoader.h"

#include <algorithm>
#include <cctype>
#include <string>

#include "audio/BPMDetector.h"
#include "audio/KeyDetector.h"
#include "audio/MetadataParser.h"

#if DJ_SIM_USE_SNDFILE
#include <sndfile.h>
#endif

// MP3 support via dr_mp3
#define DR_MP3_IMPLEMENTATION
#include "vendor/dr_mp3.h"

namespace dj {

// Helper: Check if file extension is .mp3 (case-insensitive)
static bool isMp3File(const std::string& path) {
    if (path.length() < 4) return false;
    std::string ext = path.substr(path.length() - 4);
    for (auto& ch : ext) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return ext == ".mp3";
}

// Helper: Load MP3 file using dr_mp3
static std::optional<AudioClip> loadMp3File(const std::string& path, std::string* errorOut) {
    drmp3 mp3;
    if (!drmp3_init_file(&mp3, path.c_str(), nullptr)) {
        if (errorOut != nullptr) {
            *errorOut = "Failed to open or decode MP3 file";
        }
        return std::nullopt;
    }

    if (mp3.sampleRate <= 0 || mp3.channels <= 0) {
        drmp3_uninit(&mp3);
        if (errorOut != nullptr) {
            *errorOut = "Invalid MP3 stream properties (sample rate or channels)";
        }
        return std::nullopt;
    }

    // Get total frame count
    drmp3_uint64 frameCount = drmp3_get_pcm_frame_count(&mp3);
    if (frameCount <= 0) {
        drmp3_uninit(&mp3);
        if (errorOut != nullptr) {
            *errorOut = "Could not determine MP3 frame count";
        }
        return std::nullopt;
    }

    // Allocate buffer for reading frames (in floats, which is what dr_mp3 outputs)
    std::vector<float> decoded(static_cast<std::size_t>(frameCount) * static_cast<std::size_t>(mp3.channels), 0.0f);
    
    // Read all PCM frames as float
    drmp3_uint64 framesRead = drmp3_read_pcm_frames_f32(&mp3, frameCount, decoded.data());
    
    // Save mp3 fields before uninit (avoiding undefined behavior)
    const int sampleRate = static_cast<int>(mp3.sampleRate);
    const std::size_t channelCount = static_cast<std::size_t>(mp3.channels);
    drmp3_uninit(&mp3);

    if (framesRead <= 0) {
        if (errorOut != nullptr) {
            *errorOut = "Could not decode MP3 audio frames";
        }
        return std::nullopt;
    }

    AudioClip clip;
    clip.sampleRate = sampleRate;
    clip.channels = 2;
    clip.samples.resize(static_cast<std::size_t>(framesRead) * 2U, 0.0f);
    for (drmp3_uint64 i = 0; i < framesRead; ++i) {
        const std::size_t sourceBase = static_cast<std::size_t>(i) * channelCount;
        const float left = decoded[sourceBase];
        const float right = (channelCount > 1U) ? decoded[sourceBase + 1U] : left;

        clip.samples[static_cast<std::size_t>(i) * 2U] = std::clamp(left, -1.0f, 1.0f);
        clip.samples[(static_cast<std::size_t>(i) * 2U) + 1U] = std::clamp(right, -1.0f, 1.0f);
    }

    return clip;
}

// Helper: Enrich audio clip with metadata, BPM detection, and key detection
static std::optional<AudioClip> enrichAudioClipWithMetadata(AudioClip& clip, const std::string& path, std::string* errorOut) {
    // Parse file metadata (ID3 tags)
    auto metadata = MetadataParser::parseFile(path, nullptr);
    if (!metadata.has_value()) {
        metadata = TrackMetadata();
    }
    
    // Calculate duration
    if (clip.sampleRate > 0) {
        metadata->durationSeconds = static_cast<float>(clip.frameCount()) / static_cast<float>(clip.sampleRate);
    }
    
    // Detect BPM if not already present in metadata
    if (!metadata->bpm.has_value() && clip.frameCount() >= static_cast<std::size_t>(clip.sampleRate) * 2) {
        auto detectedBpm = BPMDetector::estimate(clip, 70.0f, 180.0f);
        if (detectedBpm.has_value()) {
            metadata->bpm = *detectedBpm;
        }
    }
    
    // Detect key if not already present in metadata
    if (!metadata->key.has_value() && clip.frameCount() >= static_cast<std::size_t>(clip.sampleRate) * 2) {
        auto detectedKey = KeyDetector::detect(clip, nullptr);
        if (detectedKey.has_value()) {
            metadata->key = *detectedKey;
        }
    }
    
    // Store metadata in clip
    clip.metadata_ = *metadata;
    
    return clip;
}

std::optional<AudioClip> TrackLoader::loadFile(const std::string& path, std::string* errorOut) {
    // Check if file is MP3 and try MP3 decoder first
    if (isMp3File(path)) {
        auto clipOpt = loadMp3File(path, errorOut);
        if (!clipOpt.has_value()) {
            return std::nullopt;
        }
        
        auto clip = std::move(*clipOpt);
        // Enrich with metadata
        return enrichAudioClipWithMetadata(clip, path, errorOut);
    }

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

    // Enrich with metadata
    return enrichAudioClipWithMetadata(clip, path, errorOut);
#else
    (void)path;
    if (errorOut != nullptr) {
        *errorOut = "libsndfile is not linked. Reconfigure CMake with libsndfile installed.";
    }
    return std::nullopt;
#endif
}

} // namespace dj
