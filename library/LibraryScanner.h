#pragma once

#include <optional>
#include <string>
#include <vector>
#include <filesystem>
#include <functional>

namespace dj::library {

struct ScanResult {
    int scannedCount = 0;
    int failedCount = 0;
    int skippedCount = 0;
};

class LibraryScanner {
public:
    using ProgressCallback = std::function<void(const ScanResult& progress, const std::string& currentPath)>;

    LibraryScanner();

    // Scan directory recursively for audio files (MP3, WAV, FLAC)
    bool scanDirectory(const std::filesystem::path& rootPath, 
                       ProgressCallback onProgress = nullptr,
                       std::string* errorOut = nullptr);

    // Get the file extensions we support
    static std::vector<std::string> getSupportedExtensions();

private:
    bool isAudioFile(const std::filesystem::path& path) const;
};

} // namespace dj::library
