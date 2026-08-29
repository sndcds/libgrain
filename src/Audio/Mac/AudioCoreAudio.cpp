#include "Audio/Mac/AudioCoreAudio.hpp"
#include <vector>
#include <iostream>

namespace Grain {

AudioDeviceList AudioCoreAudio::enumerateDevices() {
    AudioDeviceList list;

    AudioObjectPropertyAddress addr = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };

    UInt32 size = 0;
    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &addr, 0, nullptr, &size);

    size_t count = size / sizeof(AudioDeviceID);
    std::vector<AudioDeviceID> ids(count);

    AudioObjectGetPropertyData(
        kAudioObjectSystemObject,
        &addr,
        0,
        nullptr,
        &size,
        ids.data()
    );

    list.devices.reserve(count);

    for (auto id : ids) {
        AudioDevice dev;
        dev.id = id;
        dev.name = getDeviceName(id);
        dev.inputChannels = getChannelCount(id, kAudioDevicePropertyScopeInput);
        dev.outputChannels = getChannelCount(id, kAudioDevicePropertyScopeOutput);
        dev.sampleRate = getSampleRate(id);

        list.devices.push_back(dev);
    }

    return list;
}

std::string AudioCoreAudio::getDeviceName(AudioDeviceId id) {
    CFStringRef name = nullptr;
    UInt32 size = sizeof(CFStringRef);

    AudioObjectPropertyAddress addr = {
        kAudioObjectPropertyName,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };

    if (AudioObjectGetPropertyData(id, &addr, 0, nullptr, &size, &name) != noErr)
        return {};

    char buffer[256];
    CFStringGetCString(name, buffer, sizeof(buffer), kCFStringEncodingUTF8);
    CFRelease(name);

    return std::string(buffer);
}

uint32_t AudioCoreAudio::getChannelCount(AudioDeviceId id, AudioObjectPropertyScope scope) {
    AudioObjectPropertyAddress addr = {
        kAudioDevicePropertyStreamConfiguration,
        scope,
        kAudioObjectPropertyElementMain
    };

    UInt32 size = 0;
    AudioObjectGetPropertyDataSize(id, &addr, 0, nullptr, &size);

    auto bufferList = (AudioBufferList*)malloc(size);

    uint32_t channels = 0;

    if (bufferList) {
        AudioObjectGetPropertyData(id, &addr, 0, nullptr, &size, bufferList);

        for (UInt32 i = 0; i < bufferList->mNumberBuffers; i++) {
            channels += bufferList->mBuffers[i].mNumberChannels;
        }

        free(bufferList);
    }

    return channels;
}

double AudioCoreAudio::getSampleRate(AudioDeviceId id) {
    Float64 rate = 0;
    UInt32 size = sizeof(rate);

    AudioObjectPropertyAddress addr = {
        kAudioDevicePropertyNominalSampleRate,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };

    if (AudioObjectGetPropertyData(id, &addr, 0, nullptr, &size, &rate) != noErr)
        return 0.0;

    return rate;
}

}