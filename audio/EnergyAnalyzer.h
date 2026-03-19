#pragma once

#include <string>

namespace dj {

/**
 * @brief RMS and peak energy analyzer with qualitative labels.
 *
 * Computes real-time RMS energy, peak amplitude, dynamic range, and provides
 * symbolic energy ratings (Ambient, Chill, Moderate, Upbeat, Peak Hour).
 */
class EnergyAnalyzer {
public:
    /**
     * @brief Construct energy analyzer.
     */
    EnergyAnalyzer();
    /**
     * @brief Destructor.
     */
    ~EnergyAnalyzer() = default;
    
    // Process audio samples
    // Accumulates RMS, peak, and dynamic metrics
    /**
     * @brief Accumulate audio samples for energy analysis.
     * @param samples Float sample buffer [-1.0, 1.0].
     * @param count Number of samples to process.
     */
    void processSamples(const float* samples, size_t count);
    
    // Get energy metrics
    /**
     * @brief Get RMS energy level.
     * @return Normalized RMS in range [0.0, 1.0].
     */
    float getRMS() const;           // Root Mean Square energy (0.0-1.0)
    /**
     * @brief Get peak amplitude.
     * @return Normalized peak in range [0.0, 1.0].
     */
    float getPeak() const;          // Peak amplitude (0.0-1.0)
    /**
     * @brief Get dynamic range.
     * @return Dynamic range in decibels (dB).
     */
    float getDynamicRange() const;  // Dynamic range in dB
    
    // Get 1-10 energy rating based on RMS
    /**
     * @brief Get 1–10 energy rating based on RMS.
     * @return Energy rating (1=quiet, 10=loud).
     */
    int getEnergyRating() const;
    
    // Get descriptive energy label
    // Returns: "Ambient", "Chill", "Moderate", "Upbeat", or "Peak Hour"
    /**
     * @brief Get human-readable energy label.
     * @return One of: "Ambient", "Chill", "Moderate", "Upbeat", or "Peak Hour".
     */
    std::string getEnergyLabel() const;
    
    // Reset analyzer for new track
    /**
     * @brief Reset all accumulators for a new track.
     */
    void reset();
    
private:
    double sumSquares_;      // Accumulator for RMS calculation
    size_t sampleCount_;     // Number of samples processed
    float peakAmplitude_;    // Maximum absolute amplitude
    
    // Helper: Calculate energy rating from RMS value
    int calculateRating(float rms) const;
};

} // namespace dj
