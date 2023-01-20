#pragma once

#include "Core/Core.hpp"
#include "SDL.h"
#include "SDL_audio.h"
#include "Sound.hpp"

#include <iostream>

namespace IE::Audio {
class AudioEngine : public IE::Core::Engine {
public:
    using AspectType = IE::Audio::Sound;

    size_t        bufferSize{1024};
    SDL_AudioSpec audioSpec;

    AudioEngine() {
        SDL_AudioSpec requestedAudioSpec{
          .freq     = 48000,
          .format   = AUDIO_F32,
          .channels = 2,
          .samples  = 0,
          .padding  = 0,
          .callback = NULL,
          .userdata = this,
        };
        SDL_OpenAudioDevice(
          NULL,
          0,
          &requestedAudioSpec,
          &audioSpec,
          SDL_AUDIO_ALLOW_ANY_CHANGE
        );  // NULL requests the default device
    }

    static std::vector<std::string> listAudioDevices() {
        std::vector<std::string> audioDevices{};
        // Get audio device list
        int audioDeviceCount{SDL_GetNumAudioDevices(0)};  // Zero represents only playback devices
        if (audioDeviceCount < 0) return {};
        for (int j{}; j < audioDeviceCount; ++j) audioDevices.push_back(SDL_GetAudioDeviceName(j, 0));
        return audioDevices;
    }

    void playSound(IE::Audio::Sound *t_soundFile) {
        SDL_AudioStream *stream = SDL_NewAudioStream(AUDIO_F32, 2, 48000, AUDIO_F32, 2, 48000);
        if (stream == NULL) printf("Uhoh, stream failed to create: %s\n", SDL_GetError());
        else {
            float samples[bufferSize];
            int   num_samples = t_soundFile->getNextSamples<float>(bufferSize, &(samples[0]));

            int rc = SDL_AudioStreamPut(stream, samples, num_samples * sizeof(float));
            if (rc == -1) {
                printf("Uhoh, failed to put samples in stream: %s\n", SDL_GetError());
                return;
            }
        }
    }

    IEAspect *createAspect(std::weak_ptr<IEAsset> asset, const std::string &filename) override {
        return nullptr;
    }
};
}  // namespace IE::Audio