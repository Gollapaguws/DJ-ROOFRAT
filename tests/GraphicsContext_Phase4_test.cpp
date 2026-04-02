#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

#define DJROOFRAT_OPENGL_MIGRATION 1

#include <glad/glad.h>

/* Prevent system GL functions from conflicting with GLAD function pointers */
#define glClear __system_glClear
#define glClearColor __system_glClearColor
#define glGetString __system_glGetString
#define glGetIntegerv __system_glGetIntegerv
#define glViewport __system_glViewport
#define glEnable __system_glEnable
#define glDisable __system_glDisable
#define glDrawArrays __system_glDrawArrays
#define glDrawElements __system_glDrawElements

#include <GLFW/glfw3.h>

/* Undefine the system GL renamings to restore our GLAD pointers */
#undef glClear
#undef glClearColor
#undef glGetString
#undef glGetIntegerv
#undef glViewport
#undef glEnable
#undef glDisable
#undef glDrawArrays
#undef glDrawElements

#include "visuals/GraphicsContext.h"

namespace dj {

// Test 1: Window creation succeeds
void test_WindowCreation() {
    std::cout << "Test 1: Window creation..." << std::endl;
    
    GraphicsContext ctx;
    std::string error;
    bool success = ctx.initialize(800, 600, &error);
    
    assert(success && "Window creation should succeed");
    assert(error.empty() && "No error message should be set on success");
    assert(ctx.isAvailable() && "Graphics context should be available");
    assert(ctx.getGLFWWindow() != nullptr && "GLFW window should be created");
    
    ctx.shutdown();
    std::cout << "  PASSED: Window created successfully" << std::endl;
}

// Test 2: OpenGL context initialization succeeds
void test_ContextInitialization() {
    std::cout << "Test 2: OpenGL context initialization..." << std::endl;
    
    GraphicsContext ctx;
    std::string error;
    bool success = ctx.initialize(800, 600, &error);
    
    assert(success && "Context initialization should succeed");
    
    // Verify OpenGL version is at least 4.3
    const GLubyte* version = glGetString(GL_VERSION);
    assert(version != nullptr && "OpenGL version string should be available");
    std::cout << "  OpenGL Version: " << version << std::endl;
    
    GLint major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    assert(major >= 4 && "OpenGL major version should be at least 4");
    assert(minor >= 3 && "OpenGL minor version should be at least 3");
    
    ctx.shutdown();
    std::cout << "  PASSED: OpenGL 4.3+ context initialized" << std::endl;
}

// Test 3: Window resize handling
void test_WindowResize() {
    std::cout << "Test 3: Window resize handling..." << std::endl;
    
    GraphicsContext ctx;
    std::string error;
    ctx.initialize(800, 600, &error);
    
    // Resize window
    ctx.resize(1024, 768);
    
    // Verify viewport was updated
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    assert(viewport[2] == 1024 && "Viewport width should be updated");
    assert(viewport[3] == 768 && "Viewport height should be updated");
    
    ctx.shutdown();
    std::cout << "  PASSED: Window resize updates viewport correctly" << std::endl;
}

// Test 4: Swap buffers operation
void test_SwapBuffers() {
    std::cout << "Test 4: Swap buffers operation..." << std::endl;
    
    GraphicsContext ctx;
    std::string error;
    ctx.initialize(800, 600, &error);
    
    // Clear to a color and swap buffers
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Should not crash
    ctx.swapBuffers();
    
    ctx.shutdown();
    std::cout << "  PASSED: Swap buffers completes without error" << std::endl;
}

// Test 5: Window close detection
void test_WindowClose() {
    std::cout << "Test 5: Window close detection..." << std::endl;
    
    GraphicsContext ctx;
    std::string error;
    ctx.initialize(800, 600, &error);
    
    // Initially, window should not be closed
    assert(!ctx.shouldClose() && "Window should not be closed initially");
    
    // Set window to close
    glfwSetWindowShouldClose(ctx.getGLFWWindow(), GLFW_TRUE);
    
    // Now it should report as closed
    assert(ctx.shouldClose() && "Window should report as closed after flag set");
    
    ctx.shutdown();
    std::cout << "  PASSED: Window close detection works" << std::endl;
}

// Test 6: Viewport updates
void test_ViewportUpdate() {
    std::cout << "Test 6: Viewport updates..." << std::endl;
    
    GraphicsContext ctx;
    std::string error;
    ctx.initialize(640, 480, &error);
    
    // Check initial viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    assert(viewport[2] == 640 && "Initial viewport width should match window");
    assert(viewport[3] == 480 && "Initial viewport height should match window");
    
    // Update viewport via resize
    ctx.resize(1280, 720);
    glGetIntegerv(GL_VIEWPORT, viewport);
    assert(viewport[2] == 1280 && "Updated viewport width");
    assert(viewport[3] == 720 && "Updated viewport height");
    
    ctx.shutdown();
    std::cout << "  PASSED: Viewport updates correctly" << std::endl;
}

// Test 7: Context activation (make current)
void test_ContextActivation() {
    std::cout << "Test 7: Context activation..." << std::endl;
    
    GraphicsContext ctx;
    std::string error;
    ctx.initialize(800, 600, &error);
    
    // Context should be current after initialization
    GLFWwindow* current = glfwGetCurrentContext();
    assert(current == ctx.getGLFWWindow() && "Context should be current");
    
    // Deactivate context
    glfwMakeContextCurrent(nullptr);
    current = glfwGetCurrentContext();
    assert(current == nullptr && "Context should be deactivated");
    
    // Reactivate via makeContextCurrent
    ctx.makeContextCurrent();
    current = glfwGetCurrentContext();
    assert(current == ctx.getGLFWWindow() && "Context should be reactivated");
    
    ctx.shutdown();
    std::cout << "  PASSED: Context activation works" << std::endl;
}

// Test 8: Error handling - invalid window size
void test_ErrorHandling_InvalidSize() {
    std::cout << "Test 8: Error handling - invalid window size..." << std::endl;
    
    GraphicsContext ctx;
    std::string error;
    
    // Try to create window with invalid size
    bool success = ctx.initialize(0, 0, &error);
    
    assert(!success && "Initialization should fail with invalid size");
    assert(!error.empty() && "Error message should be set");
    assert(!ctx.isAvailable() && "Context should not be available after failure");
    
    std::cout << "  Error message: " << error << std::endl;
    std::cout << "  PASSED: Invalid size detected and handled" << std::endl;
}

} // namespace dj

int main() {
    std::cout << "=== GraphicsContext Phase 4 Tests ===" << std::endl;
    std::cout << "Testing GLFW window and OpenGL context creation" << std::endl;
    std::cout << std::endl;
    
    try {
        dj::test_WindowCreation();
        dj::test_ContextInitialization();
        dj::test_WindowResize();
        dj::test_SwapBuffers();
        dj::test_WindowClose();
        dj::test_ViewportUpdate();
        dj::test_ContextActivation();
        dj::test_ErrorHandling_InvalidSize();
        
        std::cout << std::endl;
        std::cout << "=== ALL TESTS PASSED ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "TEST FAILED: " << e.what() << std::endl;
        return 1;
    }
}
