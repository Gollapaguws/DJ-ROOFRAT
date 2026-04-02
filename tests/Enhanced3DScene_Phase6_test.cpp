#include <cassert>
#include <iostream>
#include <vector>
#include <cstring>
#include <cmath>

#define DJROOFRAT_OPENGL_MIGRATION 1
#define DJROOFRAT_ENABLE_GRAPHICS 1

// Prevent Windows from including its own OpenGL headers
#define __gl_h_
#define __GL_H__

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
#define glGenVertexArrays __system_glGenVertexArrays
#define glBindVertexArray __system_glBindVertexArray
#define glDeleteVertexArrays __system_glDeleteVertexArrays
#define glVertexAttribPointer __system_glVertexAttribPointer
#define glEnableVertexAttribArray __system_glEnableVertexAttribArray

#include <GLFW/glfw3.h>

/* Undefine the system GL renamings */
#undef glClear
#undef glClearColor
#undef glGetString
#undef glGetIntegerv
#undef glViewport
#undef glEnable
#undef glDisable
#undef glDrawArrays
#undef glDrawElements
#undef glGenVertexArrays
#undef glBindVertexArray
#undef glDeleteVertexArrays
#undef glVertexAttribPointer
#undef glEnableVertexAttribArray

#include "visuals/Enhanced3DScene.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
        
        m_window = glfwCreateWindow(800, 600, "Enhanced3DScene Test Context", nullptr, nullptr);
        if (!m_window) {
            glfwTerminate();
            throw std::runtime_error("GLFW window creation failed");
        }
        
        glfwMakeContextCurrent(m_window);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            glfwDestroyWindow(m_window);
            glfwTerminate();
            throw std::runtime_error("GLAD loader failed");
        }
    }
    
    ~OpenGLTestContext() {
        if (m_window) {
            glfwDestroyWindow(m_window);
        }
        glfwTerminate();
    }
    
    GLFWwindow* getWindow() const { return m_window; }
    
private:
    GLFWwindow* m_window = nullptr;
};

// Test 1: VAO creation and binding
void test_VAOCreationAndBinding() {
    std::cout << "Test 1: VAO creation and binding..." << std::endl;
    
    OpenGLTestContext ctx;
    
    // Create VAO
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    assert(vao != 0 && "VAO should be created");
    
    // Bind VAO
    glBindVertexArray(vao);
    
    // Verify VAO is bound
    GLint boundVAO = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &boundVAO);
    assert(boundVAO == static_cast<GLint>(vao) && "VAO should be bound");
    
    // Cleanup
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
    
    std::cout << "  PASSED: VAO creation and binding works" << std::endl;
}

// Test 2: VBO creation and attribute setup
void test_VBOAndAttributeSetup() {
    std::cout << "Test 2: VBO creation and attribute setup..." << std::endl;
    
    OpenGLTestContext ctx;
    
    // Vertex data (position, normal, texCoord)
    struct Vertex {
        float position[3];
        float normal[3];
        float texCoord[2];
    };
    
    Vertex vertices[] = {
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{ 0.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}}
    };
    
    // Create VAO and VBO
    GLuint vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Setup vertex attributes
    GLsizei stride = sizeof(Vertex);
    
    // Position (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)12);
    glEnableVertexAttribArray(1);
    
    // TexCoord (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)24);
    glEnableVertexAttribArray(2);
    
    // Verify attributes are enabled
    GLint enabled0 = 0, enabled1 = 0, enabled2 = 0;
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled0);
    glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled1);
    glGetVertexAttribiv(2, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled2);
    
    assert(enabled0 && "Position attribute should be enabled");
    assert(enabled1 && "Normal attribute should be enabled");
    assert(enabled2 && "TexCoord attribute should be enabled");
    
    // Cleanup
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    
    std::cout << "  PASSED: VBO and attributes configured correctly" << std::endl;
}

// Test 3: Shader binding and uniform setting
void test_ShaderBindingAndUniforms() {
    std::cout << "Test 3: Shader binding and uniform setting..." << std::endl;
    
    OpenGLTestContext ctx;
    
    const char* vertexSource = R"(
        #version 430 core
        layout(location = 0) in vec3 aPosition;
        
        uniform mat4 uModel;
        uniform mat4 uView;
        uniform mat4 uProjection;
        
        void main() {
            gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
        }
    )";
    
    const char* fragmentSource = R"(
        #version 430 core
        out vec4 FragColor;
        
        void main() {
            FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    )";
    
    // Compile shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);
    
    // Link program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    // Use program
    glUseProgram(program);
    
    // Verify program is active
    GLint currentProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    assert(currentProgram == static_cast<GLint>(program) && "Shader program should be active");
    
    // Get uniform locations
    GLint locModel = glGetUniformLocation(program, "uModel");
    GLint locView = glGetUniformLocation(program, "uView");
    GLint locProj = glGetUniformLocation(program, "uProjection");
    
    assert(locModel >= 0 && "uModel uniform should exist");
    assert(locView >= 0 && "uView uniform should exist");
    assert(locProj >= 0 && "uProjection uniform should exist");
    
    // Set uniforms
    glm::mat4 identity = glm::mat4(1.0f);
    glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(identity));
    glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(identity));
    glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(identity));
    
    // Cleanup
    glUseProgram(0);
    glDeleteProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    std::cout << "  PASSED: Shader binding and uniform setting works" << std::endl;
}

// Test 4: Draw call execution
void test_DrawCallExecution() {
    std::cout << "Test 4: Draw call execution..." << std::endl;
    
    OpenGLTestContext ctx;
    
    // Setup minimal rendering pipeline
    const char* vertexSource = R"(
        #version 430 core
        layout(location = 0) in vec3 aPosition;
        void main() { gl_Position = vec4(aPosition, 1.0); }
    )";
    
    const char* fragmentSource = R"(
        #version 430 core
        out vec4 FragColor;
        void main() { FragColor = vec4(1.0, 0.0, 0.0, 1.0); }
    )";
    
    // Create and compile shader
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexSource, nullptr);
    glCompileShader(vs);
    
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentSource, nullptr);
    glCompileShader(fs);
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    
    // Create geometry
    float vertices[] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         0.0f,  1.0f, 0.0f
    };
    
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    
    // Clear and draw
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(program);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    // Verify no errors
    GLenum error = glGetError();
    assert(error == GL_NO_ERROR && "Draw call should not produce errors");
    
    // Cleanup
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    std::cout << "  PASSED: Draw call executes without errors" << std::endl;
}

// Test 5: Model/View/Projection matrix setup
void test_MVPMatrixSetup() {
    std::cout << "Test 5: Model/View/Projection matrix setup..." << std::endl;
    
    OpenGLTestContext ctx;
    
    // Create matrices using GLM
    glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    
    // Verify matrices are not identity
    assert(model != glm::mat4(1.0f) && "Model matrix should be transformed");
    assert(view != glm::mat4(1.0f) && "View matrix should be transformed");
    assert(projection != glm::mat4(1.0f) && "Projection matrix should be transformed");
    
    // Verify matrix multiplication works
    glm::mat4 mvp = projection * view * model;
    
    // Check that MVP is valid (not NaN)
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            assert(!std::isnan(mvp[i][j]) && "MVP matrix should contain valid values");
        }
    }
    
    std::cout << "  PASSED: MVP matrix setup works correctly" << std::endl;
}

// Test 6: Multiple objects rendering (multiple VAOs)
void test_MultipleVAOs() {
    std::cout << "Test 6: Multiple VAO rendering..." << std::endl;
    
    OpenGLTestContext ctx;
    
    // Create two VAOs with different geometry
    GLuint vao1, vao2, vbo1, vbo2;
    
    float vertices1[] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         0.0f,  1.0f, 0.0f
    };
    
    float vertices2[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };
    
    // VAO 1
    glGenVertexArrays(1, &vao1);
    glGenBuffers(1, &vbo1);
    glBindVertexArray(vao1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    
    // VAO 2
    glGenVertexArrays(1, &vao2);
    glGenBuffers(1, &vbo2);
    glBindVertexArray(vao2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    
    // Verify both VAOs exist
    assert(vao1 != 0 && vao2 != 0 && "Both VAOs should be created");
    assert(vao1 != vao2 && "VAOs should have different IDs");
    
    // Verify each VAO can be bound independently
    glBindVertexArray(vao1);
    GLint bound1;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &bound1);
    assert(bound1 == static_cast<GLint>(vao1) && "VAO1 should be bound");
    
    glBindVertexArray(vao2);
    GLint bound2;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &bound2);
    assert(bound2 == static_cast<GLint>(vao2) && "VAO2 should be bound");
    
    // Cleanup
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao1);
    glDeleteVertexArrays(1, &vao2);
    glDeleteBuffers(1, &vbo1);
    glDeleteBuffers(1, &vbo2);
    
    std::cout << "  PASSED: Multiple VAOs can be created and bound" << std::endl;
}

// Test 7: Texture binding before draw
void test_TextureBinding() {
    std::cout << "Test 7: Texture binding before draw..." << std::endl;
    
    OpenGLTestContext ctx;
    
    // Create a simple texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Set simple texture data
    unsigned char pixels[] = {255, 0, 0, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Bind texture to unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Verify texture is bound
    GLint boundTexture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);
    assert(boundTexture == static_cast<GLint>(texture) && "Texture should be bound");
    
    // Cleanup
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &texture);
    
    std::cout << "  PASSED: Texture binding works correctly" << std::endl;
}

// Test 8: Depth testing enabled
void test_DepthTesting() {
    std::cout << "Test 8: Depth testing enabled..." << std::endl;
    
    OpenGLTestContext ctx;
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Verify depth testing is enabled
    GLboolean depthEnabled = glIsEnabled(GL_DEPTH_TEST);
    assert(depthEnabled && "Depth testing should be enabled");
    
    // Set depth function
    glDepthFunc(GL_LESS);
    
    // Verify depth function
    GLint depthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
    assert(depthFunc == GL_LESS && "Depth function should be GL_LESS");
    
    // Disable depth testing
    glDisable(GL_DEPTH_TEST);
    depthEnabled = glIsEnabled(GL_DEPTH_TEST);
    assert(!depthEnabled && "Depth testing should be disabled");
    
    std::cout << "  PASSED: Depth testing configuration works" << std::endl;
}

// Test 9: Viewport configuration
void test_ViewportConfiguration() {
    std::cout << "Test 9: Viewport configuration..." << std::endl;
    
    OpenGLTestContext ctx;
    
    // Set viewport
    glViewport(0, 0, 800, 600);
    
    // Verify viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    assert(viewport[0] == 0 && "Viewport x should be 0");
    assert(viewport[1] == 0 && "Viewport y should be 0");
    assert(viewport[2] == 800 && "Viewport width should be 800");
    assert(viewport[3] == 600 && "Viewport height should be 600");
    
    // Change viewport
    glViewport(100, 100, 640, 480);
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    assert(viewport[0] == 100 && viewport[1] == 100 && 
           viewport[2] == 640 && viewport[3] == 480 && 
           "Viewport should update correctly");
    
    std::cout << "  PASSED: Viewport configuration works correctly" << std::endl;
}

// Test 10: Resource cleanup (glDeleteVertexArrays)
void test_ResourceCleanup() {
    std::cout << "Test 10: Resource cleanup..." << std::endl;
    
    OpenGLTestContext ctx;
    
    // Create resources
    GLuint vao, vbo, texture;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenTextures(1, &texture);
    
    assert(vao != 0 && vbo != 0 && texture != 0 && "Resources should be created");
    
    // Delete resources
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &texture);
    
    // Note: OpenGL doesn't provide a reliable way to verify deletion,
    // but we can verify no errors occurred
    GLenum error = glGetError();
    assert(error == GL_NO_ERROR && "Resource cleanup should not produce errors");
    
    std::cout << "  PASSED: Resource cleanup works correctly" << std::endl;
}

// Test 11: Frame rendering (clear, draw, swap)
void test_FrameRendering() {
    std::cout << "Test 11: Frame rendering (clear, draw, swap)..." << std::endl;
    
    OpenGLTestContext ctx;
    
    // Clear color and depth buffers
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Verify clear color
    GLfloat clearColor[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);
    assert(std::abs(clearColor[0] - 0.1f) < 0.01f && "Clear color R should be 0.1");
    assert(std::abs(clearColor[1] - 0.2f) < 0.01f && "Clear color G should be 0.2");
    assert(std::abs(clearColor[2] - 0.3f) < 0.01f && "Clear color B should be 0.3");
    
    // Swap buffers (GLFW)
    glfwSwapBuffers(ctx.getWindow());
    
    // Verify no errors
    GLenum error = glGetError();
    assert(error == GL_NO_ERROR && "Frame rendering should not produce errors");
    
    std::cout << "  PASSED: Frame rendering works correctly" << std::endl;
}

// Test 12: Error handling (invalid shader, missing buffers)
void test_ErrorHandling() {
    std::cout << "Test 12: Error handling..." << std::endl;
    
    OpenGLTestContext ctx;
    
    // Test 1: Invalid shader compilation
    const char* invalidShaderSource = "THIS IS NOT VALID GLSL CODE";
    GLuint shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader, 1, &invalidShaderSource, nullptr);
    glCompileShader(shader);
    
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    assert(success == GL_FALSE && "Invalid shader should fail to compile");
    glDeleteShader(shader);
    
    // Test 2: Attempting to draw without VAO bound should not crash
    glBindVertexArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 3);  // Will produce error but shouldn't crash
    GLenum error = glGetError();
    assert(error != GL_NO_ERROR && "Drawing without VAO should produce error");
    
    // Test 3: Attempting to use invalid program
    glUseProgram(9999);  // Invalid program ID
    error = glGetError();
    assert(error != GL_NO_ERROR && "Using invalid program should produce error");
    
    std::cout << "  PASSED: Error handling works correctly" << std::endl;
}

} // namespace dj

int main() {
    std::cout << "========== Enhanced3DScene Phase 6 OpenGL Tests ==========" << std::endl;
    std::cout << std::endl;

    try {
        dj::test_VAOCreationAndBinding();
        dj::test_VBOAndAttributeSetup();
        dj::test_ShaderBindingAndUniforms();
        dj::test_DrawCallExecution();
        dj::test_MVPMatrixSetup();
        dj::test_MultipleVAOs();
        dj::test_TextureBinding();
        dj::test_DepthTesting();
        dj::test_ViewportConfiguration();
        dj::test_ResourceCleanup();
        dj::test_FrameRendering();
        dj::test_ErrorHandling();

        std::cout << std::endl;
        std::cout << "========== ALL TESTS PASSED (12/12) ==========" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}
