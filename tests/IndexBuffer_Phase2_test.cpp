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

#include "visuals/IndexBuffer.h"

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
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "IndexBuffer Test", nullptr, nullptr);
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

// Test 1: EBO creation with index data
void test_IndexBuffer_Creation() {
    std::cout << "Testing IndexBuffer creation..." << std::endl;
    
    GLFWwindow* window = initOpenGLContext();
    
    try {
        // Create test index data (6 indices for 2 triangles)
        std::vector<uint32_t> indices = {
            0, 1, 2,  // Triangle 1
            1, 2, 3   // Triangle 2
        };
        
        dj::IndexBuffer ib;
        bool created = ib.create(indices.data(), static_cast<uint32_t>(indices.size()), GL_STATIC_DRAW);
        
        assert(created && "IndexBuffer::create should return true");
        assert(ib.getIndexCount() == 6 && "Expected 6 indices");
        
        std::cout << "✓ test_IndexBuffer_Creation passed" << std::endl;
    } catch (const std::exception&) {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw;
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Test 2: EBO update with glBufferSubData
void test_IndexBuffer_Update() {
    std::cout << "Testing IndexBuffer update..." << std::endl;
    
    GLFWwindow* window = initOpenGLContext();
    
    try {
        // Create initial index data with dynamic draw usage
        std::vector<uint32_t> initialIndices = {
            0, 1, 2,
            1, 2, 3
        };
        
        dj::IndexBuffer ib;
        bool created = ib.create(initialIndices.data(), static_cast<uint32_t>(initialIndices.size()), GL_DYNAMIC_DRAW);
        assert(created && "IndexBuffer::create should succeed");
        
        // Update part of the buffer (first triangle)
        std::vector<uint32_t> updatedIndices = {
            0, 2, 1  // Reversed winding
        };
        
        bool updated = ib.update(updatedIndices.data(), static_cast<uint32_t>(updatedIndices.size()), 0);
        assert(updated && "IndexBuffer::update should succeed");
        
        std::cout << "✓ test_IndexBuffer_Update passed" << std::endl;
    } catch (const std::exception&) {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw;
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Test 3: EBO binding and unbinding
void test_IndexBuffer_Binding() {
    std::cout << "Testing IndexBuffer binding..." << std::endl;
    
    GLFWwindow* window = initOpenGLContext();
    
    try {
        std::vector<uint32_t> indices = {
            0, 1, 2,
            1, 2, 3
        };
        
        dj::IndexBuffer ib;
        bool created = ib.create(indices.data(), static_cast<uint32_t>(indices.size()), GL_STATIC_DRAW);
        assert(created && "IndexBuffer creation failed");
        
        // Bind the buffer
        bool bound = ib.bind();
        assert(bound && "IndexBuffer::bind should succeed");
        
        // Verify buffer is bound to GL_ELEMENT_ARRAY_BUFFER
        GLint boundBuffer = 0;
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &boundBuffer);
        assert(boundBuffer != 0 && "Buffer should be bound to GL_ELEMENT_ARRAY_BUFFER");
        
        // Unbind
        ib.unbind();
        
        // Verify buffer is unbound
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &boundBuffer);
        assert(boundBuffer == 0 && "Buffer should be unbound");
        
        std::cout << "✓ test_IndexBuffer_Binding passed" << std::endl;
    } catch (const std::exception&) {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw;
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Test 4: EBO deletion and cleanup
void test_IndexBuffer_Cleanup() {
    std::cout << "Testing IndexBuffer cleanup..." << std::endl;
    
    GLFWwindow* window = initOpenGLContext();
    
    try {
        std::vector<uint32_t> indices = {
            0, 1, 2,
            1, 2, 3
        };
        
        {
            dj::IndexBuffer ib;
            bool created = ib.create(indices.data(), static_cast<uint32_t>(indices.size()), GL_STATIC_DRAW);
            assert(created && "IndexBuffer creation failed");
            
            // Buffer should be destroyed when ib goes out of scope
        }
        
        // If we reach here without crashes, cleanup worked
        std::cout << "✓ test_IndexBuffer_Cleanup passed" << std::endl;
    } catch (const std::exception&) {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw;
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Test 5: DirectX-compatible create() API
void test_IndexBuffer_DirectXCompatibility() {
    std::cout << "Testing IndexBuffer DirectX-compatible API..." << std::endl;
    
    GLFWwindow* window = initOpenGLContext();
    
    try {
        uint32_t indices[] = {
            0, 1, 2,
            1, 2, 3
        };
        
        dj::IndexBuffer ib;
        // DirectX-compatible API: create(device, indices, count)
        bool success = ib.create(nullptr, indices, 6);
        assert(success && "DirectX-compatible create() failed");
        assert(ib.getIndexCount() == 6 && "Index count should be 6");
        
        std::cout << "✓ test_IndexBuffer_DirectXCompatibility passed" << std::endl;
    } catch (const std::exception&) {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw;
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Test 6: DirectX-compatible bind() API
void test_IndexBuffer_DirectXCompatibleBind() {
    std::cout << "Testing IndexBuffer DirectX-compatible bind..." << std::endl;
    
    GLFWwindow* window = initOpenGLContext();
    
    try {
        uint32_t indices[] = { 0, 1, 2 };
        
        dj::IndexBuffer ib;
        bool created = ib.create(nullptr, indices, 3);
        assert(created && "Creation failed");
        
        // DirectX-compatible bind: bind(context, slot) - should not crash
        ib.bind(nullptr, 0);
        
        // Verify buffer is bound
        GLint boundBuffer = 0;
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &boundBuffer);
        assert(boundBuffer != 0 && "Buffer should be bound to GL_ELEMENT_ARRAY_BUFFER");
        
        ib.unbind();
        
        std::cout << "✓ test_IndexBuffer_DirectXCompatibleBind passed" << std::endl;
    } catch (const std::exception&) {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw;
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Test 7: Resource leak prevention (create() called twice)
void test_IndexBuffer_ResourceLeakPrevention() {
    std::cout << "Testing IndexBuffer resource leak prevention..." << std::endl;
    
    GLFWwindow* window = initOpenGLContext();
    
    try {
        uint32_t indices1[] = { 0, 1, 2 };
        uint32_t indices2[] = { 1, 2, 3, 0 };
        
        dj::IndexBuffer ib;
        
        // First create
        bool created1 = ib.create(indices1, 3, GL_STATIC_DRAW);
        assert(created1 && "First create() failed");
        GLuint ebo1 = ib.getEBO();
        assert(ebo1 != 0 && "EBO should be created");
        
        // Second create should clean up the first
        bool created2 = ib.create(indices2, 4, GL_STATIC_DRAW);
        assert(created2 && "Second create() failed");
        GLuint ebo2 = ib.getEBO();
        assert(ebo2 != 0 && "EBO should be created");
        assert(ib.getIndexCount() == 4 && "Index count should be updated to 4");
        
        // They should be different handles (old one deleted first)
        // Note: We can't directly verify the old handle is deleted, but the code should not crash
        
        std::cout << "✓ test_IndexBuffer_ResourceLeakPrevention passed" << std::endl;
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
        dj::test_IndexBuffer_Creation();
        dj::test_IndexBuffer_Update();
        dj::test_IndexBuffer_Binding();
        dj::test_IndexBuffer_Cleanup();
        dj::test_IndexBuffer_DirectXCompatibility();
        dj::test_IndexBuffer_DirectXCompatibleBind();
        dj::test_IndexBuffer_ResourceLeakPrevention();
        
        std::cout << "\n✅ All IndexBuffer Phase 2 tests passed!\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
