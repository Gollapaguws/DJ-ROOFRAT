#pragma once

#include <array>

namespace dj {

class Bitcrusher {
public:
    Bitcrusher();
    
    void setSampleRate(int sampleRate);
    void setBitDepth(int bits);              // 1 - 16 bits
    void setDownsampleFactor(int factor);    // 1 = no downsampling, 2 = half sample rate, etc.
    
    std::array<float, 2> process(const std::array<float, 2>& input);
    void reset();

private:
    int sampleRate_ = 44100;
    int bitDepth_ = 16;
    int downsampleFactor_ = 1;
    
    // For downsampling: track internal sample counter
    int sampleCounter_ = 0;
    
    // Hold previous sample for downsampling
    std::array<float, 2> heldSample_ = {0.0f, 0.0f};
    
    // Quantization: number of levels = 2^bitDepth
    float levels_ = 65536.0f;  // 2^16 for 16-bit
    
    float quantize(float input);
};

} // namespace dj
