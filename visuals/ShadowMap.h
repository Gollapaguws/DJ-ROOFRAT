#pragma once

#include <memory>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11SamplerState;

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;
#endif

namespace dj {

/// ShadowMap manages shadow depth texture, DSV, SRV, and light-space transformations
/// Supports single directional light shadow mapping with depth pass and 2x2 PCF filtering
class ShadowMap {
public:
    ShadowMap();
    ~ShadowMap();

    /// Initialize shadow map resources for a given resolution
    /// @param device D3D11 device
    /// @param width Depth texture width (typically 1024)
    /// @param height Depth texture height (typically 1024)
    /// @return true if initialization succeeded
    bool initialize(ID3D11Device* device, uint32_t width, uint32_t height);

    /// Set directional light position (for view matrix setup)
    void setLightPosition(const float* position);

    /// Set directional light direction (for view matrix setup)
    void setLightDirection(const float* direction);

    /// Get depth texture for rendering
    ID3D11Texture2D* getDepthTexture() const;

    /// Get depth stencil view for rendering to shadow map
    ID3D11DepthStencilView* getDepthStencilView() const;

    /// Get shader resource view for sampling in lighting pass
    ID3D11ShaderResourceView* getShaderResourceView() const;

    /// Get comparison sampler for PCF shadow map sampling
    ID3D11SamplerState* getComparisonSampler() const;

    /// Get light-space view matrix
    const float* getLightViewMatrix() const;

    /// Get light-space orthographic projection matrix
    const float* getLightProjectionMatrix() const;

    /// Get shadow map viewport
    void getViewport(uint32_t* outWidth, uint32_t* outHeight) const;

    /// Bind shadow map for rendering depth pass
    bool bindForDepthPass(ID3D11DeviceContext* context);

    /// Unbind shadow map after depth pass
    void unbindDepthPass(ID3D11DeviceContext* context);

private:
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Resolution
    uint32_t width_ = 1024;
    uint32_t height_ = 1024;

    // Depth texture and views
    ComPtr<ID3D11Texture2D> depthTexture_;
    ComPtr<ID3D11DepthStencilView> depthStencilView_;
    ComPtr<ID3D11ShaderResourceView> shaderResourceView_;

    // Rendering state
    ComPtr<ID3D11DepthStencilState> depthStencilState_;
    ComPtr<ID3D11RasterizerState> rasterizerState_;
    ComPtr<ID3D11SamplerState> comparisonSampler_;  // For PCF shadow map sampling

    // Light properties
    float lightPosition_[3] = {0.0f, 10.0f, 0.0f};
    float lightDirection_[3] = {0.0f, -1.0f, 0.0f};

    // Light-space transform matrices (4x4, row-major)
    float lightViewMatrix_[16];
    float lightProjMatrix_[16];

    // Helper methods
    bool createDepthTexture(ID3D11Device* device);
    bool createDepthStencilView(ID3D11Device* device);
    bool createShaderResourceView(ID3D11Device* device);
    bool setupRenderState(ID3D11Device* device);
    void computeLightMatrices();
#endif
};

} // namespace dj
