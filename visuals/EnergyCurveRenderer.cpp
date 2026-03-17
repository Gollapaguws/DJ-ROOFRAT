#include "visuals/EnergyCurveRenderer.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace dj {

EnergyCurveRenderer::EnergyCurveRenderer(const EnergyCurveRenderOptions& options)
    : options_(options)
{
}

std::string EnergyCurveRenderer::render(const EnergyCurve& curve, 
                                       const MixQualityAnalyzer* analyzer) const {
    std::ostringstream oss;
    
    auto samples = curve.getCurve();
    if (samples.empty()) {
        oss << "Energy Curve: No data\n";
        return oss.str();
    }
    
    const int width = options_.width;
    const int height = options_.height;
    
    // Sample down to fit width (group samples into columns)
    std::vector<float> columnValues(width, 0.0f);
    int samplesPerColumn = static_cast<int>(samples.size()) / width;
    if (samplesPerColumn < 1) samplesPerColumn = 1;
    
    for (int col = 0; col < width && col * samplesPerColumn < static_cast<int>(samples.size()); ++col) {
        // Average energy values in this column's range
        float sum = 0.0f;
        int count = 0;
        int startIdx = col * samplesPerColumn;
        int endIdx = std::min(startIdx + samplesPerColumn, static_cast<int>(samples.size()));
        
        for (int i = startIdx; i < endIdx; ++i) {
            sum += samples[i].energy;
            ++count;
        }
        
        columnValues[col] = (count > 0) ? (sum / static_cast<float>(count)) : 0.0f;
    }
    
    // Build visualization grid
    std::vector<std::string> grid(height, std::string(width, ' '));
    
    // Plot curve
    for (int col = 0; col < width; ++col) {
        float energy = columnValues[col];
        int row = height - 1 - static_cast<int>(energy * (height - 1));
        row = std::clamp(row, 0, height - 1);
        
        // Use simple characters that are ASCII-safe
        grid[row][col] = '#';
    }
    
    // Render grid to string
    oss << "Energy Curve (Last 30 min)";
    if (analyzer != nullptr) {
        oss << "  Mix Quality: " << static_cast<int>(std::lround(analyzer->getOverallScore())) << "/100";
    }
    oss << "\n";
    
    for (int row = 0; row < height; ++row) {
        // Add Y-axis labels
        if (row == 0) {
            oss << "100%|";
        } else if (row == height / 2) {
            oss << " 50%|";
        } else if (row == height - 1) {
            oss << "  0%|";
        } else {
            oss << "    |";
        }
        
        oss << grid[row] << "\n";
    }
    
    // Add X-axis
    if (options_.showGrid) {
        oss << "    +";
        for (int i = 0; i < width; ++i) {
            oss << "-";
        }
        oss << "+\n";
        
        // Time labels (0, 10, 20, 30 min)
        oss << "    0          10         20         30 min\n";
    }

    if (options_.showStats) {
        const int currentEnergy = static_cast<int>(std::lround(samples.back().energy * 100.0f));
        const int averageEnergy = static_cast<int>(std::lround(curve.getAverage() * 100.0f));
        const int peakEnergy = static_cast<int>(std::lround(curve.getPeak() * 100.0f));
        const int dipEnergy = static_cast<int>(std::lround(curve.getDip() * 100.0f));

        oss << "Energy: " << currentEnergy << "%"
            << "  Avg: " << averageEnergy << "%"
            << "  Peak: " << peakEnergy << "%"
            << "  Dip: " << dipEnergy << "%\n";

        if (analyzer != nullptr) {
            oss << "Mix Quality: " << static_cast<int>(std::lround(analyzer->getOverallScore())) << "/100";
            if (analyzer->hasBassClash()) {
                oss << "  ! Bass Clash!";
            }
            oss << "\n";
        }
    }
    
    return oss.str();
}

std::string EnergyCurveRenderer::renderMiniCurve(const EnergyCurve& curve) const {
    auto samples = curve.getCurve();
    if (samples.empty()) {
        return "[No energy data]";
    }

    constexpr int miniWidth = 32;
    std::string line(miniWidth, '.');

    const int samplesPerColumn = std::max(1, static_cast<int>(samples.size()) / miniWidth);
    for (int col = 0; col < miniWidth; ++col) {
        const int start = col * samplesPerColumn;
        if (start >= static_cast<int>(samples.size())) {
            break;
        }

        const int end = std::min(start + samplesPerColumn, static_cast<int>(samples.size()));
        float average = 0.0f;
        for (int index = start; index < end; ++index) {
            average += samples[static_cast<size_t>(index)].energy;
        }
        average /= static_cast<float>(end - start);

        if (average >= 0.8f) {
            line[static_cast<size_t>(col)] = '#';
        } else if (average >= 0.5f) {
            line[static_cast<size_t>(col)] = '=';
        } else if (average >= 0.2f) {
            line[static_cast<size_t>(col)] = '-';
        }
    }

    std::ostringstream oss;
    oss << "[" << line << "] " << static_cast<int>(std::lround(samples.back().energy * 100.0f)) << "%";
    return oss.str();
}

} // namespace dj
