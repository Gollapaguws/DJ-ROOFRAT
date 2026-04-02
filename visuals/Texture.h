#pragma once

#include <cstdint>

// Must include glad FIRST before any system GL headers
#ifdef DJROOFRAT_ENABLE_GRAPHICS
#include <glad/glad.h>
#endif

// Stub definitions when graphics is disabled
#ifndef DJROOFRAT_ENABLE_GRAPHICS
using GLuint = unsigned int;
using GLenum = unsigned int;
constexpr GLenum GL_RGBA = 0x1908;
constexpr GLenum GL_RGB = 0x1907;
constexpr GLenum GL_RED = 0x1903;
constexpr GLenum GL_UNSIGNED_BYTE = 0x1401;
constexpr GLenum GL_TEXTURE_2D = 0x0DE1;
constexpr GLenum GL_REPEAT = 0x2901;
constexpr GLenum GL_CLAMP_TO_EDGE = 0x812F;
constexpr GLenum GL_LINEAR = 0x2601;
constexpr GLenum GL_NEAREST = 0x2600;
constexpr GLenum GL_LINEAR_MIPMAP_LINEAR = 0x2703;
#endif

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS) && !defined(DJROOFRAT_OPENGL_MIGRATION)
// DirectX 11 headers (legacy support)
#include <d3d11.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;
#endif

namespace dj {

class Texture {
public:
    Texture();
    ~Texture();
    
    // Rule of Five: Prevent accidental copies, allow moves
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    // ========== OpenGL Methods (Phase 5+) ==========
    
    // Create an empty texture with given dimensions and format
    // width, height: texture dimensions in pixels
    // format: GL_RGBA, GL_RGB, GL_RED, etc.
    // Returns true if successful
    bool create(uint32_t width, uint32_t height, GLenum format = 0x1908); // GL_RGBA

    // Load texture from raw pixel data
    // data: pointer to pixel data (can be nullptr to allocate empty texture)
    // width, height: texture dimensions
    // format: pixel format (GL_RGBA, GL_RGB, etc.)
    // Returns true if successful
    bool loadFromData(const void* data, uint32_t width, uint32_t height, GLenum format = 0x1908);

    // Set texture wrap mode (how texture coordinates outside [0,1] are handled)
    // wrapS: wrap mode for S coordinate (GL_REPEAT, GL_CLAMP_TO_EDGE, etc.)
    // wrapT: wrap mode for T coordinate
    void setWrapMode(GLenum wrapS, GLenum wrapT);

    // Set texture filtering mode
    // minFilter: minification filter (GL_LINEAR, GL_NEAREST, GL_LINEAR_MIPMAP_LINEAR, etc.)
    // magFilter: magnification filter (GL_LINEAR, GL_NEAREST)
    void setFilterMode(GLenum minFilter, GLenum magFilter);

    // Bind texture to a texture unit
    // textureUnit: texture unit index (0-31 typically)
    void bind(uint32_t textureUnit = 0) const;

    // Unbind texture (binds 0 to GL_TEXTURE_2D on current active unit)
    void unbind() const;

    // Generate mipmaps for this texture
    // Should be called after loading texture data
    void generateMipmaps();

    // Resize texture to new dimensions
    // Preserves format but discards old data
    bool resize(uint32_t newWidth, uint32_t newHeight);

    // Update a region of the texture
    // xoffset, yoffset: offset in pixels
    // width, height: size of the region to update
    // data: new pixel data
    bool updateSubImage(uint32_t xoffset, uint32_t yoffset, 
                       uint32_t width, uint32_t height, 
                       const void* data);

    // Release texture resources
    void cleanup();

    // Get OpenGL texture ID
    GLuint getTextureID() const { 
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
        return textureID_; 
#else
        return 0;
#endif
    }

    // Get texture dimensions
    uint32_t getWidth() const { return width_; }
    uint32_t getHeight() const { return height_; }
    GLenum getFormat() const { return format_; }

    // ========== Legacy DirectX Methods (for backward compatibility) ==========
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS) && !defined(DJROOFRAT_OPENGL_MIGRATION)
    // Create DirectX 11 texture
    bool createD3D11(ID3D11Device* device, uint32_t width, uint32_t height, 
                     DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);

    // Load from data (DirectX)
    bool loadFromDataD3D11(ID3D11Device* device, const void* data, 
                           uint32_t width, uint32_t height, 
                           DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);

    // Get DirectX texture interface
    ID3D11Texture2D* getD3D11Texture() const { return texture_.Get(); }
    ID3D11ShaderResourceView* getD3D11SRV() const { return textureSRV_.Get(); }

private:
    ComPtr<ID3D11Texture2D> texture_;
    ComPtr<ID3D11ShaderResourceView> textureSRV_;
#endif

private:
    // OpenGL members (Phase 5+)
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    GLuint textureID_ = 0;
#endif
    uint32_t width_ = 0;
    uint32_t height_ = 0;
    GLenum format_ = 0x1908; // GL_RGBA default
    GLenum internalFormat_ = 0x1908; // GL_RGBA default
    GLenum dataType_ = 0x1401; // GL_UNSIGNED_BYTE default
};

} // namespace dj
