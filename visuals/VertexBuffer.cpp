#include "visuals/VertexBuffer.h"

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <glad/glad.h>
#endif

namespace dj {

VertexBuffer::VertexBuffer() = default;

VertexBuffer::~VertexBuffer() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (vbo_ != 0) {
        glDeleteBuffers(1, &vbo_);
        vbo_ = 0;
    }
#endif
}

bool VertexBuffer::create(const void* vertices, uint32_t size, GLenum usage) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!vertices || size == 0) {
        return false;
    }

    // Clean up old buffer if exists
    if (vbo_ != 0) {
        glDeleteBuffers(1, &vbo_);
        vbo_ = 0;
    }

    // Generate a VBO handle
    glGenBuffers(1, &vbo_);
    if (vbo_ == 0) {
        return false;
    }

    // Bind the buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    // Upload data to GPU
    glBufferData(GL_ARRAY_BUFFER, size, vertices, usage);

    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    bufferSize_ = size;
    vertexCount_ = 0;  // Not known in this API
    vertexSize_ = 0;   // Not known in this API
    return true;
#else
    (void)vertices;
    (void)size;
    (void)usage;
    return false;
#endif
}

bool VertexBuffer::update(const void* data, uint32_t size, uint32_t offset) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!data || size == 0 || vbo_ == 0) {
        return false;
    }

    // Check that offset + size doesn't exceed buffer
    if (offset + size > bufferSize_) {
        return false;
    }

    // Bind and update
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return true;
#else
    (void)data;
    (void)size;
    (void)offset;
    return false;
#endif
}

bool VertexBuffer::bind() const {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (vbo_ == 0) {
        return false;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    return true;
#else
    return false;
#endif
}

void VertexBuffer::unbind() const {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
}

// DirectX-compatible overload
bool VertexBuffer::create(void* device, const void* vertices, uint32_t count, uint32_t stride) {
    // Ignore device parameter (not needed in OpenGL)
    (void)device;

    if (count == 0 || stride == 0 || !vertices) {
        return false;
    }

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Clean up old buffer if exists
    if (vbo_ != 0) {
        glDeleteBuffers(1, &vbo_);
        vbo_ = 0;
    }

    size_t size = static_cast<size_t>(count) * stride;
    
    // Generate a VBO handle
    glGenBuffers(1, &vbo_);
    if (vbo_ == 0) {
        return false;
    }

    // Bind the buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    // Upload data to GPU
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    bufferSize_ = static_cast<uint32_t>(size);
    vertexCount_ = count;
    vertexSize_ = stride;
    return true;
#else
    return false;
#endif
}

// DirectX-compatible bind overload (ignores parameters)
void VertexBuffer::bind(void* context, uint32_t slot) const {
    // Ignore context and slot parameters (not needed in OpenGL)
    (void)context;
    (void)slot;

    // Call OpenGL implementation
    bind();
}

} // namespace dj

