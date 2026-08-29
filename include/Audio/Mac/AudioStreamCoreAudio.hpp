#pragma once


#include "Audio/AudioTypes.hpp"
#include "Audio/AudioDevice.hpp"

#include <AudioToolbox/AudioToolbox.h>
#include <functional>

namespace Grain {

    using AudioCallback = std::function<void(float* output, uint32_t frames, uint32_t channels)>;

    class AudioStreamCoreAudio {
    public:
        AudioStreamCoreAudio() = default;
        ~AudioStreamCoreAudio();

        bool open(AudioDeviceId deviceId,
                  uint32_t inputChannels,
                  uint32_t outputChannels,
                  double sampleRate,
                  uint32_t framesPerBuffer);

        bool start();
        void stop();
        void close();

        void setClientData(void* ptr);
        void setCallback(AudioCallback callback);

    private:
        AudioDeviceId deviceId_ = 0;
        AudioDeviceIOProcID ioProcId_ = nullptr;

        uint32_t inputChannels_ = 0;
        uint32_t outputChannels_ = 0;
        uint32_t framesPerBuffer_ = 0;
        double sampleRate_ = 0.0;

        void* clientData_ = nullptr;
        AudioCallback callback_ = nullptr;

        static OSStatus ioCallback(
            AudioDeviceID device,
            const AudioTimeStamp* time,
            const AudioBufferList* inData,
            const AudioTimeStamp* inTime,
            AudioBufferList* outData,
            const AudioTimeStamp* outTime,
            void* client
        );
    };

}