#pragma once

#include <cstdint>

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;
#else
// Forward declarations for non-graphics builds
struct ID3D11Device;
struct ID3D11DeviceContext;
#endif

namespace dj {

class VertexBuffer {
public:
    VertexBuffer();
    ~VertexBuffer() = default;

    // Create vertex buffer from vertex data
    bool create(ID3D11Device* device, const void* vertices, uint32_t vertexCount, uint32_t vertexSize);

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Get D3D11 buffer
    ID3D11Buffer* getBuffer() const { return buffer_.Get(); }
#else
    void* getBuffer() const { return nullptr; }
#endif

    // Get vertex count
    uint32_t getVertexCount() const { return vertexCount_; }
    uint32_t getVertexSize() const { return vertexSize_; }

    // Bind buffer for rendering
    bool bind(ID3D11DeviceContext* context, uint32_t slot = 0);

private:
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    ComPtr<ID3D11Buffer> buffer_;
#endif
    uint32_t vertexCount_ = 0;
    uint32_t vertexSize_ = 0;
};

} // namespace dj
