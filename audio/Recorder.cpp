#include "audio/Recorder.h"

#include <algorithm>
#include <cstring>

namespace dj {

Recorder::Recorder(std::size_t sampleRate, std::size_t channels, std::size_t capacitySeconds)
    : sampleRate_(sampleRate)
    , channels_(channels)
    , capacityFrames_(sampleRate * capacitySeconds)
    , ringBuffer_(capacityFrames_ * channels_, 0.0f)
    , writePos_(0)
    , hasWrapped_(false)
    , isRecording_(false)
    , isPaused_(false)
{
}

Recorder::~Recorder() {
    stop();
}

bool Recorder::start() {
    std::lock_guard<std::mutex> lock(bufferMutex_);
    isRecording_ = true;
    isPaused_ = false;
    writePos_ = 0;
    hasWrapped_ = false;
    return true;
}

void Recorder::stop() {
    std::lock_guard<std::mutex> lock(bufferMutex_);
    isRecording_ = false;
    isPaused_ = false;
}

void Recorder::pause() {
    std::lock_guard<std::mutex> lock(bufferMutex_);
    if (isRecording_) {
        isPaused_ = true;
    }
}

void Recorder::resume() {
    std::lock_guard<std::mutex> lock(bufferMutex_);
    isPaused_ = false;
}

bool Recorder::isRecording() const {
    std::lock_guard<std::mutex> lock(bufferMutex_);
    return isRecording_ && !isPaused_;
}

bool Recorder::isPaused() const {
    std::lock_guard<std::mutex> lock(bufferMutex_);
    return isPaused_;
}

void Recorder::submitFrames(const float* data, std::size_t numFrames) {
    if (!isRecording_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(bufferMutex_);
    
    if (isPaused_) {
        return;
    }
    
    const std::size_t samplesToCopy = numFrames * channels_;
    const std::size_t bufferCapacity = ringBuffer_.size();
    
    // Handle potential wrapping around the ring buffer
    if (writePos_ + samplesToCopy < bufferCapacity) {
        // Simple case: no wrap-around
        std::memcpy(ringBuffer_.data() + writePos_, data, samplesToCopy * sizeof(float));
        writePos_ += samplesToCopy;
    } else if (writePos_ + samplesToCopy == bufferCapacity) {
        // Exact fit to end of buffer - mark as wrapped for next submission
        std::memcpy(ringBuffer_.data() + writePos_, data, samplesToCopy * sizeof(float));
        writePos_ = 0;
        hasWrapped_ = true;
    } else {
        // Wrap-around case
        hasWrapped_ = true;
        const std::size_t firstChunk = bufferCapacity - writePos_;
        std::memcpy(ringBuffer_.data() + writePos_, data, firstChunk * sizeof(float));
        
        const std::size_t secondChunk = samplesToCopy - firstChunk;
        std::memcpy(ringBuffer_.data(), data + firstChunk, secondChunk * sizeof(float));
        
        writePos_ = secondChunk;
    }
}

std::vector<float> Recorder::getRecordedData() const {
    std::lock_guard<std::mutex> lock(bufferMutex_);
    
    std::vector<float> result;
    const std::size_t bufferCapacity = ringBuffer_.size();
    
    if (!hasWrapped_) {
        // If we haven't wrapped around, return from start to writePos in chronological order
        result.resize(writePos_);
        if (writePos_ > 0) {
            std::memcpy(result.data(), ringBuffer_.data(), writePos_ * sizeof(float));
        }
    } else {
        // If we have wrapped, reconstruct in chronological order: [writePos_, end) + [0, writePos_)
        result.resize(bufferCapacity);
        const std::size_t secondChunkSize = bufferCapacity - writePos_;
        
        // Copy [writePos_, end) to result start
        std::memcpy(result.data(), ringBuffer_.data() + writePos_, secondChunkSize * sizeof(float));
        
        // Copy [0, writePos_) to result after second chunk
        std::memcpy(result.data() + secondChunkSize, ringBuffer_.data(), writePos_ * sizeof(float));
    }
    
    return result;
}

void Recorder::clear() {
    std::lock_guard<std::mutex> lock(bufferMutex_);
    std::fill(ringBuffer_.begin(), ringBuffer_.end(), 0.0f);
    writePos_ = 0;
    hasWrapped_ = false;
}

float Recorder::getDuration() const {
    std::lock_guard<std::mutex> lock(bufferMutex_);
    
    std::size_t totalSamples;
    if (hasWrapped_) {
        // If wrapped, we have a full buffer's worth of audio
        totalSamples = ringBuffer_.size();
    } else if (writePos_ == 0 && !isRecording_) {
        totalSamples = 0;
    } else {
        totalSamples = writePos_;
    }
    
    return static_cast<float>(totalSamples) / (static_cast<float>(sampleRate_) * static_cast<float>(channels_));
}

} // namespace dj
