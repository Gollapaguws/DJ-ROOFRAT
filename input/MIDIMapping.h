#pragma once

#include <string>
#include <optional>
#include <cstdint>

namespace dj::input {

// Message types for MIDI mapping targets
enum class MIDICurveType {
    ControlChange,  // CC (0xB0)
    Note,           // NoteOn/NoteOff (0x90/0x80)
    PitchBend,      // Pitch bend (0xE0)
};

// Curve application types for value mapping
enum class CurveType {
    Linear,         // Direct 0-127 -> 0.0-1.0 mapping
    Logarithmic,    // Log curve for perceived volume/gain
    Exponential,    // Exponential curve for velocity sensitivity
};

/// Represents a single MIDI control mapping
/// Maps a MIDI message (CC, Note, or PitchBend) to a DJ function with curve application
class MIDIMapping {
public:
    MIDIMapping();
    ~MIDIMapping() = default;

    // Configuration
    void setMessageType(MIDICurveType type) { messageType_ = type; }
    void setChannel(int channel) { channel_ = channel; }
    void setControllerNumber(int ccNumber) { controllerNumber_ = ccNumber; }
    void setNote(int note) { note_ = note; }
    void setCurveType(CurveType curve) { curveType_ = curve; }
    void setValueRange(float minValue, float maxValue);
    void setTargetFunction(const std::string& function) { targetFunction_ = function; }
    void setJogWheelAcceleration(bool enabled) { jogWheelAcceleration_ = enabled; }

    // Getters
    MIDICurveType getMessageType() const { return messageType_; }
    int getChannel() const { return channel_; }
    int getControllerNumber() const { return controllerNumber_; }
    int getNote() const { return note_; }
    CurveType getCurveType() const { return curveType_; }
    float getMinValue() const { return minValue_; }
    float getMaxValue() const { return maxValue_; }
    const std::string& getTargetFunction() const { return targetFunction_; }
    bool hasJogWheelAcceleration() const { return jogWheelAcceleration_; }

    /// Apply mapping curve to normalize MIDI value (0-127 or 0-16383) to target range
    /// @param midiValue Raw MIDI value (0-127 for CC/Note, 0-16383 for 14-bit PitchBend)
    /// @return Mapped value in the configured range (default 0.0-1.0)
    float applyMapping(int midiValue) const;

    /// Serialize mapping to JSON string
    /// @return JSON representation
    std::string toJSON() const;

    /// Deserialize mapping from JSON string
    /// @param json JSON representation
    /// @return Deserialized mapping, or std::nullopt if invalid JSON
    static std::optional<MIDIMapping> fromJSON(const std::string& json);

private:
    // Message configuration
    MIDICurveType messageType_ = MIDICurveType::ControlChange;
    int channel_ = 0;               // 0-15 for MIDI channels
    int controllerNumber_ = 0;      // CC number (0-119)
    int note_ = 60;                 // Note number (0-127), middle C = 60

    // Curve and range
    CurveType curveType_ = CurveType::Linear;
    float minValue_ = 0.0f;         // Output range minimum
    float maxValue_ = 1.0f;         // Output range maximum
    std::string targetFunction_;    // Target DJ function name

    // Special handling
    bool jogWheelAcceleration_ = false;  // Enable acceleration for jog wheels

    // Curve application functions
    float applyLinearCurve(float normalized) const;
    float applyLogarithmicCurve(float normalized) const;
    float applyExponentialCurve(float normalized) const;
};

} // namespace dj::input
