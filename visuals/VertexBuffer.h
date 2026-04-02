#pragma once

#include <cstdint>

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

class VertexBuffer {
public:
    VertexBuffer();
    ~VertexBuffer();

    // Create vertex buffer from vertex data
    // size: total size in bytes
    // usage: GL_STATIC_DRAW, GL_DYNAMIC_DRAW, GL_STREAM_DRAW
    bool create(const void* vertices, uint32_t size, GLenum usage = 0x88E4); // GL_STATIC_DRAW

    // DirectX-compatible API (for gradual migration)
    // device: ignored (not needed in OpenGL)
    // vertices: pointer to vertex data
    // count: number of vertices
    // stride: size of each vertex in bytes
    bool create(void* device, const void* vertices, uint32_t count, uint32_t stride);

    // Update buffer data (requires GL_DYNAMIC_DRAW or GL_STREAM_DRAW)
    // offset: offset in bytes
    // size: size in bytes
    bool update(const void* data, uint32_t size, uint32_t offset = 0);

    // Bind buffer for rendering
    bool bind() const;

    // DirectX-compatible bind (ignores parameters)
    void bind(void* context, uint32_t slot) const;
    
    // Unbind buffer
    void unbind() const;

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Get OpenGL VBO handle
    GLuint getVBO() const { return vbo_; }
#else
    GLuint getVBO() const { return 0; }
#endif

    // Get vertex count
    uint32_t getVertexCount() const { return vertexCount_; }
    uint32_t getVertexSize() const { return vertexSize_; }

private:
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    GLuint vbo_ = 0;
#endif
    uint32_t vertexCount_ = 0;
    uint32_t vertexSize_ = 0;
    uint32_t bufferSize_ = 0;
};

} // namespace dj

