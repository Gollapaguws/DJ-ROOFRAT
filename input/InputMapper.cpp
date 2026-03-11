#include "input/InputMapper.h"

#include <algorithm>
#include <cctype>

namespace dj {

InputCommand InputMapper::parse(const std::string& rawCommand) {
    std::string command = rawCommand;
    std::transform(command.begin(), command.end(), command.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (command == "playa" || command == "a play") {
        return InputCommand::PlayA;
    }
    if (command == "togglea" || command == "a toggle") {
        return InputCommand::PlayPauseA;
    }
    if (command == "playb" || command == "b play") {
        return InputCommand::PlayB;
    }
    if (command == "toggleb" || command == "b toggle") {
        return InputCommand::PlayPauseB;
    }
    if (command == "pausea" || command == "a pause") {
        return InputCommand::PauseA;
    }
    if (command == "pauseb" || command == "b pause") {
        return InputCommand::PauseB;
    }
    if (command == "xf left" || command == "crossfade left") {
        return InputCommand::CrossfadeLeft;
    }
    if (command == "xf center" || command == "crossfade center") {
        return InputCommand::CrossfadeCenter;
    }
    if (command == "xf right" || command == "crossfade right") {
        return InputCommand::CrossfadeRight;
    }
    if (command == "tempo b+" || command == "tempo up" || command == "nudge up") {
        return InputCommand::NudgeTempoBUp;
    }
    if (command == "tempo b-" || command == "tempo down" || command == "nudge down") {
        return InputCommand::NudgeTempoBDown;
    }
    if (command == "tempo reset" || command == "tempo b reset") {
        return InputCommand::ResetTempoB;
    }
    if (command == "loop a") {
        return InputCommand::ToggleLoopA;
    }
    if (command == "cue a set") {
        return InputCommand::SetCueA;
    }
    if (command == "cue a jump") {
        return InputCommand::JumpCueA;
    }
    if (command == "loop b" || command == "toggle loop") {
        return InputCommand::ToggleLoopB;
    }
    if (command == "cue b set" || command == "set cue") {
        return InputCommand::SetCueB;
    }
    if (command == "cue b jump" || command == "jump cue") {
        return InputCommand::JumpCueB;
    }
    if (command == "quit" || command == "exit") {
        return InputCommand::Quit;
    }
    // Phase 4: Multi-cue and tempo ramp commands
    if (command == "cue a1 set") {
        return InputCommand::SetCueA1;
    }
    if (command == "cue a2 set") {
        return InputCommand::SetCueA2;
    }
    if (command == "cue a3 set") {
        return InputCommand::SetCueA3;
    }
    if (command == "cue a1 jump") {
        return InputCommand::JumpCueA1;
    }
    if (command == "cue a2 jump") {
        return InputCommand::JumpCueA2;
    }
    if (command == "cue a3 jump") {
        return InputCommand::JumpCueA3;
    }
    if (command == "cue b1 set") {
        return InputCommand::SetCueB1;
    }
    if (command == "cue b2 set") {
        return InputCommand::SetCueB2;
    }
    if (command == "cue b3 set") {
        return InputCommand::SetCueB3;
    }
    if (command == "cue b1 jump") {
        return InputCommand::JumpCueB1;
    }
    if (command == "cue b2 jump") {
        return InputCommand::JumpCueB2;
    }
    if (command == "cue b3 jump") {
        return InputCommand::JumpCueB3;
    }
    if (command == "tempo ramp toggle") {
        return InputCommand::TempoRampToggle;
    }

    return InputCommand::None;
}

InputCommand InputMapper::parseKey(char rawKey) {
    const char lowerKey = static_cast<char>(std::tolower(static_cast<unsigned char>(rawKey)));
    const bool isShifted = (rawKey != lowerKey);  // Heuristic: treat uppercase as "Shift held" (Windows console / common layouts; may not hold for all layouts or input methods)
    
    if (isShifted) {
        // Shifted keys for isolators, filter order, and Phase 4 multi-cue jumps
        switch (lowerKey) {
        case 'q':
            return InputCommand::IsolatorLowA;
        case 'w':
            return InputCommand::IsolatorMidA;
        case 'e':
            return InputCommand::IsolatorHighA;
        case 'd':
            return InputCommand::IsolatorLowB;
        case 'f':
            return InputCommand::IsolatorMidB;
        case 'g':
            return InputCommand::IsolatorHighB;
        case 'u':
        case 'p':
            return InputCommand::FilterOrderToggle;
        // Phase 4: Jump to cue banks
        case '1':
            return InputCommand::JumpCueA1;
        case '2':
            return InputCommand::JumpCueA2;
        case '3':
            return InputCommand::JumpCueA3;
        case '4':
            return InputCommand::JumpCueB1;
        case '5':
            return InputCommand::JumpCueB2;
        case '6':
            return InputCommand::JumpCueB3;
        case 'r':
            return InputCommand::TempoRampToggle;
        case 's':
            return InputCommand::SaveRecording;
        default:
            return InputCommand::None;
        }
    }
    
    // Non-shifted keys (normal EQ controls)
    const char key = lowerKey;
    switch (key) {
    case 's':
        return InputCommand::RecordToggle;
    case '1':
        return InputCommand::SetCueA1;  // Phase 4: Multi-cue A1 set
    case '2':
        return InputCommand::SetCueA2;  // Phase 4: Multi-cue A2 set
    case '3':
        return InputCommand::SetCueA3;  // Phase 4: Multi-cue A3 set
    case '4':
        return InputCommand::SetCueB1;  // Phase 4: Multi-cue B1 set
    case '5':
        return InputCommand::SetCueB2;  // Phase 4: Multi-cue B2 set
    case '6':
        return InputCommand::SetCueB3;  // Phase 4: Multi-cue B3 set
    case '[':
        return InputCommand::CrossfadeLeft;
    case '\\':
        return InputCommand::CrossfadeCenter;
    case ']':
        return InputCommand::CrossfadeRight;
    case 'a':
        return InputCommand::PlayPauseA;
    case 'b':
        return InputCommand::PlayPauseB;
    case 'z':
        return InputCommand::NudgeTempoAUp;
    case 'x':
        return InputCommand::NudgeTempoADown;
    case 'i':
        return InputCommand::NudgeTempoBUp;
    case 'k':
        return InputCommand::NudgeTempoBDown;
    case 'o':
        return InputCommand::ResetTempoB;
    case 'l':
        return InputCommand::ToggleLoopB;
    case 'c':
        return InputCommand::ResetTempoA;
    case 'v':
        return InputCommand::JumpCueB;
    case ';':
        return InputCommand::LoopBeatsToggleA;
    case '\'':
        return InputCommand::LoopBeatsToggleB;
    case 'q':
        return InputCommand::DeckALowDown;
    case 'w':
        return InputCommand::DeckALowUp;
    case 'e':
        return InputCommand::DeckAMidDown;
    case 'r':
        return InputCommand::DeckAMidUp;
    case 't':
        return InputCommand::DeckAHighDown;
    case 'y':
        return InputCommand::DeckAHighUp;
    case 'u':
        return InputCommand::DeckAFilterDown;
    case 'p':
        return InputCommand::DeckAFilterUp;
    case 'd':
        return InputCommand::DeckBLowDown;
    case 'f':
        return InputCommand::DeckBLowUp;
    case 'g':
        return InputCommand::DeckBMidDown;
    case 'h':
        return InputCommand::DeckBMidUp;
    case 'j':
        return InputCommand::DeckBHighDown;
    case 'n':
        return InputCommand::DeckBHighUp;
    case 'm':
        return InputCommand::DeckBFilterDown;
    case ',':
        return InputCommand::DeckBFilterUp;
    default:
        return InputCommand::None;
    }
}

} // namespace dj
