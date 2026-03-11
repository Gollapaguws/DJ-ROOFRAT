#include "visuals/IndexBuffer.h"

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#endif

namespace dj {

IndexBuffer::IndexBuffer() = default;

bool IndexBuffer::create(ID3D11Device* device, const uint32_t* indices, uint32_t indexCount) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!device || !indices || indexCount == 0) {
        return false;
    }

    indexCount_ = indexCount;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = indexCount * sizeof(uint32_t);
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = indices;

    HRESULT hr = device->CreateBuffer(&bufferDesc, &initData, buffer_.GetAddressOf());
    return SUCCEEDED(hr);
#else
    (void)device;
    (void)indices;
    (void)indexCount;
    return false;
#endif
}

bool IndexBuffer::bind(ID3D11DeviceContext* context) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!context || !buffer_) {
        return false;
    }

    context->IASetIndexBuffer(buffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
    return true;
#else
    (void)context;
    return false;
#endif
}

} // namespace dj
