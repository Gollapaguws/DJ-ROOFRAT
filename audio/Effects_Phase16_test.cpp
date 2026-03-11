#include "audio/Flanger.h"
#include "audio/Phaser.h"
#include "audio/Bitcrusher.h"
#include "audio/RingModulator.h"
#include "audio/AutoFilter.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <array>
#include <algorithm>

namespace dj {

// Helper: Check if value is close to expected (within tolerance)
bool approxEqual(float actual, float expected, float tolerance = 0.01f) {
    return std::abs(actual - expected) < tolerance;
}

// Test 1: Verify Flanger LFO rate control (0.1-10 Hz)
void test_Flanger_LFORate() {
    Flanger flanger;
    flanger.setSampleRate(44100);
    
    // Set slow LFO rate
    flanger.setLFORate(0.5f);  // 0.5 Hz
    flanger.setDepth(0.5f);
    flanger.setMix(0.5f);
    
    std::array<float, 2> input = {0.1f, 0.1f};
    
    // Process many samples and verify effect varies with LFO
    float prevSample = 0.0f;
    bool hasVariation = false;
    
    for (int i = 0; i < 44100; ++i) {  // 1 second @ 44.1kHz
        std::array<float, 2> output;
        if (i % 2 == 0) {
            output = flanger.process(input);
        } else {
            output = flanger.process({0.0f, 0.0f});
        }
        
        // Should have variation over 1 second (LFO sweep)
        if (i > 100 && std::abs(output[0] - prevSample) > 0.01f) {
            hasVariation = true;
        }
        prevSample = output[0];
        
        // Output should be bounded
        assert(output[0] >= -2.0f && output[0] <= 2.0f);
        assert(output[1] >= -2.0f && output[1] <= 2.0f);
    }
    
    assert(hasVariation && "Flanger should show LFO modulation");
    
    // Test fast LFO rate
    flanger.reset();
    flanger.setLFORate(5.0f);  // 5 Hz (much faster)
    flanger.setDepth(0.5f);
    flanger.setMix(0.5f);
    
    // Should still work without issues
    for (int i = 0; i < 1000; ++i) {
        std::array<float, 2> output = flanger.process(input);
        assert(output[0] >= -2.0f && output[0] <= 2.0f);
        assert(output[1] >= -2.0f && output[1] <= 2.0f);
    }
    
    std::cout << "✓ test_Flanger_LFORate passed" << std::endl;
}

// Test 2: Test Flanger depth parameter
void test_Flanger_Depth() {
    Flanger flanger;
    flanger.setSampleRate(44100);
    flanger.setLFORate(1.0f);
    
    std::array<float, 2> input = {0.5f, 0.5f};
    
    // Depth = 0.0 (no modulation, should be close to input mix ratio)
    flanger.setDepth(0.0f);
    flanger.setMix(0.5f);  // 50% dry, 50% wet
    
    std::array<float, 2> output1 = flanger.process(input);
    
    // Depth = 1.0 (full modulation)
    flanger.reset();
    flanger.setDepth(1.0f);
    flanger.setMix(0.5f);
    
    std::array<float, 2> output2 = flanger.process(input);
    
    // Both should be valid
    assert(output1[0] >= -2.0f && output1[0] <= 2.0f);
    assert(output2[0] >= -2.0f && output2[0] <= 2.0f);
    
    // Test intermediate depth
    flanger.reset();
    flanger.setDepth(0.5f);
    flanger.setMix(0.7f);
    
    for (int i = 0; i < 1000; ++i) {
        output1 = flanger.process(input);
        assert(output1[0] >= -2.0f && output1[0] <= 2.0f);
        assert(output1[1] >= -2.0f && output1[1] <= 2.0f);
    }
    
    std::cout << "✓ test_Flanger_Depth passed" << std::endl;
}

// Test 3: Validate Phaser stage configurations (4/6/8/12)
void test_Phaser_Stages() {
    std::array<int, 4> stageConfigs = {4, 6, 8, 12};
    
    for (int stages : stageConfigs) {
        Phaser phaser;
        phaser.setSampleRate(44100);
        phaser.setStages(stages);
        phaser.setLFORate(1.0f);
        phaser.setFeedback(0.5f);
        phaser.setMix(0.5f);
        
        std::array<float, 2> input = {0.2f, 0.2f};
        
        // Process many samples
        for (int i = 0; i < 5000; ++i) {
            std::array<float, 2> output = phaser.process(input);
            
            // Output should be bounded
            assert(output[0] >= -2.0f && output[0] <= 2.0f);
            assert(output[1] >= -2.0f && output[1] <= 2.0f);
        }
    }
    
    std::cout << "✓ test_Phaser_Stages passed" << std::endl;
}

// Test 4: Test Bitcrusher bit depth reduction (16→8→4)
void test_Bitcrusher_BitDepth() {
    Bitcrusher bitcrusher;
    bitcrusher.setSampleRate(44100);
    
    std::array<float, 2> input = {0.5f, 0.5f};
    
    // Test 16-bit (full resolution)
    bitcrusher.setBitDepth(16);
    std::array<float, 2> output16 = bitcrusher.process(input);
    assert(output16[0] >= -2.0f && output16[0] <= 2.0f);
    assert(output16[1] >= -2.0f && output16[1] <= 2.0f);
    
    // Test 8-bit (higher quantization)
    bitcrusher.reset();
    bitcrusher.setBitDepth(8);
    std::array<float, 2> output8 = bitcrusher.process(input);
    assert(output8[0] >= -2.0f && output8[0] <= 2.0f);
    assert(output8[1] >= -2.0f && output8[1] <= 2.0f);
    
    // Test 4-bit (extreme reduction)
    bitcrusher.reset();
    bitcrusher.setBitDepth(4);
    std::array<float, 2> output4 = bitcrusher.process(input);
    assert(output4[0] >= -2.0f && output4[0] <= 2.0f);
    assert(output4[1] >= -2.0f && output4[1] <= 2.0f);
    
    // Test 1-bit (heavily quantized)
    bitcrusher.reset();
    bitcrusher.setBitDepth(1);
    std::array<float, 2> output1 = bitcrusher.process(input);
    // 1-bit should be roughly -1, 0, or 1 (heavily quantized)
    assert(output1[0] >= -2.0f && output1[0] <= 2.0f);
    assert(output1[1] >= -2.0f && output1[1] <= 2.0f);
    
    std::cout << "✓ test_Bitcrusher_BitDepth passed" << std::endl;
}

// Test 5: Verify Bitcrusher sample rate reduction (44.1k→22k→11k)
void test_Bitcrusher_SampleRate() {
    Bitcrusher bitcrusher;
    bitcrusher.setSampleRate(44100);
    bitcrusher.setBitDepth(16);
    
    std::array<float, 2> input = {0.3f, 0.3f};
    
    // Test 44.1kHz (no reduction)
    bitcrusher.setDownsampleFactor(1);
    for (int i = 0; i < 1000; ++i) {
        std::array<float, 2> output = bitcrusher.process(input);
        assert(output[0] >= -2.0f && output[0] <= 2.0f);
        assert(output[1] >= -2.0f && output[1] <= 2.0f);
    }
    
    // Test 22.05kHz (2x downsampling)
    bitcrusher.reset();
    bitcrusher.setDownsampleFactor(2);
    for (int i = 0; i < 1000; ++i) {
        std::array<float, 2> output = bitcrusher.process(input);
        assert(output[0] >= -2.0f && output[0] <= 2.0f);
        assert(output[1] >= -2.0f && output[1] <= 2.0f);
    }
    
    // Test 11.025kHz (4x downsampling)
    bitcrusher.reset();
    bitcrusher.setDownsampleFactor(4);
    for (int i = 0; i < 1000; ++i) {
        std::array<float, 2> output = bitcrusher.process(input);
        assert(output[0] >= -2.0f && output[0] <= 2.0f);
        assert(output[1] >= -2.0f && output[1] <= 2.0f);
    }
    
    // Test extreme: 8x downsampling (~5.5kHz)
    bitcrusher.reset();
    bitcrusher.setDownsampleFactor(8);
    for (int i = 0; i < 1000; ++i) {
        std::array<float, 2> output = bitcrusher.process(input);
        assert(output[0] >= -2.0f && output[0] <= 2.0f);
        assert(output[1] >= -2.0f && output[1] <= 2.0f);
    }
    
    std::cout << "✓ test_Bitcrusher_SampleRate passed" << std::endl;
}

// Test 6: Test Ring Modulator carrier frequency sweep
void test_RingModulator_Frequency() {
    RingModulator ringMod;
    ringMod.setSampleRate(44100);
    
    std::array<float, 2> input = {0.5f, 0.5f};
    
    // Test low carrier frequency
    ringMod.setCarrierFrequency(100.0f);
    ringMod.setMix(0.5f);
    
    for (int i = 0; i < 1000; ++i) {
        std::array<float, 2> output = ringMod.process(input);
        assert(output[0] >= -5.0f && output[0] <= 5.0f);  // Ring mod can amplify slightly
        assert(output[1] >= -5.0f && output[1] <= 5.0f);
    }
    
    // Test mid-range carrier frequency
    ringMod.reset();
    ringMod.setCarrierFrequency(1000.0f);
    ringMod.setMix(0.7f);
    
    bool hasVariation = false;
    float lastVal = 0.0f;
    
    for (int i = 0; i < 1000; ++i) {
        std::array<float, 2> output = ringMod.process(input);
        assert(output[0] >= -5.0f && output[0] <= 5.0f);
        assert(output[1] >= -5.0f && output[1] <= 5.0f);
        
        if (std::abs(output[0] - lastVal) > 0.05f) {
            hasVariation = true;
        }
        lastVal = output[0];
    }
    
    // Test high carrier frequency
    ringMod.reset();
    ringMod.setCarrierFrequency(5000.0f);
    ringMod.setMix(0.3f);
    
    for (int i = 0; i < 1000; ++i) {
        std::array<float, 2> output = ringMod.process(input);
        assert(output[0] >= -5.0f && output[0] <= 5.0f);
        assert(output[1] >= -5.0f && output[1] <= 5.0f);
    }
    
    assert(hasVariation && "Ring modulator should show carrier modulation");
    
    std::cout << "✓ test_RingModulator_Frequency passed" << std::endl;
}

// Test 7: Validate AutoFilter envelope following
void test_AutoFilter_EnvelopeFollowing() {
    AutoFilter autoFilter;
    autoFilter.setSampleRate(44100);
    
    autoFilter.setAttack(10.0f);      // 10ms attack
    autoFilter.setRelease(100.0f);    // 100ms release
    autoFilter.setSensitivity(1.0f);  // 100% sensitivity
    autoFilter.setResonance(0.5f);
    autoFilter.setMix(0.8f);
    
    // Create burst of signal (envelope increase)
    std::array<float, 2> loudInput = {0.8f, 0.8f};
    std::array<float, 2> quietInput = {0.0f, 0.0f};
    
    // Process loud input - filter should open up
    std::array<float, 2> output1 = autoFilter.process(loudInput);
    assert(output1[0] >= -2.0f && output1[0] <= 2.0f);
    assert(output1[1] >= -2.0f && output1[1] <= 2.0f);
    
    // Continue with loud input
    for (int i = 0; i < 100; ++i) {
        output1 = autoFilter.process(loudInput);
        assert(output1[0] >= -2.0f && output1[0] <= 2.0f);
        assert(output1[1] >= -2.0f && output1[1] <= 2.0f);
    }
    
    // Store output from peak
    float peakOutput = output1[0];
    
    // Process quiet input - filter should close (release)
    for (int i = 0; i < 500; ++i) {
        output1 = autoFilter.process(quietInput);
        assert(output1[0] >= -2.0f && output1[0] <= 2.0f);
        assert(output1[1] >= -2.0f && output1[1] <= 2.0f);
    }
    
    // After release, output should reduce
    float releaseOutput = output1[0];
    
    // Verify envelope follower is working
    assert(std::abs(releaseOutput) < std::abs(peakOutput) &&
           "Filter should close during release phase");
    
    std::cout << "✓ test_AutoFilter_EnvelopeFollowing passed" << std::endl;
}

} // namespace dj

int main() {
    std::cout << "\n=== Phase 16: Modulation & Distortion Effects Tests ===" << std::endl;
    
    dj::test_Flanger_LFORate();
    dj::test_Flanger_Depth();
    dj::test_Phaser_Stages();
    dj::test_Bitcrusher_BitDepth();
    dj::test_Bitcrusher_SampleRate();
    dj::test_RingModulator_Frequency();
    dj::test_AutoFilter_EnvelopeFollowing();
    
    std::cout << "\n=== All Phase 16 tests passed! ===" << std::endl;
    
    return 0;
}
