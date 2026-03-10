## Phase 2 Complete: Isolator Mode & Filter Order Upgrade

Added per-band "kill" mode and higher-order Butterworth filter option (opt-in toggle) for professional mixer behavior.

**Files created/changed:**
- audio/Deck.h
- audio/Deck.cpp
- input/InputMapper.h
- input/InputMapper.cpp
- src/main.cpp
- audio/Deck_Phase2_test.cpp (new)
- input/InputMapper_Phase2_test.cpp (new)

**Functions created/changed:**
- Deck::setIsolatorMode(bool low, bool mid, bool high)
- Deck::isIsolatorEnabled(int band)
- Deck::setFilterOrder(int order)
- Deck::getFilterOrder()
- Deck::setEQ() [updated to enforce isolator kill behavior]
- Deck::applyFilter() [upgraded to support Butterworth 2nd-order filter]
- InputMapper::parseKey() [added Shift+Q/W/E/D/F/G for isolators, '~' for filter toggle]
- main.cpp command switch [added isolator and filter order commands]

**Tests created/changed:**
- test_Deck_IsolatorKillBand
- test_Deck_FilterOrderToggle
- test_Deck_IsolatorMultiBand
- test_InputMapper_IsolatorKeys
- test_InputMapper_NormalKeys

**Review Status:** APPROVED (after critical bug fix)

**Key Achievements:**
- Isolator mode forces 0.0 gain for selected bands (true "kill" behavior)
- Butterworth 2nd-order filter provides steeper rolloff than default single-pole
- Filter order defaults to 1 (single-pole) to preserve CPU performance
- New input commands: Shift+Q/W/E (Deck A low/mid/high isolator), Shift+D/F/G (Deck B isolator), ~ (toggle filter order)
- Proper biquad Direct Form I implementation with correct input/output history state management
- Full test coverage validates isolator kill and filter order switching
- Zero build errors, all existing functionality preserved

**Bug Fix Applied:**
- Fixed critical Butterworth filter bug: incorrect state variable usage in biquad formula
- Added proper output history state variables (butterworthYZ1_, butterworthYZ2_)
- Corrected Direct Form I biquad to use y[n-1] and y[n-2] instead of reusing input history

**Git Commit Message:**
```
feat: add isolator kill mode and Butterworth filter upgrade

- Add per-band isolator mode (forces 0.0 gain when enabled)
- Implement Butterworth 2nd-order low-pass filter (opt-in via keyboard toggle)
- Default filter order remains single-pole for CPU efficiency
- New input commands: Shift+Q/W/E/D/F/G for isolators, ~ for filter order
- Fixed biquad Direct Form I implementation with correct output history
- Full test coverage for isolator and filter order behavior
```
