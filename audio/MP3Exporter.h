#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace dj {

class MP3Exporter {
public:
    enum class BitrateMode {
        CBR_320kbps,
        VBR_V0,
        VBR_V2
    };
    
    // Constructor: sample_rate (Hz), channels, bitrate mode
    MP3Exporter(std::size_t sampleRate, std::size_t channels, BitrateMode mode);
    
    ~MP3Exporter();
    
    // Export float samples to MP3 file
    // data: interleaved stereo float samples [-1.0, 1.0]
    // numFrames: number of audio frames (not samples)
    // Returns: true if export was successful or LAME not available (graceful skip)
    bool exportToFile(const std::string& filename, const float* data, std::size_t numFrames);
    
    // Export vector of float samples
    bool exportToFile(const std::string& filename, const std::vector<float>& data, std::size_t numFrames);
    
    // Check if LAME is available at runtime
    static bool isLAMEAvailable();
    
private:
    std::size_t sampleRate_;
    std::size_t channels_;
    BitrateMode mode_;
};

} // namespace dj
