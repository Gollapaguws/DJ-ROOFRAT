#include "audio/SpectrumAnalyzer.h"

#include "audio/FFTEngine.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace dj {

constexpr float PI = 3.14159265359f;

SpectrumAnalyzer::SpectrumAnalyzer(int sampleRate, int windowSize)
    : sampleRate_(sampleRate),
      windowSize_(windowSize),
      bufferPosition_(0),
      spectrum_(windowSize_ / 2 + 1, 0.0f),
      frequencyBands_({}) {
    buffer_.resize(windowSize_, 0.0f);
    initializeWindow();

    // Create FFTEngine if window size is power of 2
    if (FFTEngine::isPowerOfTwo(windowSize_)) {
        fftEngine_ = std::make_unique<FFTEngine>(
            windowSize_, 
            FFTEngine::WindowFunction::Hann
        );
        fullSpectrum_.resize(windowSize_ / 2 + 1, 0.0f);
    }
}

SpectrumAnalyzer::~SpectrumAnalyzer() = default;

void SpectrumAnalyzer::initializeWindow() {
    window_.resize(windowSize_);
    for (int i = 0; i < windowSize_; ++i) {
        window_[i] = hannWindow(static_cast<float>(i), static_cast<float>(windowSize_));
    }
}

float SpectrumAnalyzer::hannWindow(float n, float N) const {
    // Hann window: w(n) = 0.5 * (1 - cos(2*pi*n/(N-1)))
    return 0.5f * (1.0f - std::cos(2.0f * PI * n / (N - 1.0f)));
}

void SpectrumAnalyzer::processSamples(const float* samples, std::size_t count) {
    if (!samples || count == 0) {
        return;
    }

    // Fill buffer with new samples (circular)
    for (std::size_t i = 0; i < count; ++i) {
        buffer_[bufferPosition_] = samples[i];
        bufferPosition_ = (bufferPosition_ + 1) % windowSize_;
    }

    // Compute FFT and frequency bands
    computeSpectrum();
    aggregateFrequencyBands();
}

void SpectrumAnalyzer::setSamples(const float* samples, std::size_t count) {
    processSamples(samples, count);
}

void SpectrumAnalyzer::computeSpectrum() {
    if (fftEngine_) {
        // Fast path: Use FFT (Cooley-Tukey, O(N log N))
        auto fftSpectrum = fftEngine_->computePowerSpectrum(
            buffer_.data(), 
            buffer_.size()
        );
        
        // Thread-safe update to fullSpectrum_
        {
            std::lock_guard<std::mutex> lock(spectrumMutex_);
            fullSpectrum_ = std::move(fftSpectrum);
            spectrum_ = fullSpectrum_;  // Keep legacy spectrum_ in sync
        }
    } else {
        // Fallback: Use naive DFT (O(N²), for non-power-of-2 sizes)
        // Create windowed input for FFT
        std::vector<std::complex<double>> fftInput(windowSize_);
        for (int i = 0; i < windowSize_; ++i) {
            // Apply Hann window and read from circular buffer
            int readPos = (bufferPosition_ + i) % windowSize_;
            fftInput[i] = std::complex<double>(buffer_[readPos] * window_[i], 0.0);
        }

        // Perform naive DFT
        std::vector<std::complex<double>> fftOutput(windowSize_);
        for (int k = 0; k < windowSize_; ++k) {
            std::complex<double> sum(0.0, 0.0);
            for (int n = 0; n < windowSize_; ++n) {
                double angle = -2.0 * PI * k * n / windowSize_;
                std::complex<double> twiddle(std::cos(angle), std::sin(angle));
                sum += fftInput[n] * twiddle;
            }
            fftOutput[k] = sum;
        }

        // Compute power spectrum (only positive frequencies, 0 to Nyquist)
        int numBins = windowSize_ / 2 + 1;
        float maxMagnitude = 0.0f;

        spectrum_.resize(numBins);
        for (int i = 0; i < numBins; ++i) {
            double real = fftOutput[i].real();
            double imag = fftOutput[i].imag();
            double magnitude = std::sqrt(real * real + imag * imag);
            
            // Normalize by window sum
            magnitude /= (windowSize_ / 2.0);
            
            spectrum_[i] = static_cast<float>(magnitude);
            maxMagnitude = std::max(maxMagnitude, spectrum_[i]);
        }

        // Normalize spectrum to 0.0-1.0 range
        if (maxMagnitude > 0.0f) {
            for (float& value : spectrum_) {
                value /= maxMagnitude;
            }
        }

        // Thread-safe update to fullSpectrum_
        {
            std::lock_guard<std::mutex> lock(spectrumMutex_);
            fullSpectrum_ = spectrum_;
        }
    }
}

void SpectrumAnalyzer::aggregateFrequencyBands() {
    // 20 logarithmic bands from 20Hz to 20kHz
    // Formula: freq = 20 * (20000/20)^(band/19) = 20 * 1000^(band/19)
    
    const float nyquist = sampleRate_ / 2.0f;

    for (int band = 0; band < NUM_BANDS; ++band) {
        // Calculate frequency range for this band
        float freqLow = 20.0f * std::pow(1000.0f, (band - 0.5f) / 19.0f);
        float freqHigh = 20.0f * std::pow(1000.0f, (band + 0.5f) / 19.0f);

        // Clamp to valid range
        freqLow = std::max(freqLow, 20.0f);
        freqHigh = std::min(freqHigh, nyquist);

        // Find corresponding FFT bins
        int binLow = static_cast<int>((freqLow / nyquist) * spectrum_.size());
        int binHigh = static_cast<int>((freqHigh / nyquist) * spectrum_.size());

        // Aggregate spectrum values in this band (use max to preserve peaks)
        float bandValue = 0.0f;
        for (int bin = binLow; bin <= binHigh && bin < static_cast<int>(spectrum_.size()); ++bin) {
            bandValue = std::max(bandValue, spectrum_[bin]);
        }

        frequencyBands_[band] = bandValue;
    }
}

std::vector<float> SpectrumAnalyzer::getFullSpectrum() const {
    std::lock_guard<std::mutex> lock(spectrumMutex_);
    return fullSpectrum_;
}

std::array<float, SpectrumAnalyzer::NUM_BANDS> SpectrumAnalyzer::getFrequencyBands() const {
    return frequencyBands_;
}

} // namespace dj
