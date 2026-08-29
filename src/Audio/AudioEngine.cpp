//
//  AudioEngine.cpp
//
//  Created by Roald Christesen on from 29.06.2026
//  Copyright (C) 2026 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Audio/AudioEngine.hpp"


namespace Grain {

    AudioEngine::AudioEngine() = default;

    AudioEngine::~AudioEngine() {
        shutdown();
    }

    bool AudioEngine::initialize() {
        interface_.update();

        // Backend selection happens here later
        // For now: placeholder
        return true;
    }

    void AudioEngine::shutdown() {
        if (backend_) {
            backend_->stop();
            backend_->close();
            backend_.reset();
        }
    }

    bool AudioEngine::openDevice(
        AudioDeviceId device,
        const AudioFormat& format
    )
    {
        if (!backend_)
            return false;

        backend_->setProcessor(processor_);

        return backend_->open(device, format);
    }

    void AudioEngine::start() {
        if (backend_)
            backend_->start();
    }

    void AudioEngine::stop() {
        if (backend_)
            backend_->stop();
    }

    void AudioEngine::setProcessor(
        AudioProcessor* processor
    )
    {
        processor_ = processor;

        if (backend_)
            backend_->setProcessor(processor);
    }

    const AudioInterface& AudioEngine::interface() const {
        return interface_;
    }

}