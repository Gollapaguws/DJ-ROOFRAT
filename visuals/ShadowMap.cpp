#include "visuals/ShadowMap.h"

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#include <cstring>
#include <cmath>
#endif

namespace dj {

ShadowMap::ShadowMap()
    : width_(1024)
    , height_(1024)
{
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    std::memset(lightViewMatrix_, 0, sizeof(lightViewMatrix_));
    std::memset(lightProjMatrix_, 0, sizeof(lightProjMatrix_));
    
    // Initialize identity matrices
    for (int i = 0; i < 4; ++i) {
        lightViewMatrix_[i * 4 + i] = 1.0f;
        lightProjMatrix_[i * 4 + i] = 1.0f;
    }
    
    lightPosition_[0] = 0.0f;
    lightPosition_[1] = 10.0f;
    lightPosition_[2] = 0.0f;
    
    lightDirection_[0] = 0.0f;
    lightDirection_[1] = -1.0f;
    lightDirection_[2] = 0.0f;
#endif
}

ShadowMap::~ShadowMap() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // ComPtr handles cleanup automatically
    depthTexture_.Reset();
    depthStencilView_.Reset();
    shaderResourceView_.Reset();
    depthStencilState_.Reset();
    rasterizerState_.Reset();
    comparisonSampler_.Reset();
#endif
}

bool ShadowMap::initialize(ID3D11Device* device, uint32_t width, uint32_t height) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (device == nullptr) {
        return false;
    }

    width_ = width;
    height_ = height;

    // Create depth texture
    if (!createDepthTexture(device)) {
        return false;
    }

    // Create depth stencil view for rendering
    if (!createDepthStencilView(device)) {
        return false;
    }

    // Create shader resource view for sampling
    if (!createShaderResourceView(device)) {
        return false;
    }

    // Setup render state (rasterizer, depth stencil state)
    if (!setupRenderState(device)) {
        return false;
    }

    // Compute initial light matrices
    computeLightMatrices();

    return true;
#else
    return false;
#endif
}

bool ShadowMap::createDepthTexture(ID3D11Device* device) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width_;
    desc.Height = height_;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    // Use typeless format so it can be used as both depth and SRV
    desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    HRESULT hr = device->CreateTexture2D(&desc, nullptr, depthTexture_.GetAddressOf());
    return SUCCEEDED(hr) && depthTexture_;
#else
    return false;
#endif
}

bool ShadowMap::createDepthStencilView(ID3D11Device* device) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!depthTexture_) {
        return false;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    HRESULT hr = device->CreateDepthStencilView(depthTexture_.Get(), &dsvDesc, depthStencilView_.GetAddressOf());
    return SUCCEEDED(hr) && depthStencilView_;
#else
    return false;
#endif
}

bool ShadowMap::createShaderResourceView(ID3D11Device* device) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!depthTexture_) {
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    HRESULT hr = device->CreateShaderResourceView(depthTexture_.Get(), &srvDesc, shaderResourceView_.GetAddressOf());
    return SUCCEEDED(hr) && shaderResourceView_;
#else
    return false;
#endif
}

bool ShadowMap::setupRenderState(ID3D11Device* device) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Create depth stencil state for shadow pass (depth writes enabled, normal comparison)
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    dsDesc.StencilEnable = FALSE;

    HRESULT hr = device->CreateDepthStencilState(&dsDesc, depthStencilState_.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    // Create rasterizer state for shadow pass
    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_BACK;
    rsDesc.DepthBias = 100;  // Bias for shadow acne mitigation
    rsDesc.DepthBiasClamp = 0.0f;
    rsDesc.SlopeScaledDepthBias = 1.0f;
    rsDesc.DepthClipEnable = TRUE;
    rsDesc.ScissorEnable = FALSE;
    rsDesc.AntialiasedLineEnable = FALSE;

    hr = device->CreateRasterizerState(&rsDesc, rasterizerState_.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    // Create comparison sampler for PCF shadow map sampling
    // This sampler is used with SamplerComparisonState in HLSL shaders
    // Register(s0) in lighting.hlsl expects a comparison sampler
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;  // PCF-suitable comparison filter
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.MipLODBias = 0.0f;
    sampDesc.MaxAnisotropy = 1;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;  // Compare pixel depth < shadow map depth
    sampDesc.BorderColor[0] = 1.0f;  // Clamp value (outside shadow = lit, 1.0)
    sampDesc.BorderColor[1] = 1.0f;
    sampDesc.BorderColor[2] = 1.0f;
    sampDesc.BorderColor[3] = 1.0f;
    sampDesc.MinLOD = 0.0f;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = device->CreateSamplerState(&sampDesc, comparisonSampler_.GetAddressOf());
    return SUCCEEDED(hr) && comparisonSampler_;
#else
    return false;
#endif
}

void ShadowMap::computeLightMatrices() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Normalize light direction
    float dirLen = std::sqrt(lightDirection_[0] * lightDirection_[0] +
                             lightDirection_[1] * lightDirection_[1] +
                             lightDirection_[2] * lightDirection_[2]);
    if (dirLen > 0.001f) {
        lightDirection_[0] /= dirLen;
        lightDirection_[1] /= dirLen;
        lightDirection_[2] /= dirLen;
    }

    // Compute light view matrix (look at matrix)
    // Light position + light direction gives the look-at target
    float targetX = lightPosition_[0] + lightDirection_[0];
    float targetY = lightPosition_[1] + lightDirection_[1];
    float targetZ = lightPosition_[2] + lightDirection_[2];

    // Up vector (typically +Y)
    float upX = 0.0f, upY = 1.0f, upZ = 0.0f;

    // Forward vector (from eye to target)
    float fwdX = targetX - lightPosition_[0];
    float fwdY = targetY - lightPosition_[1];
    float fwdZ = targetZ - lightPosition_[2];

    float fwdLen = std::sqrt(fwdX * fwdX + fwdY * fwdY + fwdZ * fwdZ);
    if (fwdLen > 0.001f) {
        fwdX /= fwdLen;
        fwdY /= fwdLen;
        fwdZ /= fwdLen;
    }

    // Right vector = forward x up
    float rightX = fwdY * upZ - fwdZ * upY;
    float rightY = fwdZ * upX - fwdX * upZ;
    float rightZ = fwdX * upY - fwdY * upX;

    float rightLen = std::sqrt(rightX * rightX + rightY * rightY + rightZ * rightZ);
    if (rightLen > 0.001f) {
        rightX /= rightLen;
        rightY /= rightLen;
        rightZ /= rightLen;
    }

    // Recalculate up = right x forward
    float newUpX = rightY * fwdZ - rightZ * fwdY;
    float newUpY = rightZ * fwdX - rightX * fwdZ;
    float newUpZ = rightX * fwdY - rightY * fwdX;

    // Build view matrix (4x4 in row-major order)
    std::memset(lightViewMatrix_, 0, sizeof(lightViewMatrix_));
    lightViewMatrix_[0] = rightX;    lightViewMatrix_[1] = newUpX;    lightViewMatrix_[2] = -fwdX;    lightViewMatrix_[3] = 0.0f;
    lightViewMatrix_[4] = rightY;    lightViewMatrix_[5] = newUpY;    lightViewMatrix_[6] = -fwdY;    lightViewMatrix_[7] = 0.0f;
    lightViewMatrix_[8] = rightZ;    lightViewMatrix_[9] = newUpZ;    lightViewMatrix_[10] = -fwdZ;   lightViewMatrix_[11] = 0.0f;
    lightViewMatrix_[12] = -(rightX * lightPosition_[0] + rightY * lightPosition_[1] + rightZ * lightPosition_[2]);
    lightViewMatrix_[13] = -(newUpX * lightPosition_[0] + newUpY * lightPosition_[1] + newUpZ * lightPosition_[2]);
    lightViewMatrix_[14] = -(-fwdX * lightPosition_[0] - fwdY * lightPosition_[1] - fwdZ * lightPosition_[2]);
    lightViewMatrix_[15] = 1.0f;

    // Build orthographic projection matrix (suitable for directional light)
    float orthoWidth = 40.0f;   // Adjust based on scene size
    float orthoHeight = 40.0f;
    float nearPlane = 1.0f;
    float farPlane = 50.0f;

    std::memset(lightProjMatrix_, 0, sizeof(lightProjMatrix_));
    lightProjMatrix_[0] = 2.0f / orthoWidth;
    lightProjMatrix_[5] = 2.0f / orthoHeight;
    lightProjMatrix_[10] = 1.0f / (farPlane - nearPlane);
    lightProjMatrix_[14] = -nearPlane / (farPlane - nearPlane);
    lightProjMatrix_[15] = 1.0f;
#endif
}

void ShadowMap::setLightPosition(const float* position) {
    if (position) {
        lightPosition_[0] = position[0];
        lightPosition_[1] = position[1];
        lightPosition_[2] = position[2];
        computeLightMatrices();
    }
}

void ShadowMap::setLightDirection(const float* direction) {
    if (direction) {
        lightDirection_[0] = direction[0];
        lightDirection_[1] = direction[1];
        lightDirection_[2] = direction[2];
        computeLightMatrices();
    }
}

ID3D11Texture2D* ShadowMap::getDepthTexture() const {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    return depthTexture_.Get();
#else
    return nullptr;
#endif
}

ID3D11DepthStencilView* ShadowMap::getDepthStencilView() const {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    return depthStencilView_.Get();
#else
    return nullptr;
#endif
}

ID3D11ShaderResourceView* ShadowMap::getShaderResourceView() const {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    return shaderResourceView_.Get();
#else
    return nullptr;
#endif
}

ID3D11SamplerState* ShadowMap::getComparisonSampler() const {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    return comparisonSampler_.Get();
#else
    return nullptr;
#endif
}

const float* ShadowMap::getLightViewMatrix() const {
    return lightViewMatrix_;
}

const float* ShadowMap::getLightProjectionMatrix() const {
    return lightProjMatrix_;
}

void ShadowMap::getViewport(uint32_t* outWidth, uint32_t* outHeight) const {
    if (outWidth) *outWidth = width_;
    if (outHeight) *outHeight = height_;
}

bool ShadowMap::bindForDepthPass(ID3D11DeviceContext* context) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (!context || !depthStencilView_) {
        return false;
    }

    // Set depth stencil view as render target
    context->OMSetRenderTargets(0, nullptr, depthStencilView_.Get());

    // Set rasterizer state
    if (rasterizerState_) {
        context->RSSetState(rasterizerState_.Get());
    }

    // Set depth stencil state
    if (depthStencilState_) {
        context->OMSetDepthStencilState(depthStencilState_.Get(), 0);
    }

    // Set viewport
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(width_);
    viewport.Height = static_cast<float>(height_);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    context->RSSetViewports(1, &viewport);

    // Clear depth stencil
    context->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    return true;
#else
    return false;
#endif
}

void ShadowMap::unbindDepthPass(ID3D11DeviceContext* context) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (context) {
        // Clear render target and depth stencil
        context->OMSetRenderTargets(0, nullptr, nullptr);
    }
#endif
}

} // namespace dj
