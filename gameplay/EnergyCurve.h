#pragma once

#include <deque>
#include <vector>

namespace dj {

/**
 * @brief Single energy measurement snapshot.
 *
 * Captures a point in time with energy level, timestamp, and RMS amplitude.
 */
struct EnergySample {
    /// Energy level (0.0–1.0)
    float energy;
    /// Unix timestamp or elapsed time in seconds
    double timestamp;
    /// RMS amplitude
    float rms;
};

/**
 * @brief Rolling energy history curve for a DJ session.
 *
 * Maintains a time-windowed history of energy samples for tracking session
 * energy trends and computing statistical measures.
 */
class EnergyCurve {
public:
    // Constructor: timeWindow in seconds (e.g., 1800 for 30 min), sampleRate in Hz (e.g., 1.0)
    /**
     * @brief Construct energy curve.
     * @param timeWindow Rolling window duration in seconds (e.g., 1800 for 30 minutes).
     * @param sampleRate Sample rate in Hz (e.g., 1.0 for 1 sample per second).
     */
    EnergyCurve(double timeWindow, double sampleRate);
    
    // Add a new energy sample to the curve
    /**
     * @brief Append a new energy measurement.
     * @param energy Energy level [0.0–1.0].
     * @param timestamp Elapsed time in seconds (or unix timestamp).
     * @param rms RMS amplitude.
     */
    void addSample(float energy, double timestamp, float rms);
    
    // Get the full curve as a vector of samples
    /**
     * @brief Return all samples in the current window.
     * @return Vector of EnergySample objects in chronological order.
     */
    std::vector<EnergySample> getCurve() const;
    
    // Get statistical measures
    /**
     * @brief Get average energy over the window.
     * @return Average energy (0.0–1.0).
     */
    float getAverage() const;

    /**
     * @brief Get peak energy in the window.
     * @return Maximum energy (0.0–1.0).
     */
    float getPeak() const;

    /**
     * @brief Get minimum (dip) energy in the window.
     * @return Minimum energy (0.0–1.0).
     */
    float getDip() const;
    
private:
    double timeWindow_;      // Time window in seconds
    double sampleRate_;      // Sample rate in Hz
    size_t maxSamples_;      // Maximum number of samples to keep
    std::deque<EnergySample> samples_;
};

} // namespace dj
