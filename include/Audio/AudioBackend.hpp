//
//  AudioBackend.hpp
//
//  Created by Roald Christesen on from 29.06.2026
//  Copyright (C) 2026 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainAudioBackend_hpp
#define GrainAudioBackend_hpp


#include "AudioTypes.hpp"
#include "AudioFormat.hpp"
#include "AudioProcessContext.hpp"

#include <functional>


namespace Grain {

    class AudioProcessor;

    using AudioCallback = std::function<void(float* output, uint32_t frames, uint32_t channels)>;


    class AudioBackend {
    public:
        virtual ~AudioBackend() = default;

        virtual bool open(
            AudioDeviceId device,
            const AudioFormat& format
        ) = 0;

        virtual void close() = 0;
        virtual bool start() = 0;
        virtual void stop() = 0;

        virtual void setProcessor(
            AudioProcessor* processor
        ) = 0;
    };

}

#endif