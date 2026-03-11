#include <cassert>
#include <cmath>
#include <iostream>
#include <numbers>
#include <string>

#include "visuals/GraphicsContext.h"
#include "visuals/LaserController.h"
#include "visuals/CrowdRenderer.h"

namespace {

// Test: GraphicsContext can be initialized (or gracefully fails without graphics)
void test_GraphicsContext_Initialize() {
    // Arrange
    dj::GraphicsContext graphics;

    // Act
    bool initialized = graphics.initialize(1280, 720);

    // Assert: Should either initialize or fail gracefully
    bool available = graphics.isAvailable();
    assert((initialized && available) || (!initialized && !available) && 
           "GraphicsContext should either initialize successfully or report unavailable");

    // Cleanup
    if (available) {
        graphics.shutdown();
    }

    std::cout << "✓ test_GraphicsContext_Initialize passed\n";
}

// Test: GraphicsContext falls back gracefully when not available
void test_GraphicsContext_Fallback() {
    // Arrange
    dj::GraphicsContext graphics;

    // Act
    bool initialized = graphics.initialize(800, 600);
    (void)initialized;
    
    // Assert: If not available, renderFrame should return false gracefully
    if (!graphics.isAvailable()) {
        bool render_result = graphics.renderFrame(120.0f, 0.5f, 0, -1.0f);
        assert(!render_result && "renderFrame should return false when graphics unavailable");
    }

    std::cout << "✓ test_GraphicsContext_Fallback passed\n";
}

// Test: GraphicsContext can render frames with valid parameters
void test_GraphicsContext_RenderFrame() {
    // Arrange
    dj::GraphicsContext graphics;
    bool initialized = graphics.initialize(1024, 768);
    (void)initialized;

    if (graphics.isAvailable()) {
        // Act
        bool result = graphics.renderFrame(124.0f, 0.75f, 1, 0.5f);

        // Assert
        assert(result && "renderFrame should return true when graphics available");

        // Cleanup
        graphics.shutdown();
    }

    std::cout << "✓ test_GraphicsContext_RenderFrame passed\n";
}

// Test: GraphicsContext composite rendering with laser and crowd
void test_GraphicsContext_CompositeRenderWithLaser() {
    // Arrange
    dj::GraphicsContext graphics;
    bool initialized = graphics.initialize(1280, 720);
    (void)initialized;

    if (graphics.isAvailable()) {
        // Act: Render multiple frames with different crossfader positions (laser varies)
        bool result1 = graphics.renderFrame(124.0f, 0.5f, 2, -1.0f);   // Left crossfader
        bool result2 = graphics.renderFrame(124.0f, 0.5f, 2, 0.0f);    // Center
        bool result3 = graphics.renderFrame(124.0f, 0.5f, 2, 1.0f);    // Right

        // Assert
        assert(result1 && "Composite render should succeed with left crossfader");
        assert(result2 && "Composite render should succeed with center crossfader");
        assert(result3 && "Composite render should succeed with right crossfader");

        // Cleanup
        graphics.shutdown();
    }

    std::cout << "✓ test_GraphicsContext_CompositeRenderWithLaser passed\n";
}

// Test: GraphicsContext composite rendering with crowd based on mood
void test_GraphicsContext_CompositeRenderWithCrowd() {
    // Arrange
    dj::GraphicsContext graphics;
    bool initialized = graphics.initialize(1280, 720);
    (void)initialized;

    if (graphics.isAvailable()) {
        // Act: Render frames with different moods and energy levels
        bool result1 = graphics.renderFrame(124.0f, 0.2f, 0, 0.0f);    // Unimpressed, low energy
        bool result2 = graphics.renderFrame(124.0f, 0.5f, 2, 0.0f);    // Grooving, medium energy
        bool result3 = graphics.renderFrame(124.0f, 0.9f, 3, 0.0f);    // Hyped, high energy

        // Assert
        assert(result1 && "Composite render should succeed with low mood/energy");
        assert(result2 && "Composite render should succeed with medium mood/energy");
        assert(result3 && "Composite render should succeed with high mood/energy");

        // Cleanup
        graphics.shutdown();
    }

    std::cout << "✓ test_GraphicsContext_CompositeRenderWithCrowd passed\n";
}

// Test: GraphicsContext handles BPM variation (laser oscillation changes)
void test_GraphicsContext_BPMOscillation() {
    // Arrange
    dj::GraphicsContext graphics;
    bool initialized = graphics.initialize(1280, 720);
    (void)initialized;

    if (graphics.isAvailable()) {
        // Act: Render frames with different BPMs (laser intensity/oscillation varies)
        bool result1 = graphics.renderFrame(60.0f, 0.5f, 2, 0.0f);     // Slow BPM
        bool result2 = graphics.renderFrame(124.0f, 0.5f, 2, 0.0f);    // Medium BPM
        bool result3 = graphics.renderFrame(200.0f, 0.5f, 2, 0.0f);    // Fast BPM

        // Assert
        assert(result1 && "Composite render should succeed at slow BPM");
        assert(result2 && "Composite render should succeed at medium BPM");
        assert(result3 && "Composite render should succeed at fast BPM");

        // Cleanup
        graphics.shutdown();
    }

    std::cout << "✓ test_GraphicsContext_BPMOscillation passed\n";
}

// Test: GraphicsContext clamps extreme inputs safely
void test_GraphicsContext_SafeClamp() {
    // Arrange
    dj::GraphicsContext graphics;
    bool initialized = graphics.initialize(1280, 720);
    (void)initialized;

    if (graphics.isAvailable()) {
        // Act: Render with extreme values (should be clamped internally)
        bool result1 = graphics.renderFrame(999.0f, -0.5f, -1, -5.0f);  // Extreme inputs
        bool result2 = graphics.renderFrame(0.0f, 1.5f, 100, 2.0f);     // More extreme inputs

        // Assert
        assert(result1 && "Composite render should clamp extreme values gracefully");
        assert(result2 && "Composite render should clamp extreme values gracefully");

        // Cleanup
        graphics.shutdown();
    }

    std::cout << "✓ test_GraphicsContext_SafeClamp passed\n";
}

// Test: GraphicsContext multi-frame rendering maintains state consistency
void test_GraphicsContext_MultiFrameConsistency() {
    // Arrange
    dj::GraphicsContext graphics;
    bool initialized = graphics.initialize(1280, 720);
    (void)initialized;

    if (graphics.isAvailable()) {
        // Act: Render 10 consecutive frames to check state consistency
        bool allSucceeded = true;
        for (int i = 0; i < 10; ++i) {
            const float phase = 2.0f * std::numbers::pi_v<float> * static_cast<float>(i) / 10.0f;
            const float crossfader = std::sin(phase);
            const float energy = 0.3f + 0.7f * std::abs(std::cos(phase));
            const int mood = (i % 4);
            
            bool result = graphics.renderFrame(124.0f, energy, mood, crossfader);
            allSucceeded = allSucceeded && result;
        }

        // Assert
        assert(allSucceeded && "All consecutive composite renders should succeed");

        // Cleanup
        graphics.shutdown();
    }

    std::cout << "✓ test_GraphicsContext_MultiFrameConsistency passed\n";
}

} // namespace

int main() {
    try {
        test_GraphicsContext_Initialize();
        test_GraphicsContext_Fallback();
        test_GraphicsContext_RenderFrame();
        test_GraphicsContext_CompositeRenderWithLaser();
        test_GraphicsContext_CompositeRenderWithCrowd();
        test_GraphicsContext_BPMOscillation();
        test_GraphicsContext_SafeClamp();
        test_GraphicsContext_MultiFrameConsistency();

        std::cout << "\n✓ All GraphicsContext Phase 8 tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << "\n";
        return 1;
    }
}
