#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

// GLAD must be included before GLFW to prevent GL header conflicts
#include <glad/glad.h>

// Define GLFW_INCLUDE_NONE to prevent GLFW from including GL headers
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// ImGui headers
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

namespace {

// RAII wrapper for GLFW window
class GLFWWindowHelper {
public:
    GLFWWindowHelper(int width = 800, int height = 600) : window_(nullptr) {
        if (!glfwInit()) {
            std::cerr << "GLFW initialization failed\n";
            return;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);  // Headless for testing

        window_ = glfwCreateWindow(width, height, "ImGui OpenGL3 Test", nullptr, nullptr);
        if (!window_) {
            std::cerr << "GLFW window creation failed\n";
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(window_);
        glfwSwapInterval(0);  // Disable vsync for tests

        // Load OpenGL functions with GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "GLAD initialization failed\n";
            glfwDestroyWindow(window_);
            glfwTerminate();
            window_ = nullptr;
            return;
        }
    }

    ~GLFWWindowHelper() {
        if (window_) {
            glfwDestroyWindow(window_);
            glfwTerminate();
        }
    }

    GLFWwindow* get() const { return window_; }
    bool isValid() const { return window_ != nullptr; }

private:
    GLFWwindow* window_;
};

// Helper to check for OpenGL errors
bool checkGLError(const char* context = "GL") {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL Error in " << context << ": " << std::hex << err << std::dec << "\n";
        return false;
    }
    return true;
}

// Test 1: ImGui OpenGL3 backend initializes
bool testImGuiOpenGL3BackendInitialization() {
    std::cout << "Test 1: ImGui OpenGL3 backend initializes...\n";

    GLFWWindowHelper window;
    if (!window.isValid()) {
        std::cerr << "  FAILED: Window creation failed\n";
        return false;
    }

    // Create ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Initialize ImGui OpenGL3 backend
    const char* glsl_version = "#version 430";
    if (!ImGui_ImplGLFW_InitForOpenGL(window.get(), true)) {
        std::cerr << "  FAILED: ImGui GLFW initialization failed\n";
        ImGui::DestroyContext();
        return false;
    }

    if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
        std::cerr << "  FAILED: ImGui OpenGL3 initialization failed\n";
        ImGui_ImplGLFW_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    // Verify ImGui::GetIO() is valid
    if (!ImGui::GetCurrentContext()) {
        std::cerr << "  FAILED: ImGui context not valid\n";
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGLFW_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    // Check for OpenGL errors
    if (!checkGLError("ImGui Init")) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGLFW_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLFW_Shutdown();
    ImGui::DestroyContext();

    std::cout << "  PASSED\n";
    return true;
}

// Test 2: ImGui renders simple window
bool testImGuiRenderSimpleWindow() {
    std::cout << "Test 2: ImGui renders simple window...\n";

    GLFWWindowHelper window;
    if (!window.isValid()) {
        std::cerr << "  FAILED: Window creation failed\n";
        return false;
    }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    const char* glsl_version = "#version 430";
    if (!ImGui_ImplGLFW_InitForOpenGL(window.get(), true) ||
        !ImGui_ImplOpenGL3_Init(glsl_version)) {
        std::cerr << "  FAILED: ImGui backend initialization failed\n";
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGLFW_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    ImGui::StyleColorsDark();

    // Set display size
    io.DisplaySize = ImVec2(800.0f, 600.0f);

    // Run frame cycle
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLFW_NewFrame();
    ImGui::NewFrame();

    // Create simple window
    ImGui::Begin("Test Window");
    ImGui::Text("Hello from ImGui OpenGL3!");
    ImGui::SliderFloat("Test Slider", &io.MouseDelta.x, 0.0f, 1.0f);
    ImGui::Button("Test Button");
    ImGui::End();

    ImGui::Render();

    // Verify draw data exists
    ImDrawData* drawData = ImGui::GetDrawData();
    if (!drawData) {
        std::cerr << "  FAILED: No draw data generated\n";
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGLFW_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    if (drawData->TotalVtxCount < 0 || drawData->TotalIdxCount < 0) {
        std::cerr << "  FAILED: Invalid draw data\n";
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGLFW_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    // Check for OpenGL errors
    if (!checkGLError("ImGui Render")) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGLFW_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLFW_Shutdown();
    ImGui::DestroyContext();

    std::cout << "  PASSED\n";
    return true;
}

// Test 3: ImGui handles input events
bool testImGuiHandlesInputEvents() {
    std::cout << "Test 3: ImGui handles input events...\n";

    GLFWWindowHelper window;
    if (!window.isValid()) {
        std::cerr << "  FAILED: Window creation failed\n";
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    const char* glsl_version = "#version 430";
    if (!ImGui_ImplGLFW_InitForOpenGL(window.get(), true) ||
        !ImGui_ImplOpenGL3_Init(glsl_version)) {
        std::cerr << "  FAILED: ImGui backend initialization failed\n";
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGLFW_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    // Simulate input
    io.MousePos = ImVec2(100.0f, 100.0f);
    io.MouseDown[0] = true;
    io.KeysDown[ImGuiKey_A] = true;
    io.DisplaySize = ImVec2(800.0f, 600.0f);

    // Process frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLFW_NewFrame();
    ImGui::NewFrame();

    // Verify input is reflected
    bool mousePositionValid = io.MousePos.x == 100.0f && io.MousePos.y == 100.0f;
    if (!mousePositionValid) {
        std::cerr << "  FAILED: Mouse position not reflected in IO\n";
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGLFW_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    ImGui::Render();

    // Check for OpenGL errors
    if (!checkGLError("ImGui Input")) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGLFW_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLFW_Shutdown();
    ImGui::DestroyContext();

    std::cout << "  PASSED\n";
    return true;
}

// Test 4: ImGui cleanup works
bool testImGuiCleanup() {
    std::cout << "Test 4: ImGui cleanup works...\n";

    GLFWWindowHelper window;
    if (!window.isValid()) {
        std::cerr << "  FAILED: Window creation failed\n";
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    const char* glsl_version = "#version 430";
    if (!ImGui_ImplGLFW_InitForOpenGL(window.get(), true) ||
        !ImGui_ImplOpenGL3_Init(glsl_version)) {
        std::cerr << "  FAILED: ImGui backend initialization failed\n";
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGLFW_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    ImGui::StyleColorsDark();

    // Add some UI elements
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(800.0f, 600.0f);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLFW_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Cleanup Test");
    ImGui::Text("Testing cleanup");
    ImGui::Button("Button 1");
    ImGui::Button("Button 2");
    ImGui::End();

    ImGui::Render();

    // Perform cleanup
    if (!ImGui::GetCurrentContext()) {
        std::cerr << "  FAILED: Context already destroyed\n";
        return false;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLFW_Shutdown();
    ImGui::DestroyContext();

    // Check no GL errors occurred during cleanup
    if (!checkGLError("ImGui Cleanup")) {
        return false;
    }

    std::cout << "  PASSED\n";
    return true;
}

// Test 5: Multiple frames render
bool testMultipleFramesRender() {
    std::cout << "Test 5: Multiple frames render...\n";

    GLFWWindowHelper window;
    if (!window.isValid()) {
        std::cerr << "  FAILED: Window creation failed\n";
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    const char* glsl_version = "#version 430";
    if (!ImGui_ImplGLFW_InitForOpenGL(window.get(), true) ||
        !ImGui_ImplOpenGL3_Init(glsl_version)) {
        std::cerr << "  FAILED: ImGui backend initialization failed\n";
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGLFW_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    ImGui::StyleColorsDark();
    io.DisplaySize = ImVec2(800.0f, 600.0f);

    // Render 10 frames
    for (int frame = 0; frame < 10; ++frame) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGLFW_NewFrame();
        ImGui::NewFrame();

        // Create different UI each frame
        ImGui::Begin("Frame Counter");
        ImGui::Text("Frame: %d", frame);
        ImGui::Text("FPS: %.1f", io.Framerate);

        if (frame % 2 == 0) {
            ImGui::Button("Even Frame");
        } else {
            ImGui::Button("Odd Frame");
        }

        for (int i = 0; i < frame; ++i) {
            ImGui::Text("  Item %d", i);
        }

        ImGui::End();

        ImGui::Render();

        ImDrawData* drawData = ImGui::GetDrawData();
        if (!drawData) {
            std::cerr << "  FAILED: No draw data in frame " << frame << "\n";
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGLFW_Shutdown();
            ImGui::DestroyContext();
            return false;
        }
    }

    // Check for OpenGL errors after all frames
    if (!checkGLError("ImGui Multi-Frame")) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGLFW_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLFW_Shutdown();
    ImGui::DestroyContext();

    std::cout << "  PASSED\n";
    return true;
}

}  // namespace

int main() {
    std::cout << "=== ImGui Phase 7A Test Suite ===\n";
    std::cout << "Testing ImGui OpenGL3 Backend Integration\n\n";

    bool allPassed = true;
    int passCount = 0;
    int totalTests = 5;

    // Run all tests
    if (testImGuiOpenGL3BackendInitialization()) {
        passCount++;
    } else {
        allPassed = false;
    }

    if (testImGuiRenderSimpleWindow()) {
        passCount++;
    } else {
        allPassed = false;
    }

    if (testImGuiHandlesInputEvents()) {
        passCount++;
    } else {
        allPassed = false;
    }

    if (testImGuiCleanup()) {
        passCount++;
    } else {
        allPassed = false;
    }

    if (testMultipleFramesRender()) {
        passCount++;
    } else {
        allPassed = false;
    }

    std::cout << "\n=== Test Results ===\n";
    std::cout << "Passed: " << passCount << " / " << totalTests << "\n";

    if (allPassed) {
        std::cout << "All tests passed!\n";
        return 0;
    } else {
        std::cout << "Some tests failed.\n";
        return 1;
    }
}
