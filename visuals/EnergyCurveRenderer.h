#pragma once

#include "gameplay/EnergyCurve.h"
#include "gameplay/MixQualityAnalyzer.h"
#include <string>

namespace dj {

struct EnergyCurveRenderOptions {
    int width = 80;
    int height = 15;
    double timeWindow = 1800.0;  // 30 minutes
    bool showGrid = true;
    bool showStats = true;
    int colorScheme = 0;  // 0=default, 1=monochrome, etc.
};

class EnergyCurveRenderer {
public:
    EnergyCurveRenderer(const EnergyCurveRenderOptions& options = {});
    
    // Render full energy curve with statistics
    std::string render(const EnergyCurve& curve, const MixQualityAnalyzer* analyzer = nullptr) const;
    
    // Render compact mini-curve (for HUD overlay)
    std::string renderMiniCurve(const EnergyCurve& curve) const;
    
private:
    EnergyCurveRenderOptions options_;
};

} // namespace dj
