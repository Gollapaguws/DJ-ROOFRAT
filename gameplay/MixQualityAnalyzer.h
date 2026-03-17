#pragma once

#include <cstddef>
#include <string>

namespace dj {

// Forward declarations
class Deck;
class Mixer;
struct TrackMetadata;
struct SpectrumData;

struct MixQualityComponents {
    float beatmatchScore;      // 0-25
    float eqBalanceScore;      // 0-25
    float transitionScore;     // 0-25
    float harmonicScore;       // 0-25
};

class MixQualityAnalyzer {
public:
    MixQualityAnalyzer() = default;
    
    // Analyze mix quality between two decks
    // Returns overall score 0-100
    float analyzeMix(const Deck* deckA, const Deck* deckB, 
                     const Mixer* mixer = nullptr);
    
    // Get individual component scores
    MixQualityComponents getComponentScores() const;
    
    // Get overall score (sum of components, clamped 0-100)
    float getOverallScore() const;

    // Whether a bass clash was detected in the latest analysis window
    bool hasBassClash() const;

    // Set runtime analysis context from main loop data
    void setAnalysisContext(float effectiveBpmA,
                            float effectiveBpmB,
                            float transitionSmoothness,
                            const std::string& keyA,
                            const std::string& keyB);

    // Set externally computed bass clash state (e.g., from spectrum proxy metrics)
    void setBassClashState(bool clashDetected);
    
    // Detect bass frequency clash
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
