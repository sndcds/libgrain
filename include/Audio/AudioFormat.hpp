//
//  AudioFormat.hpp
//
//  Created by Roald Christesen on from 29.06.2026
//  Copyright (C) 2026 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainAudioFormat_hpp
#define GrainAudioFormat_hpp

#include "AudioTypes.hpp"


namespace Grain {

    struct AudioFormat {
        double sample_rate = 48000.0;
        uint32_t input_channels = 0;
        uint32_t output_channels = 2;
        uint32_t block_size = 512;

        AudioSampleFormat sample_format =
            AudioSampleFormat::Float32;

        bool interleaved = false;
    };

}

#endif