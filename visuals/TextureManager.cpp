#include "visuals/TextureManager.h"

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#include <cstring>
#endif

namespace dj {

TextureManager::TextureManager() = default;

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)

bool TextureManager::createCheckerboard(uint32_t width, uint32_t height) {
    if (!device_) {
        return false;
    }

    // Create pixel data for checkerboard pattern
    std::vector<uint32_t> pixelData;
    pixelData.resize(width * height);

    // Generate checkerboard pattern (alternating black and white)
    // Each pixel is RGBA (0xAABBGGRR format for DirectX)
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            uint32_t squareSize = 32;  // 32x32 pixel squares
            bool isWhite = ((x / squareSize) ^ (y / squareSize)) & 1;
            
            // RGBA: white or black with full alpha
            pixelData[y * width + x] = isWhite ? 0xFFFFFFFF : 0xFF000000;
        }
    }

    // Create texture descriptor
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // RGBA format
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    // Create initial data
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixelData.data();
    initData.SysMemPitch = width * sizeof(uint32_t);

    // Create texture
    HRESULT hr = device_->CreateTexture2D(&texDesc, &initData, texture_.GetAddressOf());
    if (FAILED(hr)) {
        texture_.Reset();
        return false;
    }

    // Create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    hr = device_->CreateShaderResourceView(texture_.Get(), &srvDesc, textureSRV_.GetAddressOf());
    if (FAILED(hr)) {
        textureSRV_.Reset();
        return false;
    }

    // Create sampler state
    if (!createSamplerState()) {
        return false;
    }

    return true;
}

bool TextureManager::createSamplerState() {
    if (!device_) {
        return false;
    }

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;  // Linear filtering
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.MipLODBias = 0.0f;
    sampDesc.MaxAnisotropy = 1;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sampDesc.BorderColor[0] = 0.0f;
    sampDesc.BorderColor[1] = 0.0f;
    sampDesc.BorderColor[2] = 0.0f;
    sampDesc.BorderColor[3] = 1.0f;
    sampDesc.MinLOD = 0.0f;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    HRESULT hr = device_->CreateSamplerState(&sampDesc, samplerState_.GetAddressOf());
    if (FAILED(hr)) {
        samplerState_.Reset();
        return false;
    }

    return true;
}

void TextureManager::bind(ID3D11DeviceContext* context, uint32_t slotIndex) {
    if (!context) {
        return;
    }

    // Bind texture SRV to pixel shader
    if (textureSRV_) {
        context->PSSetShaderResources(slotIndex, 1, textureSRV_.GetAddressOf());
    }

    // Bind sampler state to pixel shader
    if (samplerState_) {
        context->PSSetSamplers(slotIndex, 1, samplerState_.GetAddressOf());
    }
}

#endif

} // namespace dj
