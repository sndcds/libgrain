//
//  AudioInterface.hpp
//
//  Created by Roald Christesen on from 29.06.2026
//  Copyright (C) 2026 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.

#ifndef GrainAudioInterface_hpp
#define GrainAudioInterface_hpp

#include <vector>

#include "AudioDevice.hpp"


namespace Grain {

    class AudioInterface {
    public:
        AudioInterface() = default;
        bool update();
        int deviceCount() const;
        const AudioDevice& device(int index) const;
        const AudioDevice* defaultInput() const;
        const AudioDevice* defaultOutput() const;

    private:
        std::vector<AudioDevice> devices_;
        int default_input_index_ = -1;
        int default_output_index_ = -1;
    };

}

#endif