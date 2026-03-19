#pragma once

#include <cstddef>
#include <string>

namespace dj {

// Forward declarations
class Deck;
class Mixer;
struct TrackMetadata;
struct SpectrumData;

/**
 * @brief Individual component scores contributing to overall mix quality.
 *
 * Each score is in range 0–25, with total maximum 100.
 */
struct MixQualityComponents {
    /// Beatmatch accuracy score (0–25)
    float beatmatchScore;
    /// EQ balance and frequency separation (0–25)
    float eqBalanceScore;
    /// Transition smoothness and blend quality (0–25)
    float transitionScore;
    /// Harmonic compatibility (0–25)
    float harmonicScore;
};

/**
 * @brief Real-time mix quality analyzer scoring beatmatch, EQ balance, transitions, and harmonic compatibility.
 *
 * Evaluates four key aspects of a DJ mix in real-time and provides component scores
 * as well as overall quality rating (0–100).
 */
class MixQualityAnalyzer {
public:
    /**
     * @brief Construct analyzer.
     */
    MixQualityAnalyzer() = default;
    
    // Analyze mix quality between two decks
    // Returns overall score 0-100
    /**
     * @brief Analyze and score the current mix.
     * @param deckA Pointer to Deck A.
     * @param deckB Pointer to Deck B.
     * @param mixer Optional Mixer pointer for gain-staging context.
     * @return Overall score (sum of components, 0–100).
     */
    float analyzeMix(const Deck* deckA, const Deck* deckB, 
                     const Mixer* mixer = nullptr);
    
    // Get individual component scores
    /**
     * @brief Get individual component scores from the last analysis.
     * @return MixQualityComponents struct with four scores (0–25 each).
     */
    MixQualityComponents getComponentScores() const;
    
    // Get overall score (sum of components, clamped 0-100)
    /**
     * @brief Get overall score (sum of components, 0–100).
     * @return Overall quality score.
     */
    float getOverallScore() const;

    // Whether a bass clash was detected in the latest analysis window
    /**
     * @brief Returns true if a bass frequency clash was detected in the last analysis.
     * @return true if bass clash detected.
     */
    bool hasBassClash() const;

    // Set runtime analysis context from main loop data
    /**
     * @brief Provide runtime BPM and key context for scoring.
     * @param effectiveBpmA Current BPM of Deck A.
     * @param effectiveBpmB Current BPM of Deck B.
     * @param transitionSmoothness Mix smoothness (0.0–1.0).
     * @param keyA Musical key of Deck A (e.g., "C minor").
     * @param keyB Musical key of Deck B (e.g., "F# major").
     */
    void setAnalysisContext(float effectiveBpmA,
                            float effectiveBpmB,
                            float transitionSmoothness,
                            const std::string& keyA,
                            const std::string& keyB);

    // Set externally computed bass clash state (e.g., from spectrum proxy metrics)
    /**
     * @brief Set externally computed bass clash state.
     * @param clashDetected true if bass clash detected.
     */
    void setBassClashState(bool clashDetected);
    
    // Detect bass frequency clash
    /**
     * @brief Detect bass frequency clash.
     * @param spectrumA Spectrum bins for Deck A.
     * @param spectrumB Spectrum bins for Deck B.
     * @param numBins Number of bins.
     * @return true if bass clash detected (both decks have strong bass in same range).
     */
    bool detectBassClash(const float* spectrumA, const float* spectrumB, size_t numBins) const;
    
    // Apply Camelot compatibility bonus
    float applyCamelotBonus(const std::string& keyA, const std::string& keyB, 
                           float compatibilityScore) const;
    
    // Scoring helper functions (exposed for testing)
    float scoreBeatmatch(float bpmA, float bpmB) const;
    float scoreHarmonic(const std::string& keyA, const std::string& keyB) const;
    
private:
    MixQualityComponents components_{};
    float overallScore_ = 0.0f;
    bool bassClash_ = false;
    float effectiveBpmA_ = 128.0f;
    float effectiveBpmB_ = 128.0f;
    float transitionSmoothness_ = 1.0f;
    std::string keyA_;
    std::string keyB_;
    
    // Private scoring helper functions
    float scoreEQBalance(const float* spectrumA, const float* spectrumB, size_t numBins) const;
    float scoreEQBalanceFromEnergy(float energyA, float energyB) const;
    float scoreTransition(float smoothness) const;
};

} // namespace dj
