#pragma once

#include <cstddef>
#include <vector>
#include <mutex>

namespace dj {

class Recorder {
public:
    // Constructor: sample_rate (Hz), channels, capacity (seconds)
    Recorder(std::size_t sampleRate, std::size_t channels, std::size_t capacitySeconds);
    
    ~Recorder();
    
    // Recording control
    bool start();
    void stop();
    void pause();
    void resume();
    
    // Status queries
    bool isRecording() const;
    bool isPaused() const;
    
    // Submit frames for recording (stereo interleaved float samples [-1.0, 1.0])
    // numFrames: number of audio frames (not samples; stereo has 2 samples per frame)
    void submitFrames(const float* data, std::size_t numFrames);
    
    // Retrieve recorded data (float interleaved stereo)
    std::vector<float> getRecordedData() const;
    
    // Clear recorded data and reset
    void clear();
    
    // Get duration of recorded audio in seconds
    float getDuration() const;
    
private:
    std::size_t sampleRate_;
    std::size_t channels_;
    std::size_t capacityFrames_;
    
    std::vector<float> ringBuffer_;
    std::size_t writePos_;
    bool hasWrapped_;
    bool isRecording_;
    bool isPaused_;
    
    mutable std::mutex bufferMutex_;
};

} // namespace dj
