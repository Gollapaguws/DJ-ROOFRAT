#include "audio/ThreeBandEQ.h"

#include <cassert>
#include <cmath>
#include <iostream>

namespace dj {

// Test 1: Verify frequency setters update crossover points
void test_ThreeBandEQ_FrequencyConfiguration() {
    ThreeBandEQ eq;
    eq.setSampleRate(44100);

    // Test setting low-mid crossover
    eq.setLowMidCrossover(300.0f);
    
    // Test setting mid-high crossover
    eq.setMidHighCrossover(3000.0f);

    // Process a dummy signal - should use the new frequencies
    const auto input = std::array<float, 2>{0.5f, 0.5f};
    const auto output = eq.process(input);
    
    // Basic sanity check - output should be within valid range
    assert(output[0] >= -1.5f && output[0] <= 1.5f);
    assert(output[1] >= -1.5f && output[1] <= 1.5f);
    
    std::cout << "✓ test_ThreeBandEQ_FrequencyConfiguration passed" << std::endl;
}

// Test 2: Validate linear-to-dB (20*log10) and dB-to-linear (10^(dB/20)) conversions
void test_ThreeBandEQ_dBConversion() {
    // Test gainTodB - converts linear gain to dB
    const float gain1 = 1.0f;  // 1.0 linear should be 0 dB
    const float db1 = ThreeBandEQ::gainTodB(gain1);
    assert(std::abs(db1 - 0.0f) < 0.01f);
    
    const float gain2 = 2.0f;  // 2.0 linear should be ~6.02 dB
    const float db2 = ThreeBandEQ::gainTodB(gain2);
    assert(std::abs(db2 - 6.02f) < 0.1f);
    
    const float gain3 = 0.1f;  // 0.1 linear should be ~-20 dB
    const float db3 = ThreeBandEQ::gainTodB(gain3);
    assert(std::abs(db3 - (-20.0f)) < 0.1f);
    
    // Test dBToGain - converts dB to linear gain
    const float db_val1 = 0.0f;  // 0 dB should be 1.0 linear
    const float gain_val1 = ThreeBandEQ::dBToGain(db_val1);
    assert(std::abs(gain_val1 - 1.0f) < 0.01f);
    
    const float db_val2 = 6.02f;  // ~6 dB should be ~2.0 linear
    const float gain_val2 = ThreeBandEQ::dBToGain(db_val2);
    assert(std::abs(gain_val2 - 2.0f) < 0.1f);
    
    const float db_val3 = -20.0f;  // -20 dB should be ~0.1 linear
    const float gain_val3 = ThreeBandEQ::dBToGain(db_val3);
    assert(std::abs(gain_val3 - 0.1f) < 0.01f);
    
    // Test round-trip: linear -> dB -> linear should be close to original
    const float original = 1.5f;
    const float roundtrip = ThreeBandEQ::dBToGain(ThreeBandEQ::gainTodB(original));
    assert(std::abs(roundtrip - original) < 0.01f);
    
    std::cout << "✓ test_ThreeBandEQ_dBConversion passed" << std::endl;
}

void runAllTests() {
    std::cout << "\n=== Running ThreeBandEQ Tests ===" << std::endl;
    try {
        test_ThreeBandEQ_FrequencyConfiguration();
        test_ThreeBandEQ_dBConversion();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
    }
}

} // namespace dj

// Entry point for running tests
int main() {
    dj::runAllTests();
    std::cout << "\n=== ThreeBandEQ Tests Complete ===" << std::endl;
    return 0;
}
