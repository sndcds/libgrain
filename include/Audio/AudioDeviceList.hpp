#pragma once

#include "AudioDevice.hpp"
#include <vector>

namespace Grain {

    class AudioDeviceList {
    public:
        std::vector<AudioDevice> devices;

        const AudioDevice* find(AudioDeviceId id) const {
            for (auto const& d : devices)
                if (d.id == id) return &d;
            return nullptr;
        }
    };

}