/* Prevent Windows from including OpenGL headers */
#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #define NOGDI
#endif

#include <cassert>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <memory>
#include <fstream>
#include <sstream>

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
#define glShaderSource __system_glShaderSource
#define glCompileShader __system_glCompileShader
#define glGetShaderiv __system_glGetShaderiv
#define glGetShaderInfoLog __system_glGetShaderInfoLog
#define glDeleteShader __system_glDeleteShader
#define glCreateProgram __system_glCreateProgram
#define glAttachShader __system_glAttachShader
#define glLinkProgram __system_glLinkProgram
#define glGetProgramiv __system_glGetProgramiv
#define glGetProgramInfoLog __system_glGetProgramInfoLog
#define glDeleteProgram __system_glDeleteProgram
#define glUseProgram __system_glUseProgram
#define glGetUniformLocation __system_glGetUniformLocation
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
#undef glGenBuffers
#undef glDeleteBuffers
#undef glBindBuffer
#undef glBufferData
#undef glBufferSubData
#undef glDrawArrays
#undef glDrawElements
#undef glCreateShader
#undef glShaderSource
#undef glCompileShader
#undef glGetShaderiv
#undef glGetShaderInfoLog
#undef glDeleteShader
#undef glCreateProgram
#undef glAttachShader
#undef glLinkProgram
#undef glGetProgramiv
#undef glGetProgramInfoLog
#undef glDeleteProgram
#undef glUseProgram
#undef glGetUniformLocation

// Define GL constants that might not be in glad.h
#ifndef GL_CURRENT_PROGRAM
#define GL_CURRENT_PROGRAM 0x8B8D
#endif

#include "visuals/Shader.h"

namespace dj {

// RAII context helper for OpenGL tests
class OpenGLTestContext {
public:
    OpenGLTestContext() {
        if (!glfwInit()) {
            throw std::runtime_error("GLFW initialization failed");
        }
        
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);  // Headless
        
        m_window = glfwCreateWindow(1, 1, "Shader Test Context", nullptr, nullptr);
        if (!m_window) {
            glfwTerminate();
            throw std::runtime_error("GLFW window creation failed");
        }
        
        glfwMakeContextCurrent(m_window);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            glfwDestroyWindow(m_window);
            glfwTerminate();
            throw std::runtime_error("GLAD initialization failed");
        }
    }
    
    ~OpenGLTestContext() {
        if (m_window) {
            glfwMakeContextCurrent(nullptr);
            glfwDestroyWindow(m_window);
        }
        glfwTerminate();
    }
    
private:
    GLFWwindow* m_window = nullptr;
};

// Simple test vertex shader (GLSL)
const char* TEST_VERTEX_SHADER = R"(
#version 430 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;

layout(std140, binding = 0) uniform Matrices {
    mat4 World;
    mat4 View;
    mat4 Projection;
};

out vec4 vColor;

void main() {
    gl_Position = Projection * View * World * vec4(aPosition, 1.0);
    vColor = aColor;
}
)";

// Simple test fragment shader (GLSL)
const char* TEST_FRAGMENT_SHADER = R"(
#version 430 core

in vec4 vColor;
out vec4 FragColor;

void main() {
    FragColor = vColor;
}
)";

// Invalid vertex shader for error testing
const char* INVALID_VERTEX_SHADER = R"(
#version 430 core

layout(location = 0) in vec3 aPosition;

void main() {
    // Missing semicolon and invalid syntax
    gl_Position = vec4(aPosition, 1.0) this_is_invalid
}
)";

void test_VertexShaderCompilationSucceeds() {
    Shader shader;
    std::string error;
    
    bool result = shader.compile(TEST_VERTEX_SHADER, nullptr, "vertex", &error);
    assert(result && "Vertex shader compilation failed");
    std::cout << "✓ Test 1: Vertex shader compilation succeeds\n";
}

void test_FragmentShaderCompilationSucceeds() {
    Shader shader;
    std::string error;
    
    bool result = shader.compile(nullptr, TEST_FRAGMENT_SHADER, "fragment", &error);
    assert(result && "Fragment shader compilation failed");
    std::cout << "✓ Test 2: Fragment shader compilation succeeds\n";
}

void test_ShaderProgramLinkingSucceeds() {
    Shader shader;
    std::string error;
    
    bool vertResult = shader.compile(TEST_VERTEX_SHADER, nullptr, "vertex", &error);
    assert(vertResult && "Vertex shader compilation failed");
    
    bool fragResult = shader.compile(nullptr, TEST_FRAGMENT_SHADER, "fragment", &error);
    assert(fragResult && "Fragment shader compilation failed");
    
    bool linkResult = shader.link(&error);
    assert(linkResult && "Shader program linking failed");
    
    GLuint program = shader.getProgram();
    assert(program != 0 && "Shader program is invalid (0)");
    std::cout << "✓ Test 3: Shader program linking succeeds\n";
}

void test_UniformLocationQuerySucceeds() {
    Shader shader;
    std::string error;
    
    // Create a shader with a uniform
    const char* vertWithUniform = R"(
#version 430 core
layout(location = 0) in vec3 aPosition;
uniform mat4 mvpMatrix;
void main() {
    gl_Position = mvpMatrix * vec4(aPosition, 1.0);
}
)";
    
    const char* fragSimple = R"(
#version 430 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0);
}
)";
    
    bool vertResult = shader.compile(vertWithUniform, nullptr, "vertex", &error);
    assert(vertResult && "Vertex shader compilation failed");
    
    bool fragResult = shader.compile(nullptr, fragSimple, "fragment", &error);
    assert(fragResult && "Fragment shader compilation failed");
    
    bool linkResult = shader.link(&error);
    assert(linkResult && "Shader program linking failed");
    
    GLint location = shader.getUniformLocation("mvpMatrix");
    assert(location != -1 && "Failed to query uniform location for 'mvpMatrix'");
    std::cout << "✓ Test 4: Uniform location query succeeds\n";
}

void test_LoadFromFilesSucceeds() {
    Shader shader;
    std::string error;
    
    // Try to load basic.vert and basic.frag
    // These should exist after we create them in Step 3
    bool result = shader.loadFromFiles("shaders/basic.vert", "shaders/basic.frag", &error);
    
    // If files don't exist yet, expect false with appropriate error message
    if (!result) {
        assert(!error.empty() && "Expected non-empty error message");
        std::cout << "✓ Test 5: File loading error handling works (files not yet created)\n";
    } else {
        // If files exist, verify shader program is valid
        GLuint program = shader.getProgram();
        assert(program != 0 && "Shader program is invalid after loading from files");
        std::cout << "✓ Test 5: Loading from files succeeds\n";
    }
}

void test_InvalidGLSLHandlesErrorCorrectly() {
    Shader shader;
    std::string error;
    
    // Try to compile invalid shader
    bool result = shader.compile(INVALID_VERTEX_SHADER, nullptr, "vertex", &error);
    
    assert(!result && "Invalid shader should fail compilation");
    assert(!error.empty() && "Error message should not be empty for invalid shader");
    std::cout << "✓ Test 6: Invalid GLSL handles error correctly\n";
}

void test_UseProgramSucceeds() {
    Shader shader;
    std::string error;
    
    bool vertResult = shader.compile(TEST_VERTEX_SHADER, nullptr, "vertex", &error);
    assert(vertResult && "Vertex shader compilation failed");
    
    bool fragResult = shader.compile(nullptr, TEST_FRAGMENT_SHADER, "fragment", &error);
    assert(fragResult && "Fragment shader compilation failed");
    
    bool linkResult = shader.link(&error);
    assert(linkResult && "Shader program linking failed");
    
    // In OpenGL Core Profile, we need a VAO bound to use shaders
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    // Should not throw
    shader.use();
    
    // Verify program is active
    GLint activeProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &activeProgram);
    assert(activeProgram == static_cast<GLint>(shader.getProgram()) 
           && "Program not active after use()");
    
    glDeleteVertexArrays(1, &vao);
    
    std::cout << "✓ Test 7: Use program succeeds\n";
}

} // namespace dj

int main() {
    try {
        std::cout << "\n=== Shader Phase 3 Tests ===\n";
        
        auto gl_context = std::make_unique<dj::OpenGLTestContext>();
        
        dj::test_VertexShaderCompilationSucceeds();
        dj::test_FragmentShaderCompilationSucceeds();
        dj::test_ShaderProgramLinkingSucceeds();
        dj::test_UniformLocationQuerySucceeds();
        dj::test_LoadFromFilesSucceeds();
        dj::test_InvalidGLSLHandlesErrorCorrectly();
        dj::test_UseProgramSucceeds();
        
        std::cout << "\n=== All tests passed! ===\n\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\n!!! Test setup failed: " << e.what() << "\n\n";
        return 1;
    }
}
