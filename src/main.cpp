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
#include "crowdAI/CrowdStateMachine.h"
#include "gameplay/GameModes.h"
#include "input/InputMapper.h"
#include "visuals/WaveformRenderer.h"

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
    std::cout << "Usage: DJ-ROOFRAT [trackA] [trackB] [--no-audio]\n";
    std::cout << "If no tracks are provided, generated test tones are used.\n";
}

void printLiveControls() {
    std::cout << "Live controls: [ left XF | \\ center XF | ] right XF | a toggle DeckA | b toggle DeckB\n";
    std::cout << "               i tempo B+ | k tempo B- | o tempo reset | 1 loop A | 2 cueA set | 3 cueA jump\n";
    std::cout << "               l loop B | c cueB set | v cueB jump | x quit\n";
    std::cout << "               A EQ/Filter: q/w low-/+ e/r mid-/+ t/y high-/+ u/p filter-/+\n";
    std::cout << "               B EQ/Filter: d/f low-/+ g/h mid-/+ j/n high-/+ m/, filter-/+\n";
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
    std::size_t loopFrames = static_cast<std::size_t>(secondsPerBeat * 16.0 * static_cast<double>(clip->sampleRate));
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

    deck.configureLoop(start, end, true);
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

} // namespace

int main(int argc, char** argv) {
    std::vector<std::string> tracks;
    bool disableAudio = false;

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
        tracks.push_back(arg);
    }

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

    deckA.play();
    deckB.play();

    dj::Mixer mixer;
    mixer.setMasterGain(0.9f);

    float crossfaderPosition = -1.0f;
    mixer.setCrossfader(crossfaderPosition);

    float tempoA = 0.0f;
    float tempoB = 0.0f;
    bool loopAEnabled = false;
    bool loopBEnabled = false;
    std::size_t cueA = 0;
    std::size_t cueB = 0;
    bool manualMixMode = false;
    bool quitRequested = false;

    dj::CrowdStateMachine crowd;
    dj::ScoringSystem scoring;
    dj::CareerProgression career;
    dj::WaveformRenderer waveform(68, 11);

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
        const dj::CrowdOutput crowdOut = crowd.update(blendedBpm, metrics.transitionSmoothness, metrics.rms);

        const int score = scoring.update(crowdOut.energyMeter, metrics.transitionSmoothness);
        career.update(crowdOut.energyMeter);

        if ((block % 60) == 0) {
            std::cout << "\nBlock " << block << "/" << totalBlocks
                      << "  XF: " << std::setw(5) << mixer.crossfader()
                      << "  Gains A/B: " << gains.first << " / " << gains.second << "\n";
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
            std::cout << waveform.render(mixed) << "\n";
        }

        if (!deckA.isPlaying() && !deckB.isPlaying()) {
            break;
        }
    }

    player.close();

    std::cout << "\nSet complete. Final score: " << scoring.score()
              << " | Career venue: " << career.currentVenueName() << "\n";
    std::cout << "Milestone status: dual playback, crossfade, crowd meter, waveform visualization complete.\n";
    std::cout << "Next hooks ready: EQ/filter expansion, BPM-tempo control, looping workflows, and stage visuals.\n";
    return 0;
}
