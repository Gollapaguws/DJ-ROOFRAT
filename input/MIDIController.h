#pragma once

#include "MIDIMessage.h"
#include <functional>
#include <memory>

#if defined(_WIN32)
#include <windows.h>
#include <mmsystem.h>
#endif

namespace dj::midi {

/// Callback type for MIDI message events
using MIDIMessageCallback = std::function<void(const MIDIMessage&)>;

/// Manages a single MIDI input device connection
class MIDIController {
public:
    MIDIController();
    ~MIDIController();

    // Non-copyable
    MIDIController(const MIDIController&) = delete;
    MIDIController& operator=(const MIDIController&) = delete;

    // Moveable
    MIDIController(MIDIController&& other) noexcept;
    MIDIController& operator=(MIDIController&& other) noexcept;

    /// Open a MIDI input device
    /// @param deviceId Device index (0-based)
    /// @return true if successfully opened, false otherwise
    bool open(int deviceId);

    /// Close the currently open device
    void close();

    /// Check if a device is currently open
    /// @return true if device is open, false otherwise
    bool isOpen() const;

    /// Set the callback for MIDI messages
    /// @param callback Function to invoke when a MIDI message is received
    void setMessageCallback(MIDIMessageCallback callback);

    /// Handle raw MIDI message bytes (called from callback)
    /// Used internally and for testing
    void onMIDIMessageRaw(const unsigned char* bytes, int length);

private:
#if defined(_WIN32)
    HMIDIIN hMidiIn_ = nullptr;
#endif

    int deviceId_ = -1;
    bool isOpen_ = false;
    MIDIMessageCallback messageCallback_;

    // Window message pump for MIDI events (Windows-specific)
#if defined(_WIN32)
    static void CALLBACK MIDIInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance,
                                     DWORD_PTR dwParam1, DWORD_PTR dwParam2);
    void processMIDIInput(DWORD midiData);
#endif
};

} // namespace dj::midi
