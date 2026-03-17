#include "audio/OnsetDetector.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace dj {

OnsetDetector::OnsetDetector(int sampleRate, int windowSize, int hopSize)
    : sampleRate_(sampleRate)
    , windowSize_(windowSize)
    , hopSize_(hopSize)
    , peaksPickedFlag_(false)
{
    previousSpectrum_.resize(windowSize / 2, 0.0f);
}

void OnsetDetector::processSamples(const float* samples, std::size_t count) {
    if (!samples || count == 0) {
        return;
    }

    // Accumulate samples into buffer
    audioBuffer_.insert(audioBuffer_.end(), samples, samples + count);

    // Process complete windows
    while (audioBuffer_.size() >= static_cast<std::size_t>(windowSize_)) {
        // Extract window
        std::vector<float> window(audioBuffer_.begin(), audioBuffer_.begin() + windowSize_);

        // Process this window
        computeSpectralFlux();

        // Remove processed samples (50% overlap)
        audioBuffer_.erase(audioBuffer_.begin(), audioBuffer_.begin() + hopSize_);
    }
}

std::vector<double> OnsetDetector::getOnsets() const {
    // Call peak picking if not already done
    if (!peaksPickedFlag_ && !fluxCurve_.empty()) {
        const_cast<OnsetDetector*>(this)->pickPeaks();
    }
    return onsets_;
}

std::vector<float> OnsetDetector::getSpectralFlux() const {
    return fluxCurve_;
}

void OnsetDetector::computeSpectralFlux() {
    // Get power spectrum from current buffer
    std::vector<float> currentSpectrum = computePowerSpectrum(audioBuffer_);

    if (previousSpectrum_.empty()) {
        previousSpectrum_ = currentSpectrum;
        return;
    }

    // Calculate spectral flux: sum of positive differences only (half-wave rectification)
    float flux = 0.0f;
    for (std::size_t k = 0; k < currentSpectrum.size() && k < previousSpectrum_.size(); ++k) {
        float diff = currentSpectrum[k] - previousSpectrum_[k];
        flux += std::max(0.0f, diff);  // Only count positive changes
    }

    fluxCurve_.push_back(flux);

    // Update previous spectrum
    previousSpectrum_ = currentSpectrum;

    // Once we have enough frames, perform peak picking
    if (fluxCurve_.size() > 10 && !peaksPickedFlag_) {
        // We'll pick peaks after all audio is processed (in a real implementation)
        // This is called lazily when getOnsets is accessed
    }
}

std::vector<float> OnsetDetector::computePowerSpectrum(const std::vector<float>& audioBuffer) {
    std::vector<float> spectrum(windowSize_ / 2, 0.0f);

    if (audioBuffer.size() < static_cast<std::size_t>(windowSize_)) {
        return spectrum;
    }

    // Apply Hann window and compute FFT-like spectral estimate
    std::vector<float> windowed(windowSize_);
    
    const float PI = 3.14159265359f;
    for (int i = 0; i < windowSize_; ++i) {
        // Hann window
        float hannCoeff = 0.5f * (1.0f - std::cos(2.0f * PI * i / (windowSize_ - 1)));
        windowed[i] = audioBuffer[i] * hannCoeff;
    }

    // Compute magnitude spectrum using simple binning
    // This approximates frequency bands without a full FFT
    for (int k = 0; k < windowSize_ / 2; ++k) {
        float real = 0.0f;
        float imag = 0.0f;
        
        // Goertzel-like approach: compute energy in each frequency bin
        for (int n = 0; n < windowSize_; ++n) {
            float freq = 2.0f * PI * k * n / windowSize_;
            real += windowed[n] * std::cos(freq);
            imag += windowed[n] * std::sin(freq);
        }
        
        spectrum[k] = std::sqrt(real * real + imag * imag);
    }

    // Normalize to 0-1 range
    float maxVal = 0.0f;
    for (float val : spectrum) {
        maxVal = std::max(maxVal, val);
    }
    if (maxVal > 0.0f) {
        for (float& val : spectrum) {
            val /= maxVal;
        }
    }

    return spectrum;
}

void OnsetDetector::performFFT(const std::vector<float>& input, std::vector<float>& output) {
    // Simplified: not implementing full FFT, using computePowerSpectrum instead
    output = computePowerSpectrum(input);
}

void OnsetDetector::pickPeaks() {
    if (fluxCurve_.empty() || peaksPickedFlag_) {
        return;
    }

    // Calculate adaptive threshold: mean + 2.5 * stddev (conservative to reject noise)
    float mean =
        std::accumulate(fluxCurve_.begin(), fluxCurve_.end(), 0.0f) / fluxCurve_.size();

    float variance = 0.0f;
    for (float f : fluxCurve_) {
        variance += (f - mean) * (f - mean);
    }
    float stdDev = std::sqrt(variance / fluxCurve_.size());
    float threshold = mean + 2.5f * stdDev;

    // Find peaks with minimum spacing (50ms)
    double minSpacing = 0.05;  // 50ms
    double lastOnset = -minSpacing;

    for (std::size_t i = 1; i < fluxCurve_.size() - 1; ++i) {
        bool isPeak = (fluxCurve_[i] > fluxCurve_[i - 1]) && (fluxCurve_[i] > fluxCurve_[i + 1]);
        double timestamp = static_cast<double>(i * hopSize_) / sampleRate_;

        if (isPeak && fluxCurve_[i] > threshold && (timestamp - lastOnset) >= minSpacing) {
            onsets_.push_back(timestamp);
            lastOnset = timestamp;
        }
    }

    peaksPickedFlag_ = true;
}

}  // namespace dj
