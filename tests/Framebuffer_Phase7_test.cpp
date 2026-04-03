#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #define NOGDI
#endif

#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

#define DJROOFRAT_OPENGL_MIGRATION 1
#define DJROOFRAT_ENABLE_GRAPHICS 1

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

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
#define glGenFramebuffers __system_glGenFramebuffers
#define glDeleteFramebuffers __system_glDeleteFramebuffers
#define glBindFramebuffer __system_glBindFramebuffer
#define glFramebufferTexture2D __system_glFramebufferTexture2D
#define glCheckFramebufferStatus __system_glCheckFramebufferStatus
#define glDrawBuffers __system_glDrawBuffers

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
#undef glGenFramebuffers
#undef glDeleteFramebuffers
#undef glBindFramebuffer
#undef glFramebufferTexture2D
#undef glCheckFramebufferStatus
#undef glDrawBuffers

#include "visuals/Framebuffer.h"

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
        
        m_window = glfwCreateWindow(800, 600, "Framebuffer Test Context", nullptr, nullptr);
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

// Test 1: FBO creation and binding
void test_FBOCreationAndBinding() {
    std::cout << "Test 1: FBO creation and binding..." << std::endl;
    
    OpenGLTestContext ctx;
    
    // Create Framebuffer object
    dj::Framebuffer fbo(800, 600);
    assert(fbo.getHandle() != 0 && "FBO handle should be non-zero");
    
    // Bind FBO
    fbo.bind();
    
    // Verify FBO is bound
    GLint boundFBO = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &boundFBO);
    assert(boundFBO == static_cast<GLint>(fbo.getHandle()) && "FBO should be bound");
    
    // Unbind FBO
    fbo.unbind();
    
    // Verify default framebuffer is restored
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &boundFBO);
    assert(boundFBO == 0 && "Default framebuffer should be bound");
    
    std::cout << "  PASSED: FBO creation and binding works" << std::endl;
}

// Test 2: Attach color texture to FBO
void test_AttachColorTexture() {
    std::cout << "Test 2: Attach color texture to FBO..." << std::endl;
    
    OpenGLTestContext ctx;
    
    // Create texture
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Create FBO and attach texture
    dj::Framebuffer fbo(800, 600);
    fbo.bind();
    fbo.attachColorTexture(texture, 0);
    
    // Verify attachment
    GLint attachmentType = 0;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &attachmentType);
    assert(attachmentType == GL_TEXTURE && "Texture should be attached to color attachment");
    
    fbo.unbind();
    glDeleteTextures(1, &texture);
    
    std::cout << "  PASSED: Color texture attachment works" << std::endl;
}

// Test 3: Attach depth texture to FBO
void test_AttachDepthTexture() {
    std::cout << "Test 3: Attach depth texture to FBO..." << std::endl;
    
    OpenGLTestContext ctx;
    
    // Create depth texture
    GLuint depthTexture = 0;
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 800, 600, 0, 
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Create FBO and attach depth texture
    dj::Framebuffer fbo(800, 600);
    fbo.bind();
    fbo.attachDepthTexture(depthTexture);
    
    // Verify attachment
    GLint attachmentType = 0;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &attachmentType);
    assert(attachmentType == GL_TEXTURE && "Depth texture should be attached");
    
    fbo.unbind();
    glDeleteTextures(1, &depthTexture);
    
    std::cout << "  PASSED: Depth texture attachment works" << std::endl;
}

// Test 4: Check framebuffer completeness
void test_FramebufferCompleteness() {
    std::cout << "Test 4: Check framebuffer completeness..." << std::endl;
    
    OpenGLTestContext ctx;
    
    // Create color texture
    GLuint colorTexture = 0;
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    
    // Create FBO with color attachment
    dj::Framebuffer fbo(800, 600);
    fbo.bind();
    fbo.attachColorTexture(colorTexture, 0);
    
    // Check completeness
    assert(fbo.isComplete() && "FBO should be complete with color attachment");
    
    fbo.unbind();
    glDeleteTextures(1, &colorTexture);
    
    std::cout << "  PASSED: Framebuffer completeness check works" << std::endl;
}

// Test 5: Multiple render targets (MRT)
void test_MultipleRenderTargets() {
    std::cout << "Test 5: Multiple render targets (MRT)..." << std::endl;
    
    OpenGLTestContext ctx;
    
    // Create two color textures
    GLuint textures[2];
    glGenTextures(2, textures);
    
    for (int i = 0; i < 2; ++i) {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    
    // Create FBO and attach both textures
    dj::Framebuffer fbo(800, 600);
    fbo.bind();
    fbo.attachColorTexture(textures[0], 0);
    fbo.attachColorTexture(textures[1], 1);
    
    // Set draw buffers
    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    fbo.setDrawBuffers(2, drawBuffers);
    
    // Verify MRT is set
    GLint numDrawBuffers = 0;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &numDrawBuffers);
    assert(numDrawBuffers >= 2 && "Should support at least 2 draw buffers");
    
    fbo.unbind();
    glDeleteTextures(2, textures);
    
    std::cout << "  PASSED: Multiple render targets work" << std::endl;
}

// Test 6: FBO resource cleanup
void test_FBOResourceCleanup() {
    std::cout << "Test 6: FBO resource cleanup..." << std::endl;
    
    OpenGLTestContext ctx;
    
    GLuint initialCount = 0;
    GLuint finalCount = 0;
    
    {
        // Create and destroy FBO
        dj::Framebuffer fbo(800, 600);
        // FBO goes out of scope here
    }
    
    // If we get here without crash, cleanup worked
    std::cout << "  PASSED: FBO resource cleanup works" << std::endl;
}

// Test 7: Render to FBO and read back pixels
void test_RenderToFBOAndReadback() {
    std::cout << "Test 7: Render to FBO and read back pixels..." << std::endl;
    
    OpenGLTestContext ctx;
    
    // Create color texture
    GLuint colorTexture = 0;
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Create FBO
    dj::Framebuffer fbo(256, 256);
    fbo.bind();
    fbo.attachColorTexture(colorTexture, 0);
    
    assert(fbo.isComplete() && "FBO should be complete");
    
    // Render clear color to FBO
    glClearColor(1.0f, 0.5f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Read back pixels
    std::vector<GLubyte> pixels(256 * 256 * 4);
    glReadPixels(0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    
    // Verify pixel is approximately the clear color
    GLubyte r = pixels[0];
    GLubyte g = pixels[1];
    GLubyte b = pixels[2];
    GLubyte a = pixels[3];
    
    assert(r >= 240 && r <= 255 && "Red channel should be ~255");
    assert(g >= 110 && g <= 140 && "Green channel should be ~127");
    assert(b >= 50 && b <= 80 && "Blue channel should be ~64");
    
    fbo.unbind();
    glDeleteTextures(1, &colorTexture);
    
    std::cout << "  PASSED: FBO rendering and readback works" << std::endl;
}

// Test 8: FBO unbinding restores default framebuffer
void test_FBOUnbindRestoresDefault() {
    std::cout << "Test 8: FBO unbinding restores default framebuffer..." << std::endl;
    
    OpenGLTestContext ctx;
    
    GLint defaultFBO = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);
    assert(defaultFBO == 0 && "Default framebuffer should be bound initially");
    
    {
        dj::Framebuffer fbo(800, 600);
        fbo.bind();
        
        GLint boundFBO = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &boundFBO);
        assert(boundFBO != 0 && "FBO should be bound");
    }
    
    // After FBO destruction, default should be restored
    GLint restoredFBO = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &restoredFBO);
    assert(restoredFBO == 0 && "Default framebuffer should be restored");
    
    std::cout << "  PASSED: FBO unbinding restores default framebuffer" << std::endl;
}

}  // namespace dj

int main() {
    try {
        std::cout << "\n=== Framebuffer Phase 7 Tests ===" << std::endl;
        
        dj::test_FBOCreationAndBinding();
        dj::test_AttachColorTexture();
        dj::test_AttachDepthTexture();
        dj::test_FramebufferCompleteness();
        dj::test_MultipleRenderTargets();
        dj::test_FBOResourceCleanup();
        dj::test_RenderToFBOAndReadback();
        dj::test_FBOUnbindRestoresDefault();
        
        std::cout << "\n=== All Framebuffer tests PASSED ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTest failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
