#include <cassert>
#include <iostream>
#include <stdexcept>

// Forward declarations - these will fail to compile initially
// Once we add GLFW, these will resolve
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace dj {

// Test 1: GLFW window creation
void test_GLContext_WindowCreation() {
    std::cout << "Testing GLFW window creation..." << std::endl;
    
    // Initialize GLFW
    if (!glfwInit()) {
        throw std::runtime_error("GLFW initialization failed");
    }
    
    // Request OpenGL 4.3 Core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);  // Don't show window in test
    
    // Create a hidden window for testing
    GLFWwindow* window = glfwCreateWindow(800, 600, "GL Context Test", nullptr, nullptr);
    assert(window != nullptr);
    
    // Make context current
    glfwMakeContextCurrent(window);
    
    // Verify context is active
    GLFWwindow* current = glfwGetCurrentContext();
    assert(current == window);
    
    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    
    std::cout << "✓ test_GLContext_WindowCreation passed" << std::endl;
}

// Test 2: GLAD OpenGL loader
void test_GLContext_GLADLoader() {
    std::cout << "Testing GLAD loader..." << std::endl;
    
    // Initialize GLFW and create context
    if (!glfwInit()) {
        throw std::runtime_error("GLFW initialization failed");
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "GL Context Test", nullptr, nullptr);
    assert(window != nullptr);
    
    glfwMakeContextCurrent(window);
    
    // Load GLAD - this should load all OpenGL 4.3 function pointers
    int version = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    assert(version != 0);  // gladLoadGLLoader returns version info, non-zero on success
    
    // Verify OpenGL version
    const GLubyte* glVersion = glGetString(GL_VERSION);
    assert(glVersion != nullptr);
    
    std::cout << "  OpenGL Version: " << reinterpret_cast<const char*>(glVersion) << std::endl;
    
    // Check that we have at least OpenGL 4.3
    int majorVersion, minorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
    assert(majorVersion > 4 || (majorVersion == 4 && minorVersion >= 3));
    
    std::cout << "  OpenGL " << majorVersion << "." << minorVersion << " loaded successfully" << std::endl;
    
    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    
    std::cout << "✓ test_GLContext_GLADLoader passed" << std::endl;
}

// Test 3: GLM math library - basic matrix operations
void test_GLContext_GLMMatrix() {
    std::cout << "Testing GLM matrix operations..." << std::endl;
    
    // Create identity matrix
    glm::mat4 identity = glm::mat4(1.0f);
    
    // Create translation matrix
    glm::mat4 translation = glm::translate(identity, glm::vec3(1.0f, 2.0f, 3.0f));
    
    // Create rotation matrix (45 degrees around Z axis)
    glm::mat4 rotation = glm::rotate(identity, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Create scale matrix
    glm::mat4 scale = glm::scale(identity, glm::vec3(2.0f, 2.0f, 2.0f));
    
    // Compose transformations
    glm::mat4 composed = translation * rotation * scale;
    
    // Verify composition produces a valid matrix (simple check: determinant is non-zero)
    float det = glm::determinant(composed);
    assert(det != 0.0f);  // Non-zero determinant means invertible
    
    // Verify translation component is correct
    glm::vec3 translationPos = glm::vec3(translation[3]);
    assert(glm::distance(translationPos, glm::vec3(1.0f, 2.0f, 3.0f)) < 0.001f);
    
    std::cout << "✓ test_GLContext_GLMMatrix passed" << std::endl;
}

void runAllGLContextPhase1Tests() {
    std::cout << "\n=== Running GLContext Phase 1 Tests ===" << std::endl;
    try {
        test_GLContext_WindowCreation();
        test_GLContext_GLADLoader();
        test_GLContext_GLMMatrix();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        throw;
    }
}

}  // namespace dj

// Entry point for running tests
int main() {
    dj::runAllGLContextPhase1Tests();
    std::cout << "\n=== GLContext Phase 1 Tests Complete ===" << std::endl;
    return 0;
}
