//
//  AudioBuffer.hpp
//
//  Created by Roald Christesen on from 29.06.2026
//  Copyright (C) 2026 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainAudioBuffer_hpp
#define GrainAudioBuffer_hpp

#include <Grain.hpp>


namespace Grain {

    struct AudioBuffer {
        float* channel_data[256];
        uint32_t channel_count = 0;
        uint32_t frame_count = 0;
        bool interleaved = false;

        float* operator[](uint32_t channel) noexcept {
            return channel_data[channel];
        }

        const float* operator[](uint32_t channel) const noexcept {
            return channel_data[channel];
        }
    };

}

#endif