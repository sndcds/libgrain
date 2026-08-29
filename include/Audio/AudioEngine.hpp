//
//  AudioEngine.hpp
//
//  Created by Roald Christesen on from 29.06.2026
//  Copyright (C) 2026 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainAudioEngine_hpp
#define GrainAudioEngine_hpp

#include <memory>

#include "AudioBackend.hpp"
#include "AudioProcessor.hpp"
#include "AudioInterface.hpp"
#include "AudioFormat.hpp"


namespace Grain {

    class AudioEngine {
    public:
        AudioEngine();
        ~AudioEngine();

        bool initialize();
        void shutdown();

        bool openDevice(
            AudioDeviceId device,
            const AudioFormat& format
        );

        void start();
        void stop();

        void setProcessor(AudioProcessor* processor);

        const AudioInterface& interface() const;

    private:

        std::unique_ptr<AudioBackend> backend_;
        AudioInterface interface_;
        AudioProcessor* processor_ = nullptr;
    };

}

#endif