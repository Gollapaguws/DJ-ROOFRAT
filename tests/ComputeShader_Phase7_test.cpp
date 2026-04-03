#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #define NOGDI
#endif

#include <cassert>
#include <iostream>
#include <vector>
#include <memory>

#define DJROOFRAT_OPENGL_MIGRATION 1
#define DJROOFRAT_ENABLE_GRAPHICS 1

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "visuals/ComputeShader.h"

namespace dj {

class OpenGLTestContext {
public:
    OpenGLTestContext() {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        window = glfwCreateWindow(800, 600, "Test Window", nullptr, nullptr);
        if (!window) {
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwMakeContextCurrent(window);
        
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            glfwDestroyWindow(window);
            throw std::runtime_error("Failed to load OpenGL functions");
        }
    }

    ~OpenGLTestContext() {
        if (window) {
            glfwDestroyWindow(window);
            window = nullptr;
        }
        glfwTerminate();
    }

private:
    GLFWwindow* window = nullptr;
};

// Test 1: Compute shader compilation
void test_ComputeShaderCompilation() {
    std::cout << "Test 1: Compute shader compilation..." << std::endl;

    OpenGLTestContext ctx;

    const char* computeSource = R"(
        #version 430 core
        layout(local_size_x = 8, local_size_y = 8) in;
        
        layout(std430, binding = 0) buffer Data {
            vec4 values[];
        };
        
        void main() {
            uint idx = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * 8u;
            values[idx] = vec4(1.0, 0.0, 0.0, 1.0);
        }
    )";

    ComputeShader computeShader;
    std::string error;
    bool compiled = computeShader.compileSource(computeSource, &error);
    
    if (!compiled) {
        std::cout << "Compilation error: " << error << std::endl;
    }
    assert(compiled && "Compute shader should compile successfully");
    assert(computeShader.getProgram() != 0 && "Shader program should be non-zero");

    std::cout << "✓ Test 1 passed" << std::endl;
}

// Test 2: SSBO creation and binding
void test_SSBOCreationAndBinding() {
    std::cout << "Test 2: SSBO creation and binding..." << std::endl;

    OpenGLTestContext ctx;

    const char* computeSource = R"(
        #version 430 core
        layout(local_size_x = 1) in;
        
        layout(std430, binding = 0) buffer Data {
            vec4 values[];
        };
        
        void main() {
            values[0] = vec4(1.0, 2.0, 3.0, 4.0);
        }
    )";

    ComputeShader computeShader;
    std::string error;
    assert(computeShader.compileSource(computeSource, &error) && "Compute shader should compile");

    // Create SSBO
    GLuint ssbo = 0;
    glGenBuffers(1, &ssbo);
    assert(ssbo != 0 && "SSBO handle should be non-zero");

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    std::vector<glm::vec4> data(10);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * 10, data.data(), GL_DYNAMIC_COPY);

    // Bind to compute shader
    computeShader.bindSSBO(ssbo, 0);

    // Read back to verify binding worked
    glm::vec4* mappedData = static_cast<glm::vec4*>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4), GL_MAP_READ_BIT)
    );
    assert(mappedData != nullptr && "Should be able to map SSBO");
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glDeleteBuffers(1, &ssbo);

    std::cout << "✓ Test 2 passed" << std::endl;
}

// Test 3: Dispatch compute with workgroup sizes
void test_DispatchComputeWithWorkgroups() {
    std::cout << "Test 3: Dispatch compute with workgroup sizes..." << std::endl;

    OpenGLTestContext ctx;

    const char* computeSource = R"(
        #version 430 core
        layout(local_size_x = 8, local_size_y = 8) in;
        
        layout(std430, binding = 0) buffer Data {
            uint values[];
        };
        
        void main() {
            uint idx = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * 256u;
            values[idx] = idx;
        }
    )";

    ComputeShader computeShader;
    std::string error;
    assert(computeShader.compileSource(computeSource, &error) && "Should compile");

    GLuint ssbo = 0;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    std::vector<GLuint> data(256 * 256, 0);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 256 * 256, data.data(), GL_DYNAMIC_COPY);

    computeShader.use();
    computeShader.bindSSBO(ssbo, 0);
    computeShader.dispatch(32, 32, 1);  // 32*8 = 256 in each dimension

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Verify computation
    const GLuint* resultData = static_cast<const GLuint*>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint) * 256 * 256, GL_MAP_READ_BIT)
    );
    assert(resultData != nullptr && "Should be able to read results");

    // Check a few values
    bool dataCorrect = (resultData[0] == 0 && resultData[257] == 257);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    assert(dataCorrect && "Compute shader should have written correct values");

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glDeleteBuffers(1, &ssbo);

    std::cout << "✓ Test 3 passed" << std::endl;
}

// Test 4: Memory barrier synchronization
void test_MemoryBarrierSynchronization() {
    std::cout << "Test 4: Memory barrier synchronization..." << std::endl;

    OpenGLTestContext ctx;

    const char* computeSource = R"(
        #version 430 core
        layout(local_size_x = 1) in;
        
        layout(std430, binding = 0) buffer Data {
            uint values[];
        };
        
        void main() {
            values[0] = 42u;
        }
    )";

    ComputeShader computeShader;
    std::string error;
    assert(computeShader.compileSource(computeSource, &error) && "Should compile");

    GLuint ssbo = 0;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    std::vector<GLuint> data(10, 0);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 10, data.data(), GL_DYNAMIC_COPY);

    computeShader.use();
    computeShader.bindSSBO(ssbo, 0);
    computeShader.dispatch(1, 1, 1);

    // Memory barrier should be called before reading
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

    GLuint* mappedData = static_cast<GLuint*>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint) * 10, GL_MAP_READ_BIT)
    );
    assert(mappedData != nullptr && "Should be able to map");
    assert(mappedData[0] == 42u && "Value should be 42");
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glDeleteBuffers(1, &ssbo);

    std::cout << "✓ Test 4 passed" << std::endl;
}

// Test 5: Read back compute results
void test_ReadBackComputeResults() {
    std::cout << "Test 5: Read back compute results..." << std::endl;

    OpenGLTestContext ctx;

    const char* computeSource = R"(
        #version 430 core
        layout(local_size_x = 1) in;
        
        layout(std430, binding = 0) buffer Data {
            vec4 values[];
        };
        
        void main() {
            for(uint i = 0u; i < 10u; ++i) {
                values[i] = vec4(float(i));
            }
        }
    )";

    ComputeShader computeShader;
    std::string error;
    assert(computeShader.compileSource(computeSource, &error) && "Should compile");

    GLuint ssbo = 0;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    std::vector<glm::vec4> data(10, glm::vec4(0.0f));
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * 10, data.data(), GL_DYNAMIC_COPY);

    computeShader.use();
    computeShader.bindSSBO(ssbo, 0);
    computeShader.dispatch(1, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glm::vec4* resultData = static_cast<glm::vec4*>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * 10, GL_MAP_READ_BIT)
    );
    assert(resultData != nullptr && "Should read results");

    bool correct = (resultData[0].x == 0.0f && resultData[5].x == 5.0f && resultData[9].x == 9.0f);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    assert(correct && "All compute results should be correct");

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glDeleteBuffers(1, &ssbo);

    std::cout << "✓ Test 5 passed" << std::endl;
}

// Test 6: Multiple SSBO bindings
void test_MultipleSSBOBindings() {
    std::cout << "Test 6: Multiple SSBO bindings..." << std::endl;

    OpenGLTestContext ctx;

    const char* computeSource = R"(
        #version 430 core
        layout(local_size_x = 1) in;
        
        layout(std430, binding = 0) buffer Input {
            vec4 inValues[];
        };
        
        layout(std430, binding = 1) buffer Output {
            vec4 outValues[];
        };
        
        void main() {
            outValues[0] = inValues[0] * 2.0;
        }
    )";

    ComputeShader computeShader;
    std::string error;
    assert(computeShader.compileSource(computeSource, &error) && "Should compile");

    // Create input SSBO
    GLuint inputSSBO = 0;
    glGenBuffers(1, &inputSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, inputSSBO);
    glm::vec4 inputData(1.0f, 2.0f, 3.0f, 4.0f);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4), &inputData, GL_DYNAMIC_COPY);

    // Create output SSBO
    GLuint outputSSBO = 0;
    glGenBuffers(1, &outputSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputSSBO);
    glm::vec4 outputData(0.0f, 0.0f, 0.0f, 0.0f);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4), &outputData, GL_DYNAMIC_COPY);

    computeShader.use();
    computeShader.bindSSBO(inputSSBO, 0);
    computeShader.bindSSBO(outputSSBO, 1);
    computeShader.dispatch(1, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Read output
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputSSBO);
    glm::vec4* result = static_cast<glm::vec4*>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4), GL_MAP_READ_BIT)
    );
    assert(result != nullptr && "Should read output");

    bool correct = (result->x == 2.0f && result->y == 4.0f && result->z == 6.0f && result->w == 8.0f);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    assert(correct && "Output should be doubled input");

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glDeleteBuffers(1, &inputSSBO);
    glDeleteBuffers(1, &outputSSBO);

    std::cout << "✓ Test 6 passed" << std::endl;
}

// Test 7: Compute shader uniform setting
void test_ComputeShaderUniformSetting() {
    std::cout << "Test 7: Compute shader uniform setting..." << std::endl;

    OpenGLTestContext ctx;

    const char* computeSource = R"(
        #version 430 core
        layout(local_size_x = 1) in;
        
        uniform float factor;
        
        layout(std430, binding = 0) buffer Data {
            vec4 values[];
        };
        
        void main() {
            values[0] = vec4(factor);
        }
    )";

    ComputeShader computeShader;
    std::string error;
    assert(computeShader.compileSource(computeSource, &error) && "Should compile");

    GLuint ssbo = 0;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glm::vec4 data(0.0f);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4), &data, GL_DYNAMIC_COPY);

    computeShader.use();
    computeShader.bindSSBO(ssbo, 0);
    
    GLint factorLoc = glGetUniformLocation(computeShader.getProgram(), "factor");
    glUniform1f(factorLoc, 3.14f);

    computeShader.dispatch(1, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glm::vec4* result = static_cast<glm::vec4*>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4), GL_MAP_READ_BIT)
    );
    assert(result != nullptr && "Should read result");

    float epsilon = 0.01f;
    bool correct = (std::abs(result->x - 3.14f) < epsilon);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    assert(correct && "Uniform should be set correctly");

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glDeleteBuffers(1, &ssbo);

    std::cout << "✓ Test 7 passed" << std::endl;
}

// Test 8: Error handling
void test_ErrorHandling() {
    std::cout << "Test 8: Error handling..." << std::endl;

    OpenGLTestContext ctx;

    ComputeShader computeShader;
    std::string error;

    // Test invalid compute shader source
    const char* invalidSource = "THIS IS NOT VALID GLSL";
    bool compiled = computeShader.compileSource(invalidSource, &error);
    assert(!compiled && "Invalid shader should fail to compile");
    assert(!error.empty() && "Error message should not be empty");

    std::cout << "✓ Test 8 passed" << std::endl;
}

}  // namespace dj

int main() {
    std::cout << "========== ComputeShader Phase 7 Tests ==========" << std::endl;
    std::cout << std::endl;

    try {
        dj::test_ComputeShaderCompilation();
        dj::test_SSBOCreationAndBinding();
        dj::test_DispatchComputeWithWorkgroups();
        dj::test_MemoryBarrierSynchronization();
        dj::test_ReadBackComputeResults();
        dj::test_MultipleSSBOBindings();
        dj::test_ComputeShaderUniformSetting();
        dj::test_ErrorHandling();

        std::cout << std::endl;
        std::cout << "========== ALL TESTS PASSED (8/8) ==========" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}
