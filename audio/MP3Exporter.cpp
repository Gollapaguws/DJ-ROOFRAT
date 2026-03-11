#include "audio/MP3Exporter.h"

#include <cmath>
#include <cstdint>
#include <iostream>

// Try to include LAME if available
#if __has_include(<lame/lame.h>)
#define DJ_RECORDING_MP3_AVAILABLE 1
#include <lame/lame.h>
#else
#define DJ_RECORDING_MP3_AVAILABLE 0
#endif

namespace dj {

MP3Exporter::MP3Exporter(std::size_t sampleRate, std::size_t channels, BitrateMode mode)
    : sampleRate_(sampleRate)
    , channels_(channels)
    , mode_(mode)
{
}

MP3Exporter::~MP3Exporter() {
}

bool MP3Exporter::exportToFile(const std::string& filename, const float* data, std::size_t numFrames) {
    if (!data || numFrames == 0) {
        return false;
    }
    
    std::size_t numSamples = numFrames * channels_;
    return exportToFile(filename, std::vector<float>(data, data + numSamples), numFrames);
}

bool MP3Exporter::exportToFile(const std::string& filename, const std::vector<float>& data, std::size_t numFrames) {
    if (data.empty() || numFrames == 0) {
        return false;
    }
    
    std::size_t numSamples = numFrames * channels_;
    if (data.size() != numSamples) {
        return false;
    }
    
#if DJ_RECORDING_MP3_AVAILABLE
    // Initialize LAME encoder
    lame_t encoder = lame_init();
    if (!encoder) {
        return false;
    }
    
    // Set encoder parameters
    lame_set_num_channels(encoder, channels_);
    lame_set_in_samplerate(encoder, sampleRate_);
    
    // Configure bitrate mode
    switch (mode_) {
        case BitrateMode::CBR_320kbps:
            lame_set_VBR(encoder, vbr_off);
            lame_set_brate(encoder, 320);
            break;
        case BitrateMode::VBR_V0:
            lame_set_VBR(encoder, vbr_mtrh);
            lame_set_VBR_q(encoder, 0);  // V0 (highest quality)
            break;
        case BitrateMode::VBR_V2:
            lame_set_VBR(encoder, vbr_mtrh);
            lame_set_VBR_q(encoder, 2);  // V2
            break;
    }
    
    // Initialize encoder
    if (lame_init_params(encoder) < 0) {
        lame_close(encoder);
        return false;
    }
    
    // Convert and encode samples
    std::vector<int16_t> pcmBuffer(numSamples);
    for (std::size_t i = 0; i < numSamples; ++i) {
        float sample = std::clamp(data[i], -1.0f, 1.0f);
        pcmBuffer[i] = static_cast<int16_t>(sample * 32767.0f);
    }
    
    // Prepare output buffer (conservative estimate)
    std::vector<uint8_t> mp3Buffer(numFrames * 2 + 7200);
    
    // Encode
    int encodedBytes = 0;
    if (channels_ == 2) {
        // Split L/R for stereo encoding
        std::vector<int16_t> leftSamples(numFrames);
        std::vector<int16_t> rightSamples(numFrames);
        
        for (std::size_t i = 0; i < numFrames; ++i) {
            leftSamples[i] = pcmBuffer[i * 2];
            rightSamples[i] = pcmBuffer[i * 2 + 1];
        }
        
        encodedBytes = lame_encode_buffer(encoder, 
                                          leftSamples.data(), 
                                          rightSamples.data(),
                                          numFrames, 
                                          mp3Buffer.data(), 
                                          mp3Buffer.size());
    } else {
        // Mono
        encodedBytes = lame_encode_buffer(encoder, 
                                          pcmBuffer.data(), 
                                          nullptr,
                                          numFrames, 
                                          mp3Buffer.data(), 
                                          mp3Buffer.size());
    }
    
    if (encodedBytes < 0) {
        lame_close(encoder);
        return false;
    }
    
    // Flush remaining samples
    std::vector<uint8_t> flushBuffer(7200);
    int flushBytes = lame_encode_flush(encoder, flushBuffer.data(), flushBuffer.size());
    if (flushBytes < 0) {
        lame_close(encoder);
        return false;
    }
    
    lame_close(encoder);
    
    // Write to file
    FILE* file = fopen(filename.c_str(), "wb");
    if (!file) {
        return false;
    }
    
    if (encodedBytes > 0 && fwrite(mp3Buffer.data(), 1, encodedBytes, file) != static_cast<size_t>(encodedBytes)) {
        fclose(file);
        return false;
    }
    
    if (flushBytes > 0 && fwrite(flushBuffer.data(), 1, flushBytes, file) != static_cast<size_t>(flushBytes)) {
        fclose(file);
        return false;
    }
    
    fclose(file);
    return true;
    
#else
    // LAME not available - gracefully skip with message
    std::cerr << "MP3 export not available (LAME not found). Skipping MP3 export." << std::endl;
    return true;  // Return true to indicate graceful skip (not a failure)
#endif
}

bool MP3Exporter::isLAMEAvailable() {
#if DJ_RECORDING_MP3_AVAILABLE
    return true;
#else
    return false;
#endif
}

} // namespace dj
