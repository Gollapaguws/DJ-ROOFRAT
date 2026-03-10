#include "audio/PortAudioPlayer.h"

#if DJ_SIM_USE_PORTAUDIO
#include <portaudio.h>
#endif

namespace dj {

PortAudioPlayer::PortAudioPlayer() = default;

PortAudioPlayer::~PortAudioPlayer() {
    close();
}

bool PortAudioPlayer::available() const {
#if DJ_SIM_USE_PORTAUDIO
    return true;
#else
    return false;
#endif
}

bool PortAudioPlayer::open(int sampleRate, unsigned long framesPerBuffer, std::string* errorOut) {
#if DJ_SIM_USE_PORTAUDIO
    if (open_) {
        return true;
    }

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        if (errorOut != nullptr) {
            *errorOut = Pa_GetErrorText(err);
        }
        return false;
    }
    initialized_ = true;

    PaStream* streamHandle = nullptr;
    err = Pa_OpenDefaultStream(
        &streamHandle,
        0,
        2,
        paFloat32,
        sampleRate,
        framesPerBuffer,
        nullptr,
        nullptr);

    if (err != paNoError) {
        if (errorOut != nullptr) {
            *errorOut = Pa_GetErrorText(err);
        }
        close();
        return false;
    }

    err = Pa_StartStream(streamHandle);
    if (err != paNoError) {
        if (errorOut != nullptr) {
            *errorOut = Pa_GetErrorText(err);
        }
        Pa_CloseStream(streamHandle);
        close();
        return false;
    }

    stream_ = streamHandle;
    open_ = true;
    return true;
#else
    (void)sampleRate;
    (void)framesPerBuffer;
    if (errorOut != nullptr) {
        *errorOut = "PortAudio support is disabled at build time.";
    }
    return false;
#endif
}

bool PortAudioPlayer::write(const std::vector<float>& interleavedStereo, std::string* errorOut) {
#if DJ_SIM_USE_PORTAUDIO
    if (!open_ || stream_ == nullptr) {
        if (errorOut != nullptr) {
            *errorOut = "Audio stream is not open.";
        }
        return false;
    }

    if (interleavedStereo.empty()) {
        return true;
    }

    const unsigned long frames = static_cast<unsigned long>(interleavedStereo.size() / 2U);
    const auto err = Pa_WriteStream(static_cast<PaStream*>(stream_), interleavedStereo.data(), frames);
    if (err != paNoError) {
        if (errorOut != nullptr) {
            *errorOut = Pa_GetErrorText(err);
        }
        return false;
    }

    return true;
#else
    (void)interleavedStereo;
    if (errorOut != nullptr) {
        *errorOut = "PortAudio support is disabled at build time.";
    }
    return false;
#endif
}

void PortAudioPlayer::close() {
#if DJ_SIM_USE_PORTAUDIO
    if (stream_ != nullptr) {
        Pa_StopStream(static_cast<PaStream*>(stream_));
        Pa_CloseStream(static_cast<PaStream*>(stream_));
        stream_ = nullptr;
    }

    open_ = false;

    if (initialized_) {
        Pa_Terminate();
        initialized_ = false;
    }
#else
    open_ = false;
#endif
}

bool PortAudioPlayer::isOpen() const {
    return open_;
}

} // namespace dj
