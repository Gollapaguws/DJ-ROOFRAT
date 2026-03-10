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
    NudgeTempoAUp,
    NudgeTempoADown,
    ResetTempoA,
    NudgeTempoBUp,
    NudgeTempoBDown,
    ResetTempoB,
    ToggleLoopA,
    SetCueA,
    JumpCueA,
    ToggleLoopB,
    SetCueB,
    JumpCueB,
    LoopBeatsToggleA,
    LoopBeatsToggleB,
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
    // Phase 4: Multi-cue and tempo ramp
    SetCueA1,
    SetCueA2,
    SetCueA3,
    JumpCueA1,
    JumpCueA2,
    JumpCueA3,
    SetCueB1,
    SetCueB2,
    SetCueB3,
    JumpCueB1,
    JumpCueB2,
    JumpCueB3,
    TempoRampToggle,
    Quit,
};

class InputMapper {
public:
    static InputCommand parse(const std::string& rawCommand);
    static InputCommand parseKey(char key);
};

} // namespace dj
