#pragma once

#include <cstdint>

#if defined(_WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #ifndef NOGDI
        #define NOGDI
    #endif
#endif

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <glad/glad.h>
namespace dj {
#else
namespace dj {
// Forward declarations for non-graphics builds
typedef unsigned int GLuint;
typedef unsigned int GLenum;
constexpr GLenum GL_STATIC_DRAW = 0x88E4;
constexpr GLenum GL_DYNAMIC_DRAW = 0x88E8;
constexpr GLenum GL_STREAM_DRAW = 0x88E0;
#endif

class IndexBuffer {
public:
    IndexBuffer();
    ~IndexBuffer();

    // Create index buffer from index data
    // indexCount: number of indices (not size in bytes)
    // usage: GL_STATIC_DRAW, GL_DYNAMIC_DRAW, GL_STREAM_DRAW
    bool create(const uint32_t* indices, uint32_t indexCount, GLenum usage = 0x88E4); // GL_STATIC_DRAW

    // DirectX-compatible API (for gradual migration)
    // device: ignored (not needed in OpenGL)
    // indices: pointer to index data
    // count: number of indices
    bool create(void* device, const uint32_t* indices, uint32_t count);

    // Update buffer data (requires GL_DYNAMIC_DRAW or GL_STREAM_DRAW)
    // offset: offset in indices (not bytes)
    bool update(const uint32_t* data, uint32_t count, uint32_t offset = 0);

    // Bind buffer for rendering
    bool bind() const;

    // DirectX-compatible bind overloads (ignores parameters)
    void bind(void* context) const;
    void bind(void* context, uint32_t slot) const;

    // Unbind buffer
    void unbind() const;

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Get OpenGL EBO handle
    GLuint getEBO() const { return ebo_; }
#else
    GLuint getEBO() const { return 0; }
#endif

    // Get index count
    uint32_t getIndexCount() const { return indexCount_; }

private:
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    GLuint ebo_ = 0;
#endif
    uint32_t indexCount_ = 0;
    uint32_t bufferSize_ = 0;
};

} // namespace dj

