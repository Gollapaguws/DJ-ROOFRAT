#include "visuals/VertexBuffer.h"

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#endif

namespace dj {

VertexBuffer::VertexBuffer() = default;

bool VertexBuffer::create(ID3D11Device* device, const void* vertices, uint32_t vertexCount, uint32_t vertexSize) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!device || !vertices || vertexCount == 0) {
        return false;
    }

    vertexCount_ = vertexCount;
    vertexSize_ = vertexSize;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = vertexCount * vertexSize;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;

    HRESULT hr = device->CreateBuffer(&bufferDesc, &initData, buffer_.GetAddressOf());
    return SUCCEEDED(hr);
#else
    (void)device;
    (void)vertices;
    (void)vertexCount;
    (void)vertexSize;
    return false;
#endif
}

bool VertexBuffer::bind(ID3D11DeviceContext* context, uint32_t slot) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!context || !buffer_) {
        return false;
    }

    uint32_t stride = vertexSize_;
    uint32_t offset = 0;
    context->IASetVertexBuffers(slot, 1, buffer_.GetAddressOf(), &stride, &offset);
    return true;
#else
    (void)context;
    (void)slot;
    return false;
#endif
}

} // namespace dj
