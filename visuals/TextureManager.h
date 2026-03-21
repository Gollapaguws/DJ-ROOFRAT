#pragma once

#include <cstdint>
#include <vector>

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;
#else
// Forward declarations for non-graphics builds
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;
#endif

namespace dj {

class TextureManager {
public:
    TextureManager();
    ~TextureManager() = default;

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Create a procedural checkerboard texture
    // width, height: texture dimensions in pixels
    // Returns true if successful
    bool createCheckerboard(uint32_t width, uint32_t height);

    // Get the shader resource view for binding to shaders
    ID3D11ShaderResourceView* getTextureSRV() const { return textureSRV_.Get(); }

    // Get the sampler state
    ID3D11SamplerState* getSamplerState() const { return samplerState_.Get(); }

    // Bind texture SRV and sampler state to pixel shader
    void bind(ID3D11DeviceContext* context, uint32_t slotIndex = 0);

    // Set device for texture creation (required before createCheckerboard)
    void setDevice(ID3D11Device* device) { device_ = device; }

private:
    // Helper to create sampler state
    bool createSamplerState();

    ComPtr<ID3D11Texture2D> texture_;
    ComPtr<ID3D11ShaderResourceView> textureSRV_;
    ComPtr<ID3D11SamplerState> samplerState_;
    ID3D11Device* device_ = nullptr;
#else
    bool createCheckerboard(uint32_t width, uint32_t height) { return false; }
    void* getTextureSRV() const { return nullptr; }
    void* getSamplerState() const { return nullptr; }
    void bind(void* context, uint32_t slotIndex = 0) {}
    void setDevice(void* device) {}
#endif
};

} // namespace dj
