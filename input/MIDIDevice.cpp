#include "MIDIDevice.h"

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <vector>

#pragma comment(lib, "winmm.lib")
#endif

namespace dj::midi {

int MIDIDevice::getDeviceCount() {
#if defined(_WIN32)
    return midiInGetNumDevs();
#else
    // Non-Windows platforms would implement platform-specific enumeration here
    return 0;
#endif
}

std::optional<MIDIDeviceInfo> MIDIDevice::getDeviceInfo(int deviceId) {
#if defined(_WIN32)
    int numDevices = midiInGetNumDevs();
    if (deviceId < 0 || deviceId >= numDevices) {
        return std::nullopt;
    }

    MIDIINCAPS caps;
    MMRESULT result = midiInGetDevCaps(deviceId, &caps, sizeof(MIDIINCAPS));

    if (result != MMSYSERR_NOERROR) {
        return std::nullopt;
    }

    MIDIDeviceInfo info;
    info.id = deviceId;
    info.name = caps.szPname;
    info.isActive = true;

    return info;
#else
    return std::nullopt;
#endif
}

} // namespace dj::midi
