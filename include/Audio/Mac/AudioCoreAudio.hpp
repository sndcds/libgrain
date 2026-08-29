#pragma once

#include "Audio/AudioDeviceList.hpp"

namespace Grain {

    class AudioCoreAudio {
    public:
        AudioDeviceList enumerateDevices();

    private:
        static std::string getDeviceName(AudioDeviceId id);
        static uint32_t getChannelCount(AudioDeviceId id, AudioObjectPropertyScope scope);
        static double getSampleRate(AudioDeviceId id);
    };

}