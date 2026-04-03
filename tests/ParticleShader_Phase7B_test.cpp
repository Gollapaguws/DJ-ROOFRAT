#include <cassert>
#include <iostream>
#include <vector>
#include <cstring>
#include <cmath>
#include <fstream>
#include <string>

#define DJROOFRAT_OPENGL_MIGRATION 1
#define DJROOFRAT_ENABLE_GRAPHICS 1

// Prevent Windows from including its own OpenGL headers
#define __gl_h_
#define __GL_H__

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// Particle struct (matches GLSL layout)
struct Particle {
    glm::vec3 position;
    float lifetime;
    glm::vec3 velocity;
    float initialLife;
    glm::vec4 color;
    float size;
    glm::vec3 padding;
};

// Constants struct (matches GLSL layout)
struct ParticleConstants {
    glm::vec3 Gravity;
    float DeltaTime;
    glm::vec3 WindForce;
    int ParticleCount;
};

// Helper: Load file to string
static std::string LoadFile(const char* path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) return "";
    std::string contents;
    file.seekg(0, std::ios::end);
    contents.resize(static_cast<size_t>(file.tellg()));
    file.seekg(0, std::ios::beg);
    file.read(&contents[0], contents.size());
    file.close();
    return contents;
}

// Helper: Compile compute shader
static GLuint CompileComputeShader(const char* src, std::string& error) {
    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            std::vector<char> log(logLength);
            glGetShaderInfoLog(shader, logLength, nullptr, log.data());
            error = log.data();
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

// Helper: Link compute program
static GLuint LinkComputeProgram(GLuint shader, std::string& error) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, shader);
    glLinkProgram(prog);
    GLint status = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (!status) {
        GLint logLength = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            std::vector<char> log(logLength);
            glGetProgramInfoLog(prog, logLength, nullptr, log.data());
            error = log.data();
        }
        glDeleteProgram(prog);
        return 0;
    }
    return prog;
}

int main() {
    std::cout << "========== Particle GLSL Shader Phase 7B Tests ==========" << std::endl << std::endl;
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return 1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    
    GLFWwindow* window = glfwCreateWindow(640, 480, "Particle Test", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return 1;
    }
    
    glfwMakeContextCurrent(window);
    
    // Load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    
    int testsPassed = 0;
    int totalTests = 4;
    
    // Test 1: Compile particles.glsl successfully
    std::cout << "Test 1: Compile particles.glsl shader..." << std::endl;
    {
        std::string shaderSrc = LoadFile("shaders/particles.glsl");
        if (shaderSrc.empty()) {
            std::cout << "  FAILED: Could not load shaders/particles.glsl" << std::endl;
        } else {
            std::string error;
            GLuint shader = CompileComputeShader(shaderSrc.c_str(), error);
            if (shader == 0) {
                std::cout << "  FAILED: Shader compilation failed: " << error << std::endl;
            } else {
                std::cout << "  PASSED: Shader compiled successfully" << std::endl;
                glDeleteShader(shader);
                testsPassed++;
            }
        }
    }
    
    // Test 2: Dispatch compute shader with particle data
    std::cout << "Test 2: Link and create compute program..." << std::endl;
    {
        std::string shaderSrc = LoadFile("shaders/particles.glsl");
        std::string error;
        GLuint shader = CompileComputeShader(shaderSrc.c_str(), error);
        if (shader == 0) {
            std::cout << "  FAILED: Shader compilation failed" << std::endl;
        } else {
            GLuint program = LinkComputeProgram(shader, error);
            glDeleteShader(shader);
            if (program == 0) {
                std::cout << "  FAILED: Program linking failed: " << error << std::endl;
            } else {
                std::cout << "  PASSED: Compute program created successfully" << std::endl;
                glDeleteProgram(program);
                testsPassed++;
            }
        }
    }
    
    // Test 3: Verify particle positions update correctly
    std::cout << "Test 3: Dispatch compute shader and update particles..." << std::endl;
    {
        std::string shaderSrc = LoadFile("shaders/particles.glsl");
        std::string error;
        GLuint shader = CompileComputeShader(shaderSrc.c_str(), error);
        GLuint program = LinkComputeProgram(shader, error);
        glDeleteShader(shader);
        
        if (program == 0) {
            std::cout << "  FAILED: Could not create program" << std::endl;
        } else {
            // Create particle with initial position and velocity
            Particle particle;
            particle.position = glm::vec3(0.0f, 10.0f, 0.0f);
            particle.velocity = glm::vec3(1.0f, 0.0f, 0.0f);
            particle.lifetime = 1.0f;
            particle.initialLife = 1.0f;
            particle.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            particle.size = 1.0f;
            particle.padding = glm::vec3(0.0f);
            
            // Create SSBO for particle
            GLuint ssbo;
            glGenBuffers(1, &ssbo);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particle), &particle, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
            
            // Create UBO for constants
            ParticleConstants constants;
            constants.Gravity = glm::vec3(0.0f, -9.8f, 0.0f);
            constants.DeltaTime = 0.1f;
            constants.WindForce = glm::vec3(0.0f, 0.0f, 0.0f);
            constants.ParticleCount = 1;
            
            GLuint ubo;
            glGenBuffers(1, &ubo);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(ParticleConstants), &constants, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
            
            // Dispatch compute shader
            glUseProgram(program);
            glDispatchCompute(1, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            
            // Read back particle data
            Particle result;
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
            void* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
            if (ptr) {
                std::memcpy(&result, ptr, sizeof(Particle));
                glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
                
                // Verify: position should have moved due to velocity and gravity
                // New X = 0 + 1.0 * 0.1 = 0.1
                // New Y = 10 + 0 * 0.1 + (-9.8 * 0.1) * 0.1 = 10 - 0.098 = 9.902
                float expectedX = 0.1f;
                float expectedY = 10.0f - 0.098f;
                
                float tolerance = 0.01f;
                if (std::abs(result.position.x - expectedX) < tolerance &&
                    std::abs(result.position.y - expectedY) < tolerance) {
                    std::cout << "  PASSED: Particle position updated correctly" << std::endl;
                    testsPassed++;
                } else {
                    std::cout << "  FAILED: Particle position incorrect. Expected (" 
                              << expectedX << ", " << expectedY << "), got ("
                              << result.position.x << ", " << result.position.y << ")" << std::endl;
                }
            } else {
                std::cout << "  FAILED: Could not map buffer" << std::endl;
            }
            
            glDeleteBuffers(1, &ssbo);
            glDeleteBuffers(1, &ubo);
            glDeleteProgram(program);
        }
    }
    
    // Test 4: Ensure no OpenGL errors during dispatch
    std::cout << "Test 4: Check for OpenGL errors..." << std::endl;
    {
        GLenum error = glGetError();
        if (error == GL_NO_ERROR) {
            std::cout << "  PASSED: No OpenGL errors detected" << std::endl;
            testsPassed++;
        } else {
            std::cout << "  FAILED: OpenGL error detected: 0x" << std::hex << error << std::dec << std::endl;
        }
    }
    
    // Summary
    std::cout << std::endl;
    if (testsPassed == totalTests) {
        std::cout << "========== ALL TESTS PASSED (" << testsPassed << "/" << totalTests << ") ==========" << std::endl;
    } else {
        std::cout << "========== TESTS FAILED (" << testsPassed << "/" << totalTests << " passed) ==========" << std::endl;
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return (testsPassed == totalTests) ? 0 : 1;
}
