#include <cassert>
#include <iostream>
#include <stdexcept>

#include <glad/glad.h>

/* Prevent system GL functions from conflicting with GLAD function pointers */
#define glClear __system_glClear
#define glClearColor __system_glClearColor
#define glGetString __system_glGetString
#define glGetIntegerv __system_glGetIntegerv
#define glViewport __system_glViewport
#define glEnable __system_glEnable
#define glDisable __system_glDisable
#define glCreateShader __system_glCreateShader
#define glDeleteShader __system_glDeleteShader
#define glShaderSource __system_glShaderSource
#define glCompileShader __system_glCompileShader
#define glGetShaderiv __system_glGetShaderiv
#define glGetShaderInfoLog __system_glGetShaderInfoLog
#define glCreateProgram __system_glCreateProgram
#define glDeleteProgram __system_glDeleteProgram
#define glAttachShader __system_glAttachShader
#define glLinkProgram __system_glLinkProgram
#define glGetProgramiv __system_glGetProgramiv
#define glGetProgramInfoLog __system_glGetProgramInfoLog
#define glUseProgram __system_glUseProgram
#define glGenVertexArrays __system_glGenVertexArrays
#define glDeleteVertexArrays __system_glDeleteVertexArrays
#define glBindVertexArray __system_glBindVertexArray
#define glEnableVertexAttribArray __system_glEnableVertexAttribArray
#define glVertexAttribPointer __system_glVertexAttribPointer
#define glGenBuffers __system_glGenBuffers
#define glDeleteBuffers __system_glDeleteBuffers
#define glBindBuffer __system_glBindBuffer
#define glBufferData __system_glBufferData
#define glBufferSubData __system_glBufferSubData
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
#undef glCreateShader
#undef glDeleteShader
#undef glShaderSource
#undef glCompileShader
#undef glGetShaderiv
#undef glGetShaderInfoLog
#undef glCreateProgram
#undef glDeleteProgram
#undef glAttachShader
#undef glLinkProgram
#undef glGetProgramiv
#undef glGetProgramInfoLog
#undef glUseProgram
#undef glGenVertexArrays
#undef glDeleteVertexArrays
#undef glBindVertexArray
#undef glEnableVertexAttribArray
#undef glVertexAttribPointer
#undef glGenBuffers
#undef glDeleteBuffers
#undef glBindBuffer
#undef glBufferData
#undef glBufferSubData
#undef glDrawArrays
#undef glDrawElements

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
    if (!window) {
        throw std::runtime_error("Failed to create GLFW window");
    }
    
    // Make context current
    glfwMakeContextCurrent(window);
    
    // Verify context is active
    GLFWwindow* current = glfwGetCurrentContext();
    if (current != window) {
        throw std::runtime_error("Failed to set OpenGL context as current");
    }
    
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
    if (!window) {
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
    
    // Load GLAD - this should validate that OpenGL 4.3 functions are available via the loader callback
    int version = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    if (version == 0) {
        throw std::runtime_error("Failed to load OpenGL functions via GLAD");
    }
    
    // ACTUALLY verify modern OpenGL functions are loaded
    if (glCreateShader == nullptr || glGenVertexArrays == nullptr || glCreateProgram == nullptr) {
        throw std::runtime_error("Modern OpenGL functions not loaded by GLAD");
    }
    std::cout << "  Modern OpenGL functions loaded successfully" << std::endl;
    
    // Verify OpenGL version
    const GLubyte* glVersion = glGetString(GL_VERSION);
    if (glVersion == nullptr) {
        throw std::runtime_error("Failed to get OpenGL version string");
    }
    std::cout << "  OpenGL Version: " << reinterpret_cast<const char*>(glVersion) << std::endl;
    
    // Check that we have at least OpenGL 4.3
    int majorVersion, minorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
    if (!(majorVersion > 4 || (majorVersion == 4 && minorVersion >= 3))) {
        throw std::runtime_error("OpenGL version is less than 4.3");
    }
    
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
    if (det == 0.0f) {
        throw std::runtime_error("Matrix composition resulted in singular matrix");
    }
    
    // Verify translation component is correct
    glm::vec3 translationPos = glm::vec3(translation[3]);
    if (glm::distance(translationPos, glm::vec3(1.0f, 2.0f, 3.0f)) >= 0.001f) {
        throw std::runtime_error("Translation matrix component verification failed");
    }
    
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
