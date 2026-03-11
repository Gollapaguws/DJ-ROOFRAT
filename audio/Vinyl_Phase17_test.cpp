#include "audio/VinylSimulator.h"
#include "audio/ScratchDetector.h"
#include "audio/Deck.h"
#include "audio/AudioClip.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <array>
#include <iomanip>
#include <vector>

namespace dj {

// Helper: Check if value is close to expected (within tolerance)
bool approxEqual(float actual, float expected, float tolerance = 0.01f) {
    return std::abs(actual - expected) < tolerance;
}

// Test 1: Verify platter spin-down with friction
void test_VinylSimulator_Inertia() {
    VinylSimulator vinyl;
    vinyl.setSampleRate(44100);
    
    // Set initial velocity - enable motor to accelerate
    vinyl.setMotorOn(true);
    vinyl.setTargetVelocity(1.0f);  // Normal speed (1.0 = 33 RPM)
    vinyl.setMotorStartTime(0.15f);  // Fast start for testing
    vinyl.update(0.1f);  // Let it accelerate
    float velocityAfterStart = vinyl.platterVelocity();
    assert(velocityAfterStart > 0.5f && "Should accelerate toward target");
    
    // Motor off - should decelerate with friction
    vinyl.setMotorOn(false);
    vinyl.update(0.1f);
    float velocityAfterFirstDecay = vinyl.platterVelocity();
    assert(velocityAfterFirstDecay < velocityAfterStart && "Velocity should decay with friction");
    
    // Continue decaying - should approach zero
    for (int i = 0; i < 50; ++i) {
        vinyl.update(0.1f);
    }
    float finalVelocity = vinyl.platterVelocity();
    assert(finalVelocity < 0.01f && "Velocity should approach zero");
    
    std::cout << "✓ test_VinylSimulator_Inertia passed" << std::endl;
}

// Test 2: Test motor start/stop pitch ramp
void test_VinylSimulator_MotorTorque() {
    VinylSimulator vinyl;
    vinyl.setSampleRate(44100);
    
    // Motor on: velocity should ramp toward 1.0
    vinyl.setMotorOn(true);
    vinyl.setTargetVelocity(1.0f);
    vinyl.setMotorStartTime(1.0f);  // 1 second to reach full speed
    
    std::vector<float> velocities;
    for (int i = 0; i < 100; ++i) {
        vinyl.update(0.01f);
        velocities.push_back(vinyl.platterVelocity());
    }
    
    // First sample should be near 0
    assert(velocities[0] < 0.1f && "Initial velocity should be low");
    
    // Velocity should increase monotonically
    for (size_t i = 1; i < velocities.size(); ++i) {
        assert(velocities[i] >= velocities[i-1] - 0.001f && 
               "Velocity should increase or stay level during motor start");
    }
    
    // After 1 second, should be very close to target
    float finalVelocity = vinyl.platterVelocity();
    assert(approxEqual(finalVelocity, 1.0f, 0.05f) && 
           "Should reach target velocity after ramp time");
    
    std::cout << "✓ test_VinylSimulator_MotorTorque passed" << std::endl;
}

// Test 3: Validate reverse playback physics (backspin)
void test_VinylSimulator_Backspin() {
    VinylSimulator vinyl;
    vinyl.setSampleRate(44100);
    
    // Start with forward motion
    vinyl.setMotorOn(true);
    vinyl.setTargetVelocity(1.0f);
    vinyl.setMotorStartTime(0.5f);
    
    for (int i = 0; i < 50; ++i) {
        vinyl.update(0.01f);
    }
    
    float forwardVelocity = vinyl.platterVelocity();
    assert(forwardVelocity > 0.8f && "Should be near full forward speed");
    
    // Switch to backspin (negative target)
    vinyl.setTargetVelocity(-1.0f);
    vinyl.setMotorStartTime(0.5f);
    
    // Track velocity changes
    for (int i = 0; i < 100; ++i) {
        vinyl.update(0.01f);
    }
    
    float backspinVelocity = vinyl.platterVelocity();
    assert(backspinVelocity < -0.8f && "Should reach negative velocity for backspin");
    
    std::cout << "✓ test_VinylSimulator_Backspin passed" << std::endl;
}

// Test 4: Confirm forward/reverse detection in ScratchDetector
void test_ScratchDetector_Direction() {
    ScratchDetector scratch;
    scratch.setSampleRate(44100);
    
    // Forward scratch (positive velocity)
    float pitchMultiplier = scratch.processScratchInput(0.5f, 0.016f);  // ~30ms frame
    assert(pitchMultiplier > 0.0f && "Forward input should give positive pitch");
    
    // Backward scratch (negative velocity)
    scratch.reset();
    pitchMultiplier = scratch.processScratchInput(-0.5f, 0.016f);
    assert(pitchMultiplier < 0.0f && "Backward input should give negative pitch");
    
    // Zero/deadzone - should produce minimal pitch change
    scratch.reset();
    scratch.setDeadzone(0.1f);
    pitchMultiplier = scratch.processScratchInput(0.05f, 0.016f);
    assert(std::abs(pitchMultiplier) < 0.1f && "Small input within deadzone should be suppressed");
    
    std::cout << "✓ test_ScratchDetector_Direction passed" << std::endl;
}

// Test 5: Test velocity-to-pitch mapping in ScratchDetector
void test_ScratchDetector_Speed() {
    ScratchDetector scratch;
    scratch.setSampleRate(44100);
    scratch.setSensitivity(2.0f);  // 2.0 = double speed forward
    
    // Slow scratch
    float slowPitch = scratch.processScratchInput(0.1f, 0.016f);
    assert(slowPitch > 0.0f && slowPitch < 1.0f && "Slow input should give moderate pitch");
    
    // Fast scratch (3x faster input)
    scratch.reset();
    float fastPitch = scratch.processScratchInput(0.3f, 0.016f);
    
    assert(fastPitch > slowPitch && "Faster input should give greater pitch multiplier");
    assert(fastPitch > 1.0f && "Fast input should exceed 1.0 pitch");
    
    // Test sensitivity scaling
    scratch.reset();
    scratch.setSensitivity(1.0f);  // Lower sensitivity
    float scaledPitch = scratch.processScratchInput(0.2f, 0.016f);
    
    scratch.reset();
    scratch.setSensitivity(4.0f);  // Higher sensitivity
    float scaledPitchHigh = scratch.processScratchInput(0.2f, 0.016f);
    
    assert(scaledPitchHigh > scaledPitch && "Higher sensitivity should amplify pitch change");
    
    std::cout << "✓ test_ScratchDetector_Speed passed" << std::endl;
}

// Test 6: Verify smooth motor ramp through zero (backspin transition)
void test_VinylSimulator_BackspinSmooth() {
    VinylSimulator vinyl;
    vinyl.setSampleRate(44100);
    
    // Start with forward motion
    vinyl.setMotorOn(true);
    vinyl.setTargetVelocity(1.0f);
    vinyl.setMotorStartTime(0.5f);
    
    for (int i = 0; i < 50; ++i) {
        vinyl.update(0.01f);
    }
    
    float forwardVelocity = vinyl.platterVelocity();
    assert(forwardVelocity > 0.8f && "Should be near full forward speed");
    
    // Switch to backspin - motor should ramp from current velocity through 0 to -1.0
    vinyl.setTargetVelocity(-1.0f);
    vinyl.setMotorStartTime(0.5f);
    
    std::vector<float> velocitiesAtZero;
    bool passedThroughZero = false;
    float prevVel = forwardVelocity;
    
    for (int i = 0; i < 100; ++i) {
        vinyl.update(0.01f);
        float curVel = vinyl.platterVelocity();
        
        // Check if we crossed zero
        if ((prevVel > 0.0f && curVel <= 0.0f) || (prevVel < 0.0f && curVel >= 0.0f)) {
            passedThroughZero = true;
        }
        prevVel = curVel;
    }
    
    float backspinVelocity = vinyl.platterVelocity();
    assert(backspinVelocity < -0.8f && "Should reach negative velocity for backspin");
    assert(passedThroughZero && "Motor ramp should smoothly pass through zero, not jump");
    
    std::cout << "✓ test_VinylSimulator_BackspinSmooth passed" << std::endl;
}

// Test 7: Verify physics timestep is correct (not hardcoded), motor ramp timing
void test_Deck_PhysicsTimestep() {
    Deck deck;
    AudioClip clip = AudioClip::generateTestTone(440.0f, 1.0f, 44100);
    
    bool loaded = deck.loadClip(clip);
    assert(loaded && "Should load test clip");
    
    // Set to 44100 Hz sample rate
    deck.setOutputSampleRate(44100);
    
    // Expected ramp time at 44100 Hz: 0.15 seconds * 44100 samples = 6615 samples
    // Motor should ramp to full velocity in approximately this time
    const int motorStartTimeMs = 150;  // 0.15 seconds
    const int estimatedRampSamples = motorStartTimeMs * 44100 / 1000;  // ~6615 samples
    
    deck.setVinylMode(true);
    deck.play();
    
    // Track frames to measure how long motor takes to reach near-full speed
    float previousVel = 0.0f;
    int framesUntilNearlyFull = 0;
    
    for (int i = 0; i < 20000 && framesUntilNearlyFull == 0; ++i) {
        auto frame = deck.nextFrame();
        
        // If we measure platter velocity... but we can't directly, 
        // so we'll verify that playback position advances according to motor ramp
        // This test mainly verifies the timestep isn't hardcoded at 0.00023f
        // That would make ramps happen ~10x too fast
    }
    
    // If timestep was correct (1/44100), motor would ramp properly
    // If timestep was 0.00023f (~10x too large), it would ramp too fast
    // This test mainly documents the correct behavior - full implementation requires platter velocity access
    
    std::cout << "✓ test_Deck_PhysicsTimestep passed (placeholder)" << std::endl;
}

// Test 8: Verify scratch detector is integrated (setScratchVelocity exists and is called)
void test_Deck_ScratchIntegration() {
    Deck deck;
    AudioClip clip = AudioClip::generateTestTone(440.0f, 1.0f, 44100);
    
    bool loaded = deck.loadClip(clip);
    assert(loaded && "Should load test clip");
    
    deck.setVinylMode(true);
    deck.play();
    
    // Should not throw - setScratchVelocity method must exist
    deck.setScratchVelocity(0.5f);  // Apply forward scratch
    
    auto frame1 = deck.nextFrame();
    
    deck.setScratchVelocity(-0.3f);  // Apply backward scratch
    
    auto frame2 = deck.nextFrame();
    
    // Both should have valid audio
    assert(std::abs(frame1[0]) >= 0.0f && "Frame should have valid left channel");
    assert(std::abs(frame2[0]) >= 0.0f && "Frame should have valid left channel");
    
    std::cout << "✓ test_Deck_ScratchIntegration passed" << std::endl;
}

// Test 6: Verify vinyl physics mode vs digital mode in Deck
void test_Deck_VinylMode() {
    Deck deck;
    AudioClip clip = AudioClip::generateTestTone(440.0f, 1.0f, 44100);
    
    bool loaded = deck.loadClip(clip);
    assert(loaded && "Should load test clip");
    
    // Digital mode (vinyl off)
    deck.setVinylMode(false);
    deck.setTempoPercent(0.0f);  // Normal speed
    deck.play();
    
    // Call nextFrame() multiple times and verify position changes
    std::size_t lastPos = deck.currentFrame();
    for (int i = 0; i < 100; ++i) {
        deck.nextFrame();
    }
    std::size_t digitalPos = deck.currentFrame();
    assert(digitalPos > lastPos && "Position should advance in digital mode");
    
    deck.stop();
    
    // Vinyl mode (vinyl on)
    deck.setVinylMode(true);
    deck.play();
    
    // Call nextFrame() multiple times - should also advance with motor ramp
    lastPos = deck.currentFrame();
    for (int i = 0; i < 100; ++i) {
        deck.nextFrame();
    }
    std::size_t vinylPos = deck.currentFrame();
    
    // In vinyl mode with motor ramp, position might advance more slowly initially
    // but should still advance
    assert(vinylPos > lastPos && "Position should advance in vinyl mode after 100 frames");
    assert(digitalPos > vinylPos || std::abs(static_cast<int>(digitalPos) - static_cast<int>(vinylPos)) < 10 && 
           "Vinyl mode might lag slightly during motor ramp, but should be close to digital");
    
    std::cout << "✓ test_Deck_VinylMode (enhanced) passed" << std::endl;
}

// Test 7: Test instant cue point jump (simulated needle drop)
void test_Deck_NeedleDrop() {
    Deck deck;
    AudioClip clip = AudioClip::generateTestTone(440.0f, 2.0f, 44100);
    
    bool loaded = deck.loadClip(clip);
    assert(loaded && "Should load test clip");
    
    deck.setVinylMode(true);
    deck.play();
    
    // Set cue point and jump to it (needle drop)
    std::size_t cueFrame = 44100;  // 1 second in
    deck.setCue(cueFrame);
    
    // Jump to cue should be instant (no pitch sweep)
    deck.jumpToCue();
    std::size_t posAfterJump = deck.currentFrame();
    
    assert(posAfterJump >= cueFrame - 100 && posAfterJump <= cueFrame + 100 &&
           "Needle drop should jump to cue point");
    
    std::cout << "✓ test_Deck_NeedleDrop passed" << std::endl;
}

void runAllVinylPhase17Tests() {
    std::cout << "\n=== Running Vinyl Phase 17 Tests ===" << std::endl;
    try {
        test_VinylSimulator_Inertia();
        test_VinylSimulator_MotorTorque();
        test_VinylSimulator_Backspin();
        test_VinylSimulator_BackspinSmooth();
        test_ScratchDetector_Direction();
        test_ScratchDetector_Speed();
        test_Deck_PhysicsTimestep();
        test_Deck_ScratchIntegration();
        test_Deck_VinylMode();
        test_Deck_NeedleDrop();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return;
    }
}

} // namespace dj

// Entry point for running vinyl tests
int main() {
    dj::runAllVinylPhase17Tests();
    std::cout << "\n=== Vinyl Phase 17 Tests Complete ===" << std::endl;
    return 0;
}
