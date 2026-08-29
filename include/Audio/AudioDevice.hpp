//
//  AudioDevice.hpp
//
//  Created by Roald Christesen on from 29.06.2026
//  Copyright (C) 2026 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainAudioDevice_hpp
#define GrainAudioDevice_hpp

#include <Grain.hpp>

#include "String/String.hpp"
#include "AudioTypes.hpp"


namespace Grain {

    class AudioDevice {
    public:
        AudioDeviceId id = 0;
        std::string name;
        uint32_t inputChannels = 0;
        uint32_t outputChannels = 0;
        double sampleRate = 0.0;
        bool valid() const { return id != 0; }

    };
}

#endif