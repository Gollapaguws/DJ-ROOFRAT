#if defined(_WIN32)
#define NOMINMAX
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <filesystem>

#if defined(_WIN32)
#include <conio.h>
#include <Windows.h>  // For GetKeyState(VK_SHIFT)
#endif

#include "audio/AudioClip.h"
#include "audio/BPMDetector.h"
#include "audio/Deck.h"
#include "audio/Mixer.h"
#include "audio/PortAudioPlayer.h"
#include "audio/Recorder.h"
#include "audio/SessionMetadata.h"
#include "audio/WAVExporter.h"
#include "core/ConfigManager.h"
#include "core/SessionState.h"
#include "core/PresetManager.h"
#include "crowdAI/CrowdStateMachine.h"
#include "gameplay/GameModes.h"
#include "gameplay/TutorialSystem.h"
#include "gameplay/lessons/BeatmatchingLesson.h"
#include "gameplay/lessons/EQMixingLesson.h"
#include "gameplay/ProgressTracker.h"
#include "gameplay/MissionSystem.h"
#include "gameplay/missions/SustainedBeatmatchMission.h"
#include "gameplay/missions/EnergySurvivalMission.h"
#include "gameplay/missions/TransitionMission.h"
#include "gameplay/Leaderboard.h"
#include "gameplay/Venue.h"
#include "gameplay/UnlockSystem.h"
#include "gameplay/AchievementSystem.h"
#include "input/InputMapper.h"
#include "input/MIDIController.h"
#include "input/MIDIMapping.h"
#include "input/ControllerProfiles.h"
#include "multiplayer/BattleMode.h"
#include "multiplayer/BattleRules.h"
#include "multiplayer/Judge.h"
#include "visuals/WaveformRenderer.h"
#include "visuals/GraphicsContext.h"
#include "visuals/LightingRig.h"
#include "visuals/CamelotWheel.h"
#include "visuals/SpectrumRenderer.h"
#include "library/TrackLibrary.h"
#include "library/LibraryScanner.h"
#include "library/TrackBrowser.h"
#include "audio/CamelotAnalyzer.h"
#include "audio/BeatGridEditor.h"
#include "audio/BeatGrid.h"
#include "gameplay/EnergyCurve.h"
#include "gameplay/MixQualityAnalyzer.h"
#include "visuals/BeatMarkerOverlay.h"
#include "visuals/EnergyCurveRenderer.h"

namespace {

// Phase 29: Get current timestamp for recording filenames
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
    #if defined(_WIN32)
        localtime_s(&tm_now, &time_t_now);
    #else
        localtime_r(&time_t_now, &tm_now);
    #endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d_%H-%M-%S");
    return oss.str();
}

// Phase 29: Export recording to WAV file
bool exportRecording(const dj::Recorder& recorder, const std::string& filename, int sampleRate) {
    if (recorder.getDuration() <= 0.0f) {
        return false;
    }
    
    const auto& data = recorder.getRecordedData();
    if (data.empty()) {
        return false;
    }
    
    dj::WAVExporter exporter(sampleRate, 2, 16);
    return exporter.exportToFile(filename, data.data(), data.size() / 2);
}

// Phase 26: Helper to capture current session state
dj::SessionState captureCurrentState(
    const dj::Deck& deckA, 
    const dj::Deck& deckB, 
    float crossfaderPosition,
    const dj::CareerProgression& career,
    float crowdEnergy,
    const std::string& pathA,
    const std::string& pathB,
    float tempoA,
    float tempoB,
    float eqALow, float eqAMid, float eqAHigh,
    float eqBLow, float eqBMid, float eqBHigh)
{
    dj::SessionState state;
    
    // Capture deck A
    state.deckA.trackPath = pathA;
    state.deckA.playbackPosition = static_cast<double>(deckA.currentFrame()) / 44100.0;
    state.deckA.tempoBend = tempoA / 100.0f;  // Convert percent to ratio
    state.deckA.isPlaying = deckA.isPlaying();
    state.deckA.lowGain = eqALow;
    state.deckA.midGain = eqAMid;
    state.deckA.highGain = eqAHigh;
    
    // Capture deck B
    state.deckB.trackPath = pathB;
    state.deckB.playbackPosition = static_cast<double>(deckB.currentFrame()) / 44100.0;
    state.deckB.tempoBend = tempoB / 100.0f;
    state.deckB.isPlaying = deckB.isPlaying();
    state.deckB.lowGain = eqBLow;
    state.deckB.midGain = eqBMid;
    state.deckB.highGain = eqBHigh;
    
    // Capture session state
    state.crossfader = crossfaderPosition;
    state.currentCareerTier = career.tier();
    state.crowdEnergy = crowdEnergy;
    state.venueId = career.currentVenueName();
    
    return state;
}

// Phase 26: Helper to apply session state to live objects
void applySessionState(
    const dj::SessionState& state,
    dj::Deck& deckA,
    dj::Deck& deckB,
    dj::Mixer& mixer,
    float& tempoA,
    float& tempoB,
    float& eqALow, float& eqAMid, float& eqAHigh,
    float& eqBLow, float& eqBMid, float& eqBHigh,
    float& crossfaderPosition)
{
    // Apply deck A state
    tempoA = state.deckA.tempoBend * 100.0f;  // Convert ratio to percent
    deckA.setTempoPercent(tempoA);
    eqALow = state.deckA.lowGain;
    eqAMid = state.deckA.midGain;
    eqAHigh = state.deckA.highGain;
    deckA.setEQ(eqALow, eqAMid, eqAHigh);
    
    if (state.deckA.isPlaying) {
        deckA.play();
    } else {
        deckA.pause();
    }
    
    // Apply deck B state
    tempoB = state.deckB.tempoBend * 100.0f;
    deckB.setTempoPercent(tempoB);
    eqBLow = state.deckB.lowGain;
    eqBMid = state.deckB.midGain;
    eqBHigh = state.deckB.highGain;
    deckB.setEQ(eqBLow, eqBMid, eqBHigh);
    
    if (state.deckB.isPlaying) {
        deckB.play();
    } else {
        deckB.pause();
    }
    
    // Apply mixer state
    crossfaderPosition = state.crossfader;
    mixer.setCrossfader(crossfaderPosition);
}

// Phase 27: Helper to generate preset slot name
std::string getPresetSlotName(char deck, int slot) {
    return std::string(1, deck) + "_Slot" + std::to_string(slot);
}

// Phase 27: Helper to apply EQ preset from PresetManager
void applyEQPreset(
    dj::PresetManager& manager,
    const std::string& slotName,
    dj::Deck& deck,
    float& lowVar,
    float& midVar,
    float& highVar)
{
    auto preset = manager.loadEQPreset(slotName);
    if (preset.has_value()) {
        lowVar = preset->lowGain;
        midVar = preset->midGain;
        highVar = preset->highGain;
        deck.setEQ(lowVar, midVar, highVar);
        std::cout << "Loaded preset: " << preset->name << " (L:" << lowVar << " M:" << midVar << " H:" << highVar << ")\n";
    } else {
        std::cout << "Preset " << slotName << " not found (slot empty)\n";
    }
}

std::string meterBar(float value, std::size_t width = 24) {
    const float v = std::clamp(value, 0.0f, 1.0f);
    const std::size_t filled = static_cast<std::size_t>(v * static_cast<float>(width));

    std::string out = "[";
    for (std::size_t i = 0; i < width; ++i) {
        out += (i < filled) ? '#' : ' ';
    }
    out += "]";
    return out;
}

std::string moodLabel(dj::CrowdMood mood) {
    switch (mood) {
    case dj::CrowdMood::Unimpressed:
        return "Unimpressed";
    case dj::CrowdMood::Calm:
        return "Calm";
    case dj::CrowdMood::Grooving:
        return "Grooving";
    case dj::CrowdMood::Hyped:
        return "Hyped";
    }
    return "Unknown";
}

void printUsage() {
    std::cout << "Usage: DJ-ROOFRAT [trackA] [trackB] [options]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --help, -h        Show this help message\n";
    std::cout << "  --no-audio        Disable audio output (silent simulation)\n";
    std::cout << "  --tutorial        Run tutorial mode (learn beatmatching and EQ mixing)\n";
    std::cout << "  --mission [type]  Run mission mode (beatmatch|energy|transition)\n";
    std::cout << "  --career          Run career mode with progression feedback\n";
    std::cout << "  --battle [format] Run battle mode (quick|standard|tournament)\n";
    std::cout << "\nIf no tracks are provided, generated test tones are used.\n";
}

void printLiveControls() {
    std::cout << "Live controls: [ left XF | \\ center XF | ] right XF | a toggle DeckA | b toggle DeckB\n";
    std::cout << "               i tempo B+ | k tempo B- | o tempo reset | z tempo A+ | x tempo A- | c tempo A reset\n";
    std::cout << "               l loop B | v cueB jump | ; loop beats toggle A | ' loop beats toggle B \n";
    std::cout << "               A EQ/Filter: q/w low-/+ e/r mid-/+ t/y high-/+ u/p filter-/+\n";
    std::cout << "               B EQ/Filter: d/f low-/+ g/h mid-/+ j/n high-/+ m/, filter-/+\n";
    std::cout << "               Isolators: Shift+q/w/t (A low/mid/high) Shift+d/f/g (B low/mid/high)\n";
    std::cout << "               Filter Order: Shift+u or Shift+p to toggle Butterworth mode (both decks)\n";
    std::cout << "               Phase 4 - Multi-Cue Banks: 1/2/3 set cueA1/A2/A3, 4/5/6 set cueB1/B2/B3\n";
    std::cout << "               Phase 4 - Cue Jump: Shift+1/2/3 jump cueA1/A2/A3, Shift+4/5/6 jump cueB1/B2/B3\n";
    std::cout << "               Phase 4 - Tempo Ramp: Shift+R to toggle tempo ramping (both decks)\n";
    std::cout << "               Phase 14 - Recording: s toggle record | Shift+s save recording to WAV\n";
    std::cout << "               Phase 27 - Presets: F1-F12 load Deck A presets | Shift+F1-F12 load Deck B presets\n";
    std::cout << "               Phase 35 - Spectrum: 9 toggle real-time spectrum analyzer (dual-deck view)\n";
    std::cout << "               Phase 36 - Beat Grid: - nudge left | = nudge right | Shift+- first beat left | Shift+= first beat right | Ctrl+Z undo | Ctrl+Y redo\n";
    std::cout << "               Phase 37 - Energy Curve: Shift+E toggle energy analytics + mix quality score\n";
}

std::vector<dj::InputCommand> pollKeyboardCommands() {
    std::vector<dj::InputCommand> commands;
#if defined(_WIN32)
    while (_kbhit() != 0) {
        const int first = _getch();
        if (first == 0 || first == 224) {
            const int scanCode = _getch();  // Get the scan code
            
            // F1-F12 scan codes: 0x3B to 0x46
            if (scanCode >= 0x3B && scanCode <= 0x46) {
                int fKeyNumber = (scanCode - 0x3B) + 1;  // F1=1, F2=2, etc.
                
                // Check Shift state
                bool shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                
                if (shiftPressed) {
                    // Shift+F1-F12: Deck B presets
                    dj::InputCommand cmd = static_cast<dj::InputCommand>(
                        static_cast<int>(dj::InputCommand::LoadPresetEQ_B_1) + (fKeyNumber - 1));
                    commands.push_back(cmd);
                } else {
                    // F1-F12: Deck A presets
                    dj::InputCommand cmd = static_cast<dj::InputCommand>(
                        static_cast<int>(dj::InputCommand::LoadPresetEQ_A_1) + (fKeyNumber - 1));
                    commands.push_back(cmd);
                }
            }
            // Other extended keys are ignored
            continue;
        }
        
        // Phase 36: Handle Ctrl+Z and Ctrl+Y
        bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        if (ctrlPressed) {
            if (first == 'z' || first == 'Z') {
                commands.push_back(dj::InputCommand::UndoBeatGrid);
                continue;
            }
            if (first == 'y' || first == 'Y') {
                commands.push_back(dj::InputCommand::RedoBeatGrid);
                continue;
            }
        }

        const auto command = dj::InputMapper::parseKey(static_cast<char>(first));
        if (command != dj::InputCommand::None) {
            commands.push_back(command);
        }
    }
#endif
    return commands;
}

bool configurePerformanceLoop(dj::Deck& deck, float effectiveBpm) {
    const dj::AudioClip* clip = deck.clip();
    if (clip == nullptr || clip->frameCount() <= 2U) {
        return false;
    }

    const float bpm = std::max(60.0f, effectiveBpm);
    const double secondsPerBeat = 60.0 / static_cast<double>(bpm);
    std::size_t loopFrames = static_cast<std::size_t>(secondsPerBeat * static_cast<double>(deck.loopBeats()) * static_cast<double>(clip->sampleRate));
    loopFrames = std::max<std::size_t>(1024U, loopFrames);
    loopFrames = std::min(loopFrames, clip->frameCount() - 1U);

    std::size_t start = deck.currentFrame();
    if (start + loopFrames >= clip->frameCount()) {
        start = (clip->frameCount() > loopFrames + 1U) ? (clip->frameCount() - loopFrames - 1U) : 0U;
    }

    std::size_t end = std::min(clip->frameCount(), start + loopFrames);
    if (end <= start + 1U) {
        end = std::min(clip->frameCount(), start + 2U);
    }

    if (end <= start) {
        return false;
    }

    // Phase 4: Use quantized loop configuration with BPM for beat-aligned boundaries
    deck.configureLoop(start, end, true, bpm);
    return true;
}

std::string beatmatchLabel(float bpmDelta) {
    const float absDelta = std::abs(bpmDelta);
    if (absDelta <= 0.5f) {
        return "TIGHT";
    }
    if (absDelta <= 2.0f) {
        return "GOOD";
    }
    return "DRIFT";
}

bool loadOrGenerateDeck(dj::Deck& deck, const std::string& trackPath, float fallbackFrequencyHz, const std::string& label) {
    if (!trackPath.empty()) {
        std::string error;
        if (deck.loadFromFile(trackPath, &error)) {
            std::cout << label << " loaded from: " << trackPath << "\n";
            return true;
        }

        std::cout << label << " could not load track (" << error << "), using generated tone fallback.\n";
    }

    const auto generated = dj::AudioClip::generateTestTone(fallbackFrequencyHz, 90.0f, 44100);
    deck.loadClip(generated);
    std::cout << label << " using generated " << fallbackFrequencyHz << " Hz test tone.\n";
    return false;
}

// Arc V Integration: Tutorial Mode
int runTutorialMode() {
    std::cout << "\n=== TUTORIAL MODE ===\n";
    std::cout << "Learn beatmatching and EQ mixing fundamentals.\n\n";
    
    dj::TutorialSystem tutorial;
    dj::ProgressTracker progress;
    
    // Load previous progress
    if (progress.loadProgress("tutorial_progress.json")) {
        const auto& completed = progress.getCompletedLessons();
        std::cout << "Progress loaded: " << completed.size() << " lessons completed previously.\n";
    }
    
    while (!tutorial.isComplete()) {
        auto lesson = tutorial.getCurrentLesson();
        if (!lesson) break;
        
        std::cout << "\n--- Lesson " << (tutorial.getCurrentLessonIndex() + 1) << ": " << lesson->getName() << " ---\n";
        std::cout << "Hint: " << lesson->getHint() << "\n";
        std::cout << "Press ENTER to simulate lesson completion (in real mode, you would practice)...\n";
        std::cin.get();
        
        // In real implementation, would run lesson->validate() with actual deck/mixer state
        // For now, just mark as complete
        std::cout << "✓ Lesson complete!\n";
        
        // Save progress
        progress.markLessonComplete(tutorial.getCurrentLessonIndex());
        progress.saveProgress("tutorial_progress.json");
        
        tutorial.nextLesson();
    }
    
    std::cout << "\n=== All tutorials complete! ===\n";
    std::cout << "You've mastered beatmatching and EQ mixing. Ready for missions!\n";
    return 0;
}

// Arc V Integration: Mission Mode
int runMissionMode(const std::string& missionType) {
    std::cout << "\n=== MISSION MODE: " << missionType << " ===\n";
    
    std::shared_ptr<dj::Mission> mission;
    if (missionType == "beatmatch") {
        mission = std::make_shared<dj::SustainedBeatmatchMission>();
        std::cout << "Objective: Maintain BPM delta < 2% for 30 seconds\n";
    } else if (missionType == "energy") {
        mission = std::make_shared<dj::EnergySurvivalMission>();
        std::cout << "Objective: Keep crowd energy > 0.5 for 60 seconds\n";
    } else if (missionType == "transition") {
        mission = std::make_shared<dj::TransitionMission>();
        std::cout << "Objective: Execute smooth crossfade (smoothness > 0.7)\n";
    } else {
        std::cout << "Unknown mission type: " << missionType << "\n";
        std::cout << "Valid types: beatmatch, energy, transition\n";
        return 1;
    }
    
    dj::MissionSystem missionSystem;
    mission->setup();
    missionSystem.startMission(mission);
    
    std::cout << "\nMission started!\n";
    std::cout << "Press ENTER to simulate mission completion (in real mode, you would perform)...\n";
    std::cin.get();
    
    // In real implementation, would run mission->update() with actual performance data
    std::cout << "✓ Mission complete! Score: " << mission->getScore() << "\n";
    
    // Save to leaderboard
    dj::Leaderboard leaderboard;
    leaderboard.loadFromFile("leaderboard.json");
    leaderboard.addScore("Player", mission->getScore(), mission->getName());
    leaderboard.saveToFile("leaderboard.json");
    
    // Show top scores
    auto topScores = leaderboard.getTopScores(mission->getName(), 5);
    std::cout << "\n--- Top Scores for " << mission->getName() << " ---\n";
    int rank = 1;
    for (const auto& entry : topScores) {
        std::cout << rank++ << ". " << entry.playerName << ": " << entry.score << "\n";
    }
    
    return 0;
}

// Arc VI Integration: Battle Mode
int runBattleMode(const std::string& formatType) {
    std::cout << "\n=== BATTLE MODE: " << formatType << " ===\n";
    
    // Parse format
    dj::BattleFormat format = dj::BattleFormat::Standard;
    if (formatType == "quick") {
        format = dj::BattleFormat::Quick;
        std::cout << "Format: Quick Battle (1 round, 30 seconds each)\n";
    } else if (formatType == "standard") {
        format = dj::BattleFormat::Standard;
        std::cout << "Format: Standard Battle (3 rounds, 60 seconds each)\n";
    } else if (formatType == "tournament") {
        format = dj::BattleFormat::Tournament;
        std::cout << "Format: Tournament Battle (5 rounds, 90 seconds each)\n";
    } else {
        std::cout << "Unknown format: " << formatType << "\n";
        std::cout << "Valid formats: quick, standard, tournament\n";
        return 1;
    }
    
    // Create battle rules and mode
    auto rules = std::make_shared<dj::BattleRules>(format, dj::TurnMode::TurnBased);
    dj::BattleMode battle;
    battle.startBattle(rules);
    
    std::cout << "\nBattle started! " << rules->getRounds() << " rounds\n";
    std::cout << "\nControls:\n";
    std::cout << "Player 1 (QWERTY): Q/A tempo, W/S crossfade, 1-9 effects\n";
    std::cout << "Player 2 (Arrow+Numpad): UP/DOWN tempo, LEFT/RIGHT crossfade, Numpad 0-9 effects\n";
    std::cout << "\nPress ENTER to simulate battle (in real mode, players would perform)...\n";
    std::cin.get();
    
    // Simulate rounds
    for (int round = 1; round <= rules->getRounds(); ++round) {
        std::cout << "\n--- Round " << round << " ---\n";
        
        // Simulate Player 1 performance
        std::cout << "Player 1 performing...\n";
        battle.submitPerformance(1, 0.5f, 0.8f, 0.7f, 100);
        
        // Simulate Player 2 performance
        std::cout << "Player 2 performing...\n";
        battle.submitPerformance(2, 0.3f, 0.9f, 0.75f, 100);
        
        // Show round scores
        auto p1Score = battle.getPlayerScore(1);
        auto p2Score = battle.getPlayerScore(2);
        std::cout << "Player 1 score: " << p1Score.totalScore << " (beatmatch: " << p1Score.beatmatchScore 
                  << ", transitions: " << p1Score.transitionScore << ")\n";
        std::cout << "Player 2 score: " << p2Score.totalScore << " (beatmatch: " << p2Score.beatmatchScore 
                  << ", transitions: " << p2Score.transitionScore << ")\n";
        
        if (round < rules->getRounds()) {
            battle.advanceRound();
        }
    }
    
    // Determine winner
    int winner = battle.getWinner();
    std::cout << "\n=== BATTLE COMPLETE ===\n";
    if (winner == 0) {
        std::cout << "Result: TIE!\n";
    } else {
        std::cout << "Winner: Player " << winner << "!\n";
        auto winnerScore = battle.getPlayerScore(winner);
        std::cout << "Final score: " << winnerScore.totalScore << "\n";
    }
    
    return 0;
}

// Arc V Integration: Career Mode
void printCareerStatus(const dj::CareerProgression& career, const dj::UnlockSystem& unlocks, const dj::AchievementSystem& achievements) {
    std::cout << "\n=== CAREER STATUS ===\n";
    std::cout << "Current Venue: " << career.currentVenueName() << " (Tier " << career.tier() << ")\n";
    std::cout << "Reputation: " << career.reputation() << "/100\n";
    std::cout << "Peak Tier: " << career.peakTier() << "\n";
    
    // Show unlocked effects
    auto unlockedEffects = unlocks.getUnlockedEffects(career.tier());
    std::cout << "\nUnlocked Effects (" << unlockedEffects.size() << "):\n";
    for (const auto& effect : unlockedEffects) {
        std::cout << "  ✓ " << unlocks.getEffectName(effect) << "\n";
    }
    
    // Show locked effects
    auto lockedEffects = unlocks.getLockedEffects(career.tier());
    if (!lockedEffects.empty()) {
        std::cout << "\nLocked Effects (unlock at higher tiers):\n";
        for (const auto& effect : lockedEffects) {
            std::cout << "  ✗ " << unlocks.getEffectName(effect) 
                      << " (requires Tier " << unlocks.getEffectRequiredTier(effect) << ")\n";
        }
    }
    
    // Show achievements
    std::cout << "\nAchievements: " << achievements.getUnlockedCount() << "/" 
              << achievements.getAllAchievements().size() << " unlocked\n";
    for (const auto& achievement : achievements.getAllAchievements()) {
        std::cout << "  " << (achievement.unlocked ? "✓" : "✗") << " " 
                  << achievement.name << ": " << achievement.description << "\n";
    }
    std::cout << "===================\n\n";
}

} // namespace

int main(int argc, char** argv) {
    std::vector<std::string> tracks;
    bool disableAudio = false;
    bool tutorialMode = false;
    bool missionMode = false;
    bool careerMode = false;
    bool battleMode = false;
    std::string missionType = "beatmatch";  // Default mission type
    std::string battleFormat = "standard";  // Default battle format

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        }
        if (arg == "--no-audio") {
            disableAudio = true;
            continue;
        }
        if (arg == "--tutorial") {
            tutorialMode = true;
            continue;
        }
        if (arg == "--mission") {
            missionMode = true;
            // Check if next arg is mission type
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                missionType = argv[i + 1];
                ++i;  // Skip next arg
            }
            continue;
        }
        if (arg == "--career") {
            careerMode = true;
            continue;
        }
        if (arg == "--battle") {
            battleMode = true;
            // Check if next arg is battle format
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                battleFormat = argv[i + 1];
                ++i;  // Skip next arg
            }
            continue;
        }
        tracks.push_back(arg);
    }

    // Arc VI Integration: Load configuration
    dj::ConfigManager configManager;
    if (configManager.loadConfig("config.json")) {
        std::cout << "Configuration loaded from config.json\n";
    } else {
        std::cout << "Using default configuration (config.json not found)\n";
    }

    // Phase 26: Initialize SessionManager with auto-save
    dj::SessionManager sessionManager;
    sessionManager.enableAutoSave(true);
    sessionManager.setAutoSaveInterval(configManager.getConfig().autosaveIntervalSeconds);
    
    // Phase 27: Initialize PresetManager and load presets
    dj::PresetManager presetManager;
    if (presetManager.loadFromFile("presets.json")) {
        std::cout << "Presets loaded from presets.json\n";
    } else {
        std::cout << "No presets file found (presets.json), starting with empty preset library\n";
    }
    
    // Phase 26: Check for autosave recovery (will be applied after decks are initialized)
    std::optional<dj::SessionState> recoveredSession;
    if (std::filesystem::exists("autosave.json")) {
        std::cout << "\n=== AUTOSAVE RECOVERY ===\n";
        std::cout << "Previous session found (autosave.json)\n";
        auto recovered = sessionManager.loadSession("autosave.json");
        if (recovered.has_value()) {
            std::cout << "  Deck A: " << (recovered->deckA.trackPath.empty() ? "(no track)" : recovered->deckA.trackPath) << "\n";
            std::cout << "  Deck B: " << (recovered->deckB.trackPath.empty() ? "(no track)" : recovered->deckB.trackPath) << "\n";
            std::cout << "  Career Tier: " << recovered->currentCareerTier << "\n";
            std::cout << "Session will be restored after initialization.\n";
            recoveredSession = recovered;
        } else {
            std::cout << "Failed to load autosave (corrupted?), starting fresh\n";
        }
        std::cout << "=========================\n\n";
    }

    // Arc V & VI Integration: Mode dispatcher
    if (tutorialMode) {
        return runTutorialMode();
    }
    
    if (missionMode) {
        return runMissionMode(missionType);
    }
    
    if (battleMode) {
        return runBattleMode(battleFormat);
    }
    
    // Career mode runs normal performance loop with enhanced feedback
    // (implemented below with printCareerStatus calls)

    dj::Deck deckA;
    dj::Deck deckB;

    const int outputRate = configManager.getSampleRate();
    deckA.setOutputSampleRate(outputRate);
    deckB.setOutputSampleRate(outputRate);

    const std::string pathA = tracks.size() > 0U ? tracks[0] : std::string();
    const std::string pathB = tracks.size() > 1U ? tracks[1] : std::string();
    loadOrGenerateDeck(deckA, pathA, 220.0f, "Deck A");
    loadOrGenerateDeck(deckB, pathB, 330.0f, "Deck B");

    float bpmA = 124.0f;
    float bpmB = 128.0f;

    if (const dj::AudioClip* clip = deckA.clip()) {
        if (const auto bpm = dj::BPMDetector::estimate(*clip)) {
            bpmA = *bpm;
        }
    }

    if (const dj::AudioClip* clip = deckB.clip()) {
        if (const auto bpm = dj::BPMDetector::estimate(*clip)) {
            bpmB = *bpm;
        }
    }

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Estimated BPM A/B: " << bpmA << " / " << bpmB << "\n";
    const float beatmatchTargetB = ((bpmA - bpmB) / std::max(1.0f, bpmB)) * 100.0f;
    std::cout << "Beatmatch target for Deck B (manual, no auto-sync): " << beatmatchTargetB << "%\n";
    printLiveControls();

    // Phase 36: Initialize beat grids and editors
    dj::BeatGrid beatGridA;
    dj::BeatGrid beatGridB;
    
    // Generate initial beat grids from detected BPM
    double trackDurationA = 0.0;
    double trackDurationB = 0.0;
    if (const dj::AudioClip* clip = deckA.clip()) {
        trackDurationA = static_cast<double>(clip->frameCount()) / static_cast<double>(clip->sampleRate);
        beatGridA.generateFromBPM(static_cast<double>(bpmA), 0.0, trackDurationA);
    }
    if (const dj::AudioClip* clip = deckB.clip()) {
        trackDurationB = static_cast<double>(clip->frameCount()) / static_cast<double>(clip->sampleRate);
        beatGridB.generateFromBPM(static_cast<double>(bpmB), 0.0, trackDurationB);
    }
    
    dj::BeatGridEditor beatGridEditorA(beatGridA, outputRate);
    dj::BeatGridEditor beatGridEditorB(beatGridB, outputRate);
    beatGridEditorA.setTrackDuration(trackDurationA);
    beatGridEditorB.setTrackDuration(trackDurationB);
    
    dj::BeatMarkerOverlay beatMarkerOverlay;

    deckA.setSlipMode(true);
    deckB.setSlipMode(true);
    deckA.setVinylMode(true);
    deckB.setVinylMode(true);
    deckA.setLoopBeats(16);
    deckB.setLoopBeats(16);

    deckA.setEQ(1.0f, 1.0f, 1.0f);
    deckB.setEQ(1.0f, 1.0f, 1.0f);
    deckA.setFilter(1.0f);
    deckB.setFilter(1.0f);

    float eqALow = 1.0f;
    float eqAMid = 1.0f;
    float eqAHigh = 1.0f;
    float eqBLow = 1.0f;
    float eqBMid = 1.0f;
    float eqBHigh = 1.0f;
    float filterA = 1.0f;
    float filterB = 1.0f;

    // Isolator state tracking
    bool isolatorALow = false;
    bool isolatorAMid = false;
    bool isolatorAHigh = false;
    bool isolatorBLow = false;
    bool isolatorBMid = false;
    bool isolatorBHigh = false;

    deckA.play();
    deckB.play();

    dj::Mixer mixer;
    mixer.setMasterGain(configManager.getMasterVolume());

    // Phase 29: Initialize three Recorders for multi-track recording (mix, deck A, deck B)
    dj::Recorder recorderMix(outputRate, 2, 600);  // 10 minutes capacity
    dj::Recorder recorderDeckA(outputRate, 2, 600);
    dj::Recorder recorderDeckB(outputRate, 2, 600);
    bool recordingActive = false;
    
    // Phase 29: Session metadata for cue markers and track info
    dj::SessionMetadata sessionMetadata;
    sessionMetadata.setRecordingName("DJ Session");
    sessionMetadata.setStartTime(std::time(nullptr));

    float crossfaderPosition = -1.0f;
    mixer.setCrossfader(crossfaderPosition);

    float tempoA = 0.0f;
    float tempoB = 0.0f;
    bool loopAEnabled = false;
    bool loopBEnabled = false;
    int loopBeatsA = 16;
    int loopBeatsB = 16;
    std::size_t cueA = 0;
    std::size_t cueB = 0;
    bool manualMixMode = false;
    bool quitRequested = false;
    // Phase 4: Tempo ramp state
    bool tempoRampEnabledA = false;
    bool tempoRampEnabledB = false;
    int activeCueBankA = 0;  // Active cue bank for Deck A (0, 1, or 2)
    int activeCueBankB = 0;  // Active cue bank for Deck B (0, 1, or 2)

    dj::CrowdStateMachine crowd(dj::CrowdPersonality::Default);
    dj::ScoringSystem scoring;
    dj::CareerProgression career;
    dj::WaveformRenderer waveform(68, 11);
    
    // Phase 35: Spectrum display
    dj::SpectrumRenderer::RenderOptions spectrumOptions;
    spectrumOptions.width = 80;
    spectrumOptions.height = 10;
    spectrumOptions.colorScheme = dj::SpectrumRenderer::ColorScheme::FrequencyBands;
    spectrumOptions.showPeakHold = true;
    spectrumOptions.peakDecayRate = 0.002f;  // 500ms decay @ 60 FPS
    spectrumOptions.showFrequencyLabels = true;
    spectrumOptions.sampleRate = outputRate;
    dj::SpectrumRenderer spectrumRenderer(spectrumOptions);
    bool showSpectrum = false;  // Toggle with 9 key

    // Phase 37: Energy curve + mix quality analytics
    dj::EnergyCurve energyCurve(30.0 * 60.0, 1.0);  // 30 minutes at 1Hz
    dj::MixQualityAnalyzer mixQualityAnalyzer;
    dj::EnergyCurveRenderOptions energyCurveOptions;
    energyCurveOptions.width = 80;
    energyCurveOptions.height = 15;
    energyCurveOptions.timeWindow = 30.0 * 60.0;
    energyCurveOptions.showGrid = true;
    energyCurveOptions.showStats = true;
    dj::EnergyCurveRenderer energyCurveRenderer(energyCurveOptions);
    bool showEnergyCurve = false;
    double lastEnergySampleTimeSeconds = -1.0;
    double lastMixAnalysisTimeSeconds = -1.0;

    // Arc V: Career Mode Systems
    dj::UnlockSystem unlocks;
    dj::AchievementSystem achievements;
    if (careerMode) {
        // Load saved achievements
        achievements.loadFromFile("achievements.json");
        // Check initial achievements
        if (career.tier() >= 2) {
            achievements.checkAndUnlock("tier_2");
        }
        if (career.tier() >= 4) {
            achievements.checkAndUnlock("tier_4");
        }
        std::cout << "\n=== CAREER MODE ENABLED ===\n";
        printCareerStatus(career, unlocks, achievements);
    }

    // Phase 26: Apply recovered session state if available
    if (recoveredSession.has_value()) {
        std::cout << "Applying recovered session state...\n";
        applySessionState(
            recoveredSession.value(),
            deckA, deckB, mixer,
            tempoA, tempoB,
            eqALow, eqAMid, eqAHigh,
            eqBLow, eqBMid, eqBHigh,
            crossfaderPosition
        );
        std::cout << "Session state restored.\n";
    }

    // Phase 7: Initialize graphics context and lighting rig
    dj::GraphicsContext graphics;
    dj::LightingRig lighting;
    const auto& config = configManager.getConfig();
    bool graphicsEnabled = config.enableGraphics ? graphics.initialize(config.graphicsWidth, config.graphicsHeight) : false;
    if (graphicsEnabled) {
        std::cout << "DirectX 11 graphics initialized.\n";
    } else {
        std::cout << "Graphics unavailable; using ASCII waveform fallback.\n";
    }

    dj::PortAudioPlayer player;
    bool realtimeAudio = false;

    if (!disableAudio) {
        std::string error;
        if (player.open(outputRate, 512, &error)) {
            realtimeAudio = true;
            std::cout << "PortAudio playback enabled.\n";
        } else {
            std::cout << "PortAudio unavailable (" << error << ") - running silent simulation.\n";
        }
    } else {
        std::cout << "Audio disabled by flag (--no-audio).\n";
    }

    // Phase 11: Track Library & Browser initialization
    auto library = std::make_shared<dj::library::TrackLibrary>();
    std::string libError;
    library->initialize(":memory:", &libError);  // In-memory database for development
    auto browser = std::make_shared<dj::library::TrackBrowser>(library);
    std::cout << "Track Library initialized with browser support.\n";

    // Phase 13: MIDI Controller initialization
    dj::midi::MIDIController midiController;
    auto midiProfile = dj::input::ControllerProfiles::getDefaultProfile();
    std::cout << "MIDI system initialized with default controller profile.\n"
              << "Available MIDI functions: ";
    for (const auto& fn : midiProfile->getAvailableFunctions()) {
        std::cout << fn << " ";
    }
    std::cout << "\n";
    // Note: MIDI device enumeration and opening is optional for development
    // In production, would enumerate devices and allow user selection

    constexpr std::size_t framesPerBlock = 512;
    constexpr int totalBlocks = 1200;

    // Phase 26: Auto-save timer initialization
    auto lastAutoSaveTime = std::chrono::steady_clock::now();
    const int autoSaveIntervalSeconds = configManager.getConfig().autosaveIntervalSeconds;

    for (int block = 0; block < totalBlocks; ++block) {
        const float progress = static_cast<float>(block) / static_cast<float>(totalBlocks - 1);

        const auto commands = pollKeyboardCommands();
        if (!commands.empty()) {
            manualMixMode = true;
        }

        for (const auto command : commands) {
            switch (command) {
            case dj::InputCommand::PlayPauseA:
                if (deckA.isPlaying()) {
                    deckA.pause();
                } else {
                    deckA.play();
                }
                break;
            case dj::InputCommand::PlayPauseB:
                if (deckB.isPlaying()) {
                    deckB.pause();
                } else {
                    deckB.play();
                }
                break;
            case dj::InputCommand::PlayA:
                deckA.play();
                break;
            case dj::InputCommand::PlayB:
                deckB.play();
                break;
            case dj::InputCommand::PauseA:
                deckA.pause();
                break;
            case dj::InputCommand::PauseB:
                deckB.pause();
                break;
            case dj::InputCommand::CrossfadeLeft:
                crossfaderPosition = std::clamp(crossfaderPosition - 0.08f, -1.0f, 1.0f);
                mixer.setCrossfader(crossfaderPosition);
                break;
            case dj::InputCommand::CrossfadeCenter:
                crossfaderPosition = 0.0f;
                mixer.setCrossfader(crossfaderPosition);
                break;
            case dj::InputCommand::CrossfadeRight:
                crossfaderPosition = std::clamp(crossfaderPosition + 0.08f, -1.0f, 1.0f);
                mixer.setCrossfader(crossfaderPosition);
                break;
            case dj::InputCommand::NudgeTempoBUp:
                tempoB = std::clamp(tempoB + 0.25f, -20.0f, 20.0f);
                deckB.setTempoPercent(tempoB);
                break;
            case dj::InputCommand::NudgeTempoBDown:
                tempoB = std::clamp(tempoB - 0.25f, -20.0f, 20.0f);
                deckB.setTempoPercent(tempoB);
                break;
            case dj::InputCommand::ResetTempoB:
                tempoB = 0.0f;
                deckB.setTempoPercent(tempoB);
                break;
            case dj::InputCommand::NudgeTempoAUp:
                tempoA = std::clamp(tempoA + 0.25f, -20.0f, 20.0f);
                deckA.setTempoPercent(tempoA);
                break;
            case dj::InputCommand::NudgeTempoADown:
                tempoA = std::clamp(tempoA - 0.25f, -20.0f, 20.0f);
                deckA.setTempoPercent(tempoA);
                break;
            case dj::InputCommand::ResetTempoA:
                tempoA = 0.0f;
                deckA.setTempoPercent(tempoA);
                break;
            case dj::InputCommand::LoopBeatsToggleA:
                // Cycle: 16 → 32 → 8 → 16
                if (loopBeatsA == 16) {
                    loopBeatsA = 32;
                } else if (loopBeatsA == 32) {
                    loopBeatsA = 8;
                } else {
                    loopBeatsA = 16;
                }
                deckA.setLoopBeats(loopBeatsA);
                break;
            case dj::InputCommand::LoopBeatsToggleB:
                // Cycle: 16 → 32 → 8 → 16
                if (loopBeatsB == 16) {
                    loopBeatsB = 32;
                } else if (loopBeatsB == 32) {
                    loopBeatsB = 8;
                } else {
                    loopBeatsB = 16;
                }
                deckB.setLoopBeats(loopBeatsB);
                break;
            case dj::InputCommand::ToggleLoopA:
                loopAEnabled = !loopAEnabled;
                if (loopAEnabled) {
                    const float effectiveBpmA = bpmA * (1.0f + (tempoA / 100.0f));
                    if (!configurePerformanceLoop(deckA, effectiveBpmA)) {
                        loopAEnabled = false;
                    }
                } else {
                    deckA.configureLoop(0U, 0U, false);
                }
                break;
            case dj::InputCommand::SetCueA:
                cueA = deckA.currentFrame();
                deckA.setCue(cueA);
                break;
            case dj::InputCommand::JumpCueA:
                deckA.jumpToCue();
                break;
            case dj::InputCommand::ToggleLoopB:
                loopBEnabled = !loopBEnabled;
                if (loopBEnabled) {
                    const float effectiveBpmB = bpmB * (1.0f + (tempoB / 100.0f));
                    if (!configurePerformanceLoop(deckB, effectiveBpmB)) {
                        loopBEnabled = false;
                    }
                } else {
                    deckB.configureLoop(0U, 0U, false);
                }
                break;
            case dj::InputCommand::SetCueB:
                cueB = deckB.currentFrame();
                deckB.setCue(cueB);
                break;
            case dj::InputCommand::JumpCueB:
                deckB.jumpToCue();
                break;
            case dj::InputCommand::DeckALowDown:
                eqALow = std::clamp(eqALow - 0.08f, 0.0f, 2.0f);
                deckA.setEQ(eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::DeckALowUp:
                eqALow = std::clamp(eqALow + 0.08f, 0.0f, 2.0f);
                deckA.setEQ(eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::DeckAMidDown:
                eqAMid = std::clamp(eqAMid - 0.08f, 0.0f, 2.0f);
                deckA.setEQ(eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::DeckAMidUp:
                eqAMid = std::clamp(eqAMid + 0.08f, 0.0f, 2.0f);
                deckA.setEQ(eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::DeckAHighDown:
                eqAHigh = std::clamp(eqAHigh - 0.08f, 0.0f, 2.0f);
                deckA.setEQ(eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::DeckAHighUp:
                eqAHigh = std::clamp(eqAHigh + 0.08f, 0.0f, 2.0f);
                deckA.setEQ(eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::DeckAFilterDown:
                filterA = std::clamp(filterA - 0.05f, 0.0f, 1.0f);
                deckA.setFilter(filterA);
                break;
            case dj::InputCommand::DeckAFilterUp:
                filterA = std::clamp(filterA + 0.05f, 0.0f, 1.0f);
                deckA.setFilter(filterA);
                break;
            case dj::InputCommand::DeckBLowDown:
                eqBLow = std::clamp(eqBLow - 0.08f, 0.0f, 2.0f);
                deckB.setEQ(eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::DeckBLowUp:
                eqBLow = std::clamp(eqBLow + 0.08f, 0.0f, 2.0f);
                deckB.setEQ(eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::DeckBMidDown:
                eqBMid = std::clamp(eqBMid - 0.08f, 0.0f, 2.0f);
                deckB.setEQ(eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::DeckBMidUp:
                eqBMid = std::clamp(eqBMid + 0.08f, 0.0f, 2.0f);
                deckB.setEQ(eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::DeckBHighDown:
                eqBHigh = std::clamp(eqBHigh - 0.08f, 0.0f, 2.0f);
                deckB.setEQ(eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::DeckBHighUp:
                eqBHigh = std::clamp(eqBHigh + 0.08f, 0.0f, 2.0f);
                deckB.setEQ(eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::DeckBFilterDown:
                filterB = std::clamp(filterB - 0.05f, 0.0f, 1.0f);
                deckB.setFilter(filterB);
                break;
            case dj::InputCommand::DeckBFilterUp:
                filterB = std::clamp(filterB + 0.05f, 0.0f, 1.0f);
                deckB.setFilter(filterB);
                break;
            case dj::InputCommand::IsolatorLowA:
                isolatorALow = !isolatorALow;
                deckA.setIsolatorMode(isolatorALow, isolatorAMid, isolatorAHigh);
                deckA.setEQ(eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::IsolatorMidA:
                isolatorAMid = !isolatorAMid;
                deckA.setIsolatorMode(isolatorALow, isolatorAMid, isolatorAHigh);
                deckA.setEQ(eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::IsolatorHighA:
                isolatorAHigh = !isolatorAHigh;
                deckA.setIsolatorMode(isolatorALow, isolatorAMid, isolatorAHigh);
                deckA.setEQ(eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::IsolatorLowB:
                isolatorBLow = !isolatorBLow;
                deckB.setIsolatorMode(isolatorBLow, isolatorBMid, isolatorBHigh);
                deckB.setEQ(eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::IsolatorMidB:
                isolatorBMid = !isolatorBMid;
                deckB.setIsolatorMode(isolatorBLow, isolatorBMid, isolatorBHigh);
                deckB.setEQ(eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::IsolatorHighB:
                isolatorBHigh = !isolatorBHigh;
                deckB.setIsolatorMode(isolatorBLow, isolatorBMid, isolatorBHigh);
                deckB.setEQ(eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::FilterOrderToggle:
                // Toggle between single-pole (1) and Butterworth (2)
                deckA.setFilterOrder(deckA.getFilterOrder() == 1 ? 2 : 1);
                deckB.setFilterOrder(deckB.getFilterOrder() == 1 ? 2 : 1);
                break;
            // Phase 4: Multi-cue hotspot bank A commands
            case dj::InputCommand::SetCueA1:
                deckA.setCue(deckA.currentFrame(), 0);
                deckA.setActiveCueBank(0);
                activeCueBankA = 0;
                break;
            case dj::InputCommand::SetCueA2:
                deckA.setCue(deckA.currentFrame(), 1);
                deckA.setActiveCueBank(1);
                activeCueBankA = 1;
                break;
            case dj::InputCommand::SetCueA3:
                deckA.setCue(deckA.currentFrame(), 2);
                deckA.setActiveCueBank(2);
                activeCueBankA = 2;
                break;
            case dj::InputCommand::JumpCueA1:
                deckA.setActiveCueBank(0);
                deckA.jumpToCue(0);
                activeCueBankA = 0;
                break;
            case dj::InputCommand::JumpCueA2:
                deckA.setActiveCueBank(1);
                deckA.jumpToCue(1);
                activeCueBankA = 1;
                break;
            case dj::InputCommand::JumpCueA3:
                deckA.setActiveCueBank(2);
                deckA.jumpToCue(2);
                activeCueBankA = 2;
                break;
            // Phase 4: Multi-cue hotspot bank B commands
            case dj::InputCommand::SetCueB1:
                deckB.setCue(deckB.currentFrame(), 0);
                deckB.setActiveCueBank(0);
                activeCueBankB = 0;
                break;
            case dj::InputCommand::SetCueB2:
                deckB.setCue(deckB.currentFrame(), 1);
                deckB.setActiveCueBank(1);
                activeCueBankB = 1;
                break;
            case dj::InputCommand::SetCueB3:
                deckB.setCue(deckB.currentFrame(), 2);
                deckB.setActiveCueBank(2);
                activeCueBankB = 2;
                break;
            case dj::InputCommand::JumpCueB1:
                deckB.setActiveCueBank(0);
                deckB.jumpToCue(0);
                activeCueBankB = 0;
                break;
            case dj::InputCommand::JumpCueB2:
                deckB.setActiveCueBank(1);
                deckB.jumpToCue(1);
                activeCueBankB = 1;
                break;
            case dj::InputCommand::JumpCueB3:
                deckB.setActiveCueBank(2);
                deckB.jumpToCue(2);
                activeCueBankB = 2;
                break;
            // Phase 4: Tempo ramp toggle
            case dj::InputCommand::TempoRampToggle:
                // Toggle tempo ramp for both decks
                tempoRampEnabledA = !tempoRampEnabledA;
                tempoRampEnabledB = !tempoRampEnabledB;
                deckA.setTempoRampEnabled(tempoRampEnabledA);
                deckB.setTempoRampEnabled(tempoRampEnabledB);
                // When enabling, set target to current tempo for smooth start
                if (tempoRampEnabledA) {
                    deckA.setTargetTempo(tempoA);
                    deckA.setTempoRampRate(0.01f);  // Moderate ramp rate
                }
                if (tempoRampEnabledB) {
                    deckB.setTargetTempo(tempoB);
                    deckB.setTempoRampRate(0.01f);  // Moderate ramp rate
                }
                break;
            // Phase 29: Multi-track recording control
            case dj::InputCommand::RecordToggle:
                if (!recordingActive) {
                    recorderMix.clear();
                    recorderDeckA.clear();
                    recorderDeckB.clear();
                    recorderMix.start();
                    recorderDeckA.start();
                    recorderDeckB.start();
                    recordingActive = true;
                    sessionMetadata.setStartTime(std::time(nullptr));
                } else {
                    recorderMix.stop();
                    recorderDeckA.stop();
                    recorderDeckB.stop();
                    recordingActive = false;
                    sessionMetadata.setDuration(recorderMix.getDuration());
                }
                break;
            case dj::InputCommand::SaveRecording:
                if (recordingActive) {
                    recorderMix.stop();
                    recorderDeckA.stop();
                    recorderDeckB.stop();
                    recordingActive = false;
                }
                if (recorderMix.getDuration() > 0.0f) {
                    // Export recorded mix session to WAV file
                    const std::string filename = "session_recording_mix.wav";
                    const auto recordedData = recorderMix.getRecordedData();
                    if (!recordedData.empty()) {
                        dj::WAVExporter exporter(outputRate, 2, 16);
                        const std::size_t numFrames = recordedData.size() / 2;  // 2 channels
                        if (exporter.exportToFile(filename, recordedData.data(), numFrames)) {
                            std::cout << "Mix recording saved to " << filename << "\n";
                        } else {
                            std::cout << "Failed to save mix recording to " << filename << "\n";
                        }
                    }
                } else {
                    std::cout << "No recording data to save.\n";
                }
                break;
            // Phase 27: Preset hotkeys - F1-F12 for Deck A
            case dj::InputCommand::LoadPresetEQ_A_1:
                applyEQPreset(presetManager, getPresetSlotName('A', 1), deckA, eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_A_2:
                applyEQPreset(presetManager, getPresetSlotName('A', 2), deckA, eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_A_3:
                applyEQPreset(presetManager, getPresetSlotName('A', 3), deckA, eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_A_4:
                applyEQPreset(presetManager, getPresetSlotName('A', 4), deckA, eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_A_5:
                applyEQPreset(presetManager, getPresetSlotName('A', 5), deckA, eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_A_6:
                applyEQPreset(presetManager, getPresetSlotName('A', 6), deckA, eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_A_7:
                applyEQPreset(presetManager, getPresetSlotName('A', 7), deckA, eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_A_8:
                applyEQPreset(presetManager, getPresetSlotName('A', 8), deckA, eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_A_9:
                applyEQPreset(presetManager, getPresetSlotName('A', 9), deckA, eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_A_10:
                applyEQPreset(presetManager, getPresetSlotName('A', 10), deckA, eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_A_11:
                applyEQPreset(presetManager, getPresetSlotName('A', 11), deckA, eqALow, eqAMid, eqAHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_A_12:
                applyEQPreset(presetManager, getPresetSlotName('A', 12), deckA, eqALow, eqAMid, eqAHigh);
                break;
            // Phase 27: Preset hotkeys - Shift+F1-F12 for Deck B
            case dj::InputCommand::LoadPresetEQ_B_1:
                applyEQPreset(presetManager, getPresetSlotName('B', 1), deckB, eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_B_2:
                applyEQPreset(presetManager, getPresetSlotName('B', 2), deckB, eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_B_3:
                applyEQPreset(presetManager, getPresetSlotName('B', 3), deckB, eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_B_4:
                applyEQPreset(presetManager, getPresetSlotName('B', 4), deckB, eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_B_5:
                applyEQPreset(presetManager, getPresetSlotName('B', 5), deckB, eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_B_6:
                applyEQPreset(presetManager, getPresetSlotName('B', 6), deckB, eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_B_7:
                applyEQPreset(presetManager, getPresetSlotName('B', 7), deckB, eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_B_8:
                applyEQPreset(presetManager, getPresetSlotName('B', 8), deckB, eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_B_9:
                applyEQPreset(presetManager, getPresetSlotName('B', 9), deckB, eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_B_10:
                applyEQPreset(presetManager, getPresetSlotName('B', 10), deckB, eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_B_11:
                applyEQPreset(presetManager, getPresetSlotName('B', 11), deckB, eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::LoadPresetEQ_B_12:
                applyEQPreset(presetManager, getPresetSlotName('B', 12), deckB, eqBLow, eqBMid, eqBHigh);
                break;
            case dj::InputCommand::ToggleSpectrum:
                showSpectrum = !showSpectrum;
                std::cout << (showSpectrum ? "Spectrum: ON\n" : "Spectrum: OFF\n");
                break;
            case dj::InputCommand::ToggleEnergyCurve:
                showEnergyCurve = !showEnergyCurve;
                std::cout << (showEnergyCurve ? "Energy Curve: ON\n" : "Energy Curve: OFF\n");
                break;
            // Phase 36: Beat Grid Nudge Editor
            case dj::InputCommand::NudgeBeatGridLeft:
                // Nudge all beats left by 10ms (both decks)
                beatGridEditorA.nudgeBeats(-10.0);
                beatGridEditorB.nudgeBeats(-10.0);
                std::cout << "Beat grid nudged left by 10ms\n";
                break;
            case dj::InputCommand::NudgeBeatGridRight:
                // Nudge all beats right by 10ms (both decks)
                beatGridEditorA.nudgeBeats(10.0);
                beatGridEditorB.nudgeBeats(10.0);
                std::cout << "Beat grid nudged right by 10ms\n";
                break;
            case dj::InputCommand::AdjustFirstBeatLeft:
                // Adjust first beat offset left by 10ms (both decks)
                beatGridEditorA.setFirstBeatOffset(-10.0);
                beatGridEditorB.setFirstBeatOffset(-10.0);
                std::cout << "First beat offset adjusted left by 10ms\n";
                break;
            case dj::InputCommand::AdjustFirstBeatRight:
                // Adjust first beat offset right by 10ms (both decks)
                beatGridEditorA.setFirstBeatOffset(10.0);
                beatGridEditorB.setFirstBeatOffset(10.0);
                std::cout << "First beat offset adjusted right by 10ms\n";
                break;
            case dj::InputCommand::UndoBeatGrid:
                // Undo beat grid edits (both decks)
                if (beatGridEditorA.canUndo()) {
                    beatGridEditorA.undo();
                    beatGridEditorB.undo();
                    std::cout << "Beat grid edit undone\n";
                } else {
                    std::cout << "No beat grid edits to undo\n";
                }
                break;
            case dj::InputCommand::RedoBeatGrid:
                // Redo beat grid edits (both decks)
                if (beatGridEditorA.canRedo()) {
                    beatGridEditorA.redo();
                    beatGridEditorB.redo();
                    std::cout << "Beat grid edit redone\n";
                } else {
                    std::cout << "No beat grid edits to redo\n";
                }
                break;
            case dj::InputCommand::ToggleSyncDeckA:
                // Toggle sync on Deck A targeting Deck B
                if (deckA.isSyncEnabled()) {
                    deckA.disableAutoSync();
                    std::cout << "Sync disabled on Deck A\n";
                } else {
                    deckA.setAutoSyncTarget(&deckB);
                    std::cout << "Sync enabled on Deck A (targeting Deck B)\n";
                }
                break;
            case dj::InputCommand::ToggleSyncDeckB:
                // Toggle sync on Deck B targeting Deck A
                if (deckB.isSyncEnabled()) {
                    deckB.disableAutoSync();
                    std::cout << "Sync disabled on Deck B\n";
                } else {
                    deckB.setAutoSyncTarget(&deckA);
                    std::cout << "Sync enabled on Deck B (targeting Deck A)\n";
                }
                break;
            case dj::InputCommand::BeatJumpForwardA:
                // Jump +4 beats on Deck A
                deckA.beatJump(4);
                std::cout << "Deck A beat jump +4\n";
                break;
            case dj::InputCommand::BeatJumpBackwardA:
                // Jump -4 beats on Deck A
                deckA.beatJump(-4);
                std::cout << "Deck A beat jump -4\n";
                break;
            case dj::InputCommand::BeatJumpForwardB:
                // Jump +4 beats on Deck B
                deckB.beatJump(4);
                std::cout << "Deck B beat jump +4\n";
                break;
            case dj::InputCommand::BeatJumpBackwardB:
                // Jump -4 beats on Deck B
                deckB.beatJump(-4);
                std::cout << "Deck B beat jump -4\n";
                break;
            case dj::InputCommand::Quit:
                quitRequested = true;
                break;
            case dj::InputCommand::None:
                break;
            }
        }

        if (quitRequested) {
            break;
        }

        if (!manualMixMode) {
            crossfaderPosition = (progress * 2.0f) - 1.0f;
            mixer.setCrossfader(crossfaderPosition);
        }

        dj::MixMetrics metrics;
        const std::vector<float> mixed = mixer.mixBlock(deckA, deckB, framesPerBlock, metrics);

        // Phase 29: Submit audio to multi-track recorders if recording is active
        if (recordingActive && !mixed.empty()) {
            recorderMix.submitFrames(mixed.data(), framesPerBlock);
            // Note: Individual deck recording would require separate audio capture
        }

        if (realtimeAudio) {
            std::string writeError;
            if (!player.write(mixed, &writeError)) {
                realtimeAudio = false;
                std::cout << "Audio write failed (" << writeError << "), continuing silently.\n";
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(7));
        }

        const float effectiveBpmA = bpmA * (1.0f + (tempoA / 100.0f));
        const float effectiveBpmB = bpmB * (1.0f + (tempoB / 100.0f));
        const auto gains = mixer.crossfadeGains();
        const float gainSum = std::max(0.001f, gains.first + gains.second);
        const float blendedBpm = ((effectiveBpmA * gains.first) + (effectiveBpmB * gains.second)) / gainSum;
        
        // Phase 6: Calculate beatmatch delta for crowd energy bonus/penalty
        const float beatmatchDelta = std::abs(effectiveBpmB - effectiveBpmA);
        const dj::CrowdOutput crowdOut = crowd.update(blendedBpm, metrics.transitionSmoothness, metrics.rms, beatmatchDelta);

        const int score = scoring.update(crowdOut.energyMeter, metrics.transitionSmoothness);
        career.update(crowdOut.energyMeter);
        
        // Phase 26: Check for auto-save trigger
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastAutoSaveTime).count();
        if (elapsedSeconds >= autoSaveIntervalSeconds) {
            dj::SessionState currentState = captureCurrentState(
                deckA, deckB, crossfaderPosition, career, 
                crowdOut.energyMeter, pathA, pathB, tempoA, tempoB,
                eqALow, eqAMid, eqAHigh, eqBLow, eqBMid, eqBHigh
            );
            
            if (sessionManager.saveSession("autosave.json", currentState)) {
                lastAutoSaveTime = currentTime;
            }
        }

        // Arc V: Career mode achievement tracking
        if (careerMode) {
            // Check tier achievements
            if (career.tier() >= 2 && !achievements.isUnlocked("tier_2")) {
                achievements.checkAndUnlock("tier_2");
                std::cout << "\n🏆 Achievement Unlocked: Rising Star (Reach Tier 2)!\n";
                achievements.saveToFile("achievements.json");
            }
            if (career.tier() >= 4 && !achievements.isUnlocked("tier_4")) {
                achievements.checkAndUnlock("tier_4");
                std::cout << "\n🏆 Achievement Unlocked: Headliner (Reach Tier 4)!\n";
                achievements.saveToFile("achievements.json");
            }
            // Check energy achievement
            if (crowdOut.energyMeter > 0.8f && !achievements.isUnlocked("crowd_master")) {
                // Would need sustained tracking - simplified for now
                achievements.checkAndUnlock("crowd_master");
                std::cout << "\n🏆 Achievement Unlocked: Crowd Master!\n";
                achievements.saveToFile("achievements.json");
            }
        }

        // Phase 7: Update lighting rig and render graphics if available
        constexpr float blockDurationSeconds = framesPerBlock / 44100.0f;

        // Phase 37: Update rolling energy curve + mix quality once per second
        const double elapsedTimeSeconds = static_cast<double>(block) * blockDurationSeconds;
        const bool bassClashProxy = (metrics.deckAEnergy > 0.7f) && (metrics.deckBEnergy > 0.7f);

        if (lastEnergySampleTimeSeconds < 0.0 || (elapsedTimeSeconds - lastEnergySampleTimeSeconds) >= 1.0) {
            energyCurve.addSample(crowdOut.energyMeter, elapsedTimeSeconds, metrics.rms);
            lastEnergySampleTimeSeconds = elapsedTimeSeconds;
        }

        if (lastMixAnalysisTimeSeconds < 0.0 || (elapsedTimeSeconds - lastMixAnalysisTimeSeconds) >= 1.0) {
            mixQualityAnalyzer.setAnalysisContext(
                effectiveBpmA,
                effectiveBpmB,
                metrics.transitionSmoothness,
                "8A",
                "8B");
            mixQualityAnalyzer.setBassClashState(bassClashProxy);
            mixQualityAnalyzer.analyzeMix(&deckA, &deckB, &mixer);
            lastMixAnalysisTimeSeconds = elapsedTimeSeconds;
        }

        lighting.update(blendedBpm, crowdOut.energyMeter, blockDurationSeconds);
        
        int moodIndex = static_cast<int>(crowdOut.mood);
        if (graphicsEnabled) {
            graphics.renderFrame(blendedBpm, crowdOut.energyMeter, moodIndex, mixer.crossfader());
        }

        if ((block % 60) == 0) {
            std::cout << "\nBlock " << block << "/" << totalBlocks
                      << "  XF: " << std::setw(5) << mixer.crossfader()
                      << "  Gains A/B: " << gains.first << " / " << gains.second 
                      << (recordingActive ? "  [REC]" : "")  // Phase 14: Recording indicator
                      << "\n";
            std::cout << "Mix mode: " << (manualMixMode ? "Manual" : "Autopilot")
                      << " | Tempo B: " << tempoB << "%"
                      << " | Beat delta(B-A): " << (effectiveBpmB - effectiveBpmA)
                      << " (" << beatmatchLabel(effectiveBpmB - effectiveBpmA) << ")"
                      << " | Loop A/B: " << (loopAEnabled ? "On" : "Off") << "/" << (loopBEnabled ? "On" : "Off")
                      << " | Cue A/B: " << cueA << "/" << cueB << "\n";
            std::cout << "Tone A L/M/H/F: " << eqALow << "/" << eqAMid << "/" << eqAHigh << "/" << filterA
                      << " | Tone B L/M/H/F: " << eqBLow << "/" << eqBMid << "/" << eqBHigh << "/" << filterB << "\n";
            
            // Phase 31: Display Camelot notation if track keys are detected
            dj::CamelotAnalyzer camelotAnalyzer;
            std::string camelotA = "8A";
            std::string camelotB = "8B";
            float compatibilityScore = camelotAnalyzer.getCompatibilityScore(camelotA, camelotB);
            
            // Note: In a full implementation, we would get the detected key from the loaded tracks
            // For now, we display placeholder values. In production, integrate with KeyDetector output.
            // Example: if (auto keyA = track metadata) camelotA = camelotAnalyzer.keyToCamelot(keyA);
            
            std::cout << "Harmonic: A " << camelotA << " | B " << camelotB 
                      << " | Compatibility: " << std::fixed << std::setprecision(1) 
                      << (compatibilityScore * 100.0f) << "%\n";
            
            std::cout << "Crowd: " << moodLabel(crowdOut.mood)
                      << " | " << crowdOut.reaction
                      << " | Energy " << meterBar(crowdOut.energyMeter)
                      << " | Score " << score
                      << " | Venue " << career.currentVenueName() << "\n";
            
            // Arc V: Career mode status
            if (careerMode) {
                std::cout << "Career: Tier " << career.tier() 
                          << " | Reputation " << std::fixed << std::setprecision(1) << career.reputation() << "/100"
                          << " | Unlocked: " << unlocks.getUnlockedEffects(career.tier()).size() << " effects"
                          << " | Achievements: " << achievements.getUnlockedCount() << "/" << achievements.getAllAchievements().size() << "\n";
            }
            
            std::cout << waveform.render(mixed) << "\n";
            
            // Phase 36: Render beat markers overlay (show beats from Deck A as reference)
            if (deckA.clip()) {
                const auto& beatGrid = beatGridEditorA.getBeatGrid();
                auto beats = beatGrid.getBeats();
                
                // Convert beat timestamps to sample positions
                std::vector<size_t> beatPositions;
                int sampleRate = deckA.clip()->sampleRate;
                for (const auto& beat : beats) {
                    size_t samplePos = static_cast<size_t>(beat.timestamp * sampleRate);
                    beatPositions.push_back(samplePos);
                }
                
                // Get current playback position
                size_t currentPosA = deckA.currentFrame();
                
                // Calculate window range (show a window around current position)
                size_t windowSize = static_cast<size_t>(sampleRate * 10.0);  // 10 second window
                size_t windowStart = (currentPosA > windowSize / 2) ? (currentPosA - windowSize / 2) : 0;
                size_t windowEnd = windowStart + windowSize;
                
                // Render beat markers (overlaying a simple representation)
                // Note: This is a simplified version; full integration would overlay on actual waveform
                if (!beatPositions.empty()) {
                    std::string markerLine(68, ' ');
                    for (size_t beatPos : beatPositions) {
                        if (beatPos >= windowStart && beatPos <= windowEnd) {
                            int col = static_cast<int>((beatPos - windowStart) * 68.0 / windowSize);
                            if (col >= 0 && col < 68) {
                                // Highlight current beat
                                if (std::abs(static_cast<long long>(beatPos) - static_cast<long long>(currentPosA)) < sampleRate / 10) {
                                    markerLine[col] = 'v';
                                } else {
                                    markerLine[col] = '|';
                                }
                            }
                        }
                    }
                    std::cout << "Beats:     " << markerLine << "\n";
                }
            }

            // Phase 37: Render energy curve + mix quality analytics if enabled
            if (showEnergyCurve) {
                std::cout << "\n=== ENERGY CURVE ANALYTICS (Shift+E to toggle) ===\n";
                std::cout << energyCurveRenderer.render(energyCurve, &mixQualityAnalyzer);
                if (mixQualityAnalyzer.hasBassClash()) {
                    std::cout << "Warning: Bass clash detected between active decks.\n";
                }
                std::cout << "====================================================\n";
            }
            
            // Phase 35: Render spectrum display if enabled
            // NOTE: Currently disabled - requires Deck::getSpectrumAnalyzer() integration
            /*
            if (showSpectrum) {
                auto spectrumA = deckA.getSpectrumAnalyzer().getFullSpectrum();
                auto spectrumB = deckB.getSpectrumAnalyzer().getFullSpectrum();
                std::cout << "\n=== SPECTRUM ANALYZER (Press '9' to toggle) ===\n";
                std::cout << spectrumRenderer.renderDualDeck(spectrumA, spectrumB);
                std::cout << "================================================\n";
            }
            */
        }

        if (!deckA.isPlaying() && !deckB.isPlaying()) {
            break;
        }
    }

    // Phase 7: Clean up graphics
    if (graphicsEnabled) {
        graphics.shutdown();
    }

    player.close();

    std::cout << "\nSet complete. Final score: " << scoring.score()
              << " | Career venue: " << career.currentVenueName() << "\n";
    
    // Arc V: Final career status
    if (careerMode) {
        printCareerStatus(career, unlocks, achievements);
        achievements.saveToFile("achievements.json");
    }
    
    // Arc VI: Save configuration on exit
    if (configManager.saveConfig("config.json")) {
        std::cout << "Configuration saved to config.json\n";
    }
    
    std::cout << "Milestone status: dual playback, crossfade, crowd meter, waveform visualization complete.\n";
    std::cout << "Next hooks ready: EQ/filter expansion, BPM-tempo control, looping workflows, and stage visuals.\n";
    return 0;
}
