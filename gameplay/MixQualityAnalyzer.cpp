#include "gameplay/MixQualityAnalyzer.h"
#include "audio/CamelotAnalyzer.h"
#include "audio/Deck.h"
#include "audio/Mixer.h"
#include <algorithm>
#include <cmath>

namespace dj {

float MixQualityAnalyzer::analyzeMix(const Deck* deckA, const Deck* deckB, const Mixer* mixer) {
    (void)mixer;

    const float energyA = (deckA != nullptr) ? 0.55f : 0.5f;
    const float energyB = (deckB != nullptr) ? 0.55f : 0.5f;

    components_.beatmatchScore = scoreBeatmatch(effectiveBpmA_, effectiveBpmB_);
    components_.eqBalanceScore = scoreEQBalanceFromEnergy(energyA, energyB);
    components_.transitionScore = scoreTransition(transitionSmoothness_);
    components_.harmonicScore = scoreHarmonic(keyA_, keyB_);

    overallScore_ = std::clamp(
        components_.beatmatchScore +
        components_.eqBalanceScore +
        components_.transitionScore +
        components_.harmonicScore,
        0.0f,
        100.0f);

    return overallScore_;
}

MixQualityComponents MixQualityAnalyzer::getComponentScores() const {
    return components_;
}

float MixQualityAnalyzer::getOverallScore() const {
    return overallScore_;
}

bool MixQualityAnalyzer::hasBassClash() const {
    return bassClash_;
}

void MixQualityAnalyzer::setAnalysisContext(float effectiveBpmA,
                                            float effectiveBpmB,
                                            float transitionSmoothness,
                                            const std::string& keyA,
                                            const std::string& keyB) {
    effectiveBpmA_ = effectiveBpmA;
    effectiveBpmB_ = effectiveBpmB;
    transitionSmoothness_ = transitionSmoothness;
    keyA_ = keyA;
    keyB_ = keyB;
}

void MixQualityAnalyzer::setBassClashState(bool clashDetected) {
    bassClash_ = clashDetected;
}

bool MixQualityAnalyzer::detectBassClash(const float* spectrumA, const float* spectrumB, 
                                         size_t numBins) const {
    if (spectrumA == nullptr || spectrumB == nullptr || numBins == 0) {
        return false;
    }

    // Bass clash detection: Check if both decks have strong low-frequency energy.
    // Bass range: bins 0-20 (~0-430 Hz with 44.1kHz and 1024 FFT window).
    const size_t bassBinEnd = std::min<size_t>(21, numBins);
    if (bassBinEnd == 0) {
        return false;
    }

    float bassSumA = 0.0f;
    float bassSumB = 0.0f;
    float bassPeakA = 0.0f;
    float bassPeakB = 0.0f;

    for (size_t i = 0; i < bassBinEnd; ++i) {
        bassSumA += spectrumA[i];
        bassSumB += spectrumB[i];
        bassPeakA = std::max(bassPeakA, spectrumA[i]);
        bassPeakB = std::max(bassPeakB, spectrumB[i]);
    }

    const float bassAverageA = bassSumA / static_cast<float>(bassBinEnd);
    const float bassAverageB = bassSumB / static_cast<float>(bassBinEnd);

    // Use both average and peak thresholds to detect narrow-band bass tones as well
    // as broader low-end content.
    constexpr float averageThreshold = 0.08f;
    constexpr float peakThreshold = 0.30f;

    const bool deckAHasBass = (bassAverageA >= averageThreshold) || (bassPeakA >= peakThreshold);
    const bool deckBHasBass = (bassAverageB >= averageThreshold) || (bassPeakB >= peakThreshold);

    return deckAHasBass && deckBHasBass;
}

float MixQualityAnalyzer::applyCamelotBonus(const std::string& keyA, const std::string& keyB,
                                            float compatibilityScore) const {
    if (compatibilityScore <= 0.0f) {
        return 0.0f;
    }

    float bonus = compatibilityScore * 25.0f;

    // Small bonus for perfect same-key transitions
    if (!keyA.empty() && keyA == keyB) {
        bonus += 1.0f;
    }

    return std::clamp(bonus, 0.0f, 25.0f);
}

float MixQualityAnalyzer::scoreBeatmatch(float bpmA, float bpmB) const {
    // Beatmatch scoring formula:
    // 25 points for perfect match, linear decay to 0 at 10 BPM difference
    // Score = 25 * (1 - clamp(abs(delta) / 10.0, 0, 1))
    
    float delta = std::fabs(bpmA - bpmB);
    float normalized = std::clamp(delta / 10.0f, 0.0f, 1.0f);
    return 25.0f * (1.0f - normalized);
}

float MixQualityAnalyzer::scoreEQBalance(const float* spectrumA, const float* spectrumB,
                                        size_t numBins) const {
    if (spectrumA == nullptr || spectrumB == nullptr || numBins == 0) {
        return 12.5f;
    }

    const size_t lowEnd = std::min<size_t>(numBins, 21);
    const size_t midEnd = std::min<size_t>(numBins, 128);

    float lowEnergy = 0.0f;
    float midEnergy = 0.0f;
    float highEnergy = 0.0f;

    for (size_t bin = 0; bin < numBins; ++bin) {
        const float value = 0.5f * (spectrumA[bin] + spectrumB[bin]);
        if (bin < lowEnd) {
            lowEnergy += value;
        } else if (bin < midEnd) {
            midEnergy += value;
        } else {
            highEnergy += value;
        }
    }

    const float totalEnergy = lowEnergy + midEnergy + highEnergy;
    if (totalEnergy <= 0.0001f) {
        return 12.5f;
    }

    const float lowRatio = lowEnergy / totalEnergy;
    const float midRatio = midEnergy / totalEnergy;
    const float highRatio = highEnergy / totalEnergy;
    const float ideal = 1.0f / 3.0f;
    const float imbalance = std::fabs(lowRatio - ideal) + std::fabs(midRatio - ideal) + std::fabs(highRatio - ideal);
    const float normalizedImbalance = std::clamp(imbalance / 1.34f, 0.0f, 1.0f);

    return 25.0f * (1.0f - normalizedImbalance);
}

float MixQualityAnalyzer::scoreEQBalanceFromEnergy(float energyA, float energyB) const {
    const float clampedA = std::clamp(energyA, 0.0f, 1.0f);
    const float clampedB = std::clamp(energyB, 0.0f, 1.0f);
    const float energyDelta = std::fabs(clampedA - clampedB);
    return 25.0f * (1.0f - std::clamp(energyDelta, 0.0f, 1.0f));
}

float MixQualityAnalyzer::scoreTransition(float smoothness) const {
    return std::clamp(smoothness, 0.0f, 1.0f) * 25.0f;
}

float MixQualityAnalyzer::scoreHarmonic(const std::string& keyA, const std::string& keyB) const {
    // Harmonic scoring using Camelot compatibility
    // Convert compatibility (0.0-1.0) to score (0-25 points)
    
    CamelotAnalyzer camelot;
    float compatibility = camelot.getCompatibilityScore(keyA, keyB);
    return applyCamelotBonus(keyA, keyB, compatibility);
}

} // namespace dj
