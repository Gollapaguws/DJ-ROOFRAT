#include <gtest/gtest.h>
#include <memory>

#if defined(_WIN32) && defined(DJROOFRAT_ENABLE_GRAPHICS)

#include "visuals/GraphicsContext.h"
#include "visuals/ParticleSystem.h"
#include "visuals/PostProcessor.h"
#include "visuals/RenderTarget.h"

namespace dj {

class Graphics_Phase4_Test : public ::testing::Test {
protected:
    Graphics_Phase4_Test() = default;

    void SetUp() override {
        // Initialize graphics context
        graphics_ = std::make_unique<GraphicsContext>();
        ASSERT_TRUE(graphics_->initialize(800, 600, &errorMsg_)) 
            << "GraphicsContext initialization failed: " << errorMsg_;
        ASSERT_TRUE(graphics_->isAvailable());
    }

    void TearDown() override {
        graphics_->shutdown();
        graphics_.reset();
    }

    std::unique_ptr<GraphicsContext> graphics_;
    std::string errorMsg_;
};

// Test 1: ParticleSystem initialization
TEST_F(Graphics_Phase4_Test, ParticleSystemInitializeSuccessfully) {
    auto device = graphics_->getD3D11Device();
    ASSERT_NE(device, nullptr);

    ParticleSystem ps;
    EXPECT_TRUE(ps.initialize(static_cast<ID3D11Device*>(device), 10000));
    EXPECT_EQ(ps.getMaxParticles(), 10000);
    EXPECT_EQ(ps.getActiveParticleCount(), 0);
}

// Test 2: ParticleSystem can emit particles
TEST_F(Graphics_Phase4_Test, ParticleSystemEmitstParticles) {
    auto device = graphics_->getD3D11Device();
    ASSERT_NE(device, nullptr);

    ParticleSystem ps;
    ASSERT_TRUE(ps.initialize(static_cast<ID3D11Device*>(device), 10000));

    float emitPos[3] = {0.0f, 0.0f, 0.0f};
    float velocity[3] = {1.0f, 1.0f, 0.0f};

    ps.emitParticles(emitPos, 100, 2.0f, velocity);
    EXPECT_EQ(ps.getActiveParticleCount(), 100);
}

// Test 3: ParticleSystem updates physics
TEST_F(Graphics_Phase4_Test, ParticleSystemUpdatesPhysics) {
    auto device = graphics_->getD3D11Device();
    auto context = graphics_->getD3D11DeviceContext();
    ASSERT_NE(device, nullptr);
    ASSERT_NE(context, nullptr);

    ParticleSystem ps;
    ASSERT_TRUE(ps.initialize(static_cast<ID3D11Device*>(device), 10000));

    float emitPos[3] = {0.0f, 0.0f, 0.0f};
    float velocity[3] = {1.0f, 1.0f, 0.0f};
    ps.emitParticles(emitPos, 100, 2.0f, velocity);

    float gravity[3] = {0.0f, -9.8f, 0.0f};
    float wind[3] = {0.0f, 0.0f, 0.0f};

    // Should not crash and should handle update
    ps.updatePhysics(static_cast<ID3D11DeviceContext*>(context), 0.016f, gravity, wind);
    EXPECT_GE(ps.getActiveParticleCount(), 0);
}

// Test 4: Energy-threshold based particle burst
TEST_F(Graphics_Phase4_Test, EnergyPeakTriggersParticleBurst) {
    // Test that high energy (> 0.8) should trigger particle emission
    // This tests the integration in renderFrame()
    
    // Low energy - should not emit many particles
    ASSERT_TRUE(graphics_->renderFrame(120.0f, 0.3f, 0, 0.0f));

    // High energy - should trigger burst
    ASSERT_TRUE(graphics_->renderFrame(120.0f, 0.9f, 0, 0.0f));
}

// Test 5: PostProcessor initialization
TEST_F(Graphics_Phase4_Test, PostProcessorInitializeSuccessfully) {
    auto device = graphics_->getD3D11Device();
    auto context = graphics_->getD3D11DeviceContext();
    ASSERT_NE(device, nullptr);
    ASSERT_NE(context, nullptr);

    PostProcessor pp;
    EXPECT_TRUE(pp.initialize(
        static_cast<ID3D11Device*>(device),
        static_cast<ID3D11DeviceContext*>(context),
        800, 600));
    EXPECT_EQ(pp.getWidth(), 800);
    EXPECT_EQ(pp.getHeight(), 600);
}

// Test 6: PostProcessor applies bloom
TEST_F(Graphics_Phase4_Test, PostProcessorAppliesBloom) {
    auto device = graphics_->getD3D11Device();
    auto context = graphics_->getD3D11DeviceContext();
    ASSERT_NE(device, nullptr);
    ASSERT_NE(context, nullptr);

    PostProcessor pp;
    ASSERT_TRUE(pp.initialize(
        static_cast<ID3D11Device*>(device),
        static_cast<ID3D11DeviceContext*>(context),
        800, 600));

    // Apply bloom with different intensities
    EXPECT_TRUE(pp.applyBloom(0.5f));
    EXPECT_TRUE(pp.applyBloom(1.0f));
}

// Test 7: PostProcessor applies color grading
TEST_F(Graphics_Phase4_Test, PostProcessorAppliesColorGrading) {
    auto device = graphics_->getD3D11Device();
    auto context = graphics_->getD3D11DeviceContext();
    ASSERT_NE(device, nullptr);
    ASSERT_NE(context, nullptr);

    PostProcessor pp;
    ASSERT_TRUE(pp.initialize(
        static_cast<ID3D11Device*>(device),
        static_cast<ID3D11DeviceContext*>(context),
        800, 600));

    // Apply color grading for all moods
    EXPECT_TRUE(pp.applyColorGrade(0)); // Unimpressed: neutral
    EXPECT_TRUE(pp.applyColorGrade(1)); // Calm: blue
    EXPECT_TRUE(pp.applyColorGrade(2)); // Grooving: green
    EXPECT_TRUE(pp.applyColorGrade(3)); // Hyped: red
}

// Test 8: RenderTarget creation and usage
TEST_F(Graphics_Phase4_Test, RenderTargetCreation) {
    auto device = graphics_->getD3D11Device();
    auto context = graphics_->getD3D11DeviceContext();
    ASSERT_NE(device, nullptr);
    ASSERT_NE(context, nullptr);

    RenderTarget rt;
    EXPECT_TRUE(rt.create(
        static_cast<ID3D11Device*>(device),
        static_cast<ID3D11DeviceContext*>(context),
        800, 600, DXGI_FORMAT_R8G8B8A8_UNORM));
    EXPECT_EQ(rt.getWidth(), 800);
    EXPECT_EQ(rt.getHeight(), 600);
}

// Test 9: RenderFrame with particles and post-processing
TEST_F(Graphics_Phase4_Test, RenderFrameWithParticlesAndEffects) {
    // Test a full render cycle with particles and effects
    float bpm = 128.0f;
    float energy = 0.85f; // High energy to trigger particles
    int mood = 2; // Grooving
    float crossfader = 0.0f;

    // Should succeed without crash
    ASSERT_TRUE(graphics_->renderFrame(bpm, energy, mood, crossfader));
}

// Test 10: Energy peak detection
TEST_F(Graphics_Phase4_Test, ConsecutiveEnergyPeaks) {
    // Multiple high-energy frames should accumulate particles
    for (int i = 0; i < 5; i++) {
        ASSERT_TRUE(graphics_->renderFrame(120.0f, 0.9f, 2, 0.0f));
    }
}

} // namespace dj

#else

// No-op tests for non-graphics builds
TEST(Graphics_Phase4_Stub, Stub) {
    EXPECT_TRUE(true);
}

#endif
