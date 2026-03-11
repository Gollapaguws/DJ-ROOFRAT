#pragma once

#include <string>
#include <optional>
#include <chrono>
#include "MIDIMapping.h"
#include "MIDIMessage.h"

namespace dj::input {

/// State machine for MIDI learn mode
enum class LearnState {
    Idle,       // Not learning
    Learning,   // Waiting for MIDI input
    Complete,   // Successfully captured a mapping
};

/// Manages MIDI learn mode for easy controller setup
/// User enters learn mode for a specific function, then sends a MIDI message
/// The learned mapping is created automatically
class MIDILearnMode {
public:
    MIDILearnMode();
    ~MIDILearnMode() = default;

    /// Enter learn mode for a specific DJ function
    /// @param functionName Name of the target function (e.g., "deckA_volume")
    void enterLearnMode(const std::string& functionName);

    /// Exit learn mode without creating a mapping
    void cancelLearnMode();

    /// Check if currently in learn mode
    /// @return true if waiting for MIDI input, false otherwise
    bool isLearning() const { return state_ == LearnState::Learning; }

    /// Get current learn state
    LearnState getState() const { return state_; }

    /// Get the function name being learned (if in learning state)
    const std::string& getCurrentFunction() const { return currentFunction_; }

    /// Process an incoming MIDI message during learn mode
    /// @param message The MIDI message to process
    /// @return Mapping if successfully learned, std::nullopt if not in learning mode
    std::optional<MIDIMapping> processMIDIInput(const dj::midi::MIDIMessage& message);

    /// Set learn mode timeout (seconds, 0 = no timeout)
    void setLearnTimeout(int seconds) { learnTimeoutSeconds_ = seconds; }

    /// Check if learn mode has timed out
    /// @return true if timeout is set and exceeded
    bool hasTimedOut() const;

private:
    LearnState state_ = LearnState::Idle;
    std::string currentFunction_;
    std::chrono::steady_clock::time_point learnStartTime_;
    int learnTimeoutSeconds_ = 10;  // Default 10-second timeout

    /// Create a mapping from the learned MIDI message
    MIDIMapping createMappingFromMessage(const dj::midi::MIDIMessage& message);
};

} // namespace dj::input
