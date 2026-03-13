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

#if defined(_WIN32)
#include <conio.h>
#endif

#include "audio/AudioClip.h"
#include "audio/BPMDetector.h"
#include "audio/Deck.h"
#include "audio/Mixer.h"
#include "audio/PortAudioPlayer.h"
#include "audio/Recorder.h"
#include "audio/WAVExporter.h"
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
#include "visuals/WaveformRenderer.h"
#include "visuals/GraphicsContext.h"
#include "visuals/LightingRig.h"
#include "library/TrackLibrary.h"
#include "library/LibraryScanner.h"
#include "library/TrackBrowser.h"

namespace {

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
    std::cout << "\nIf no tracks are provided, generated test tones are used.\n";
}

void printLiveControls() {
    std::cout << "Live controls: [ left XF | \\ center XF | ] right XF | a toggle DeckA | b toggle DeckB\n";
    std::cout << "               i tempo B+ | k tempo B- | o tempo reset | z tempo A+ | x tempo A- | c tempo A reset\n";
    std::cout << "               l loop B | v cueB jump | ; loop beats toggle A | ' loop beats toggle B \n";
    std::cout << "               A EQ/Filter: q/w low-/+ e/r mid-/+ t/y high-/+ u/p filter-/+\n";
    std::cout << "               B EQ/Filter: d/f low-/+ g/h mid-/+ j/n high-/+ m/, filter-/+\n";
    std::cout << "               Isolators: Shift+q/w/e (A low/mid/high) Shift+d/f/g (B low/mid/high)\n";
    std::cout << "               Filter Order: Shift+u or Shift+p to toggle Butterworth mode (both decks)\n";
    std::cout << "               Phase 4 - Multi-Cue Banks: 1/2/3 set cueA1/A2/A3, 4/5/6 set cueB1/B2/B3\n";
    std::cout << "               Phase 4 - Cue Jump: Shift+1/2/3 jump cueA1/A2/A3, Shift+4/5/6 jump cueB1/B2/B3\n";
    std::cout << "               Phase 4 - Tempo Ramp: Shift+R to toggle tempo ramping (both decks)\n";
    std::cout << "               Phase 14 - Recording: s toggle record | Shift+s save recording to WAV\n";
}

std::vector<dj::InputCommand> pollKeyboardCommands() {
    std::vector<dj::InputCommand> commands;
#if defined(_WIN32)
    while (_kbhit() != 0) {
        const int raw = _getch();
        if (raw == 0 || raw == 224) {
            (void)_getch();
            continue;
        }

        const auto command = dj::InputMapper::parseKey(static_cast<char>(raw));
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
    std::vector<int> completed;
    if (progress.loadProgress("tutorial_progress.json")) {
        completed = progress.getCompletedLessons();
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
        completed.push_back(tutorial.getCurrentLessonIndex());
        progress.saveProgress("tutorial_progress.json", completed);
        
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
    std::string missionType = "beatmatch";  // Default mission type

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
        tracks.push_back(arg);
    }

    // Arc V Integration: Mode dispatcher
    if (tutorialMode) {
        return runTutorialMode();
    }
    
    if (missionMode) {
        return runMissionMode(missionType);
    }
    
    // Career mode runs normal performance loop with enhanced feedback
    // (implemented below with printCareerStatus calls)

    dj::Deck deckA;
    dj::Deck deckB;

    constexpr int outputRate = 44100;
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
    mixer.setMasterGain(0.9f);

    // Phase 14: Initialize Recorder for session recording (stereo, 44.1kHz, 10 minute capacity)
    dj::Recorder recorder(outputRate, 2, 600);  // 10 minutes of recording capacity
    bool recordingActive = false;

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

    // Phase 7: Initialize graphics context and lighting rig
    dj::GraphicsContext graphics;
    dj::LightingRig lighting;
    bool graphicsEnabled = graphics.initialize(1280, 720);
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
            // Phase 14: Recording control
            case dj::InputCommand::RecordToggle:
                if (!recordingActive) {
                    recorder.clear();
                    recorder.start();
                    recordingActive = true;
                } else {
                    recorder.stop();
                    recordingActive = false;
                }
                break;
            case dj::InputCommand::SaveRecording:
                if (recordingActive) {
                    recorder.stop();
                    recordingActive = false;
                }
                if (recorder.getDuration() > 0.0f) {
                    // Export recorded session to WAV file
                    const std::string filename = "session_recording.wav";
                    const auto recordedData = recorder.getRecordedData();
                    if (!recordedData.empty()) {
                        dj::WAVExporter exporter(outputRate, 2, 16);
                        const std::size_t numFrames = recordedData.size() / 2;  // 2 channels
                        if (exporter.exportToFile(filename, recordedData.data(), numFrames)) {
                            std::cout << "Recording saved to " << filename << "\n";
                        } else {
                            std::cout << "Failed to save recording to " << filename << "\n";
                        }
                    }
                } else {
                    std::cout << "No recording data to save.\n";
                }
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

        // Phase 14: Submit audio to recorder if recording is active
        if (recordingActive && !mixed.empty()) {
            recorder.submitFrames(mixed.data(), framesPerBlock);
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
    
    std::cout << "Milestone status: dual playback, crossfade, crowd meter, waveform visualization complete.\n";
    std::cout << "Next hooks ready: EQ/filter expansion, BPM-tempo control, looping workflows, and stage visuals.\n";
    return 0;
}
