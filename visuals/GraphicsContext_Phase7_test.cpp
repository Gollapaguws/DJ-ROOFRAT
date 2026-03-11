#include <cassert>
#include <iostream>
#include <string>

#include "visuals/GraphicsContext.h"

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

} // namespace

int main() {
    try {
        test_GraphicsContext_Initialize();
        test_GraphicsContext_Fallback();
        test_GraphicsContext_RenderFrame();

        std::cout << "\n✓ All GraphicsContext tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << "\n";
        return 1;
    }
}
