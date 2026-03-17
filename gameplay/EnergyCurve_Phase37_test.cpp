#include "gameplay/EnergyCurve.h"
#include "gameplay/MixQualityAnalyzer.h"
#include "visuals/EnergyCurveRenderer.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>

namespace dj {

void test_EnergyCurve_RingBuffer() {
    std::cout << "[TEST 1] EnergyCurve_RingBuffer...\n";

    EnergyCurve curve(30.0 * 60.0, 1.0);
    for (int i = 0; i < 2000; ++i) {
        curve.addSample(static_cast<float>(i % 100) / 100.0f, static_cast<double>(i), 0.5f);
    }

    const auto samples = curve.getCurve();
    assert(samples.size() == 1800);
    assert(samples.front().timestamp >= 200.0 && samples.front().timestamp <= 201.0);
    assert(samples.back().timestamp >= 1999.0 && samples.back().timestamp <= 2000.0);

    for (size_t index = 1; index < samples.size(); ++index) {
        assert(samples[index].timestamp >= samples[index - 1].timestamp);
    }

    std::cout << "PASS\n";
}

void test_EnergyCurve_Statistics() {
    std::cout << "[TEST 2] EnergyCurve_Statistics...\n";

    EnergyCurve curve(30.0 * 60.0, 1.0);
    const float values[] = {0.2f, 0.5f, 0.8f, 0.5f, 0.3f};
    for (int i = 0; i < 5; ++i) {
        curve.addSample(values[i], static_cast<double>(i), 0.5f);
    }

    assert(std::fabs(curve.getAverage() - 0.46f) < 0.001f);
    assert(std::fabs(curve.getPeak() - 0.8f) < 0.001f);
    assert(std::fabs(curve.getDip() - 0.2f) < 0.001f);

    std::cout << "PASS\n";
}

void test_MixQualityAnalyzer_BeatmatchScoring() {
    std::cout << "[TEST 3] MixQualityAnalyzer_BeatmatchScoring...\n";

    MixQualityAnalyzer analyzer;
    assert(std::fabs(analyzer.scoreBeatmatch(128.0f, 128.5f) - 23.75f) < 0.1f);
    assert(std::fabs(analyzer.scoreBeatmatch(120.0f, 125.0f) - 12.5f) < 0.1f);
    assert(std::fabs(analyzer.scoreBeatmatch(128.0f, 143.0f)) < 0.01f);

    std::cout << "PASS\n";
}

void test_MixQualityAnalyzer_HarmonicScoring() {
    std::cout << "[TEST 4] MixQualityAnalyzer_HarmonicScoring...\n";

    MixQualityAnalyzer analyzer;
    assert(std::fabs(analyzer.scoreHarmonic("8A", "8A") - 25.0f) < 0.1f);
    const float relative = analyzer.scoreHarmonic("8A", "8B");
    assert(relative >= 22.0f && relative <= 23.0f);
    const float distant = analyzer.scoreHarmonic("1A", "6B");
    assert(distant < 10.0f);

    std::cout << "PASS\n";
}

void test_MixQualityAnalyzer_BassClashDetection() {
    std::cout << "[TEST 5] MixQualityAnalyzer_BassClashDetection...\n";

    MixQualityAnalyzer analyzer;
    constexpr size_t bins = 512;
    float spectrumA[bins] = {};
    float spectrumB[bins] = {};

    for (int i = 0; i < 21; ++i) {
        spectrumA[i] = 0.85f;
        spectrumB[i] = 0.85f;
    }
    assert(analyzer.detectBassClash(spectrumA, spectrumB, bins));

    for (int i = 0; i < 21; ++i) {
        spectrumB[i] = 0.3f;
    }
    assert(!analyzer.detectBassClash(spectrumA, spectrumB, bins));

    std::cout << "PASS\n";
}

void test_EnergyCurveRenderer_BasicRender() {
    std::cout << "[TEST 6] EnergyCurveRenderer_BasicRender...\n";

    EnergyCurve curve(30.0 * 60.0, 1.0);
    for (int i = 0; i < 1800; ++i) {
        const float phase = static_cast<float>(i % 180) / 180.0f;
        const float energy = (phase < 0.5f) ? (phase * 2.0f) : ((1.0f - phase) * 2.0f);
        curve.addSample(energy, static_cast<double>(i), 0.5f);
    }

    EnergyCurveRenderOptions options;
    options.width = 80;
    options.height = 15;
    options.showGrid = true;
    options.showStats = false;

    EnergyCurveRenderer renderer(options);
    const std::string output = renderer.render(curve);

    assert(!output.empty());
    assert(output.find("0") != std::string::npos);
    assert(output.find("10") != std::string::npos);
    assert(output.find("20") != std::string::npos);
    assert(output.find("30 min") != std::string::npos);
    assert(output.find("100%") != std::string::npos);
    assert(output.find("50%") != std::string::npos);
    assert(output.find("0%") != std::string::npos);

    std::cout << "PASS\n";
}

void test_EnergyCurveRenderer_StatsOverlay() {
    std::cout << "[TEST 7] EnergyCurveRenderer_StatsOverlay...\n";

    EnergyCurve curve(30.0 * 60.0, 1.0);
    for (int i = 0; i < 120; ++i) {
        curve.addSample((i < 60) ? 0.65f : 0.92f, static_cast<double>(i), 0.5f);
    }

    MixQualityAnalyzer analyzer;
    analyzer.setAnalysisContext(128.0f, 128.2f, 0.92f, "8A", "8B");
    analyzer.setBassClashState(true);
    analyzer.analyzeMix(nullptr, nullptr, nullptr);

    EnergyCurveRenderOptions options;
    options.width = 80;
    options.height = 15;
    options.showGrid = true;
    options.showStats = true;

    EnergyCurveRenderer renderer(options);
    const std::string output = renderer.render(curve, &analyzer);

    assert(output.find("Energy:") != std::string::npos);
    assert(output.find("Avg:") != std::string::npos);
    assert(output.find("Peak:") != std::string::npos);
    assert(output.find("Mix Quality:") != std::string::npos);
    assert(output.find("Bass Clash!") != std::string::npos);

    std::cout << "PASS\n";
}

void test_MainLoopIntegration_Performance() {
    std::cout << "[TEST 8] MainLoopIntegration_Performance...\n";

    EnergyCurve curve(30.0 * 60.0, 1.0);
    MixQualityAnalyzer analyzer;

    EnergyCurveRenderOptions options;
    options.width = 80;
    options.height = 15;
    options.showGrid = true;
    options.showStats = true;
    EnergyCurveRenderer renderer(options);

    for (int second = 0; second < 60; ++second) {
        const float energy = std::clamp(0.55f + 0.25f * std::sin(static_cast<float>(second) * 0.15f), 0.0f, 1.0f);
        curve.addSample(energy, static_cast<double>(second), 0.45f);

        const float bpmA = 128.0f + static_cast<float>(second % 4) * 0.1f;
        const float bpmB = 127.8f + static_cast<float>(second % 3) * 0.1f;
        analyzer.setAnalysisContext(bpmA, bpmB, 0.85f, "8A", "8B");
        analyzer.setBassClashState((second % 25) == 0);
        analyzer.analyzeMix(nullptr, nullptr, nullptr);
    }

    const auto start = std::chrono::high_resolution_clock::now();
    const std::string output = renderer.render(curve, &analyzer);
    const auto end = std::chrono::high_resolution_clock::now();

    const auto renderUs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    const double renderMs = static_cast<double>(renderUs) / 1000.0;

    assert(curve.getCurve().size() == 60);
    assert(analyzer.getOverallScore() > 0.0f);
    assert(!output.empty());
    assert(renderMs < 10.0);

    std::cout << "PASS (" << renderMs << " ms render)\n";
}

} // namespace dj

int main() {
    std::cout << "=== Phase 37: Energy Curve & Mix Quality Tests ===\n\n";

    dj::test_EnergyCurve_RingBuffer();
    dj::test_EnergyCurve_Statistics();
    dj::test_MixQualityAnalyzer_BeatmatchScoring();
    dj::test_MixQualityAnalyzer_HarmonicScoring();
    dj::test_MixQualityAnalyzer_BassClashDetection();
    dj::test_EnergyCurveRenderer_BasicRender();
    dj::test_EnergyCurveRenderer_StatsOverlay();
    dj::test_MainLoopIntegration_Performance();

    std::cout << "\n=== All Phase 37 tests passed! ===\n";
    return 0;
}
