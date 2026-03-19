#pragma once

#include <cstddef>
#include <vector>
#include <mutex>
#include <string>

namespace dj {

/**
 * @brief Ring-buffer audio recorder with WAV/MP3 export support.
 *
 * Records incoming audio into a circular buffer and provides export functionality
 * for WAV and MP3 formats with configurable filename handling.
 */
class Recorder {
public:
    // Constructor: sample_rate (Hz), channels, capacity (seconds)
    /**
     * @brief Construct recorder.
     * @param sampleRate Sample rate in Hz (e.g., 44100).
     * @param channels Number of channels (1=mono, 2=stereo).
     * @param capacitySeconds Ring buffer capacity in seconds.
     */
    Recorder(std::size_t sampleRate, std::size_t channels, std::size_t capacitySeconds);
    
    /**
     * @brief Destructor.
     */
    ~Recorder();
    
    // Recording control
    /**
     * @brief Start recording.
     * @return true on success, false if already recording.
     */
    bool start();
    /**
     * @brief Stop recording.
     */
    void stop();
    /**
     * @brief Pause recording without stopping.
     */
    void pause();
    /**
     * @brief Resume recording after pause.
     */
    void resume();
    
    // Status queries
    /**
     * @brief Check if currently recording.
     * @return true if recording is active.
     */
    bool isRecording() const;
    /**
     * @brief Check if recording is paused.
     * @return true if paused.
     */
    bool isPaused() const;
    
    // Submit frames for recording (stereo interleaved float samples [-1.0, 1.0])
    // numFrames: number of audio frames (not samples; stereo has 2 samples per frame)
    /**
     * @brief Submit audio frames into the ring buffer.
     * @param data Interleaved stereo float samples [-1.0, 1.0] (2 samples per frame).
     * @param numFrames Number of frames (not samples).
     */
    void submitFrames(const float* data, std::size_t numFrames);
    
    // Retrieve recorded data (float interleaved stereo)
    /**
     * @brief Retrieve all recorded data.
     * @return Interleaved stereo float buffer.
     */
    std::vector<float> getRecordedData() const;
    
    // Clear recorded data and reset
    /**
     * @brief Clear recorded data and reset.
     */
    void clear();
    
    // Get duration of recorded audio in seconds
    /**
     * @brief Get duration of recording in seconds.
     * @return Recording duration.
     */
    float getDuration() const;

    // Configure/export filename handling
    /**
     * @brief Set custom export filename (without extension).
     * @param filename Export filename (extension added by export function).
     */
    void setExportFilename(const std::string& filename);
    std::string getExportFilename() const;
    /**
     * @brief Get export filename, falling back to provided default if none set.
     * @param fallbackFilename Default filename if none was set.
     * @return Filename to use for export.
     */
    std::string getExportFilenameOrDefault(const std::string& fallbackFilename) const;
    
private:
    std::size_t sampleRate_;
    std::size_t channels_;
    std::size_t capacityFrames_;
    
    std::vector<float> ringBuffer_;
    std::size_t writePos_;
    bool hasWrapped_;
    bool isRecording_;
    bool isPaused_;
    std::string exportFilename_;
    
    mutable std::mutex bufferMutex_;
};

} // namespace dj
