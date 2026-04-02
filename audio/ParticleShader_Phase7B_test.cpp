// Phase 7B: Particle GLSL Shader TDD Tests
// Tests: 1. Compile particles.glsl 2. Dispatch compute shader 3. Verify positions 4. No OpenGL errors

#include <gtest/gtest.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>
#include <string>

// Helper: Load file to string
static std::string LoadFile(const char* path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) return "";
    std::string contents;
    file.seekg(0, std::ios::end);
    contents.resize((size_t)file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&contents[0], contents.size());
    file.close();
    return contents;
}

// Helper: Compile compute shader
static GLuint CompileComputeShader(const char* src) {
    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    return status ? shader : 0;
}

// Helper: Link compute program
static GLuint LinkComputeProgram(GLuint shader) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, shader);
    glLinkProgram(prog);
    GLint status = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    return status ? prog : 0;
}

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

// Test 1: Compile particles.glsl
TEST(ParticleShader_Phase7B, CompileGLSL) {
    if (!glfwInit()) GTEST_SKIP();
    GLFWwindow* win = glfwCreateWindow(32, 32, "", nullptr, nullptr);
    if (!win) { glfwTerminate(); GTEST_SKIP(); }
    glfwMakeContextCurrent(win);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    std::string src = LoadFile("shaders/particles.glsl");
    ASSERT_FALSE(src.empty());
    GLuint shader = CompileComputeShader(src.c_str());
    EXPECT_NE(shader, 0u);
    if (shader) glDeleteShader(shader);
    glfwDestroyWindow(win);
    glfwTerminate();
}

// Test 2: Dispatch compute shader with particle data
TEST(ParticleShader_Phase7B, DispatchComputeShader) {
    if (!glfwInit()) GTEST_SKIP();
    GLFWwindow* win = glfwCreateWindow(32, 32, "", nullptr, nullptr);
    if (!win) { glfwTerminate(); GTEST_SKIP(); }
    glfwMakeContextCurrent(win);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    std::string src = LoadFile("shaders/particles.glsl");
    GLuint shader = CompileComputeShader(src.c_str());
    GLuint prog = LinkComputeProgram(shader);
    glDeleteShader(shader);
    ASSERT_NE(prog, 0u);
    // Setup SSBO
    Particle p = {glm::vec3(0,0,0), 1.0f, glm::vec3(1,0,0), 1.0f, glm::vec4(1,1,1,1), 1.0f, glm::vec3(0)};
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particle), &p, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    // Setup UBO
    struct Consts { glm::vec3 Gravity; float DeltaTime; glm::vec3 WindForce; int ParticleCount; };
    Consts c = {glm::vec3(0,-9.8f,0), 0.1f, glm::vec3(0), 1};
    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Consts), &c, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
    // Dispatch
    glUseProgram(prog);
    glDispatchCompute(1,1,1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glDeleteProgram(prog);
    glDeleteBuffers(1, &ssbo);
    glDeleteBuffers(1, &ubo);
    glfwDestroyWindow(win);
    glfwTerminate();
}

// Test 3: Verify particle positions update
TEST(ParticleShader_Phase7B, ParticlePositionUpdate) {
    if (!glfwInit()) GTEST_SKIP();
    GLFWwindow* win = glfwCreateWindow(32, 32, "", nullptr, nullptr);
    if (!win) { glfwTerminate(); GTEST_SKIP(); }
    glfwMakeContextCurrent(win);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    std::string src = LoadFile("shaders/particles.glsl");
    GLuint shader = CompileComputeShader(src.c_str());
    GLuint prog = LinkComputeProgram(shader);
    glDeleteShader(shader);
    // Setup SSBO
    Particle p = {glm::vec3(0,0,0), 1.0f, glm::vec3(1,0,0), 1.0f, glm::vec4(1,1,1,1), 1.0f, glm::vec3(0)};
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particle), &p, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    // Setup UBO
    struct Consts { glm::vec3 Gravity; float DeltaTime; glm::vec3 WindForce; int ParticleCount; };
    Consts c = {glm::vec3(0,-9.8f,0), 0.1f, glm::vec3(0), 1};
    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Consts), &c, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
    // Dispatch
    glUseProgram(prog);
    glDispatchCompute(1,1,1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    // Read back
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    Particle* ptr = (Particle*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Particle), GL_MAP_READ_BIT);
    ASSERT_NE(ptr, nullptr);
    EXPECT_NEAR(ptr->position.x, 0.1f, 1e-3f); // vx*dt
    EXPECT_NEAR(ptr->position.y, -0.098f, 1e-3f); // vy*dt + gravity*dt^2
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glDeleteProgram(prog);
    glDeleteBuffers(1, &ssbo);
    glDeleteBuffers(1, &ubo);
    glfwDestroyWindow(win);
    glfwTerminate();
}

// Test 4: Ensure no OpenGL errors during dispatch
TEST(ParticleShader_Phase7B, NoOpenGLErrors) {
    if (!glfwInit()) GTEST_SKIP();
    GLFWwindow* win = glfwCreateWindow(32, 32, "", nullptr, nullptr);
    if (!win) { glfwTerminate(); GTEST_SKIP(); }
    glfwMakeContextCurrent(win);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    std::string src = LoadFile("shaders/particles.glsl");
    GLuint shader = CompileComputeShader(src.c_str());
    GLuint prog = LinkComputeProgram(shader);
    glDeleteShader(shader);
    // Setup SSBO
    Particle p = {glm::vec3(0,0,0), 1.0f, glm::vec3(1,0,0), 1.0f, glm::vec4(1,1,1,1), 1.0f, glm::vec3(0)};
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particle), &p, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    // Setup UBO
    struct Consts { glm::vec3 Gravity; float DeltaTime; glm::vec3 WindForce; int ParticleCount; };
    Consts c = {glm::vec3(0,-9.8f,0), 0.1f, glm::vec3(0), 1};
    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Consts), &c, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
    // Dispatch
    glUseProgram(prog);
    glDispatchCompute(1,1,1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    GLenum err = glGetError();
    EXPECT_EQ(err, GL_NO_ERROR);
    glDeleteProgram(prog);
    glDeleteBuffers(1, &ssbo);
    glDeleteBuffers(1, &ubo);
    glfwDestroyWindow(win);
    glfwTerminate();
}
