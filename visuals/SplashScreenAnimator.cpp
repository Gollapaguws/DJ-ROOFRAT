#include "visuals/SplashScreenAnimator.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <string>

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include "imgui.h"
#endif

namespace {

constexpr float clamp01(float value) noexcept {
    return std::clamp(value, 0.0f, 1.0f);
}

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
ImU32 rgba(int r, int g, int b, float a) {
    return ImGui::GetColorU32(ImVec4(
        static_cast<float>(r) / 255.0f,
        static_cast<float>(g) / 255.0f,
        static_cast<float>(b) / 255.0f,
        clamp01(a)));
}

void drawLaserBeam(
    ImDrawList* drawList,
    const ImVec2& origin,
    const ImVec2& target,
    float width,
    ImU32 glowColor,
    ImU32 coreColor) {
    const float dx = target.x - origin.x;
    const float dy = target.y - origin.y;
    const float length = std::sqrt((dx * dx) + (dy * dy));
    if (length <= 0.001f) {
        return;
    }

    const float nx = -dy / length;
    const float ny = dx / length;
    const float glowWidth = width * 1.9f;
    const float taperWidth = width * 0.38f;

    drawList->AddQuadFilled(
        ImVec2(origin.x + (nx * glowWidth), origin.y + (ny * glowWidth)),
        ImVec2(origin.x - (nx * glowWidth), origin.y - (ny * glowWidth)),
        ImVec2(target.x - (nx * taperWidth), target.y - (ny * taperWidth)),
        ImVec2(target.x + (nx * taperWidth), target.y + (ny * taperWidth)),
        glowColor);

    drawList->AddQuadFilled(
        ImVec2(origin.x + (nx * width), origin.y + (ny * width)),
        ImVec2(origin.x - (nx * width), origin.y - (ny * width)),
        ImVec2(target.x - (nx * taperWidth * 0.58f), target.y - (ny * taperWidth * 0.58f)),
        ImVec2(target.x + (nx * taperWidth * 0.58f), target.y + (ny * taperWidth * 0.58f)),
        coreColor);
}

void drawSpotlightCone(
    ImDrawList* drawList,
    const ImVec2& origin,
    const ImVec2& leftEdge,
    const ImVec2& rightEdge,
    ImU32 color) {
    drawList->AddTriangleFilled(origin, leftEdge, rightEdge, color);
}

void drawSpeakerStack(
    ImDrawList* drawList,
    const ImVec2& min,
    const ImVec2& max,
    float visibility,
    bool warmAccent) {
    const float radius = 18.0f;
    drawList->AddRectFilled(min, max, rgba(9, 10, 18, 0.88f * visibility), radius);
    drawList->AddRect(
        min,
        max,
        warmAccent ? rgba(255, 124, 84, 0.28f * visibility) : rgba(82, 224, 230, 0.28f * visibility),
        radius,
        0,
        1.4f);

    const float centerX = (min.x + max.x) * 0.5f;
    const float width = max.x - min.x;
    const ImVec2 hornMin(centerX - (width * 0.20f), min.y + 16.0f);
    const ImVec2 hornMax(centerX + (width * 0.20f), min.y + 28.0f);
    drawList->AddRectFilled(
        hornMin,
        hornMax,
        warmAccent ? rgba(255, 162, 120, 0.50f * visibility) : rgba(142, 250, 242, 0.46f * visibility),
        4.0f);

    const float wooferRadiusLarge = width * 0.23f;
    const float wooferRadiusSmall = width * 0.16f;
    const ImVec2 wooferA(centerX, min.y + ((max.y - min.y) * 0.46f));
    const ImVec2 wooferB(centerX, min.y + ((max.y - min.y) * 0.76f));
    drawList->AddCircleFilled(wooferA, wooferRadiusLarge, rgba(18, 22, 34, 0.94f * visibility), 48);
    drawList->AddCircle(wooferA, wooferRadiusLarge, rgba(235, 243, 255, 0.18f * visibility), 48, 2.0f);
    drawList->AddCircleFilled(
        wooferA,
        wooferRadiusLarge * 0.42f,
        warmAccent ? rgba(255, 104, 66, 0.54f * visibility) : rgba(76, 214, 226, 0.54f * visibility),
        36);

    drawList->AddCircleFilled(wooferB, wooferRadiusSmall, rgba(18, 22, 34, 0.94f * visibility), 36);
    drawList->AddCircle(wooferB, wooferRadiusSmall, rgba(235, 243, 255, 0.16f * visibility), 36, 2.0f);
    drawList->AddCircleFilled(
        wooferB,
        wooferRadiusSmall * 0.42f,
        warmAccent ? rgba(255, 150, 112, 0.40f * visibility) : rgba(162, 255, 247, 0.38f * visibility),
        24);
}

void drawCrowdBand(
    ImDrawList* drawList,
    float displayWidth,
    float displayHeight,
    float time,
    float visibility) {
    const int people = 18;
    const float spacing = displayWidth / static_cast<float>(people - 1);
    const float baselineY = displayHeight - 10.0f;
    for (int i = 0; i < people; ++i) {
        const float phase = (time * 1.7f) + (static_cast<float>(i) * 0.47f);
        const float bob = std::sin(phase) * 6.0f;
        const float x = spacing * static_cast<float>(i);
        const float bodyWidth = 14.0f + (2.5f * (0.5f + (0.5f * std::sin(phase * 0.9f))));
        const float bodyHeight = 26.0f + (12.0f * (0.5f + (0.5f * std::sin((phase * 1.4f) + 1.1f))));
        const float headRadius = 6.0f + (1.2f * (0.5f + (0.5f * std::cos(phase))));
        const bool warmAccent = (i % 4) == 1;
        const float alpha = (0.17f + (0.03f * std::sin((time * 2.1f) + i))) * visibility;
        drawList->AddRectFilled(
            ImVec2(x - bodyWidth, baselineY - bodyHeight + bob),
            ImVec2(x + bodyWidth, baselineY),
            warmAccent ? rgba(255, 110, 72, alpha) : rgba(72, 212, 220, alpha),
            8.0f);
        drawList->AddCircleFilled(
            ImVec2(x, baselineY - bodyHeight - headRadius + bob),
            headRadius,
            warmAccent ? rgba(255, 160, 120, alpha * 1.15f) : rgba(176, 255, 248, alpha * 1.12f),
            24);
    }
}
#endif

} // namespace

namespace dj {

SplashScreenAnimator::SplashScreenAnimator(double durationSeconds)
    : durationSeconds_(std::max(1.0, durationSeconds)) {
}

bool SplashScreenAnimator::isActive(double elapsedSeconds) const noexcept {
    return elapsedSeconds < durationSeconds_;
}

float SplashScreenAnimator::progress(double elapsedSeconds) const noexcept {
    if (elapsedSeconds <= 0.0) {
        return 0.0f;
    }

    return clamp01(static_cast<float>(elapsedSeconds / durationSeconds_));
}

float SplashScreenAnimator::introAmount(double elapsedSeconds) const noexcept {
    return easeOutCubic(progress(elapsedSeconds) * 1.75f);
}

float SplashScreenAnimator::outroAmount(double elapsedSeconds) const noexcept {
    return easeInOutSine((progress(elapsedSeconds) - 0.72f) / 0.28f);
}

float SplashScreenAnimator::barLevel(std::size_t index, double elapsedSeconds) const noexcept {
    const float t = static_cast<float>(elapsedSeconds);
    const float lane = static_cast<float>(index);
    const float fastPulse = 0.5f + 0.5f * std::sin((t * 5.4f) + (lane * 0.63f));
    const float slowPulse = 0.5f + 0.5f * std::sin((t * 1.8f) - (lane * 0.31f) + 1.2f);
    return clamp01(0.18f + (0.52f * fastPulse) + (0.24f * slowPulse));
}

float SplashScreenAnimator::easeOutCubic(float value) noexcept {
    const float t = clamp01(value);
    const float inv = 1.0f - t;
    return 1.0f - (inv * inv * inv);
}

float SplashScreenAnimator::easeInOutSine(float value) noexcept {
    const float t = clamp01(value);
    return 0.5f - (0.5f * std::cos(std::numbers::pi_v<float> * t));
}

void SplashScreenAnimator::render(double elapsedSeconds, float displayWidth, float displayHeight) const {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!isActive(elapsedSeconds) ||
        displayWidth <= 0.0f ||
        displayHeight <= 0.0f ||
        ImGui::GetCurrentContext() == nullptr) {
        return;
    }

    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    if (drawList == nullptr) {
        return;
    }

    const float progressValue = progress(elapsedSeconds);
    const float intro = introAmount(elapsedSeconds);
    const float outro = outroAmount(elapsedSeconds);
    const float visibility = 1.0f - outro;
    if (visibility <= 0.001f) {
        return;
    }

    const ImVec2 screenMin(0.0f, 0.0f);
    const ImVec2 screenMax(displayWidth, displayHeight);
    const ImVec2 center(displayWidth * 0.5f, displayHeight * 0.5f);

    const float panelWidth = std::min(displayWidth * 0.76f, 980.0f);
    const float panelHeight = std::min(displayHeight * 0.60f, 490.0f);
    const float panelShift = ((1.0f - intro) * 48.0f) + (outro * 36.0f);
    const ImVec2 panelMin(center.x - (panelWidth * 0.5f), center.y - (panelHeight * 0.5f) - panelShift);
    const ImVec2 panelMax(center.x + (panelWidth * 0.5f), center.y + (panelHeight * 0.5f) - panelShift);
    const float panelRadius = 26.0f;
    const float time = static_cast<float>(elapsedSeconds);

    drawCrowdBand(drawList, displayWidth, displayHeight, time, visibility);

    drawList->AddRectFilledMultiColor(
        screenMin,
        screenMax,
        rgba(2, 4, 9, 0.96f * visibility),
        rgba(18, 8, 12, 0.96f * visibility),
        rgba(4, 18, 24, 0.98f * visibility),
        rgba(1, 3, 8, 0.98f * visibility));
    drawList->AddRectFilledMultiColor(
        screenMin,
        screenMax,
        rgba(0, 0, 0, 0.12f * visibility),
        rgba(255, 92, 48, 0.07f * visibility),
        rgba(36, 214, 222, 0.10f * visibility),
        rgba(0, 0, 0, 0.10f * visibility));

    const float horizontalSpacing = std::max(30.0f, displayHeight / 17.0f);
    const float horizontalOffset = std::fmod(time * 62.0f, horizontalSpacing);
    for (float y = -horizontalSpacing + horizontalOffset; y < displayHeight + horizontalSpacing; y += horizontalSpacing) {
        drawList->AddLine(
            ImVec2(0.0f, y),
            ImVec2(displayWidth, y),
            rgba(82, 202, 214, 0.05f * visibility),
            1.0f);
    }

    const float verticalSpacing = std::max(52.0f, displayWidth / 20.0f);
    const float verticalOffset = std::fmod(time * 34.0f, verticalSpacing);
    for (float x = -verticalSpacing + verticalOffset; x < displayWidth + verticalSpacing; x += verticalSpacing) {
        drawList->AddLine(
            ImVec2(x, 0.0f),
            ImVec2(x, displayHeight),
            rgba(255, 124, 74, 0.035f * visibility),
            1.0f);
    }

    const float horizonY = panelMax.y + 46.0f;
    drawList->AddRectFilledMultiColor(
        ImVec2(0.0f, horizonY - 10.0f),
        ImVec2(displayWidth, displayHeight),
        rgba(0, 0, 0, 0.00f),
        rgba(0, 0, 0, 0.00f),
        rgba(0, 0, 0, 0.42f * visibility),
        rgba(0, 0, 0, 0.42f * visibility));
    for (int i = 0; i < 12; ++i) {
        const float ratio = static_cast<float>(i) / 11.0f;
        const float x = displayWidth * ratio;
        drawList->AddLine(
            ImVec2(center.x, horizonY),
            ImVec2(x, displayHeight),
            ((i % 2) == 0) ? rgba(255, 118, 74, 0.07f * visibility) : rgba(84, 224, 228, 0.07f * visibility),
            1.0f);
    }
    for (int i = 1; i <= 6; ++i) {
        const float depth = static_cast<float>(i) / 6.0f;
        const float y = horizonY + (depth * depth * (displayHeight - horizonY - 16.0f));
        drawList->AddLine(
            ImVec2(0.0f, y),
            ImVec2(displayWidth, y),
            rgba(220, 228, 236, 0.025f * visibility),
            1.0f);
    }

    const float laserSweepA = std::sin(time * 0.95f) * (displayWidth * 0.18f);
    const float laserSweepB = std::cos((time * 0.85f) + 0.7f) * (displayWidth * 0.14f);
    const ImVec2 laserTargetA(center.x - (panelWidth * 0.14f) + laserSweepA, panelMin.y + (panelHeight * 0.28f));
    const ImVec2 laserTargetB(center.x + (panelWidth * 0.14f) + laserSweepB, panelMin.y + (panelHeight * 0.30f));
    const ImVec2 laserTargetC(center.x + std::sin((time * 1.4f) + 0.4f) * (panelWidth * 0.12f), panelMin.y + (panelHeight * 0.12f));
    drawLaserBeam(
        drawList,
        ImVec2(panelMin.x + 84.0f, 0.0f),
        laserTargetA,
        20.0f,
        rgba(56, 228, 232, 0.08f * visibility),
        rgba(114, 255, 248, 0.18f * visibility));
    drawLaserBeam(
        drawList,
        ImVec2(panelMax.x - 84.0f, 0.0f),
        laserTargetB,
        20.0f,
        rgba(255, 110, 72, 0.08f * visibility),
        rgba(255, 154, 110, 0.18f * visibility));
    drawLaserBeam(
        drawList,
        ImVec2(center.x, panelMin.y - 24.0f),
        laserTargetC,
        12.0f,
        rgba(255, 192, 118, 0.06f * visibility),
        rgba(255, 218, 170, 0.14f * visibility));

    drawList->AddRectFilled(
        ImVec2(panelMin.x + 14.0f, panelMin.y + 18.0f),
        ImVec2(panelMax.x + 14.0f, panelMax.y + 18.0f),
        rgba(0, 0, 0, 0.34f * visibility),
        panelRadius);
    drawList->AddRectFilledMultiColor(
        panelMin,
        panelMax,
        rgba(11, 10, 18, 0.97f * visibility),
        rgba(30, 12, 18, 0.94f * visibility),
        rgba(6, 16, 23, 0.96f * visibility),
        rgba(8, 8, 16, 0.97f * visibility));
    drawList->AddRect(panelMin, panelMax, rgba(255, 116, 78, 0.22f * visibility), panelRadius, 0, 1.5f);
    drawList->AddRect(
        ImVec2(panelMin.x + 10.0f, panelMin.y + 10.0f),
        ImVec2(panelMax.x - 10.0f, panelMax.y - 10.0f),
        rgba(88, 234, 226, 0.18f * visibility),
        panelRadius - 8.0f,
        0,
        1.0f);

    const float trussY = panelMin.y + 24.0f;
    drawList->AddLine(
        ImVec2(panelMin.x + 38.0f, trussY),
        ImVec2(panelMax.x - 38.0f, trussY),
        rgba(255, 148, 96, 0.30f * visibility),
        2.0f);
    drawList->AddRectFilled(
        ImVec2(panelMin.x + 112.0f, panelMin.y + 18.0f),
        ImVec2(panelMax.x - 112.0f, panelMin.y + 40.0f),
        rgba(10, 10, 16, 0.78f * visibility),
        8.0f);
    drawList->AddRect(
        ImVec2(panelMin.x + 112.0f, panelMin.y + 18.0f),
        ImVec2(panelMax.x - 112.0f, panelMin.y + 40.0f),
        rgba(255, 124, 78, 0.22f * visibility),
        8.0f,
        0,
        1.0f);
    for (int i = 0; i < 24; ++i) {
        const float x = panelMin.x + 124.0f + (i * ((panelWidth - 248.0f) / 23.0f));
        const float blink = 0.45f + (0.55f * std::sin((time * 4.8f) + (i * 0.55f)));
        drawList->AddRectFilled(
            ImVec2(x - 6.0f, panelMin.y + 24.0f),
            ImVec2(x + 6.0f, panelMin.y + 34.0f),
            (i % 3 == 0)
                ? rgba(255, 118, 74, 0.22f * blink * visibility)
                : rgba(86, 236, 228, 0.18f * blink * visibility),
            2.0f);
    }
    for (int i = 0; i < 7; ++i) {
        const float lightX = panelMin.x + 76.0f + (i * ((panelWidth - 152.0f) / 6.0f));
        const bool warmAccent = (i % 2) == 0;
        drawList->AddCircleFilled(
            ImVec2(lightX, trussY + 10.0f),
            5.0f,
            warmAccent ? rgba(255, 114, 72, 0.72f * visibility) : rgba(86, 236, 228, 0.72f * visibility),
            18);
        drawList->AddTriangleFilled(
            ImVec2(lightX - 6.0f, trussY + 16.0f),
            ImVec2(lightX + 6.0f, trussY + 16.0f),
            ImVec2(lightX, trussY + 28.0f),
            warmAccent ? rgba(255, 114, 72, 0.12f * visibility) : rgba(86, 236, 228, 0.12f * visibility));
        drawSpotlightCone(
            drawList,
            ImVec2(lightX, trussY + 18.0f),
            ImVec2(lightX - 84.0f, panelMax.y - 14.0f),
            ImVec2(lightX + 84.0f, panelMax.y - 14.0f),
            warmAccent ? rgba(255, 110, 72, 0.045f * visibility) : rgba(86, 236, 228, 0.045f * visibility));
    }

    const float panelGridY = std::fmod(time * 42.0f, 34.0f);
    for (float y = panelMin.y - 34.0f + panelGridY; y < panelMax.y; y += 34.0f) {
        drawList->AddLine(
            ImVec2(panelMin.x + 24.0f, y),
            ImVec2(panelMax.x - 24.0f, y),
            rgba(92, 190, 210, 0.06f * visibility),
            1.0f);
    }

    const ImVec2 speakerMinLeft(panelMin.x + 34.0f, panelMin.y + 68.0f);
    const ImVec2 speakerMaxLeft(panelMin.x + 128.0f, panelMax.y - 82.0f);
    const ImVec2 speakerMinRight(panelMax.x - 128.0f, panelMin.y + 68.0f);
    const ImVec2 speakerMaxRight(panelMax.x - 34.0f, panelMax.y - 82.0f);
    drawSpeakerStack(drawList, speakerMinLeft, speakerMaxLeft, visibility, false);
    drawSpeakerStack(drawList, speakerMinRight, speakerMaxRight, visibility, true);
    drawList->AddRectFilled(
        ImVec2(panelMin.x + 150.0f, panelMax.y - 34.0f),
        ImVec2(panelMax.x - 150.0f, panelMax.y - 18.0f),
        rgba(12, 12, 18, 0.72f * visibility),
        8.0f);

    const ImVec2 hub(center.x, panelMin.y + (panelHeight * 0.46f));
    const float pulse = 0.5f + (0.5f * std::sin(time * 6.2f));
    const float coreRadius = 46.0f + (12.0f * pulse);
    drawList->AddCircleFilled(hub, coreRadius * 2.45f, rgba(255, 96, 58, 0.08f * visibility), 72);
    drawList->AddCircleFilled(hub, coreRadius * 1.82f, rgba(32, 206, 222, 0.10f * visibility), 72);
    drawList->AddCircleFilled(hub, coreRadius * 1.14f, rgba(10, 12, 18, 0.96f * visibility), 72);
    drawList->AddCircleFilled(hub, coreRadius * 0.24f, rgba(255, 228, 196, 0.72f * visibility), 36);
    drawList->AddCircle(hub, coreRadius + 10.0f, rgba(255, 142, 96, 0.48f * visibility), 72, 2.0f);
    drawList->AddCircle(hub, coreRadius + 24.0f, rgba(104, 244, 236, 0.42f * visibility), 72, 1.6f);
    drawList->AddCircle(hub, coreRadius + 42.0f, rgba(255, 102, 60, 0.22f * visibility), 72, 1.2f);

    const float arcRotation = time * 1.85f;
    drawList->PathClear();
    drawList->PathArcTo(hub, coreRadius + 48.0f, arcRotation, arcRotation + (std::numbers::pi_v<float> * 1.05f), 64);
    drawList->PathStroke(rgba(255, 132, 90, 0.54f * visibility), 0, 3.0f);
    drawList->PathClear();
    drawList->PathArcTo(
        hub,
        coreRadius + 64.0f,
        -(arcRotation * 0.75f),
        -(arcRotation * 0.75f) + (std::numbers::pi_v<float> * 0.80f),
        64);
    drawList->PathStroke(rgba(92, 234, 226, 0.48f * visibility), 0, 3.0f);

    for (int i = 0; i < 24; ++i) {
        const float angle = arcRotation + (static_cast<float>(i) * (std::numbers::pi_v<float> / 12.0f));
        const float tickInner = coreRadius + 16.0f;
        const float tickOuter = tickInner + ((i % 4) == 0 ? 12.0f : 7.0f);
        const float cs = std::cos(angle);
        const float sn = std::sin(angle);
        drawList->AddLine(
            ImVec2(hub.x + (cs * tickInner), hub.y + (sn * tickInner)),
            ImVec2(hub.x + (cs * tickOuter), hub.y + (sn * tickOuter)),
            ((i % 2) == 0) ? rgba(255, 138, 90, 0.34f * visibility) : rgba(104, 244, 236, 0.32f * visibility),
            1.4f);
    }

    const float needleAngle = -(std::numbers::pi_v<float> * 0.20f) + (std::sin(time * 1.3f) * 0.18f);
    drawList->AddLine(
        hub,
        ImVec2(
            hub.x + (std::cos(needleAngle) * (coreRadius + 62.0f)),
            hub.y + (std::sin(needleAngle) * (coreRadius + 62.0f))),
        rgba(255, 236, 220, 0.34f * visibility),
        2.0f);

    const int barCount = 11;
    const float barWidth = std::min(22.0f, panelWidth / 34.0f);
    const float barGap = barWidth * 0.45f;
    const float totalBarWidth = (barCount * barWidth) + ((barCount - 1) * barGap);
    const float barBaseY = panelMax.y - 88.0f;
    const float barStartX = center.x - (totalBarWidth * 0.5f);
    drawList->AddRectFilled(
        ImVec2(barStartX - 18.0f, barBaseY - 128.0f),
        ImVec2(barStartX + totalBarWidth + 18.0f, barBaseY + 10.0f),
        rgba(0, 0, 0, 0.12f * visibility),
        16.0f);
    for (int i = 0; i < barCount; ++i) {
        const float level = barLevel(static_cast<std::size_t>(i), elapsedSeconds);
        const float barHeight = 26.0f + (level * 106.0f * intro);
        const float x0 = barStartX + (i * (barWidth + barGap));
        const float x1 = x0 + barWidth;
        const ImVec2 barMin(x0, barBaseY - barHeight);
        const ImVec2 barMax(x1, barBaseY);
        const bool warmAccent = (i % 3) == 1;
        drawList->AddRectFilled(
            barMin,
            barMax,
            warmAccent ? rgba(255, 114, 72, 0.90f * visibility) : rgba(72, 228, 220, 0.90f * visibility),
            6.0f);
        drawList->AddRectFilled(
            ImVec2(x0, std::max(barMin.y, barMax.y - 14.0f)),
            barMax,
            warmAccent ? rgba(255, 190, 155, 0.24f * visibility) : rgba(202, 255, 252, 0.22f * visibility),
            6.0f);
        drawList->AddCircleFilled(
            ImVec2((x0 + x1) * 0.5f, barMin.y - 6.0f),
            3.0f,
            warmAccent ? rgba(255, 174, 140, 0.46f * visibility) : rgba(168, 255, 248, 0.42f * visibility),
            12);
    }

    ImFont* font = ImGui::GetFont();
    if (font == nullptr) {
        return;
    }

    const char* overline = "AFTER HOURS LOAD-IN";
    const char* title = "DJ-ROOFRAT";
    const char* subtitle = "sub stacks / lasers / crowd heat";
    const char* status = progressValue < 0.32f
        ? "arming sub stacks"
        : (progressValue < 0.70f ? "sweeping laser rig" : "crowd lights live");

    const float overlineSize = std::clamp(panelWidth * 0.028f, 16.0f, 22.0f);
    const float titleSize = std::clamp(panelWidth * 0.092f, 44.0f, 70.0f);
    const float subtitleSize = std::clamp(panelWidth * 0.03f, 18.0f, 24.0f);
    const float copyOpacity = intro * visibility;

    const ImVec2 overlineSizePx = font->CalcTextSizeA(overlineSize, 10000.0f, 0.0f, overline);
    const ImVec2 titleSizePx = font->CalcTextSizeA(titleSize, 10000.0f, 0.0f, title);
    const ImVec2 subtitleSizePx = font->CalcTextSizeA(subtitleSize, 10000.0f, 0.0f, subtitle);

    const float textY = panelMin.y + 48.0f + ((1.0f - intro) * 22.0f) - (outro * 24.0f);
    drawList->AddRectFilled(
        ImVec2(center.x - 170.0f, textY - 16.0f),
        ImVec2(center.x + 170.0f, textY + 8.0f),
        rgba(7, 10, 16, 0.54f * copyOpacity),
        10.0f);
    drawList->AddText(
        font,
        overlineSize,
        ImVec2(center.x - (overlineSizePx.x * 0.5f), textY),
        rgba(255, 171, 126, 0.76f * copyOpacity),
        overline);
    drawList->AddText(
        font,
        titleSize,
        ImVec2(center.x - (titleSizePx.x * 0.5f) + 4.0f, textY + 34.0f),
        rgba(255, 110, 70, 0.54f * copyOpacity),
        title);
    drawList->AddText(
        font,
        titleSize,
        ImVec2(center.x - (titleSizePx.x * 0.5f), textY + 30.0f),
        rgba(245, 248, 255, 0.96f * copyOpacity),
        title);
    drawList->AddText(
        font,
        subtitleSize,
        ImVec2(center.x - (subtitleSizePx.x * 0.5f), textY + 104.0f),
        rgba(142, 234, 228, 0.74f * copyOpacity),
        subtitle);

    const ImVec2 progressMin(panelMin.x + 42.0f, panelMax.y - 40.0f);
    const ImVec2 progressMax(panelMax.x - 42.0f, panelMax.y - 22.0f);
    drawList->AddRectFilled(progressMin, progressMax, rgba(8, 10, 18, 0.88f * visibility), 10.0f);

    const float progressWidth = (progressMax.x - progressMin.x) * easeOutCubic(progressValue * 1.05f);
    drawList->AddRectFilledMultiColor(
        progressMin,
        ImVec2(progressMin.x + progressWidth, progressMax.y),
        rgba(88, 232, 224, 0.90f * visibility),
        rgba(255, 132, 86, 0.92f * visibility),
        rgba(255, 92, 56, 0.88f * visibility),
        rgba(78, 212, 226, 0.88f * visibility));
    for (int segment = 1; segment < 16; ++segment) {
        const float x = progressMin.x + ((progressMax.x - progressMin.x) * (static_cast<float>(segment) / 16.0f));
        drawList->AddLine(
            ImVec2(x, progressMin.y + 2.0f),
            ImVec2(x, progressMax.y - 2.0f),
            rgba(255, 255, 255, 0.06f * visibility),
            1.0f);
    }

    drawList->AddText(
        font,
        18.0f,
        ImVec2(progressMin.x, progressMin.y - 28.0f),
        rgba(220, 226, 236, 0.82f * visibility),
        status);

    const std::string percentLabel = std::to_string(static_cast<int>(std::round(progressValue * 100.0f))) + "%";
    const ImVec2 percentSize = font->CalcTextSizeA(18.0f, 10000.0f, 0.0f, percentLabel.c_str());
    drawList->AddText(
        font,
        18.0f,
        ImVec2(progressMax.x - percentSize.x, progressMin.y - 28.0f),
        rgba(255, 196, 158, 0.84f * visibility),
        percentLabel.c_str());
#else
    (void)elapsedSeconds;
    (void)displayWidth;
    (void)displayHeight;
#endif
}

} // namespace dj
