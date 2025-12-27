//
//  AudioInterface.cpp
//
//  Created by Roald Christesen on from 28.11.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Audio/AudioInterface.hpp"
#include "Type/Type.hpp"

#include <iostream>
#include <vector>



namespace Grain {

void AudioInterface::updateInterfaceInfo() noexcept {

    // Get all device IDs
    AudioObjectPropertyAddress addr = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };

    UInt32 size = 0;
    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &addr, 0, nullptr, &size);

    UInt32 device_count = size / sizeof(AudioDeviceID);
    std::vector<AudioDeviceID> devices(device_count);

    AudioObjectGetPropertyData(
        kAudioObjectSystemObject,
        &addr,
        0,
        nullptr,
        &size,
        devices.data()
    );

    // Print device information
    std::cout << "Found " << device_count << " audio devices:\n";

    for (auto dev : devices) {
        String device_name;
        _mac_deviceName(dev, device_name);
        uint32_t input_count = _mac_channelCount(dev, kAudioDevicePropertyScopeInput);
        uint32_t output_count = _mac_channelCount(dev, kAudioDevicePropertyScopeOutput);

        std::cout << "-------------------------------\n";
        std::cout << "Device ID : " << dev << "\n";
        std::cout << "Name      : " << device_name << "\n";
        std::cout << "Input Ch  : " << input_count << "\n";
        std::cout << "Output Ch : " << output_count << "\n";
    }
}

bool AudioInterface::_mac_deviceName(AudioDeviceID mac_device_id, String& out_device_name) {
    CFStringRef name = nullptr;
    UInt32 size = sizeof(CFStringRef);

    AudioObjectPropertyAddress addr = {
        kAudioObjectPropertyName,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };

    if (AudioObjectGetPropertyData(mac_device_id, &addr, 0, nullptr, &size, &name) != noErr) {
        return false;
    }

    out_device_name.set(name);
    CFRelease(name);
    return true;
}
    uint32_t AudioInterface::_mac_channelCount(AudioDeviceID mac_device_id, AudioObjectPropertyScope mac_scope) {
    int32_t channel_count = 0;

    AudioObjectPropertyAddress addr = {
        kAudioDevicePropertyStreamConfiguration,
        mac_scope,
        kAudioObjectPropertyElementMain
    };


    UInt32 size = 0;
    AudioObjectGetPropertyDataSize(mac_device_id, &addr, 0, nullptr, &size);

    auto buffer_list = static_cast<AudioBufferList*>(malloc(size));
    if (buffer_list) {
        AudioObjectGetPropertyData(mac_device_id, &addr, 0, nullptr, &size, buffer_list);
        for (UInt32 i = 0; i < buffer_list->mNumberBuffers; ++i) {
            channel_count += static_cast<int32_t>(buffer_list->mBuffers[i].mNumberChannels);
        }
    }

    free(buffer_list);
    return channel_count;
}


}  // End of namespace Grain