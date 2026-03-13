#pragma once

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace dj {

// RenderTarget: Off-screen render target for multi-pass rendering (bloom, etc.)
class RenderTarget {
public:
    RenderTarget();
    ~RenderTarget() = default;

    // Create off-screen render target
    // Returns true if successful
    bool create(ID3D11Device* device, ID3D11DeviceContext* context,
                int width, int height, DXGI_FORMAT format);

    // Get dimensions
    int getWidth() const noexcept { return width_; }
    int getHeight() const noexcept { return height_; }

    // Get D3D11 resources
    ID3D11RenderTargetView* getRenderTargetView() const noexcept { return renderTargetView_.Get(); }
    ID3D11ShaderResourceView* getShaderResourceView() const noexcept { return shaderResourceView_.Get(); }
    ID3D11Texture2D* getTexture() const noexcept { return texture_.Get(); }

    // Set as active render target
    void setActive(ID3D11DeviceContext* context) const;

    // Clear to color
    void clear(ID3D11DeviceContext* context, float r, float g, float b, float a) const;

private:
    int width_;
    int height_;
    ComPtr<ID3D11Texture2D> texture_;
    ComPtr<ID3D11RenderTargetView> renderTargetView_;
    ComPtr<ID3D11ShaderResourceView> shaderResourceView_;
};

}  // namespace dj

#else  // Not graphics enabled

namespace dj {

class RenderTarget {
public:
    RenderTarget() {}
    ~RenderTarget() = default;
    bool create(void*, void*, int, int, void*) { return false; }
    int getWidth() const noexcept { return 0; }
    int getHeight() const noexcept { return 0; }
    void* getRenderTargetView() const noexcept { return nullptr; }
    void* getShaderResourceView() const noexcept { return nullptr; }
    void* getTexture() const noexcept { return nullptr; }
    void setActive(void*) const {}
    void clear(void*, float, float, float, float) const {}
};

}  // namespace dj

#endif
