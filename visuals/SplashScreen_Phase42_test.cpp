#include "visuals/SplashScreenAnimator.h"

#include <cassert>
#include <cmath>
#include <iostream>

namespace {

void test_SplashScreenAnimator_TimingWindow() {
    std::cout << "[TEST 1] SplashScreenAnimator_TimingWindow...\n";

    dj::SplashScreenAnimator splash(4.0);
    assert(splash.isActive(0.0) && "Splash should be active at launch");
    assert(splash.isActive(3.99) && "Splash should remain active before its duration elapses");
    assert(!splash.isActive(4.0) && "Splash should end exactly at its duration");

    std::cout << "PASS\n";
}

void test_SplashScreenAnimator_ProgressClamp() {
    std::cout << "[TEST 2] SplashScreenAnimator_ProgressClamp...\n";

    dj::SplashScreenAnimator splash(5.0);
    assert(std::abs(splash.progress(-2.0) - 0.0f) < 0.0001f && "Negative time should clamp to zero");
    assert(std::abs(splash.progress(2.5) - 0.5f) < 0.0001f && "Halfway time should report halfway progress");
    assert(std::abs(splash.progress(6.0) - 1.0f) < 0.0001f && "Elapsed time beyond duration should clamp to one");

    std::cout << "PASS\n";
}

void test_SplashScreenAnimator_AnimationSignalsStayBounded() {
    std::cout << "[TEST 3] SplashScreenAnimator_AnimationSignalsStayBounded...\n";

    dj::SplashScreenAnimator splash(4.25);
    for (int i = 0; i < 11; ++i) {
        const float level = splash.barLevel(static_cast<std::size_t>(i), 1.5);
        assert(level >= 0.0f && level <= 1.0f && "Bar animation should stay normalized");
    }

    const float intro = splash.introAmount(0.6);
    const float outro = splash.outroAmount(3.7);
    assert(intro >= 0.0f && intro <= 1.0f && "Intro amount should stay normalized");
    assert(outro >= 0.0f && outro <= 1.0f && "Outro amount should stay normalized");
    assert(intro > 0.0f && "Intro should ramp up shortly after launch");
    assert(outro > 0.0f && "Outro should ramp up near the end of the splash");

    std::cout << "PASS\n";
}

} // namespace

int main() {
    std::cout << "=== Running Splash Screen Phase 42 Tests ===\n\n";

    test_SplashScreenAnimator_TimingWindow();
    test_SplashScreenAnimator_ProgressClamp();
    test_SplashScreenAnimator_AnimationSignalsStayBounded();

    std::cout << "\nAll Splash Screen Phase 42 tests passed.\n";
    return 0;
}
