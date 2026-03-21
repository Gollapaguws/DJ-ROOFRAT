#pragma once

#include <cstddef>

namespace dj {

class SplashScreenAnimator {
public:
    explicit SplashScreenAnimator(double durationSeconds = 4.25);

    [[nodiscard]] bool isActive(double elapsedSeconds) const noexcept;
    [[nodiscard]] float progress(double elapsedSeconds) const noexcept;
    [[nodiscard]] float introAmount(double elapsedSeconds) const noexcept;
    [[nodiscard]] float outroAmount(double elapsedSeconds) const noexcept;
    [[nodiscard]] float barLevel(std::size_t index, double elapsedSeconds) const noexcept;

    void render(double elapsedSeconds, float displayWidth, float displayHeight) const;

    static float easeOutCubic(float value) noexcept;
    static float easeInOutSine(float value) noexcept;

private:
    double durationSeconds_;
};

} // namespace dj
