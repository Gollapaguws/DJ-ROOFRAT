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

class IndexBuffer {
public:
    IndexBuffer();
    ~IndexBuffer() = default;

    // Create index buffer from index data
    bool create(ID3D11Device* device, const uint32_t* indices, uint32_t indexCount);

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Get D3D11 buffer
    ID3D11Buffer* getBuffer() const { return buffer_.Get(); }
#else
    void* getBuffer() const { return nullptr; }
#endif

    // Get index count
    uint32_t getIndexCount() const { return indexCount_; }

    // Bind buffer for rendering
    bool bind(ID3D11DeviceContext* context);

private:
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    ComPtr<ID3D11Buffer> buffer_;
#endif
    uint32_t indexCount_ = 0;
};

} // namespace dj
