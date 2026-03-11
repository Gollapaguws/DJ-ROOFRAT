#pragma once

#include <string>
#include <optional>

namespace dj::midi {

/// Information about a MIDI input device
struct MIDIDeviceInfo {
    int id = -1;
    std::string name;
    bool isActive = false;
};

/// Provides MIDI device enumeration and information
class MIDIDevice {
public:
    /// Get the number of available MIDI input devices
    /// @return Number of devices (0 if none)
    static int getDeviceCount();

    /// Get information about a specific MIDI device
    /// @param deviceId Device index (0-based)
    /// @return Device info if available, std::nullopt otherwise
    static std::optional<MIDIDeviceInfo> getDeviceInfo(int deviceId);

private:
    MIDIDevice() = default;
};

} // namespace dj::midi
