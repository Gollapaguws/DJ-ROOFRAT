#include "MIDIController.h"

#if defined(_WIN32)
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")
#endif

namespace dj::midi {

MIDIController::MIDIController()
    : deviceId_(-1), isOpen_(false), messageCallback_(nullptr) {
}

MIDIController::~MIDIController() {
    close();
}

MIDIController::MIDIController(MIDIController&& other) noexcept
    : deviceId_(other.deviceId_), isOpen_(other.isOpen_), 
      messageCallback_(other.messageCallback_) {
#if defined(_WIN32)
    hMidiIn_ = other.hMidiIn_;
    other.hMidiIn_ = nullptr;
#endif
    other.deviceId_ = -1;
    other.isOpen_ = false;
}

MIDIController& MIDIController::operator=(MIDIController&& other) noexcept {
    if (this != &other) {
        close();
        deviceId_ = other.deviceId_;
        isOpen_ = other.isOpen_;
        messageCallback_ = other.messageCallback_;
#if defined(_WIN32)
        hMidiIn_ = other.hMidiIn_;
        other.hMidiIn_ = nullptr;
#endif
        other.deviceId_ = -1;
        other.isOpen_ = false;
    }
    return *this;
}

bool MIDIController::open(int deviceId) {
    if (isOpen_) {
        return false; // Already open
    }

#if defined(_WIN32)
    MMRESULT result = midiInOpen(&hMidiIn_, deviceId, 
                                  reinterpret_cast<DWORD_PTR>(MIDIInProc),
                                  reinterpret_cast<DWORD_PTR>(this), 
                                  CALLBACK_FUNCTION);

    if (result != MMSYSERR_NOERROR) {
        hMidiIn_ = nullptr;
        return false;
    }

    // Start receiving MIDI input
    result = midiInStart(hMidiIn_);
    if (result != MMSYSERR_NOERROR) {
        midiInClose(hMidiIn_);
        hMidiIn_ = nullptr;
        return false;
    }

    deviceId_ = deviceId;
    isOpen_ = true;
    return true;
#else
    // Non-Windows platforms would implement platform-specific opening here
    return false;
#endif
}

void MIDIController::close() {
    if (!isOpen_) {
        return;
    }

#if defined(_WIN32)
    if (hMidiIn_ != nullptr) {
        midiInStop(hMidiIn_);
        midiInClose(hMidiIn_);
        hMidiIn_ = nullptr;
    }
#endif

    deviceId_ = -1;
    isOpen_ = false;
}

bool MIDIController::isOpen() const {
    return isOpen_;
}

void MIDIController::setMessageCallback(MIDIMessageCallback callback) {
    messageCallback_ = callback;
}

void MIDIController::onMIDIMessageRaw(const unsigned char* bytes, int length) {
    if (messageCallback_) {
        MIDIMessage msg = MIDIMessage::parseRawBytes(bytes, length);
        messageCallback_(msg);
    }
}

#if defined(_WIN32)
void CALLBACK MIDIController::MIDIInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance,
                                          DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    (void)hMidiIn;
    (void)wMsg;
    (void)dwParam2;

    // Retrieve the MIDIController instance
    MIDIController* pThis = reinterpret_cast<MIDIController*>(dwInstance);
    if (!pThis) {
        return;
    }

    if (wMsg == MIM_DATA) {
        pThis->processMIDIInput(static_cast<DWORD>(dwParam1));
    }
}

void MIDIController::processMIDIInput(DWORD midiData) {
    // Extract the three MIDI bytes from the packed DWORD
    unsigned char bytes[3];
    bytes[0] = (midiData & 0xFF);         // Status byte
    bytes[1] = ((midiData >> 8) & 0xFF);  // Data byte 1
    bytes[2] = ((midiData >> 16) & 0xFF); // Data byte 2

    onMIDIMessageRaw(bytes, 3);
}
#endif

} // namespace dj::midi
