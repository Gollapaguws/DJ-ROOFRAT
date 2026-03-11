#include "audio/MetadataParser.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>

namespace dj {

// Helper: Extract string from binary at offset, up to maxLen bytes (stops at null terminator)
static std::string extractString(const std::vector<uint8_t>& data, std::size_t offset, std::size_t maxLen) {
    if (offset >= data.size()) {
        return {};
    }
    
    std::string result;
    for (std::size_t i = 0; i < maxLen && offset + i < data.size(); ++i) {
        const uint8_t ch = data[offset + i];
        if (ch == 0) {
            break;
        }
        if (ch >= 32 && ch < 127) {
            result += static_cast<char>(ch);
        }
    }
    
    return result;
}

// Helper: Check if file extension is MP3
static bool isMp3File(const std::string& path) {
    if (path.length() < 4) {
        return false;
    }
    std::string ext = path.substr(path.length() - 4);
    for (auto& ch : ext) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return ext == ".mp3";
}

// Helper: Parse ID3v1 tags (128 bytes at end of file)
static std::optional<TrackMetadata> parseID3v1(const std::vector<uint8_t>& fileData, std::string* errorOut) {
    if (fileData.size() < 128) {
        return std::nullopt;
    }
    
    // Check for "TAG" marker at offset (fileSize - 128)
    const std::size_t tagOffset = fileData.size() - 128;
    if (fileData[tagOffset] != 'T' || fileData[tagOffset + 1] != 'A' || fileData[tagOffset + 2] != 'G') {
        return std::nullopt;
    }
    
    TrackMetadata metadata;
    
    // Extract fields from ID3v1
    metadata.title = extractString(fileData, tagOffset + 3, 30);
    metadata.artist = extractString(fileData, tagOffset + 33, 30);
    std::string album = extractString(fileData, tagOffset + 63, 30);
    std::string yearStr = extractString(fileData, tagOffset + 93, 4);
    metadata.genre = extractString(fileData, tagOffset + 125, 1);
    
    // Try to parse BPM if available in comment
    std::string comment = extractString(fileData, tagOffset + 97, 28);
    if (!comment.empty()) {
        try {
            metadata.bpm = std::stof(comment);
        } catch (...) {
            // BPM not available in comment
        }
    }
    
    return metadata;
}

// Helper: Parse ID3v2 frame
static bool parseID3v2Frame(const std::vector<uint8_t>& data, std::size_t& offset, TrackMetadata& metadata) {
    // ID3v2 frame header: 4 bytes ID, 3 bytes size (synchsafe), 2 bytes flags
    if (offset + 10 > data.size()) {
        return false;
    }
    
    char frameID[5] = {0};
    std::memcpy(frameID, &data[offset], 4);
    
    // Parse synchsafe size (7 bits per byte)
    uint32_t size = ((data[offset + 4] & 0x7F) << 21) |
                    ((data[offset + 5] & 0x7F) << 14) |
                    ((data[offset + 6] & 0x7F) << 7) |
                    (data[offset + 7] & 0x7F);
    
    offset += 10;
    
    if (offset + size > data.size()) {
        return false;
    }
    
    // Extract text encoding (first byte)
    uint8_t encoding = data[offset];
    std::size_t textStart = offset + 1;
    
    // Extract text value (simplified: assume ISO-8859-1 for now)
    std::string value = extractString(data, textStart, size - 1);
    
    offset += size;
    
    // Map frame IDs to metadata fields
    if (std::string(frameID) == "TIT2") {
        metadata.title = value;
    } else if (std::string(frameID) == "TPE1") {
        metadata.artist = value;
    } else if (std::string(frameID) == "TCON") {
        metadata.genre = value;
    } else if (std::string(frameID) == "TBPM") {
        try {
            metadata.bpm = std::stof(value);
        } catch (...) {
            // BPM not parseable
        }
    } else if (std::string(frameID) == "TKEY") {
        metadata.key = value;
    }
    
    return true;
}

// Helper: Parse ID3v2 tags at start of file
static std::optional<TrackMetadata> parseID3v2(const std::vector<uint8_t>& fileData, std::string* errorOut) {
    // ID3v2 header: "ID3" (3 bytes), version (2 bytes), flags (1 byte), size (4 synchsafe bytes)
    if (fileData.size() < 10 || fileData[0] != 'I' || fileData[1] != 'D' || fileData[2] != '3') {
        return std::nullopt;
    }
    
    // Parse synchsafe tag size
    uint32_t tagSize = ((fileData[6] & 0x7F) << 21) |
                       ((fileData[7] & 0x7F) << 14) |
                       ((fileData[8] & 0x7F) << 7) |
                       (fileData[9] & 0x7F);
    
    TrackMetadata metadata;
    
    // Skip ID3v2 header and parse frames
    std::size_t offset = 10;
    while (offset < 10 + tagSize && offset + 10 <= fileData.size()) {
        // Check for frame header
        if (fileData[offset] == 0) {
            break; // Padding reached
        }
        
        parseID3v2Frame(fileData, offset, metadata);
    }
    
    return metadata;
}

std::optional<TrackMetadata> MetadataParser::parseFile(const std::string& path, std::string* errorOut) {
    // Read file into memory
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        if (errorOut != nullptr) {
            *errorOut = "Cannot open file: " + path;
        }
        return std::nullopt;
    }
    
    const std::streamsize fileSize = file.tellg();
    if (fileSize <= 0) {
        if (errorOut != nullptr) {
            *errorOut = "File is empty";
        }
        return std::nullopt;
    }
    
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> fileData(static_cast<std::size_t>(fileSize));
    file.read(reinterpret_cast<char*>(fileData.data()), fileSize);
    file.close();
    
    // Try ID3v2 first (at start of file)
    auto metadata = parseID3v2(fileData, errorOut);
    if (metadata.has_value()) {
        return metadata;
    }
    
    // Try ID3v1 (at end of file, for MP3s)
    if (isMp3File(path)) {
        metadata = parseID3v1(fileData, errorOut);
        if (metadata.has_value()) {
            return metadata;
        }
    }
    
    // If no tags found, return minimal metadata
    TrackMetadata result;
    result.title = "Unknown";
    result.artist = "Unknown";
    result.genre = "Unknown";
    
    return result;
}

} // namespace dj
