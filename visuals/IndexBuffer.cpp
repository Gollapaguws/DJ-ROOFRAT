#include "visuals/IndexBuffer.h"

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <glad/glad.h>
#endif

namespace dj {

IndexBuffer::IndexBuffer() = default;

IndexBuffer::~IndexBuffer() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (ebo_ != 0) {
        glDeleteBuffers(1, &ebo_);
        ebo_ = 0;
    }
#endif
}

bool IndexBuffer::create(const uint32_t* indices, uint32_t indexCount, GLenum usage) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!indices || indexCount == 0) {
        return false;
    }

    // Clean up old buffer if exists
    if (ebo_ != 0) {
        glDeleteBuffers(1, &ebo_);
        ebo_ = 0;
    }

    // Generate an EBO handle
    glGenBuffers(1, &ebo_);
    if (ebo_ == 0) {
        return false;
    }

    // Bind the buffer to GL_ELEMENT_ARRAY_BUFFER
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

    // Upload data to GPU
    uint32_t size = indexCount * sizeof(uint32_t);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, usage);

    // Unbind
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    indexCount_ = indexCount;
    bufferSize_ = size;
    return true;
#else
    (void)indices;
    (void)indexCount;
    (void)usage;
    return false;
#endif
}

bool IndexBuffer::update(const uint32_t* data, uint32_t count, uint32_t offset) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!data || count == 0 || ebo_ == 0) {
        return false;
    }

    uint32_t sizeBytes = count * sizeof(uint32_t);
    uint32_t offsetBytes = offset * sizeof(uint32_t);

    // Check that offset + size doesn't exceed buffer
    if (offsetBytes + sizeBytes > bufferSize_) {
        return false;
    }

    // Bind and update
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offsetBytes, sizeBytes, data);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return true;
#else
    (void)data;
    (void)count;
    (void)offset;
    return false;
#endif
}

bool IndexBuffer::bind() const {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (ebo_ == 0) {
        return false;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    return true;
#else
    return false;
#endif
}

void IndexBuffer::unbind() const {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
}

// DirectX-compatible overload
bool IndexBuffer::create(void* device, const uint32_t* indices, uint32_t count) {
    // Ignore device parameter (not needed in OpenGL)
    (void)device;

    if (count == 0 || !indices) {
        return false;
    }

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Clean up old buffer if exists
    if (ebo_ != 0) {
        glDeleteBuffers(1, &ebo_);
        ebo_ = 0;
    }

    // Generate an EBO handle
    glGenBuffers(1, &ebo_);
    if (ebo_ == 0) {
        return false;
    }

    // Bind the buffer to GL_ELEMENT_ARRAY_BUFFER
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

    // Upload data to GPU
    uint32_t size = count * sizeof(uint32_t);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);

    // Unbind
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    indexCount_ = count;
    bufferSize_ = size;
    return true;
#else
    return false;
#endif
}

// DirectX-compatible bind overloads (ignores parameters)
void IndexBuffer::bind(void* context) const {
    // Ignore context parameter (not needed in OpenGL)
    (void)context;

    // Call OpenGL implementation
    bind();
}

void IndexBuffer::bind(void* context, uint32_t slot) const {
    // Ignore context and slot parameters (not needed in OpenGL)
    (void)context;
    (void)slot;

    // Call OpenGL implementation
    bind();
}

} // namespace dj

