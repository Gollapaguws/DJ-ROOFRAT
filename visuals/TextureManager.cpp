#include "visuals/TextureManager.h"

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#include <cstring>
#include <cmath>
#include <algorithm>
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

    // Generate high-quality procedural checkerboard with richer color palette
    // Each pixel is RGBA (0xAABBGGRR format for DirectX)
    const float squareSizeRatio = 32.0f / 256.0f;  // Normalize for any texture size
    uint32_t squareSize = static_cast<uint32_t>(width * squareSizeRatio);
    if (squareSize < 4) squareSize = 4;  // Minimum square size
    
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            // Determine base checker pattern
            uint32_t gridX = x / squareSize;
            uint32_t gridY = y / squareSize;
            bool isWhiteSquare = ((gridX ^ gridY) & 1) == 0;
            
            // Intra-square positioning for procedural variation
            uint32_t localX = x % squareSize;
            uint32_t localY = y % squareSize;
            float normLocalX = static_cast<float>(localX) / squareSize;
            float normLocalY = static_cast<float>(localY) / squareSize;
            
            // Create radial gradient within each square for anti-flat appearance
            float centerX = 0.5f;
            float centerY = 0.5f;
            float distFromCenter = sqrtf((normLocalX - centerX) * (normLocalX - centerX) + 
                                           (normLocalY - centerY) * (normLocalY - centerY));
            float gradient = 1.0f - (distFromCenter * 0.4f);  // Brighten center
            gradient = gradient < 0.0f ? 0.0f : (gradient > 1.0f ? 1.0f : gradient);
            
            // Create richer color palette with metallic variations
            uint8_t r, g, b;
            if (isWhiteSquare) {
                // Light square: metallic silver-to-white
                uint8_t baseLight = static_cast<uint8_t>(200 + gradient * 55);
                r = baseLight;
                g = baseLight;
                b = baseLight;
            } else {
                // Dark square: deep blue-gray with procedural variation
                float dark = 40.0f + distFromCenter * 60.0f;
                uint8_t baseDark = static_cast<uint8_t>(dark);
                r = static_cast<uint8_t>(baseDark * 0.8f);
                g = static_cast<uint8_t>(baseDark * 0.9f);
                b = static_cast<uint8_t>(baseDark);
            }
            
            // Pack as RGBA (0xAABBGGRR format for DirectX)
            pixelData[y * width + x] = 0xFF000000 | (static_cast<uint32_t>(b) << 16) | 
                                       (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(r);
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
    sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;  // Anisotropic filtering for high-quality results
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.MipLODBias = 0.0f;
    sampDesc.MaxAnisotropy = 8;  // Use 8x anisotropy for improved visual quality
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
