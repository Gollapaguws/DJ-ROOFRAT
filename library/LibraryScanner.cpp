#include "library/LibraryScanner.h"

#include <algorithm>
#include <iostream>

namespace dj::library {

LibraryScanner::LibraryScanner() = default;

std::vector<std::string> LibraryScanner::getSupportedExtensions() {
    return {".wav", ".mp3", ".flac", ".WAV", ".MP3", ".FLAC"};
}

bool LibraryScanner::isAudioFile(const std::filesystem::path& path) const {
    const auto ext = path.extension().string();
    const auto supportedExts = getSupportedExtensions();
    return std::find(supportedExts.begin(), supportedExts.end(), ext) != supportedExts.end();
}

bool LibraryScanner::scanDirectory(const std::filesystem::path& rootPath,
                                    ProgressCallback onProgress,
                                    std::string* errorOut) {
    try {
        if (!std::filesystem::exists(rootPath)) {
            if (errorOut) {
                *errorOut = "Directory does not exist: " + rootPath.string();
            }
            return false;
        }

        if (!std::filesystem::is_directory(rootPath)) {
            if (errorOut) {
                *errorOut = "Path is not a directory: " + rootPath.string();
            }
            return false;
        }

        ScanResult result;
        result.scannedCount = 0;
        result.failedCount = 0;
        result.skippedCount = 0;

        // Recursively iterate through all files
        for (const auto& entry : std::filesystem::recursive_directory_iterator(
                 rootPath, std::filesystem::directory_options::skip_permission_denied)) {
            try {
                if (entry.is_regular_file()) {
                    if (isAudioFile(entry.path())) {
                        result.scannedCount++;
                        if (onProgress) {
                            onProgress(result, entry.path().string());
                        }
                    } else {
                        result.skippedCount++;
                    }
                }
            } catch (const std::exception& /*e*/) {
                result.failedCount++;
            }
        }

        if (onProgress) {
            onProgress(result, "Scan complete");
        }

        return true;
    } catch (const std::exception& e) {
        if (errorOut) {
            *errorOut = std::string("Scan failed: ") + e.what();
        }
        return false;
    }
}

} // namespace dj::library
