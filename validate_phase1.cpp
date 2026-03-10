#include <iostream>
#include <cmath>
#include <cassert>

// Quick inline validation of dB conversion formulas
void validateDBConversions() {
    // Test gainTodB formula
    auto gainTodB = [](float linear) -> float {
        if (linear <= 0.0f) return -80.0f;
        return 20.0f * std::log10(linear);
    };
    
    // Test dBToGain formula
    auto dBToGain = [](float dB) -> float {
        return std::pow(10.0f, dB / 20.0f);
    };
    
    // Test cases
    float testGains[] = {0.1f, 0.5f, 1.0f, 1.5f, 2.0f};
    
    for (float gain : testGains) {
        float db = gainTodB(gain);
        float recoveredGain = dBToGain(db);
        float error = std::abs(recoveredGain - gain);
        
        std::cout << "Gain: " << gain << " -> dB: " << db 
                  << " -> Recovered: " << recoveredGain 
                  << " (error: " << error << ")" << std::endl;
        
        assert(error < 0.01f);  // Round-trip should be accurate
    }
    
    std::cout << "✓ dB conversion validation passed\n" << std::endl;
}

int main() {
    std::cout << "=== Phase 1 Implementation Validation ===\n" << std::endl;
    validateDBConversions();
    std::cout << "All validations passed!" << std::endl;
    return 0;
}
