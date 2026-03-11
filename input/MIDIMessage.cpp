#include "MIDIMessage.h"
#include <cstring>

namespace dj::midi {

MIDIMessage MIDIMessage::parseRawBytes(const unsigned char* bytes, int length) {
    MIDIMessage msg;

    if (length < 1) {
        return msg;
    }

    unsigned char statusByte = bytes[0];
    unsigned char status = statusByte & 0xF0;
    unsigned char channel = statusByte & 0x0F;

    msg.channel_ = channel;

    // Parse based on status byte
    switch (status) {
        case 0x80: // Note Off
            msg.status_ = MIDIStatus::NoteOff;
            if (length >= 3) {
                msg.note_ = bytes[1] & 0x7F;
                msg.velocity_ = bytes[2] & 0x7F;
            }
            break;

        case 0x90: // Note On
            msg.status_ = MIDIStatus::NoteOn;
            if (length >= 3) {
                msg.note_ = bytes[1] & 0x7F;
                msg.velocity_ = bytes[2] & 0x7F;
            }
            break;

        case 0xB0: // Control Change
            msg.status_ = MIDIStatus::ControlChange;
            if (length >= 3) {
                msg.controller_ = bytes[1] & 0x7F;
                msg.value_ = bytes[2] & 0x7F;
            }
            break;

        case 0xE0: // Pitch Bend
            msg.status_ = MIDIStatus::PitchBend;
            if (length >= 3) {
                // Pitch bend is 14-bit: LSB (bytes[1]) and MSB (bytes[2])
                // Combine them: (MSB << 7) | LSB
                int lsb = bytes[1] & 0x7F;
                int msb = bytes[2] & 0x7F;
                msg.pitchBend_ = (msb << 7) | lsb;
            }
            break;

        default:
            msg.status_ = MIDIStatus::Unknown;
            break;
    }

    return msg;
}

} // namespace dj::midi
