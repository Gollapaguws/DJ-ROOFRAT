#include "MIDILearnMode.h"
#include <iostream>

namespace dj::input {

MIDILearnMode::MIDILearnMode() = default;

void MIDILearnMode::enterLearnMode(const std::string& functionName) {
    state_ = LearnState::Learning;
    currentFunction_ = functionName;
    learnStartTime_ = std::chrono::steady_clock::now();
    
    std::cout << "[MIDI Learn] Waiting for input to map to function: " << functionName << std::endl;
}

void MIDILearnMode::cancelLearnMode() {
    state_ = LearnState::Idle;
    currentFunction_.clear();
}

bool MIDILearnMode::hasTimedOut() const {
    if (learnTimeoutSeconds_ <= 0 || state_ != LearnState::Learning) {
        return false;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - learnStartTime_);
    return elapsed.count() >= learnTimeoutSeconds_;
}

MIDIMapping MIDILearnMode::createMappingFromMessage(const dj::midi::MIDIMessage& message) {
    MIDIMapping mapping;
    
    mapping.setTargetFunction(currentFunction_);
    mapping.setChannel(message.getChannel());
    
    switch (message.getStatus()) {
    case dj::midi::MIDIStatus::NoteOn:
    case dj::midi::MIDIStatus::NoteOff:
        mapping.setMessageType(MIDICurveType::Note);
        mapping.setNote(message.getNote());
        break;
        
    case dj::midi::MIDIStatus::ControlChange:
        mapping.setMessageType(MIDICurveType::ControlChange);
        mapping.setControllerNumber(message.getController());
        break;
        
    case dj::midi::MIDIStatus::PitchBend:
        mapping.setMessageType(MIDICurveType::PitchBend);
        // Check if this is a jog wheel (fast pitch bends indicate velocity)
        mapping.setJogWheelAcceleration(true);
        mapping.setValueRange(-1.0f, 1.0f);
        break;
        
    default:
        break;
    }
    
    return mapping;
}

std::optional<MIDIMapping> MIDILearnMode::processMIDIInput(const dj::midi::MIDIMessage& message) {
    if (state_ != LearnState::Learning) {
        return std::nullopt;
    }
    
    if (hasTimedOut()) {
        std::cout << "[MIDI Learn] Learn mode timed out" << std::endl;
        cancelLearnMode();
        return std::nullopt;
    }
    
    // Ignore unknown messages
    if (message.getStatus() == dj::midi::MIDIStatus::Unknown) {
        return std::nullopt;
    }
    
    // Create mapping from this message
    MIDIMapping mapping = createMappingFromMessage(message);
    
    std::cout << "[MIDI Learn] Captured mapping for " << currentFunction_ << std::endl;
    
    // Exit learning mode
    state_ = LearnState::Complete;
    currentFunction_.clear();
    
    return mapping;
}

} // namespace dj::input
