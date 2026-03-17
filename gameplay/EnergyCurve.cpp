#include "gameplay/EnergyCurve.h"
#include <algorithm>
#include <cmath>

namespace dj {

EnergyCurve::EnergyCurve(double timeWindow, double sampleRate)
    : timeWindow_(timeWindow)
    , sampleRate_(sampleRate)
    , maxSamples_(static_cast<size_t>(timeWindow * sampleRate))
{
}

void EnergyCurve::addSample(float energy, double timestamp, float rms) {
    EnergySample sample{energy, timestamp, rms};
    samples_.push_back(sample);
    
    // Remove old samples beyond time window
    while (!samples_.empty() && 
           (timestamp - samples_.front().timestamp) > timeWindow_) {
        samples_.pop_front();
    }
    
    // Also enforce max sample count as backup
    while (samples_.size() > maxSamples_) {
        samples_.pop_front();
    }
}

std::vector<EnergySample> EnergyCurve::getCurve() const {
    return std::vector<EnergySample>(samples_.begin(), samples_.end());
}

float EnergyCurve::getAverage() const {
    if (samples_.empty()) {
        return 0.0f;
    }
    
    float sum = 0.0f;
    for (const auto& sample : samples_) {
        sum += sample.energy;
    }
    
    return sum / static_cast<float>(samples_.size());
}

float EnergyCurve::getPeak() const {
    if (samples_.empty()) {
        return 0.0f;
    }
    
    float peak = samples_[0].energy;
    for (const auto& sample : samples_) {
        peak = std::max(peak, sample.energy);
    }
    
    return peak;
}

float EnergyCurve::getDip() const {
    if (samples_.empty()) {
        return 0.0f;
    }
    
    float dip = samples_[0].energy;
    for (const auto& sample : samples_) {
        dip = std::min(dip, sample.energy);
    }
    
    return dip;
}

} // namespace dj
