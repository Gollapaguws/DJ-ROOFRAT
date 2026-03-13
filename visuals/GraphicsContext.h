#pragma once

#include <memory>
#include <string>

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include <d3d11.h>
#include <dxgi1_3.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

// Constant buffer structure matching HLSL shader (16-byte aligned)
// Must match the layout in basic.hlsl
struct ConstantBufferData {
    float world[16];        // 64 bytes: world transformation matrix
    float view[16];         // 64 bytes: view/camera matrix
    float projection[16];   // 64 bytes: projection matrix  
    float lightDir[3];      // 12 bytes: light direction
    float padding;          // 4 bytes: padding for 16-byte alignment (total: 196 bytes)
};

#endif

namespace dj {

class LaserController;
class CrowdRenderer;
class Camera;
class Shader;
class VertexBuffer;
class IndexBuffer;
class StageGeometry;

class GraphicsContext {
public:
    GraphicsContext();
    ~GraphicsContext();

    // Initialize graphics context. Returns true if graphics are available after init.
    bool initialize(int width, int height, std::string* errorOut = nullptr);

    // Check if graphics are currently available
    bool isAvailable() const;

    // Render a frame with current playback state.
    // Returns true if render succeeded; false if graphics unavailable.
    // Parameters: BPM, crowd energy [0,1], mood (0-3), crossfader [-1,1]
    bool renderFrame(float bpm, float energy, int mood, float crossfader);

    // Clean up graphics resources
    void shutdown();

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Get D3D11 resources
    ID3D11Device* getD3D11Device() const { return device_.Get(); }
    ID3D11DeviceContext* getD3D11DeviceContext() const { return context_.Get(); }
    IDXGISwapChain1* getSwapChain() const { return swapChain_.Get(); }
    ID3D11RenderTargetView* getRenderTargetView() const { return renderTargetView_.Get(); }
    ID3D11DepthStencilView* getDepthStencilView() const { return depthStencilView_.Get(); }
    ID3D11InputLayout* getInputLayout() const { return inputLayout_.Get(); }
    ID3D11Buffer* getConstantBuffer() const { return constantBuffer_.Get(); }
#else
    void* getD3D11Device() const { return nullptr; }
    void* getD3D11DeviceContext() const { return nullptr; }
    void* getSwapChain() const { return nullptr; }
    void* getRenderTargetView() const { return nullptr; }
    void* getDepthStencilView() const { return nullptr; }
    void* getInputLayout() const { return nullptr; }
    void* getConstantBuffer() const { return nullptr; }
#endif

    // Get current FPS
    float getFPS() const { return fps_; }


#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // Create additive blend state for laser effects (Phase 19)
    ID3D11BlendState* createAdditiveBlendState() const;
#endif

private:
    bool available_;
    int width_;
    int height_;
    float fps_ = 60.0f;

    std::unique_ptr<LaserController> laserController_;
    std::unique_ptr<CrowdRenderer> crowdRenderer_;

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // D3D11 resources
    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> context_;
    ComPtr<IDXGISwapChain1> swapChain_;
    ComPtr<ID3D11RenderTargetView> renderTargetView_;
    ComPtr<ID3D11DepthStencilView> depthStencilView_;
    ComPtr<ID3D11InputLayout> inputLayout_;         // BUG 1 FIX: Input layout for vertex interpretation
    ComPtr<ID3D11Buffer> constantBuffer_;           // BUG 2 FIX: Constant buffer for camera matrices

    // Camera and geometry
    std::unique_ptr<Camera> camera_;
    std::unique_ptr<Shader> shader_;
    std::unique_ptr<VertexBuffer> vertexBuffer_;
    std::unique_ptr<IndexBuffer> indexBuffer_;
    std::unique_ptr<StageGeometry> stageGeometry_;

    // Window handle
    void* hwnd_ = nullptr;

    // Render loop timing
    float frameTimer_ = 0.0f;
    int frameCount_ = 0;
#endif
};

} // namespace dj
