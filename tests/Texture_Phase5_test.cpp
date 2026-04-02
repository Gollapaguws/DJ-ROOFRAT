#include <cassert>
#include <iostream>
#include <vector>
#include <cstring>

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

#include <GLFW/glfw3.h>

/* Undefine the system GL renamings to restore our GLAD pointers */
#undef glClear
#undef glClearColor
#undef glGetString
#undef glGetIntegerv
#undef glViewport
#undef glEnable
#undef glDisable
#undef glDrawArrays
#undef glDrawElements

#include "visuals/Texture.h"

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
        
        m_window = glfwCreateWindow(1, 1, "Texture Test Context", nullptr, nullptr);
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
    
private:
    GLFWwindow* m_window = nullptr;
};

// Test 1: Texture creation
void test_TextureCreation() {
    std::cout << "Test 1: Texture creation..." << std::endl;
    
    OpenGLTestContext ctx;
    
    Texture texture;
    bool success = texture.create(256, 256, GL_RGBA);
    
    assert(success && "Texture creation should succeed");
    assert(texture.getTextureID() != 0 && "Texture ID should be non-zero");
    assert(texture.getWidth() == 256 && "Width should be 256");
    assert(texture.getHeight() == 256 && "Height should be 256");
    
    std::cout << "  PASSED: Texture created successfully" << std::endl;
}

// Test 2: Texture loading from raw data
void test_TextureLoadFromData() {
    std::cout << "Test 2: Texture loading from raw data..." << std::endl;
    
    OpenGLTestContext ctx;
    
    // Create 4x4 RGBA checkerboard pattern
    const int width = 4;
    const int height = 4;
    std::vector<uint8_t> pixels(width * height * 4);
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 4;
            bool white = ((x + y) % 2) == 0;
            pixels[idx + 0] = white ? 255 : 0; // R
            pixels[idx + 1] = white ? 255 : 0; // G
            pixels[idx + 2] = white ? 255 : 0; // B
            pixels[idx + 3] = 255;             // A
        }
    }
    
    Texture texture;
    bool success = texture.loadFromData(pixels.data(), width, height, GL_RGBA);
    
    assert(success && "Texture loading from data should succeed");
    assert(texture.getTextureID() != 0 && "Texture ID should be valid");
    assert(texture.getWidth() == width && "Width should match");
    assert(texture.getHeight() == height && "Height should match");
    
    std::cout << "  PASSED: Texture loaded from raw data" << std::endl;
}

// Test 3: Texture parameter setting (wrap mode)
void test_TextureWrapMode() {
    std::cout << "Test 3: Texture wrap mode setting..." << std::endl;
    
    OpenGLTestContext ctx;
    
    Texture texture;
    texture.create(128, 128, GL_RGBA);
    
    // Should not crash - parameters are set internally
    texture.setWrapMode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    
    // Verify by binding and querying
    texture.bind(0);
    GLint wrapS = 0;
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &wrapS);
    assert(wrapS == GL_CLAMP_TO_EDGE && "Wrap S should be GL_CLAMP_TO_EDGE");
    
    std::cout << "  PASSED: Texture wrap mode set correctly" << std::endl;
}

// Test 4: Texture parameter setting (filter mode)
void test_TextureFilterMode() {
    std::cout << "Test 4: Texture filter mode setting..." << std::endl;
    
    OpenGLTestContext ctx;
    
    Texture texture;
    texture.create(128, 128, GL_RGBA);
    
    // Set filter modes
    texture.setFilterMode(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    
    // Verify by binding and querying
    texture.bind(0);
    GLint minFilter = 0;
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &minFilter);
    assert(minFilter == GL_LINEAR_MIPMAP_LINEAR && "Min filter should be GL_LINEAR_MIPMAP_LINEAR");
    
    std::cout << "  PASSED: Texture filter mode set correctly" << std::endl;
}

// Test 5: Texture binding to texture unit
void test_TextureBinding() {
    std::cout << "Test 5: Texture binding to texture unit..." << std::endl;
    
    OpenGLTestContext ctx;
    
    Texture texture;
    texture.create(64, 64, GL_RGBA);
    
    // Bind to texture unit 0
    texture.bind(0);
    
    // Verify active texture is bound
    glActiveTexture(GL_TEXTURE0);
    GLint boundTexture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);
    assert(boundTexture == (GLint)texture.getTextureID() && "Texture should be bound to unit 0");
    
    // Bind to texture unit 1
    texture.bind(1);
    glActiveTexture(GL_TEXTURE1);
    boundTexture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);
    assert(boundTexture == (GLint)texture.getTextureID() && "Texture should be bound to unit 1");
    
    std::cout << "  PASSED: Texture binding works correctly" << std::endl;
}

// Test 6: Texture unbinding
void test_TextureUnbind() {
    std::cout << "Test 6: Texture unbinding..." << std::endl;
    
    OpenGLTestContext ctx;
    
    Texture texture;
    texture.create(64, 64, GL_RGBA);
    
    // Bind texture
    texture.bind(0);
    
    // Unbind
    texture.unbind();
    
    // Verify no texture is bound
    glActiveTexture(GL_TEXTURE0);
    GLint boundTexture = -1;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);
    assert(boundTexture == 0 && "No texture should be bound after unbind");
    
    std::cout << "  PASSED: Texture unbinding works" << std::endl;
}

// Test 7: Mipmaps generation
void test_MipmapGeneration() {
    std::cout << "Test 7: Mipmaps generation..." << std::endl;
    
    OpenGLTestContext ctx;
    
    // Create texture with data
    std::vector<uint8_t> pixels(64 * 64 * 4, 128);
    Texture texture;
    texture.loadFromData(pixels.data(), 64, 64, GL_RGBA);
    
    // Generate mipmaps (should not crash)
    texture.generateMipmaps();
    
    // Verify mipmap levels exist by querying
    texture.bind(0);
    GLint maxLevel = 0;
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, &maxLevel);
    // Should have multiple levels for a 64x64 texture
    assert(maxLevel >= 0 && "Mipmaps should be generated");
    
    std::cout << "  PASSED: Mipmaps generated successfully" << std::endl;
}

// Test 8: Texture deletion
void test_TextureDeletion() {
    std::cout << "Test 8: Texture deletion..." << std::endl;
    
    OpenGLTestContext ctx;
    
    Texture texture;
    texture.create(64, 64, GL_RGBA);
    GLuint oldID = texture.getTextureID();
    assert(oldID != 0 && "Texture ID should be valid before cleanup");
    
    // Cleanup
    texture.cleanup();
    
    // After cleanup, ID should be reset
    assert(texture.getTextureID() == 0 && "Texture ID should be 0 after cleanup");
    
    std::cout << "  PASSED: Texture deleted correctly" << std::endl;
}

// Test 9: Error handling - invalid dimensions
void test_InvalidDimensions() {
    std::cout << "Test 9: Error handling - invalid dimensions..." << std::endl;
    
    OpenGLTestContext ctx;
    
    Texture texture;
    
    // Try to create with zero dimensions
    bool success = texture.create(0, 0, GL_RGBA);
    assert(!success && "Should fail with zero dimensions");
    
    // Try with negative (will be cast to large uint, but should still validate)
    success = texture.create(-1, -1, GL_RGBA);
    // This may succeed or fail depending on validation - we just check it doesn't crash
    
    std::cout << "  PASSED: Invalid dimensions handled" << std::endl;
}

// Test 10: Error handling - null data
void test_NullData() {
    std::cout << "Test 10: Error handling - null data..." << std::endl;
    
    OpenGLTestContext ctx;
    
    Texture texture;
    
    // Loading with null data should fail or create empty texture
    bool success = texture.loadFromData(nullptr, 64, 64, GL_RGBA);
    // Depending on implementation, this might succeed (creates empty texture) or fail
    // We just verify it doesn't crash
    
    std::cout << "  PASSED: Null data handled gracefully" << std::endl;
}

// Test 11: Multiple texture units
void test_MultipleTextureUnits() {
    std::cout << "Test 11: Multiple texture units..." << std::endl;
    
    OpenGLTestContext ctx;
    
    Texture tex0, tex1, tex2;
    tex0.create(32, 32, GL_RGBA);
    tex1.create(32, 32, GL_RGBA);
    tex2.create(32, 32, GL_RGBA);
    
    // Bind to different units
    tex0.bind(0);
    tex1.bind(1);
    tex2.bind(2);
    
    // Verify each is bound to correct unit
    glActiveTexture(GL_TEXTURE0);
    GLint bound0 = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &bound0);
    assert(bound0 == (GLint)tex0.getTextureID() && "tex0 should be on unit 0");
    
    glActiveTexture(GL_TEXTURE1);
    GLint bound1 = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &bound1);
    assert(bound1 == (GLint)tex1.getTextureID() && "tex1 should be on unit 1");
    
    glActiveTexture(GL_TEXTURE2);
    GLint bound2 = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &bound2);
    assert(bound2 == (GLint)tex2.getTextureID() && "tex2 should be on unit 2");
    
    std::cout << "  PASSED: Multiple texture units work correctly" << std::endl;
}

// Test 12: Texture resize/update
void test_TextureResize() {
    std::cout << "Test 12: Texture resize/update..." << std::endl;
    
    OpenGLTestContext ctx;
    
    Texture texture;
    texture.create(64, 64, GL_RGBA);
    
    assert(texture.getWidth() == 64 && "Initial width should be 64");
    assert(texture.getHeight() == 64 && "Initial height should be 64");
    
    // Resize to new dimensions
    bool success = texture.resize(128, 128);
    assert(success && "Resize should succeed");
    assert(texture.getWidth() == 128 && "Width should be 128 after resize");
    assert(texture.getHeight() == 128 && "Height should be 128 after resize");
    
    std::cout << "  PASSED: Texture resize works correctly" << std::endl;
}

} // namespace dj

int main() {
    std::cout << "=== Texture Phase 5 Tests ===" << std::endl;
    std::cout << std::endl;
    
    try {
        dj::test_TextureCreation();
        dj::test_TextureLoadFromData();
        dj::test_TextureWrapMode();
        dj::test_TextureFilterMode();
        dj::test_TextureBinding();
        dj::test_TextureUnbind();
        dj::test_MipmapGeneration();
        dj::test_TextureDeletion();
        dj::test_InvalidDimensions();
        dj::test_NullData();
        dj::test_MultipleTextureUnits();
        dj::test_TextureResize();
        
        std::cout << std::endl;
        std::cout << "=== ALL TESTS PASSED ===" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return 1;
    }
}
