#pragma once

#include <string>
#include <vector>

namespace dj {

class PortAudioPlayer {
public:
    PortAudioPlayer();
    ~PortAudioPlayer();

    bool available() const;
    bool open(int sampleRate, unsigned long framesPerBuffer, std::string* errorOut = nullptr);
    bool write(const std::vector<float>& interleavedStereo, std::string* errorOut = nullptr);
    void close();

    bool isOpen() const;

private:
    void* stream_ = nullptr;
    bool initialized_ = false;
    bool open_ = false;
};

} // namespace dj
