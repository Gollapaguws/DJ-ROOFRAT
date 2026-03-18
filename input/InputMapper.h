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
    RecordToggle,
    SaveRecording,
    SetRecordingFilename,
    Quit,
    
    // Phase 24: Player 2 commands for multiplayer battles
    Player2CrossfadeLeft,
    Player2CrossfadeCenter,
    Player2CrossfadeRight,
    Player2NudgeTempoBUp,
    Player2NudgeTempoBDown,
    Player2ResetTempoB,
    Player2PlayPauseB,
    Player2PlayB,
    Player2PauseB,
    
    // Phase 27: Preset Hotkey System (F1-F12 for Deck A, Shift+F1-F12 for Deck B)
    LoadPresetEQ_A_1,
    LoadPresetEQ_A_2,
    LoadPresetEQ_A_3,
    LoadPresetEQ_A_4,
    LoadPresetEQ_A_5,
    LoadPresetEQ_A_6,
    LoadPresetEQ_A_7,
    LoadPresetEQ_A_8,
    LoadPresetEQ_A_9,
    LoadPresetEQ_A_10,
    LoadPresetEQ_A_11,
    LoadPresetEQ_A_12,
    
    LoadPresetEQ_B_1,
    LoadPresetEQ_B_2,
    LoadPresetEQ_B_3,
    LoadPresetEQ_B_4,
    LoadPresetEQ_B_5,
    LoadPresetEQ_B_6,
    LoadPresetEQ_B_7,
    LoadPresetEQ_B_8,
    LoadPresetEQ_B_9,
    LoadPresetEQ_B_10,
    LoadPresetEQ_B_11,
    LoadPresetEQ_B_12,
    
    // Phase 30: Spectrum Analyzer toggle
    ToggleSpectrum,

    // Phase 37: Energy Curve & Mix Quality toggle
    ToggleEnergyCurve,
    
    // Phase 36: Beat Grid Nudge Editor
    // NOTE: Originally specified J/K keys, but those are already assigned (DeckBHighDown, NudgeTempoBDown).
    // Using minus/equals keys instead as they're adjacent and available.
    NudgeBeatGridLeft,       // minus '-' key (nudge beats -10ms)
    NudgeBeatGridRight,      // equals '=' key (nudge beats +10ms)
    AdjustFirstBeatLeft,     // Shift+minus (adjust first beat -10ms)
    AdjustFirstBeatRight,    // Shift+equals (adjust first beat +10ms)  
    UndoBeatGrid,            // Ctrl+Z (undo beat grid edit)
    RedoBeatGrid,            // Ctrl+Y (redo beat grid edit)

    // Phase 39: Auto-Tempo Matching & Sync Lock
    ToggleSyncDeckA,         // 'S' key (toggle sync on Deck A)
    ToggleSyncDeckB,         // 'D' key (toggle sync on Deck B)
    BeatJumpForwardA,        // ']' key (+4 beats on Deck A)
    BeatJumpBackwardA,       // '[' key (-4 beats on Deck A)
    BeatJumpForwardB,        // ''' key (+4 beats on Deck B)
    BeatJumpBackwardB,       // ';' key (-4 beats on Deck B)
    
    // Phase 41: Beat Jump, Warp Grid & Polish
    BeatJump1ForwardA,       // '1' key (+1 beat on Deck A)
    BeatJump1BackwardA,      // '!' key (-1 beat on Deck A)
    BeatJump8ForwardA,       // '8' key (+8 beats on Deck A)
    BeatJump8BackwardA,      // '*' key (-8 beats on Deck A)
    BeatJump1ForwardB,       // '1' key (+1 beat on Deck B) - combined with Player2 parsing
    BeatJump1BackwardB,      // '!' key (-1 beat on Deck B)
    BeatJump8ForwardB,       // '8' key (+8 beats on Deck B)
    BeatJump8BackwardB,      // '*' key (-8 beats on Deck B)
    WarpUpA,                 // '+' or '=' key (warp +0.01% on Deck A)
    WarpDownA,               // '-' or '_' key (warp -0.01% on Deck A)
    WarpUpB,                 // '+' or '=' key (warp +0.01% on Deck B)
    WarpDownB,               // '-' or '_' key (warp -0.01% on Deck B)
    SyncUndoA,               // Ctrl+Z (undo sync on Deck A)
    SyncUndoB,               // Ctrl+Z (undo sync on Deck B)
    
    // Phase 40: Transition Coaching & Phrasing
    ToggleCoaching,          // 'C' key (toggle coaching overlay)
};

class InputMapper {
public:
    static InputCommand parse(const std::string& rawCommand);
    static InputCommand parseKey(char key);
    
    // Phase 24: Player-specific parsing
    static InputCommand parseKeyPlayer1(char key);
    static InputCommand parseKeyPlayer2(char key);
};

} // namespace dj
