#include "Texture.h"

#include <iostream>

namespace dj {

Texture::Texture() = default;

Texture::~Texture() {
    cleanup();
}

bool Texture::create(uint32_t width, uint32_t height, GLenum format) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Validate dimensions
    if (width == 0 || height == 0) {
        std::cerr << "Texture::create: Invalid dimensions (" << width << "x" << height << ")" << std::endl;
        return false;
    }

    // Clean up existing texture if any
    cleanup();

    // Store dimensions and format
    width_ = width;
    height_ = height;
    format_ = format;
    internalFormat_ = format; // For most cases, internal format matches format
    dataType_ = GL_UNSIGNED_BYTE;

    // Generate texture
    glGenTextures(1, &textureID_);
    if (textureID_ == 0) {
        std::cerr << "Texture::create: glGenTextures failed" << std::endl;
        return false;
    }

    // Bind texture to configure it
    glBindTexture(GL_TEXTURE_2D, textureID_);

    // Allocate texture storage (data = nullptr creates empty texture)
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat_, width_, height_, 0, 
                 format_, dataType_, nullptr);

    // Set default texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Unbind
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
#else
    (void)width;
    (void)height;
    (void)format;
    return false;
#endif
}

bool Texture::loadFromData(const void* data, uint32_t width, uint32_t height, GLenum format) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Validate dimensions
    if (width == 0 || height == 0) {
        std::cerr << "Texture::loadFromData: Invalid dimensions (" << width << "x" << height << ")" << std::endl;
        return false;
    }

    // Clean up existing texture
    cleanup();

    // Store dimensions and format
    width_ = width;
    height_ = height;
    format_ = format;
    internalFormat_ = format;
    dataType_ = GL_UNSIGNED_BYTE;

    // Generate texture
    glGenTextures(1, &textureID_);
    if (textureID_ == 0) {
        std::cerr << "Texture::loadFromData: glGenTextures failed" << std::endl;
        return false;
    }

    // Bind and upload data
    glBindTexture(GL_TEXTURE_2D, textureID_);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat_, width_, height_, 0, 
                 format_, dataType_, data);

    // Set default texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Unbind
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
#else
    (void)data;
    (void)width;
    (void)height;
    (void)format;
    return false;
#endif
}

void Texture::setWrapMode(GLenum wrapS, GLenum wrapT) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (textureID_ == 0) {
        std::cerr << "Texture::setWrapMode: No texture created" << std::endl;
        return;
    }

    // Bind texture to set parameters
    glBindTexture(GL_TEXTURE_2D, textureID_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    glBindTexture(GL_TEXTURE_2D, 0);
#else
    (void)wrapS;
    (void)wrapT;
#endif
}

void Texture::setFilterMode(GLenum minFilter, GLenum magFilter) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (textureID_ == 0) {
        std::cerr << "Texture::setFilterMode: No texture created" << std::endl;
        return;
    }

    // Bind texture to set parameters
    glBindTexture(GL_TEXTURE_2D, textureID_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glBindTexture(GL_TEXTURE_2D, 0);
#else
    (void)minFilter;
    (void)magFilter;
#endif
}

void Texture::bind(uint32_t textureUnit) const {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (textureID_ == 0) {
        std::cerr << "Texture::bind: No texture to bind" << std::endl;
        return;
    }

    // Activate texture unit and bind texture
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureID_);
#else
    (void)textureUnit;
#endif
}

void Texture::unbind() const {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Unbind texture from GL_TEXTURE_2D target on current active unit
    glBindTexture(GL_TEXTURE_2D, 0);
#endif
}

void Texture::generateMipmaps() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (textureID_ == 0) {
        std::cerr << "Texture::generateMipmaps: No texture created" << std::endl;
        return;
    }

    // Bind texture and generate mipmaps
    glBindTexture(GL_TEXTURE_2D, textureID_);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
#endif
}

bool Texture::resize(uint32_t newWidth, uint32_t newHeight) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Validate dimensions
    if (newWidth == 0 || newHeight == 0) {
        std::cerr << "Texture::resize: Invalid dimensions (" << newWidth << "x" << newHeight << ")" << std::endl;
        return false;
    }

    if (textureID_ == 0) {
        std::cerr << "Texture::resize: No texture created" << std::endl;
        return false;
    }

    // Update dimensions
    width_ = newWidth;
    height_ = newHeight;

    // Bind and resize (reallocates storage, discards old data)
    glBindTexture(GL_TEXTURE_2D, textureID_);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat_, width_, height_, 0, 
                 format_, dataType_, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
#else
    (void)newWidth;
    (void)newHeight;
    return false;
#endif
}

bool Texture::updateSubImage(uint32_t xoffset, uint32_t yoffset, 
                            uint32_t width, uint32_t height, 
                            const void* data) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (textureID_ == 0) {
        std::cerr << "Texture::updateSubImage: No texture created" << std::endl;
        return false;
    }

    if (data == nullptr) {
        std::cerr << "Texture::updateSubImage: Null data pointer" << std::endl;
        return false;
    }

    // Validate region bounds
    if (xoffset + width > width_ || yoffset + height > height_) {
        std::cerr << "Texture::updateSubImage: Region out of bounds" << std::endl;
        return false;
    }

    // Bind and update sub-region
    glBindTexture(GL_TEXTURE_2D, textureID_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, 
                    format_, dataType_, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
#else
    (void)xoffset;
    (void)yoffset;
    (void)width;
    (void)height;
    (void)data;
    return false;
#endif
}

void Texture::cleanup() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (textureID_ != 0) {
        glDeleteTextures(1, &textureID_);
        textureID_ = 0;
    }
    width_ = 0;
    height_ = 0;
#endif
}

} // namespace dj
