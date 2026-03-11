#include "audio/WAVExporter.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace dj {

namespace {

// Helper to write little-endian 16-bit integer
inline void writeLE16(uint8_t* buffer, uint16_t value) {
    buffer[0] = static_cast<uint8_t>(value & 0xFF);
    buffer[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
}

// Helper to write little-endian 32-bit integer
inline void writeLE32(uint8_t* buffer, uint32_t value) {
    buffer[0] = static_cast<uint8_t>(value & 0xFF);
    buffer[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    buffer[2] = static_cast<uint8_t>((value >> 16) & 0xFF);
    buffer[3] = static_cast<uint8_t>((value >> 24) & 0xFF);
}

}

WAVExporter::WAVExporter(std::size_t sampleRate, std::size_t channels, std::size_t bitDepth)
    : sampleRate_(sampleRate)
    , channels_(channels)
    , bitDepth_(bitDepth)
{
}

bool WAVExporter::exportToFile(const std::string& filename, const float* data, std::size_t numFrames) {
    if (!data || numFrames == 0) {
        return false;
    }
    
    std::size_t numSamples = numFrames * channels_;
    
    return exportToFile(filename, std::vector<float>(data, data + numSamples), numFrames);
}

bool WAVExporter::exportToFile(const std::string& filename, const std::vector<float>& data, std::size_t numFrames) {
    if (data.empty() || numFrames == 0) {
        return false;
    }
    
    std::size_t numSamples = numFrames * channels_;
    if (data.size() != numSamples) {
        return false;
    }
    
    // Convert float samples to integer format
    std::vector<uint8_t> byteBuffer;
    convertSamplesToInt(data.data(), numSamples, byteBuffer);
    
    // Calculate chunk sizes
    const std::size_t dataSize = byteBuffer.size();
    const std::size_t fmtChunkSize = 16;  // Standard fmt chunk
    const std::size_t headerSize = 12 + 8 + fmtChunkSize + 8;  // RIFF + fmt chunk header + fmt data + data chunk header
    const std::size_t fileSize = headerSize + dataSize;
    
    // Open file for binary writing
    FILE* file = fopen(filename.c_str(), "wb");
    if (!file) {
        return false;
    }
    
    // Write RIFF header
    writeRIFFHeader(file, dataSize);
    
    // Write fmt chunk
    writeFmtChunk(file);
    
    // Write data chunk header
    writeDataChunkHeader(file, dataSize);
    
    // Write audio data
    if (fwrite(byteBuffer.data(), 1, byteBuffer.size(), file) != byteBuffer.size()) {
        fclose(file);
        return false;
    }
    
    fclose(file);
    return true;
}

void WAVExporter::convertSamplesToInt(const float* samples, std::size_t numSamples,
                                       std::vector<uint8_t>& byteBuffer) {
    const std::size_t bytesPerSample = bitDepth_ / 8;
    byteBuffer.resize(numSamples * bytesPerSample);
    
    uint8_t* dst = byteBuffer.data();
    
    for (std::size_t i = 0; i < numSamples; ++i) {
        float sample = std::clamp(samples[i], -1.0f, 1.0f);
        
        if (bitDepth_ == 16) {
            // Convert to 16-bit signed integer
            int16_t intSample = static_cast<int16_t>(sample * 32767.0f);
            writeLE16(dst, static_cast<uint16_t>(intSample));
            dst += 2;
        } else if (bitDepth_ == 24) {
            // Convert to 24-bit signed integer (stored as 3 bytes)
            int32_t intSample = static_cast<int32_t>(sample * 8388607.0f);
            dst[0] = static_cast<uint8_t>(intSample & 0xFF);
            dst[1] = static_cast<uint8_t>((intSample >> 8) & 0xFF);
            dst[2] = static_cast<uint8_t>((intSample >> 16) & 0xFF);
            dst += 3;
        } else if (bitDepth_ == 32) {
            // Convert to 32-bit signed integer
            int32_t intSample = static_cast<int32_t>(sample * 2147483647.0f);
            writeLE32(dst, static_cast<uint32_t>(intSample));
            dst += 4;
        }
    }
}

void WAVExporter::writeRIFFHeader(FILE* file, std::size_t dataSize) {
    // Calculate total file size
    const std::size_t bytesPerSample = bitDepth_ / 8;
    const std::size_t fmtChunkSize = 16;
    const std::size_t fileSize = 36 + fmtChunkSize + dataSize;
    
    // Write RIFF header
    fwrite("RIFF", 1, 4, file);
    
    // Write file size - 8
    uint8_t sizeBuffer[4];
    writeLE32(sizeBuffer, static_cast<uint32_t>(fileSize - 8));
    fwrite(sizeBuffer, 1, 4, file);
    
    // Write WAVE marker
    fwrite("WAVE", 1, 4, file);
}

void WAVExporter::writeFmtChunk(FILE* file) {
    // Write fmt chunk header
    fwrite("fmt ", 1, 4, file);
    
    // fmt chunk size (always 16 for PCM)
    uint8_t chunkSizeBuffer[4];
    writeLE32(chunkSizeBuffer, 16);
    fwrite(chunkSizeBuffer, 1, 4, file);
    
    // Build fmt chunk data
    uint8_t fmtData[16];
    
    // Audio format (1 = PCM)
    writeLE16(fmtData, 1);
    
    // Number of channels
    writeLE16(fmtData + 2, static_cast<uint16_t>(channels_));
    
    // Sample rate
    writeLE32(fmtData + 4, static_cast<uint32_t>(sampleRate_));
    
    // Byte rate (sample rate * channel count * bits per sample / 8)
    uint32_t byteRate = sampleRate_ * channels_ * (bitDepth_ / 8);
    writeLE32(fmtData + 8, byteRate);
    
    // Block align (channels * bits per sample / 8)
    uint16_t blockAlign = channels_ * (bitDepth_ / 8);
    writeLE16(fmtData + 12, blockAlign);
    
    // Bits per sample
    writeLE16(fmtData + 14, static_cast<uint16_t>(bitDepth_));
    
    fwrite(fmtData, 1, 16, file);
}

void WAVExporter::writeDataChunkHeader(FILE* file, std::size_t dataSize) {
    // Write data chunk header
    fwrite("data", 1, 4, file);
    
    // Data size
    uint8_t sizeBuffer[4];
    writeLE32(sizeBuffer, static_cast<uint32_t>(dataSize));
    fwrite(sizeBuffer, 1, 4, file);
}

} // namespace dj
