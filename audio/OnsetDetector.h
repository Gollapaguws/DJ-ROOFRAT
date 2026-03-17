#pragma once

#include <cstddef>
#include <vector>

namespace dj {

class OnsetDetector {
public:
    // Constructor: sampleRate (typically 44100), windowSize (512), hopSize (~50% overlap = 256)
    explicit OnsetDetector(int sampleRate = 44100, int windowSize = 512, int hopSize = 256);

    // Feed audio samples to the detector
    void processSamples(const float* samples, std::size_t count);

    // Get detected onset timestamps (in seconds)
    std::vector<double> getOnsets() const;

    // Get spectral flux curve (for visualization/debugging)
    std::vector<float> getSpectralFlux() const;

private:
    void computeSpectralFlux();
    void pickPeaks();
    std::vector<float> computePowerSpectrum(const std::vector<float>& audioBuffer);
    void performFFT(const std::vector<float>& input, std::vector<float>& output);

    int sampleRate_;
    int windowSize_;
    int hopSize_;
    
    std::vector<float> audioBuffer_;           // Accumulating audio buffer
    std::vector<float> previousSpectrum_;      // Previous FFT result for flux calculation
    std::vector<double> onsets_;               // Detected onset timestamps
    std::vector<float> fluxCurve_;             // Spectral flux values over time
    bool peaksPickedFlag_;                     // Track if peaks have been picked
};

} // namespace dj
