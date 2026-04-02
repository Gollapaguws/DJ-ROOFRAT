#pragma once

// Must include glad FIRST before any system GL headers
#ifdef DJROOFRAT_ENABLE_GRAPHICS
#include <glad/glad.h>
#endif

namespace dj {

#if defined(DJROOFRAT_ENABLE_GRAPHICS) && defined(DJROOFRAT_OPENGL_MIGRATION)

/// Framebuffer Object (FBO) for off-screen rendering
/// Supports color and depth attachments for post-processing effects
class Framebuffer {
public:
    /// Constructor - creates framebuffer with specified dimensions
    explicit Framebuffer(int width, int height);
    
    /// Destructor - cleans up FBO resources
    ~Framebuffer();
    
    // Rule of Five - move semantics
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    
    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(Framebuffer&& other) noexcept;
    
    /// Bind this framebuffer for rendering
    void bind();
    
    /// Unbind framebuffer (restore default framebuffer)
    void unbind();
    
    /// Attach color texture to this FBO
    /// @param texture OpenGL texture handle
    /// @param colorIndex Attachment index (GL_COLOR_ATTACHMENT0 + colorIndex)
    void attachColorTexture(GLuint texture, int colorIndex);
    
    /// Attach depth texture to this FBO
    /// @param texture OpenGL depth texture handle
    void attachDepthTexture(GLuint texture);
    
    /// Attach stencil texture to this FBO
    /// @param texture OpenGL stencil texture handle
    void attachStencilTexture(GLuint texture);
    
    /// Set draw buffers for multiple render targets (MRT)
    /// @param count Number of draw buffers
    /// @param buffers Array of draw buffer attachments
    void setDrawBuffers(int count, const GLenum* buffers);
    
    /// Check if framebuffer is complete
    /// @return true if framebuffer is complete and ready for rendering
    bool isComplete() const;
    
    /// Get framebuffer object handle
    GLuint getHandle() const { return fbo_; }
    
    /// Get framebuffer width
    int getWidth() const { return width_; }
    
    /// Get framebuffer height
    int getHeight() const { return height_; }
    
    /// Get OpenGL error if framebuffer is incomplete
    GLenum getFramebufferStatus() const;
    
private:
    GLuint fbo_ = 0;
    int width_ = 0;
    int height_ = 0;
};

#else

/// Stub implementation when graphics is disabled
class Framebuffer {
public:
    explicit Framebuffer(int, int) {}
    ~Framebuffer() = default;
    
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    
    Framebuffer(Framebuffer&&) noexcept = default;
    Framebuffer& operator=(Framebuffer&&) noexcept = default;
    
    void bind() {}
    void unbind() {}
    void attachColorTexture(unsigned int, int) {}
    void attachDepthTexture(unsigned int) {}
    void attachStencilTexture(unsigned int) {}
    void setDrawBuffers(int, const unsigned int*) {}
    bool isComplete() const { return false; }
    unsigned int getHandle() const { return 0; }
    int getWidth() const { return 0; }
    int getHeight() const { return 0; }
    unsigned int getFramebufferStatus() const { return 0; }
};

#endif

}  // namespace dj
