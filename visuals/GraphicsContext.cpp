#include "visuals/GraphicsContext.h"

#if defined(_WIN32)
#include <d3d11.h>
#endif

namespace dj {

GraphicsContext::GraphicsContext()
    : available_(false), width_(0), height_(0) {
}

GraphicsContext::~GraphicsContext() {
    shutdown();
}

bool GraphicsContext::initialize(int width, int height, std::string* errorOut) {
    width_ = width;
    height_ = height;

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)
    // DirectX 11 initialization would go here
    // For now, gracefully indicate availability
    // In a real implementation, this would:
    // - Create IDXGIFactory
    // - Create IDXGIAdapter
    // - Create ID3D11Device and ID3D11DeviceContext
    // - Create IDXGISwapChain
    // For this phase, we mark as available if we're compiled with graphics enabled
    available_ = true;
    return true;
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
    // DirectX 11 rendering would go here
    // For this phase, simply succeed if graphics are available
    // A real implementation would render lighting bars, waveforms, etc.
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
    // DirectX 11 cleanup would go here
#endif
    available_ = false;
}

} // namespace dj
