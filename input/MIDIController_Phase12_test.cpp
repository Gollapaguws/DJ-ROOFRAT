#include <cassert>
#include <iostream>
#include <vector>
#include <cstring>

#include "MIDIDevice.h"
#include "MIDIMessage.h"
#include "MIDIController.h"

// ============================================================================
// Test 1: MIDIDevice Enumeration
// ============================================================================
void test_MIDIDevice_Enumeration() {
    std::cout << "Test 1: MIDIDevice_Enumeration..." << std::endl;
    
    using namespace dj::midi;
    
    // Get device count
    int deviceCount = MIDIDevice::getDeviceCount();
    std::cout << "  Device count: " << deviceCount << std::endl;
    assert(deviceCount >= 0 && "Device count must be non-negative");
    
    if (deviceCount == 0) {
        std::cout << "  [SKIPPED] No MIDI devices found - hardware test skipped" << std::endl;
    } else {
        // Get info about first device
        auto deviceInfo = MIDIDevice::getDeviceInfo(0);
        assert(deviceInfo.has_value() && "Device 0 must exist if count > 0");
        assert(!deviceInfo.value().name.empty() && "Device name must not be empty");
        assert(deviceInfo.value().id == 0 && "First device ID must be 0");
        std::cout << "  Device 0: " << deviceInfo.value().name << std::endl;
    }
    
    std::cout << "  PASSED" << std::endl;
}

// ============================================================================
// Test 2: MIDIMessage Parsing
// ============================================================================
void test_MIDIMessage_Parsing() {
    std::cout << "Test 2: MIDIMessage_Parsing..." << std::endl;
    
    using namespace dj::midi;
    
    // Test NoteOn message: 0x90 0x3C 0x64 (Ch1, Note 60 Middle C, Velocity 100)
    {
        unsigned char bytes[3] = {0x90, 0x3C, 0x64};
        MIDIMessage msg = MIDIMessage::parseRawBytes(bytes, 3);
        
        assert(msg.getStatus() == MIDIStatus::NoteOn && "Must parse as NoteOn");
        assert(msg.getChannel() == 0 && "Channel must be 0");
        assert(msg.getNote() == 0x3C && "Note must be 60");
        assert(msg.getVelocity() == 0x64 && "Velocity must be 100");
        std::cout << "  NoteOn (Ch0, Note 60, Vel 100): OK" << std::endl;
    }
    
    // Test NoteOff message: 0x80 0x3C 0x40 (Ch1, Note 60, Velocity 64)
    {
        unsigned char bytes[3] = {0x80, 0x3C, 0x40};
        MIDIMessage msg = MIDIMessage::parseRawBytes(bytes, 3);
        
        assert(msg.getStatus() == MIDIStatus::NoteOff && "Must parse as NoteOff");
        assert(msg.getChannel() == 0 && "Channel must be 0");
        assert(msg.getNote() == 0x3C && "Note must be 60");
        assert(msg.getVelocity() == 0x40 && "Velocity must be 64");
        std::cout << "  NoteOff (Ch0, Note 60, Vel 64): OK" << std::endl;
    }
    
    // Test ControlChange message: 0xB0 0x07 0x50 (Ch1, Volume CC7, Value 80)
    {
        unsigned char bytes[3] = {0xB0, 0x07, 0x50};
        MIDIMessage msg = MIDIMessage::parseRawBytes(bytes, 3);
        
        assert(msg.getStatus() == MIDIStatus::ControlChange && "Must parse as ControlChange");
        assert(msg.getChannel() == 0 && "Channel must be 0");
        assert(msg.getController() == 0x07 && "Controller must be 7");
        assert(msg.getValue() == 0x50 && "Value must be 80");
        std::cout << "  ControlChange (Ch0, CC7 Volume, Val 80): OK" << std::endl;
    }
    
    // Test PitchBend message: 0xE0 0x00 0x40 (Ch1, Neutral pitch bend)
    {
        unsigned char bytes[3] = {0xE0, 0x00, 0x40};
        MIDIMessage msg = MIDIMessage::parseRawBytes(bytes, 3);
        
        assert(msg.getStatus() == MIDIStatus::PitchBend && "Must parse as PitchBend");
        assert(msg.getChannel() == 0 && "Channel must be 0");
        int bend = msg.getPitchBend();
        assert(bend == 0x2000 && "Neutral bend must be 0x2000 (8192)");
        std::cout << "  PitchBend (Ch0, Neutral): OK" << std::endl;
    }
    
    // Test PitchBend bend up: 0xE0 0x00 0x50 (higher pitch)
    {
        unsigned char bytes[3] = {0xE0, 0x00, 0x50};
        MIDIMessage msg = MIDIMessage::parseRawBytes(bytes, 3);
        
        int bend = msg.getPitchBend();
        assert(bend > 0x2000 && "Upward bend must be > 0x2000");
        std::cout << "  PitchBend (Ch0, Bend up): OK" << std::endl;
    }
    
    // Test different channel: NoteOn on Ch4 (0x93)
    {
        unsigned char bytes[3] = {0x93, 0x48, 0x50};
        MIDIMessage msg = MIDIMessage::parseRawBytes(bytes, 3);
        
        assert(msg.getStatus() == MIDIStatus::NoteOn && "Must parse as NoteOn");
        assert(msg.getChannel() == 3 && "Channel must be 3 (0-indexed)");
        assert(msg.getNote() == 0x48 && "Note must be 72");
        std::cout << "  NoteOn (Ch3, Note 72): OK" << std::endl;
    }
    
    std::cout << "  PASSED" << std::endl;
}

// ============================================================================
// Test 3: MIDIController Connection and Lifecycle
// ============================================================================
void test_MIDIController_Connection() {
    std::cout << "Test 3: MIDIController_Connection..." << std::endl;
    
    using namespace dj::midi;
    
    // Create controller (no device opened yet)
    MIDIController controller;
    assert(!controller.isOpen() && "Controller should not be open initially");
    
    // Try to open device 0 if available
    int deviceCount = MIDIDevice::getDeviceCount();
    if (deviceCount == 0) {
        std::cout << "  [SKIPPED] No MIDI devices - controller tests skipped" << std::endl;
    } else {
        bool opened = controller.open(0);
        assert(opened && "Controller should open successfully");
        assert(controller.isOpen() && "Controller should report as open");
        
        // Should not be able to open again without closing
        bool reopened = controller.open(0);
        assert(!reopened && "Should not be able to open twice");
        
        // Close the device
        controller.close();
        assert(!controller.isOpen() && "Controller should not be open after close");
        
        std::cout << "  Open/close lifecycle: OK" << std::endl;
    }
    
    std::cout << "  PASSED" << std::endl;
}

// ============================================================================
// Test 4: MIDIController Callback System
// ============================================================================
void test_MIDIController_Callback() {
    std::cout << "Test 4: MIDIController_Callback..." << std::endl;
    
    using namespace dj::midi;
    
    // Create controller
    MIDIController controller;
    
    // Track callback invocations
    struct CallbackTracker {
        int count = 0;
        MIDIStatus lastStatus = MIDIStatus::Unknown;
        int lastChannel = -1;
        int lastNote = -1;
        int lastVelocity = -1;
    } tracker;
    
    // Register callback
    controller.setMessageCallback([&tracker](const MIDIMessage& msg) {
        tracker.count++;
        tracker.lastStatus = msg.getStatus();
        tracker.lastChannel = msg.getChannel();
        if (msg.getStatus() == MIDIStatus::NoteOn || msg.getStatus() == MIDIStatus::NoteOff) {
            tracker.lastNote = msg.getNote();
            tracker.lastVelocity = msg.getVelocity();
        }
    });
    
    // Simulate raw MIDI message dispatch
    unsigned char noteOnBytes[3] = {0x90, 0x3C, 0x64};
    controller.onMIDIMessageRaw(noteOnBytes, 3);
    
    assert(tracker.count == 1 && "Callback should be invoked once");
    assert(tracker.lastStatus == MIDIStatus::NoteOn && "Callback should receive NoteOn");
    assert(tracker.lastNote == 0x3C && "Callback should receive correct note");
    assert(tracker.lastVelocity == 0x64 && "Callback should receive correct velocity");
    
    // Simulate another message
    unsigned char ccBytes[3] = {0xB0, 0x07, 0x50};
    controller.onMIDIMessageRaw(ccBytes, 3);
    
    assert(tracker.count == 2 && "Callback should be invoked twice");
    assert(tracker.lastStatus == MIDIStatus::ControlChange && "Callback should receive CC");
    
    std::cout << "  Single callback dispatcher: OK" << std::endl;
    std::cout << "  PASSED" << std::endl;
}

// ============================================================================
// Test 5: MIDIController Multiple Devices
// ============================================================================
void test_MIDIController_MultiDevice() {
    std::cout << "Test 5: MIDIController_MultiDevice..." << std::endl;
    
    using namespace dj::midi;
    
    int deviceCount = MIDIDevice::getDeviceCount();
    std::cout << "  Available devices: " << deviceCount << std::endl;
    
    if (deviceCount < 2) {
        std::cout << "  [SKIPPED] Fewer than 2 MIDI devices - multi-device test skipped" << std::endl;
    } else {
        // Create two controllers
        MIDIController controller1;
        MIDIController controller2;
        
        // Track which controller received messages
        int controller1_count = 0;
        int controller2_count = 0;
        
        controller1.setMessageCallback([&controller1_count](const MIDIMessage&) {
            controller1_count++;
        });
        
        controller2.setMessageCallback([&controller2_count](const MIDIMessage&) {
            controller2_count++;
        });
        
        // Open different devices
        bool open1 = controller1.open(0);
        bool open2 = controller2.open(1);
        
        if (open1 && open2) {
            // Simulate message on controller 1
            unsigned char noteOnBytes[3] = {0x90, 0x3C, 0x64};
            controller1.onMIDIMessageRaw(noteOnBytes, 3);
            
            assert(controller1_count == 1 && "Controller 1 should receive message");
            assert(controller2_count == 0 && "Controller 2 should not receive message");
            
            // Simulate message on controller 2
            controller2.onMIDIMessageRaw(noteOnBytes, 3);
            
            assert(controller1_count == 1 && "Controller 1 count should not change");
            assert(controller2_count == 1 && "Controller 2 should receive message");
            
            std::cout << "  Independent device callbacks: OK" << std::endl;
            
            controller1.close();
            controller2.close();
        }
    }
    
    std::cout << "  PASSED" << std::endl;
}

// ============================================================================
// Main Test Runner
// ============================================================================
int main() {
    std::cout << "=== DJ-ROOFRAT Phase 12: MIDI Controller Foundation Tests ===" << std::endl;
    std::cout << std::endl;
    
    try {
        test_MIDIDevice_Enumeration();
        std::cout << std::endl;
        
        test_MIDIMessage_Parsing();
        std::cout << std::endl;
        
        test_MIDIController_Connection();
        std::cout << std::endl;
        
        test_MIDIController_Callback();
        std::cout << std::endl;
        
        test_MIDIController_MultiDevice();
        std::cout << std::endl;
        
        std::cout << "=== ALL TESTS PASSED ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}
