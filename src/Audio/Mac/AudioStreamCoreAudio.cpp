#include "Audio/Mac/AudioStreamCoreAudio.hpp"
#include <iostream>

namespace Grain {

    AudioStreamCoreAudio::~AudioStreamCoreAudio() {
        close();
    }

    bool AudioStreamCoreAudio::open(AudioDeviceId deviceId,
                                   uint32_t inputChannels,
                                   uint32_t outputChannels,
                                   double sampleRate,
                                   uint32_t framesPerBuffer)
    {
        deviceId_ = deviceId;
        inputChannels_ = inputChannels;
        outputChannels_ = outputChannels;
        sampleRate_ = sampleRate;
        framesPerBuffer_ = framesPerBuffer;

        if (deviceId_ == 0)
            return false;

        // 1. Set buffer size
        UInt32 bufferSize = framesPerBuffer_;

        AudioObjectPropertyAddress addr = {
            kAudioDevicePropertyBufferFrameSize,
            kAudioObjectPropertyScopeGlobal,
            kAudioObjectPropertyElementMain
        };

        OSStatus err = AudioObjectSetPropertyData(
            deviceId_,
            &addr,
            0,
            nullptr,
            sizeof(bufferSize),
            &bufferSize
        );

        if (err != noErr) {
            std::cerr << "Failed to set buffer size: " << err << "\n";
            return false;
        }

        // 2. Create IOProc (IMPORTANT: correct CoreAudio API usage)
        err = AudioDeviceCreateIOProcID(
            deviceId_,
            ioCallback,
            this,
            &ioProcId_
        );

        if (err != noErr) {
            std::cerr << "Failed to create IOProc: " << err << "\n";
            return false;
        }

        return true;
    }

    bool AudioStreamCoreAudio::start() {
        if (!deviceId_ || !ioProcId_)
            return false;

        OSStatus err = AudioDeviceStart(deviceId_, ioProcId_);
        if (err != noErr) {
            std::cerr << "AudioDeviceStart failed: " << err << "\n";
            return false;
        }

        return true;
    }

    void AudioStreamCoreAudio::stop() {
        if (!deviceId_ || !ioProcId_)
            return;

        AudioDeviceStop(deviceId_, ioProcId_);
    }

    void AudioStreamCoreAudio::close() {
        if (!deviceId_)
            return;

        stop();

        if (ioProcId_) {
            AudioDeviceDestroyIOProcID(deviceId_, ioProcId_);
            ioProcId_ = nullptr;
        }

        deviceId_ = 0;
        clientData_ = nullptr;
    }

    void AudioStreamCoreAudio::setClientData(void* ptr) {
        clientData_ = ptr;
    }

    void AudioStreamCoreAudio::setCallback(AudioCallback callback) {
        callback_ = callback;
    }

    OSStatus AudioStreamCoreAudio::ioCallback(
            AudioDeviceID,
            const AudioTimeStamp*,
            const AudioBufferList*,
            const AudioTimeStamp*,
            AudioBufferList* outData,
            const AudioTimeStamp*,
            void* client)
    {
        auto* self = static_cast<AudioStreamCoreAudio*>(client);

        if (!self || !outData)
            return noErr;

        float* out = static_cast<float*>(outData->mBuffers[0].mData);

        const uint32_t channels = self->outputChannels_;
        const uint32_t frames =
            outData->mBuffers[0].mDataByteSize /
            (sizeof(float) * channels);

        if (self->callback_) {
            self->callback_(out, frames, channels);
        }
        else {
            for (uint32_t i = 0; i < frames * channels; i++) {
                out[i] = 0.0;
            }
        }

        return noErr;
    }

}