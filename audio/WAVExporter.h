#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace dj {

class WAVExporter {
public:
    // Constructor: sample_rate (Hz), channels, bit_depth (16, 24, or 32)
    WAVExporter(std::size_t sampleRate, std::size_t channels, std::size_t bitDepth);
    
    ~WAVExporter() = default;
    
    // Export float samples to WAV file
    // data: interleaved stereo float samples [-1.0, 1.0]
    // numFrames: number of audio frames (not samples)
    bool exportToFile(const std::string& filename, const float* data, std::size_t numFrames);
    
    // Export vector of float samples
    bool exportToFile(const std::string& filename, const std::vector<float>& data, std::size_t numFrames);
    
private:
    std::size_t sampleRate_;
    std::size_t channels_;
    std::size_t bitDepth_;
    
    // Convert float sample [-1.0, 1.0] to integer samples
    void convertSamplesToInt(const float* samples, std::size_t numSamples, 
                              std::vector<uint8_t>& byteBuffer);
    
    // Write RIFF header
    void writeRIFFHeader(FILE* file, std::size_t dataSize);
    
    // Write fmt chunk
    void writeFmtChunk(FILE* file);
    
    // Write data chunk header
    void writeDataChunkHeader(FILE* file, std::size_t dataSize);
};

} // namespace dj
