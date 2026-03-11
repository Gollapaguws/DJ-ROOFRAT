#pragma once

#include <cstdint>
#include <optional>

namespace dj::midi {

enum class MIDIStatus {
    Unknown,
    NoteOff,      // 0x80
    NoteOn,       // 0x90
    ControlChange, // 0xB0
    PitchBend,    // 0xE0
};

/// Represents a parsed MIDI message
class MIDIMessage {
public:
    /// Parse raw MIDI bytes into a message
    /// @param bytes Raw MIDI bytes (typically 3 bytes)
    /// @param length Length of the bytes array
    /// @return Parsed MIDI message
    static MIDIMessage parseRawBytes(const unsigned char* bytes, int length);

    // Getters for message properties
    MIDIStatus getStatus() const { return status_; }
    int getChannel() const { return channel_; }
    int getNote() const { return note_; }
    int getVelocity() const { return velocity_; }
    int getController() const { return controller_; }
    int getValue() const { return value_; }
    int getPitchBend() const { return pitchBend_; }

private:
    MIDIStatus status_ = MIDIStatus::Unknown;
    int channel_ = 0;
    int note_ = 0;
    int velocity_ = 0;
    int controller_ = 0;
    int value_ = 0;
    int pitchBend_ = 0;

    // Private constructors for controlled creation
    MIDIMessage() = default;
    friend class MIDIMessage;
};

} // namespace dj::midi
