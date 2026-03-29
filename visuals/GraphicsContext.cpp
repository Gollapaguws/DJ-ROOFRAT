#include "visuals/GraphicsContext.h"

#include <memory>
#include <algorithm>
#include <cmath>

#include "visuals/LightingRig.h"
#include "visuals/LaserController.h"
#include "visuals/CrowdRenderer.h"

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
#include "visuals/Enhanced3DScene.h"
#include <d3d11.h>
#include <dxgi1_3.h>
#include <windows.h>
#include <wrl.h>

#include "visuals/Camera.h"
#include "visuals/Shader.h"
#include "visuals/VertexBuffer.h"
#include "visuals/IndexBuffer.h"
#include "visuals/StageGeometry.h"

using Microsoft::WRL::ComPtr;

// Custom window procedure: handles WM_DESTROY to trigger WM_QUIT,
// and falls back to DefWindowProcA for all other messages.
static LRESULT CALLBACK DJRoofratWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}
#endif

namespace dj {

GraphicsContext::GraphicsContext()
    : available_(false), width_(0), height_(0),
      lightingRig_(std::make_unique<LightingRig>()),
      laserController_(std::make_unique<LaserController>()),
      crowdRenderer_(std::make_unique<CrowdRenderer>())
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
      , enhancedScene_(std::make_unique<Enhanced3DScene>())
#endif
{
}

GraphicsContext::~GraphicsContext() {
    shutdown();
}

bool GraphicsContext::initialize(int width, int height, std::string* errorOut) {
    width_ = width;
    height_ = height;

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    try {
        // Create D3D11 device and context
        D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
        uint32_t createDeviceFlags = 0;
        
#ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        HRESULT hr = D3D11CreateDevice(
            nullptr,                           // adapter
            D3D_DRIVER_TYPE_HARDWARE,          // driver type
            nullptr,                           // software
            createDeviceFlags,                 // flags
            featureLevels, 1,                  // feature levels
            D3D11_SDK_VERSION,                 // SDK version
            device_.GetAddressOf(),            // output device
            nullptr,                           // feature level
            context_.GetAddressOf()            // output context
        );

        if (FAILED(hr)) {
            if (errorOut) {
                *errorOut = "Failed to create D3D11 device";
            }
            return false;
        }

        // Get DXGI factory
        ComPtr<IDXGIDevice> dxgiDevice;
        hr = device_.As(&dxgiDevice);
        if (FAILED(hr)) {
            if (errorOut) {
                *errorOut = "Failed to get DXGI device";
            }
            return false;
        }

        ComPtr<IDXGIAdapter> dxgiAdapter;
        hr = dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());
        if (FAILED(hr)) {
            if (errorOut) {
                *errorOut = "Failed to get DXGI adapter";
            }
            return false;
        }

        ComPtr<IDXGIFactory2> dxgiFactory;
        hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)dxgiFactory.GetAddressOf());
        if (FAILED(hr)) {
            if (errorOut) {
                *errorOut = "Failed to get DXGI factory";
            }
            return false;
        }

        // Create swap chain
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = width_;
        swapChainDesc.Height = height_;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        // Create a window for the swap chain
        HINSTANCE hInstance = GetModuleHandleA(nullptr);
        const char* className = "DJ_ROOFRAT_Graphics_Window";
        
        WNDCLASSA wndClass = {};
        wndClass.style = CS_HREDRAW | CS_VREDRAW;
        wndClass.lpfnWndProc = DJRoofratWndProc;
        wndClass.cbClsExtra = 0;
        wndClass.cbWndExtra = 0;
        wndClass.hInstance = hInstance;
        wndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wndClass.lpszMenuName = nullptr;
        wndClass.lpszClassName = className;
        
        // Try to register class (ignore error if already registered)
        ATOM classAtom = RegisterClassA(&wndClass);
        if (!classAtom) {
            DWORD error = GetLastError();
            if (error != ERROR_CLASS_ALREADY_EXISTS) {
                if (errorOut) {
                    *errorOut = "Failed to register window class (error " + std::to_string(error) + ")";
                }
                return false;
            }
        }

        hwnd_ = CreateWindowA(className, "DJ-ROOFRAT",
                              WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width_, height_,
                              nullptr, nullptr, hInstance, nullptr);

        if (!hwnd_) {
            DWORD error = GetLastError();
            if (errorOut) {
                *errorOut = "Failed to create window (error " + std::to_string(error) + ")";
            }
            return false;
        }

        // Don't show window yet - will be shown after ImGui initialization

        hr = dxgiFactory->CreateSwapChainForHwnd(
            device_.Get(),
            (HWND)hwnd_,
            &swapChainDesc,
            nullptr,
            nullptr,
            swapChain_.GetAddressOf()
        );

        if (FAILED(hr)) {
            if (errorOut) {
                *errorOut = "Failed to create swap chain";
            }
            return false;
        }

        // Create render target view
        ComPtr<ID3D11Texture2D> backBuffer;
        hr = swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf());
        if (FAILED(hr)) {
            if (errorOut) {
                *errorOut = "Failed to get back buffer";
            }
            return false;
        }

        hr = device_->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTargetView_.GetAddressOf());
        if (FAILED(hr)) {
            if (errorOut) {
                *errorOut = "Failed to create render target view";
            }
            return false;
        }

        // Create depth stencil view
        D3D11_TEXTURE2D_DESC depthDesc = {};
        depthDesc.Width = width_;
        depthDesc.Height = height_;
        depthDesc.MipLevels = 1;
        depthDesc.ArraySize = 1;
        depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthDesc.SampleDesc.Count = 1;
        depthDesc.SampleDesc.Quality = 0;
        depthDesc.Usage = D3D11_USAGE_DEFAULT;
        depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        ComPtr<ID3D11Texture2D> depthStencilBuffer;
        hr = device_->CreateTexture2D(&depthDesc, nullptr, depthStencilBuffer.GetAddressOf());
        if (FAILED(hr)) {
            if (errorOut) {
                *errorOut = "Failed to create depth stencil buffer";
            }
            return false;
        }

        hr = device_->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, depthStencilView_.GetAddressOf());
        if (FAILED(hr)) {
            if (errorOut) {
                *errorOut = "Failed to create depth stencil view";
            }
            return false;
        }

        // Set render target and depth stencil
        context_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), depthStencilView_.Get());

        // Set viewport
        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = (float)width_;
        viewport.Height = (float)height_;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        context_->RSSetViewports(1, &viewport);

        // Initialize camera
        camera_ = std::make_unique<Camera>();
        camera_->setProjection(width_, height_, 45.0f, 0.1f, 1000.0f);

        // Initialize shader
        shader_ = std::make_unique<Shader>();
        if (!shader_->createShaders(device_.Get())) {
            if (errorOut) {
                *errorOut = "Failed to compile shaders";
            }
            return false;
        }

        // BUG 1 FIX: Create input layout from vertex shader blob
        // D3D11 requires input layout to describe vertex buffer structure to GPU
        D3D11_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        ID3DBlob* vertexShaderBlob = shader_->getVertexShaderBlob();
        if (!vertexShaderBlob) {
            if (errorOut) {
                *errorOut = "Vertex shader blob not available for input layout creation";
            }
            return false;
        }

        hr = device_->CreateInputLayout(
            layout, 3,
            vertexShaderBlob->GetBufferPointer(),
            vertexShaderBlob->GetBufferSize(),
            inputLayout_.GetAddressOf()
        );

        if (FAILED(hr)) {
            if (errorOut) {
                *errorOut = "Failed to create input layout";
            }
            return false;
        }

        // BUG 2 FIX: Create constant buffer for camera matrices (WVP)
        // Must be 16-byte aligned for D3D11
        D3D11_BUFFER_DESC cbDesc = {};
        cbDesc.ByteWidth = sizeof(ConstantBufferData);
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        hr = device_->CreateBuffer(&cbDesc, nullptr, constantBuffer_.GetAddressOf());
        if (FAILED(hr)) {
            if (errorOut) {
                *errorOut = "Failed to create constant buffer";
            }
            return false;
        }

        // Initialize stage geometry
        stageGeometry_ = std::make_unique<StageGeometry>();
        stageGeometry_->generate();

        // Create buffers
        vertexBuffer_ = std::make_unique<VertexBuffer>();
        if (!vertexBuffer_->create(device_.Get(), 
                                   stageGeometry_->getVertices().data(),
                                   (uint32_t)stageGeometry_->getVertices().size(),
                                   sizeof(Vertex))) {
            if (errorOut) {
                *errorOut = "Failed to create vertex buffer";
            }
            return false;
        }

        indexBuffer_ = std::make_unique<IndexBuffer>();
        if (!indexBuffer_->create(device_.Get(),
                                  stageGeometry_->getIndices().data(),
                                  (uint32_t)stageGeometry_->getIndices().size())) {
            if (errorOut) {
                *errorOut = "Failed to create index buffer";
            }
            return false;
        }

        // Initialize Enhanced3DScene
        if (enhancedScene_) {
            if (!enhancedScene_->initialize(device_.Get(), context_.Get())) {
                if (errorOut) {
                    *errorOut = "Failed to initialize Enhanced3DScene";
                }
                return false;
            }

            // Enable advanced visuals in live app mode for a fully playable graphics experience.
            enhancedScene_->setTunnelEffect(true);
            enhancedScene_->setShadowMapping(true);
        }

        available_ = true;
        return true;
    } catch (const std::exception& e) {
        if (errorOut) {
            *errorOut = std::string("D3D11 initialization exception: ") + e.what();
        }
        return false;
    }

#else
    if (errorOut) {
        *errorOut = "Graphics not available (build without DJROOFRAT_ENABLE_GRAPHICS or non-Windows platform)";
    }
    available_ = false;
    return false;
#endif
}

bool GraphicsContext::isAvailable() const {
    return available_;
}

bool GraphicsContext::renderFrame(float bpm, float energy, int mood, float crossfader) {
    if (!available_) {
        return false;
    }

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    context_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), depthStencilView_.Get());

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(width_);
    viewport.Height = static_cast<float>(height_);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    context_->RSSetViewports(1, &viewport);

    // Update timing
    frameTimer_ += 1.0f / 60.0f; // Assume 60 FPS for now
    frameCount_++;
    if (frameTimer_ >= 1.0f) {
        fps_ = static_cast<float>(frameCount_);
        frameCount_ = 0;
        frameTimer_ = 0.0f;
    }

    // Update camera with beat-reactive effects
    if (camera_) {
        camera_->updateBeatReactive(bpm, 1.0f / 60.0f);
    }

    // Update laser and crowd animation state
    const float blockDurationSeconds = 512.0f / 44100.0f;  // Typical audio block duration
    
    if (lightingRig_) {
        lightingRig_->update(bpm, energy, blockDurationSeconds);
    }
    
    if (laserController_) {
        laserController_->update(bpm, crossfader, blockDurationSeconds);
    }
    
    if (crowdRenderer_) {
        crowdRenderer_->update(mood, energy, blockDurationSeconds);
    }

    // Update Enhanced3DScene with music data
    if (enhancedScene_) {
        // Calculate beat phase (0 to 1) based on BPM
        // For Phase 1, we use a simple calculation. More sophisticated timing can be added later.
        static float musicTime = 0.0f;
        musicTime += blockDurationSeconds;
        float beatDuration = 60.0f / bpm;
        float beatPhase = fmodf(musicTime / beatDuration, 1.0f);
        
        enhancedScene_->update(bpm, energy, beatPhase, blockDurationSeconds);
    }

    // Clear render target and depth stencil
    float clearColor[4] = {0.04f, 0.07f, 0.12f, 1.0f};
    context_->ClearRenderTargetView(renderTargetView_.Get(), clearColor);
    context_->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Set vertex and index buffers
    if (vertexBuffer_) {
        vertexBuffer_->bind(context_.Get(), 0);
    }
    if (indexBuffer_) {
        indexBuffer_->bind(context_.Get());
    }

    // BUG 1 FIX: Bind input layout so GPU understands vertex structure
    if (inputLayout_) {
        context_->IASetInputLayout(inputLayout_.Get());
    }

    // Set shaders
    if (shader_) {
        context_->VSSetShader(shader_->getVertexShader(), nullptr, 0);
        context_->PSSetShader(shader_->getPixelShader(), nullptr, 0);
    }

    // BUG 2 FIX: Update constant buffer with camera matrices and bind to vertex shader
    if (constantBuffer_ && camera_) {
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        HRESULT hr = context_->Map(constantBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        
        if (SUCCEEDED(hr)) {
            ConstantBufferData* cbData = (ConstantBufferData*)mapped.pData;

            // Identity world matrix for now
            float worldMatrix[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };

            // Get matrices from camera
            const Matrix4* viewMatrixPtr = camera_->getViewMatrix();
            const Matrix4& projMatrix = camera_->getProjectionMatrix();

            // Copy matrices to constant buffer
            memcpy(cbData->world, worldMatrix, 64);
            memcpy(cbData->view, viewMatrixPtr->m[0], 64);
            memcpy(cbData->projection, &projMatrix.m[0][0], 64);

            // Set light direction
            cbData->lightDir[0] = 1.0f;
            cbData->lightDir[1] = 1.0f;
            cbData->lightDir[2] = -1.0f;
            cbData->padding = enhancedScene_ ? enhancedScene_->getBeatIntensity() : 0.0f;

            context_->Unmap(constantBuffer_.Get(), 0);
        }

        // Bind constant buffer to vertex shader
        context_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
        context_->PSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
    }

    // BUG 3 FIX: Set primitive topology (D3D11 requires explicit topology)
    context_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Draw indexed primitives
    if (indexBuffer_ && indexBuffer_->getIndexCount() > 0) {
        context_->DrawIndexed(indexBuffer_->getIndexCount(), 0, 0);
    }

    // Render Enhanced3DScene
    if (enhancedScene_ && camera_) {
        const Matrix4* viewMatrixPtr = camera_->getViewMatrix();
        const Matrix4& projMatrix = camera_->getProjectionMatrix();
        enhancedScene_->render(viewMatrixPtr->m[0], &projMatrix.m[0][0]);
    }

    // Phase 3: Render laser beams
    // Laser beams are synchronized to BPM via LaserController which was updated above.
    // The beam geometry is available via laserController_->getBeamGeometry().
    // Integration with D3D11 rendering buffers can be added as needed for full visibility.
    if (laserController_) {
        // Get current laser intensity and beam geometry
        float laserIntensity = laserController_->intensity();
        if (laserIntensity > 0.0f) {
            // Beam geometry is available for rendering:
            // auto beamGeom = laserController_->getBeamGeometry(1.0f);
            // beamGeom.vertices, beamGeom.indices can be used to render to D3D11
            // For now, laser state is active and ready for rendering
        }
    }

    // Phase 3: Lighting effects from LightingRig
    // Lighting rig provides both bar intensities and multi-light system data.
    // Light positions, colors, and intensities are synchronized to BPM via LightingRig which was updated above.
    if (lightingRig_) {
        // Get current lighting state
        const auto& lights = lightingRig_->getLights();
        float strobeIntensity = lightingRig_->getStrobeIntensity();
        // Lights vector can be used to apply dynamic lighting to the scene
        // Strobe intensity can modulate the scene brightness for beat-sync effects
        // For now, lighting state is active and ready for application to scene
    }

    // Note: presentation (Present) is handled by the caller via graphics.present()
    // to allow overlaying ImGui on top of the 3D scene in a single frame.
    return true;
#else
    // Suppress unreferenced parameter warnings
    (void)bpm;
    (void)energy;
    (void)mood;
    (void)crossfader;
    return false;
#endif
}

void GraphicsContext::shutdown() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (context_) {
        context_->ClearState();
        context_->Flush();
    }

    vertexBuffer_.reset();
    indexBuffer_.reset();
    shader_.reset();
    stageGeometry_.reset();
    camera_.reset();

    constantBuffer_.Reset();       // Clean up constant buffer (BUG 2)
    inputLayout_.Reset();          // Clean up input layout (BUG 1)
    renderTargetView_.Reset();
    depthStencilView_.Reset();
    swapChain_.Reset();
    context_.Reset();
    device_.Reset();

    if (hwnd_) {
        DestroyWindow((HWND)hwnd_);
        hwnd_ = nullptr;
        
        // Unregister window class to prevent resource leak
        UnregisterClassA("DJ_ROOFRAT_Graphics_Window", GetModuleHandleA(nullptr));
    }
#endif
    available_ = false;
}

void GraphicsContext::present() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (swapChain_) {
        swapChain_->Present(1, 0);  // vsync enabled
    }
#endif
}

void GraphicsContext::showWindow() {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (hwnd_) {
        ShowWindow((HWND)hwnd_, SW_SHOW);
        UpdateWindow((HWND)hwnd_);
    }
#endif
}

void GraphicsContext::clearRenderTarget(float r, float g, float b, float a) {
#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    if (context_ && renderTargetView_) {
        const float clearColor[4] = { r, g, b, a };
        context_->ClearRenderTargetView(renderTargetView_.Get(), clearColor);
        
        if (depthStencilView_) {
            context_->ClearDepthStencilView(depthStencilView_.Get(), 
                                           D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 
                                           1.0f, 0);
        }
    }
#else
    (void)r; (void)g; (void)b; (void)a;
#endif
}

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
ID3D11BlendState* GraphicsContext::createAdditiveBlendState() const {
    if (!device_) {
        return nullptr;
    }

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    
    // Additive blending: SrcBlend = SRC_ALPHA, DestBlend = ONE
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;  // Additive
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    ID3D11BlendState* blendState = nullptr;
    HRESULT hr = device_->CreateBlendState(&blendDesc, &blendState);
    
    if (FAILED(hr)) {
        return nullptr;
    }

    return blendState;
}
#endif

} // namespace dj
