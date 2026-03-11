#include "audio/Bitcrusher.h"

#include <algorithm>
#include <cmath>

namespace dj {

Bitcrusher::Bitcrusher() {
    // Initialize with default 16-bit at 44.1kHz
    setBitDepth(16);
}

void Bitcrusher::setSampleRate(int sampleRate) {
    sampleRate_ = sampleRate;
}

void Bitcrusher::setBitDepth(int bits) {
    bitDepth_ = std::clamp(bits, 1, 16);
    // Calculate number of quantization levels: 2^bitDepth
    levels_ = std::pow(2.0f, static_cast<float>(bitDepth_));
}

void Bitcrusher::setDownsampleFactor(int factor) {
    downsampleFactor_ = std::clamp(factor, 1, 64);
}

float Bitcrusher::quantize(float input) {
    // Clamp input to [-1, 1]
    float clamped = std::clamp(input, -1.0f, 1.0f);
    
    // Quantize: map to nearest level
    // Rescale to [0, levels] range
    float scaled = (clamped + 1.0f) * (levels_ / 2.0f);
    
    // Round to nearest level
    float rounded = std::floor(scaled + 0.5f);
    
    // Map back to [-1, 1]
    float quantized = (rounded / (levels_ / 2.0f)) - 1.0f;
    
    return quantized;
}

std::array<float, 2> Bitcrusher::process(const std::array<float, 2>& input) {
    // Apply bit depth reduction
    float leftQuantized = quantize(input[0]);
    float rightQuantized = quantize(input[1]);
    
    // Handle downsampling (sample-and-hold)
    if (sampleCounter_ == 0) {
        heldSample_[0] = leftQuantized;
        heldSample_[1] = rightQuantized;
    }
    
    // Increment counter
    sampleCounter_ = (sampleCounter_ + 1) % downsampleFactor_;
    
    return heldSample_;
}

void Bitcrusher::reset() {
    sampleCounter_ = 0;
    heldSample_ = {0.0f, 0.0f};
}

} // namespace dj
