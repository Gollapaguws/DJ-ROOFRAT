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

#include <glad/glad.h>

/* Prevent system GL functions from conflicting with GLAD function pointers */
#define glClear __system_glClear
#define glClearColor __system_glClearColor
#define glGetString __system_glGetString
#define glGetIntegerv __system_glGetIntegerv
#define glViewport __system_glViewport
#define glEnable __system_glEnable
#define glDisable __system_glDisable
#define glGenBuffers __system_glGenBuffers
#define glDeleteBuffers __system_glDeleteBuffers
#define glBindBuffer __system_glBindBuffer
#define glBufferData __system_glBufferData
#define glBufferSubData __system_glBufferSubData
#define glDrawArrays __system_glDrawArrays
#define glDrawElements __system_glDrawElements

#define GLFW_INCLUDE_NONE  // Prevent GLFW from including GL headers
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

// Define GL constants that might not be in glad.h
#ifndef GL_ARRAY_BUFFER_BINDING
#define GL_ARRAY_BUFFER_BINDING 0x8894
#endif

#ifndef GL_ELEMENT_ARRAY_BUFFER_BINDING
#define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#endif

#include "visuals/VertexBuffer.h"

namespace dj {

// Helper function: Initialize OpenGL context for tests
GLFWwindow* initOpenGLContext() {
    if (!glfwInit()) {
        throw std::runtime_error("GLFW initialization failed");
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "VertexBuffer Test", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    
    glfwMakeContextCurrent(window);
    
    int version = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    if (version == 0) {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw std::runtime_error("Failed to load OpenGL functions via GLAD");
    }
    
    return window;
}

// Test 1: VBO creation with vertex data
void test_VertexBuffer_Creation() {
    std::cout << "Testing VertexBuffer creation..." << std::endl;
    
    GLFWwindow* window = initOpenGLContext();
    
    try {
        // Create test vertex data (3 vertices with 3 float coordinates each)
        std::vector<float> vertices = {
            -0.5f, -0.5f, 0.0f,  // Vertex 0
             0.5f, -0.5f, 0.0f,  // Vertex 1
             0.0f,  0.5f, 0.0f   // Vertex 2
        };
        
        dj::VertexBuffer vb;
        bool created = vb.create(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(float)), GL_STATIC_DRAW);
        
        assert(created && "VertexBuffer::create should return true");
        
        std::cout << "✓ test_VertexBuffer_Creation passed" << std::endl;
    } catch (const std::exception&) {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw;
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Test 2: VBO update with glBufferSubData
void test_VertexBuffer_Update() {
    std::cout << "Testing VertexBuffer update..." << std::endl;
    
    GLFWwindow* window = initOpenGLContext();
    
    try {
        // Create initial vertex data
        std::vector<float> initialVertices = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.0f,  0.5f, 0.0f
        };
        
        dj::VertexBuffer vb;
        bool created = vb.create(initialVertices.data(), static_cast<uint32_t>(initialVertices.size() * sizeof(float)), GL_DYNAMIC_DRAW);
        assert(created && "VertexBuffer::create should succeed");
        
        // Update part of the buffer (first vertex)
        std::vector<float> updatedVertices = {
            -0.25f, -0.25f, 0.0f
        };
        
        bool updated = vb.update(updatedVertices.data(), static_cast<uint32_t>(updatedVertices.size() * sizeof(float)), 0);
        assert(updated && "VertexBuffer::update should succeed");
        
        std::cout << "✓ test_VertexBuffer_Update passed" << std::endl;
    } catch (const std::exception&) {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw;
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Test 3: VBO binding and unbinding
void test_VertexBuffer_Binding() {
    std::cout << "Testing VertexBuffer binding..." << std::endl;
    
    GLFWwindow* window = initOpenGLContext();
    
    try {
        std::vector<float> vertices = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.0f,  0.5f, 0.0f
        };
        
        dj::VertexBuffer vb;
        bool created = vb.create(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(float)), GL_STATIC_DRAW);
        assert(created && "VertexBuffer creation failed");
        
        // Bind the buffer
        bool bound = vb.bind();
        assert(bound && "VertexBuffer::bind should succeed");
        
        // Verify buffer is bound to GL_ARRAY_BUFFER
        GLint boundBuffer = 0;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &boundBuffer);
        assert(boundBuffer != 0 && "Buffer should be bound to GL_ARRAY_BUFFER");
        
        // Unbind
        vb.unbind();
        
        // Verify buffer is unbound
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &boundBuffer);
        assert(boundBuffer == 0 && "Buffer should be unbound");
        
        std::cout << "✓ test_VertexBuffer_Binding passed" << std::endl;
    } catch (const std::exception&) {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw;
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Test 4: VBO deletion and cleanup
void test_VertexBuffer_Cleanup() {
    std::cout << "Testing VertexBuffer cleanup..." << std::endl;
    
    GLFWwindow* window = initOpenGLContext();
    
    try {
        std::vector<float> vertices = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.0f,  0.5f, 0.0f
        };
        
        {
            dj::VertexBuffer vb;
            bool created = vb.create(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(float)), GL_STATIC_DRAW);
            assert(created && "VertexBuffer creation failed");
            
            // Buffer should be destroyed when vb goes out of scope
        }
        
        // If we reach here without crashes, cleanup worked
        std::cout << "✓ test_VertexBuffer_Cleanup passed" << std::endl;
    } catch (const std::exception&) {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw;
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Test 5: DirectX-compatible create() API
void test_VertexBuffer_DirectXCompatibility() {
    std::cout << "Testing VertexBuffer DirectX-compatible API..." << std::endl;
    
    GLFWwindow* window = initOpenGLContext();
    
    try {
        float vertices[] = {
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.5f, 0.5f
        };
        
        dj::VertexBuffer vb;
        // DirectX-compatible API: create(device, vertices, count, stride)
        bool success = vb.create(nullptr, vertices, 3, sizeof(float) * 2);
        assert(success && "DirectX-compatible create() failed");
        assert(vb.getVertexCount() == 3 && "Vertex count should be 3");
        assert(vb.getVertexSize() == sizeof(float) * 2 && "Vertex size should be 8 bytes");
        
        std::cout << "✓ test_VertexBuffer_DirectXCompatibility passed" << std::endl;
    } catch (const std::exception&) {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw;
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Test 6: DirectX-compatible bind() API
void test_VertexBuffer_DirectXCompatibleBind() {
    std::cout << "Testing VertexBuffer DirectX-compatible bind..." << std::endl;
    
    GLFWwindow* window = initOpenGLContext();
    
    try {
        float vertices[] = { 0.0f, 0.0f, 1.0f, 1.0f };
        
        dj::VertexBuffer vb;
        bool created = vb.create(nullptr, vertices, 2, sizeof(float) * 2);
        assert(created && "Creation failed");
        
        // DirectX-compatible bind: bind(context, slot) - should not crash
        vb.bind(nullptr, 0);
        
        // Verify buffer is bound
        GLint boundBuffer = 0;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &boundBuffer);
        assert(boundBuffer != 0 && "Buffer should be bound to GL_ARRAY_BUFFER");
        
        vb.unbind();
        
        std::cout << "✓ test_VertexBuffer_DirectXCompatibleBind passed" << std::endl;
    } catch (const std::exception&) {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw;
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Test 7: Resource leak prevention (create() called twice)
void test_VertexBuffer_ResourceLeakPrevention() {
    std::cout << "Testing VertexBuffer resource leak prevention..." << std::endl;
    
    GLFWwindow* window = initOpenGLContext();
    
    try {
        float vertices1[] = { 0.0f, 0.0f, 1.0f, 1.0f };
        float vertices2[] = { 0.5f, 0.5f, 1.5f, 1.5f };
        
        dj::VertexBuffer vb;
        
        // First create
        bool created1 = vb.create(vertices1, 2 * sizeof(float) * 2, GL_STATIC_DRAW);
        assert(created1 && "First create() failed");
        GLuint vbo1 = vb.getVBO();
        assert(vbo1 != 0 && "VBO should be created");
        
        // Second create should clean up the first
        bool created2 = vb.create(vertices2, 2 * sizeof(float) * 2, GL_STATIC_DRAW);
        assert(created2 && "Second create() failed");
        GLuint vbo2 = vb.getVBO();
        assert(vbo2 != 0 && "VBO should be created");
        
        // They should be different handles (old one deleted first)
        // Note: We can't directly verify the old handle is deleted, but the code should not crash
        
        std::cout << "✓ test_VertexBuffer_ResourceLeakPrevention passed" << std::endl;
    } catch (const std::exception&) {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw;
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
}

} // namespace dj

int main() {
    try {
        dj::test_VertexBuffer_Creation();
        dj::test_VertexBuffer_Update();
        dj::test_VertexBuffer_Binding();
        dj::test_VertexBuffer_Cleanup();
        dj::test_VertexBuffer_DirectXCompatibility();
        dj::test_VertexBuffer_DirectXCompatibleBind();
        dj::test_VertexBuffer_ResourceLeakPrevention();
        
        std::cout << "\n✅ All VertexBuffer Phase 2 tests passed!\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
