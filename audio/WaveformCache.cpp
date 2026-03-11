#include "audio/WaveformCache.h"

#include <algorithm>
#include <cmath>

#include "audio/AudioClip.h"

namespace dj {

std::vector<WaveformBucket> WaveformCache::generate(const AudioClip& clip, int bucketCount) {
    if (clip.empty() || bucketCount <= 0) {
        return {};
    }
    
    const std::size_t frameCount = clip.frameCount();
    if (frameCount == 0) {
        return {};
    }
    
    // Clamp bucket count to reasonable range
    bucketCount = std::clamp(bucketCount, 1, 10000);
    
    std::vector<WaveformBucket> buckets(bucketCount);
    
    // Compute samples per bucket
    const float samplesPerBucket = static_cast<float>(frameCount) / static_cast<float>(bucketCount);
    
    for (int i = 0; i < bucketCount; ++i) {
        // Compute range of sample indices for this bucket
        const float startFrame = samplesPerBucket * static_cast<float>(i);
        const float endFrame = samplesPerBucket * static_cast<float>(i + 1);
        
        const std::size_t startIdx = static_cast<std::size_t>(startFrame);
        const std::size_t endIdx = std::min(static_cast<std::size_t>(std::ceil(endFrame)), frameCount);
        
        // Find min/max in this bucket
        float minVal = 0.0f;
        float maxVal = 0.0f;
        bool initialized = false;
        
        for (std::size_t frameIdx = startIdx; frameIdx < endIdx; ++frameIdx) {
            // Get frame samples (both channels)
            const std::size_t sampleBase = frameIdx * static_cast<std::size_t>(clip.channels);
            if (sampleBase >= clip.samples.size()) {
                break;
            }
            
            // For stereo, take absolute maximum across both channels
            float sampleVal = clip.samples[sampleBase];
            if (clip.channels > 1 && sampleBase + 1 < clip.samples.size()) {
                const float rightVal = clip.samples[sampleBase + 1];
                sampleVal = (std::abs(sampleVal) + std::abs(rightVal)) / 2.0f;
            } else {
                sampleVal = std::abs(sampleVal);
            }
            
            if (!initialized) {
                minVal = sampleVal;
                maxVal = sampleVal;
                initialized = true;
            } else {
                minVal = std::min(minVal, sampleVal);
                maxVal = std::max(maxVal, sampleVal);
            }
        }
        
        // Normalize to -1 to 1 range
        buckets[i].minSample = -maxVal;
        buckets[i].maxSample = maxVal;
    }
    
    return buckets;
}

} // namespace dj
