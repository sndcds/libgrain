//
//  AudioTypes.hpp
//
//  Created by Roald Christesen on from 29.06.2026
//  Copyright (C) 2026 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainAudioTypes_hpp
#define GrainAudioTypes_hpp

#include <Grain.hpp>

#include <CoreAudio/CoreAudio.h>


namespace Grain {

    using AudioDeviceId = uint64_t;

    enum class AudioBackendType {
        Unknown,
        CoreAudio,
        WASAPI,
        ALSA,
        PipeWire
    };

    enum class AudioSampleFormat {
        Float32,
        Float64,
        Int16,
        Int24,
        Int32
    };

    enum class AudioScope {
        Input,
        Output
    };
}

#endif