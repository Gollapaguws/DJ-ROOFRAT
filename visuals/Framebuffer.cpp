#include "visuals/Framebuffer.h"
#include <iostream>
#include <stdexcept>

namespace dj {

#if defined(DJROOFRAT_ENABLE_GRAPHICS) && defined(DJROOFRAT_OPENGL_MIGRATION)

Framebuffer::Framebuffer(int width, int height) : width_(width), height_(height) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Framebuffer dimensions must be positive");
    }
    
    glGenFramebuffers(1, &fbo_);
    if (fbo_ == 0) {
        throw std::runtime_error("Failed to generate framebuffer object");
    }
}

Framebuffer::~Framebuffer() {
    if (fbo_ != 0) {
        glDeleteFramebuffers(1, &fbo_);
        fbo_ = 0;
    }
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept 
    : fbo_(other.fbo_), width_(other.width_), height_(other.height_) {
    other.fbo_ = 0;
    other.width_ = 0;
    other.height_ = 0;
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept {
    if (this != &other) {
        if (fbo_ != 0) {
            glDeleteFramebuffers(1, &fbo_);
        }
        fbo_ = other.fbo_;
        width_ = other.width_;
        height_ = other.height_;
        
        other.fbo_ = 0;
        other.width_ = 0;
        other.height_ = 0;
    }
    return *this;
}

void Framebuffer::bind() {
    if (fbo_ == 0) {
        throw std::runtime_error("Cannot bind invalid framebuffer");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
}

void Framebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::attachColorTexture(GLuint texture, int colorIndex) {
    if (fbo_ == 0) {
        throw std::runtime_error("Cannot attach to invalid framebuffer");
    }
    if (texture == 0) {
        throw std::invalid_argument("Texture handle cannot be zero");
    }
    if (colorIndex < 0 || colorIndex > 7) {
        throw std::invalid_argument("Color index must be between 0-7");
    }
    
    GLenum attachment = GL_COLOR_ATTACHMENT0 + colorIndex;
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, 0);
}

void Framebuffer::attachDepthTexture(GLuint texture) {
    if (fbo_ == 0) {
        throw std::runtime_error("Cannot attach to invalid framebuffer");
    }
    if (texture == 0) {
        throw std::invalid_argument("Texture handle cannot be zero");
    }
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
}

void Framebuffer::attachStencilTexture(GLuint texture) {
    if (fbo_ == 0) {
        throw std::runtime_error("Cannot attach to invalid framebuffer");
    }
    if (texture == 0) {
        throw std::invalid_argument("Texture handle cannot be zero");
    }
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
}

void Framebuffer::setDrawBuffers(int count, const GLenum* buffers) {
    if (fbo_ == 0) {
        throw std::runtime_error("Cannot set draw buffers on invalid framebuffer");
    }
    if (count <= 0 || count > 8) {
        throw std::invalid_argument("Draw buffer count must be between 1-8");
    }
    if (buffers == nullptr) {
        throw std::invalid_argument("Buffers pointer cannot be null");
    }
    
    glDrawBuffers(count, buffers);
}

bool Framebuffer::isComplete() const {
    if (fbo_ == 0) {
        return false;
    }
    
    return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

GLenum Framebuffer::getFramebufferStatus() const {
    if (fbo_ == 0) {
        return GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
    }
    
    return glCheckFramebufferStatus(GL_FRAMEBUFFER);
}

#endif

}  // namespace dj
