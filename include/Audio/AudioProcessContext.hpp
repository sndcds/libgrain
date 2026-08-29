//
//  AudioProcessContext.hpp
//
//  Created by Roald Christesen on from 29.06.2026
//  Copyright (C) 2026 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainAudioProcessContext_hpp
#define GrainAudioProcessContext_hpp

#include "AudioBuffer.hpp"


namespace Grain {

    struct AudioProcessContext {
        AudioBuffer input;
        AudioBuffer output;
        uint64_t sample_position = 0;
        double sample_rate = 48000.0;
    };

}

#endif