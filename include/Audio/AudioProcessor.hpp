//
//  AudioProcessor.hpp
//
//  Created by Roald Christesen on from 29.06.2026
//  Copyright (C) 2026 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainAudioProcessor_hpp
#define GrainAudioProcessor_hpp

#include "AudioProcessContext.hpp"


namespace Grain {

    class AudioProcessor {
    public:
        virtual ~AudioProcessor() = default;

        virtual void process(
            AudioProcessContext& context
        ) noexcept = 0;
    };
}

#endif