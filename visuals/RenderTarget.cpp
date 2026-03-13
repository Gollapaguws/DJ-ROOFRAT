#include "visuals/RenderTarget.h"

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <cassert>
#endif

namespace dj {

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
RenderTarget::RenderTarget() : width_(0), height_(0) {}
#endif

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)

bool RenderTarget::create(ID3D11Device* device, ID3D11DeviceContext* context,
                          int width, int height, DXGI_FORMAT format) {
    if (!device || width <= 0 || height <= 0) {
        return false;
    }

    (void)context;  // Unused parameter - kept for API consistency
    
    width_ = width;
    height_ = height;

    // Create texture for render target
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = format;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, texture_.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    // Create render target view
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;

    hr = device->CreateRenderTargetView(texture_.Get(), &rtvDesc, renderTargetView_.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    // Create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    hr = device->CreateShaderResourceView(texture_.Get(), &srvDesc, shaderResourceView_.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    return true;
}

void RenderTarget::setActive(ID3D11DeviceContext* context) const {
    if (context) {
        context->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), nullptr);
    }
}

void RenderTarget::clear(ID3D11DeviceContext* context, float r, float g, float b, float a) const {
    if (context && renderTargetView_) {
        float color[4] = {r, g, b, a};
        context->ClearRenderTargetView(renderTargetView_.Get(), color);
    }
}

#endif

}  // namespace dj
