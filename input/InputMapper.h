#pragma once

#include <string>

namespace dj {

enum class InputCommand {
    None,
    PlayPauseA,
    PlayPauseB,
    PlayA,
    PlayB,
    PauseA,
    PauseB,
    CrossfadeLeft,
    CrossfadeCenter,
    CrossfadeRight,
    NudgeTempoBUp,
    NudgeTempoBDown,
    ResetTempoB,
    ToggleLoopA,
    SetCueA,
    JumpCueA,
    ToggleLoopB,
    SetCueB,
    JumpCueB,
    DeckALowDown,
    DeckALowUp,
    DeckAMidDown,
    DeckAMidUp,
    DeckAHighDown,
    DeckAHighUp,
    DeckAFilterDown,
    DeckAFilterUp,
    DeckBLowDown,
    DeckBLowUp,
    DeckBMidDown,
    DeckBMidUp,
    DeckBHighDown,
    DeckBHighUp,
    DeckBFilterDown,
    DeckBFilterUp,
    IsolatorLowA,
    IsolatorLowB,
    IsolatorMidA,
    IsolatorMidB,
    IsolatorHighA,
    IsolatorHighB,
    FilterOrderToggle,
    Quit,
};

class InputMapper {
public:
    static InputCommand parse(const std::string& rawCommand);
    static InputCommand parseKey(char key);
};

} // namespace dj
