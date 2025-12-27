//
//  AudioInterface.hpp
//
//  Created by Roald Christesen on from 28.11.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainAudioInterface_hpp
#define GrainAudioInterface_hpp

#include <Grain.hpp>
#include "String/String.hpp"

#if defined(__APPLE__) && defined(__MACH__)
#include <CoreAudio/CoreAudio.h>
#endif


namespace Grain {

typedef int64_t AudioInterfaceDeviceId;

    enum class AudioInterfaceDeviceScope {
        Global = 0,
        Input = 1,
        Output = 2,
        PlayThrough = 3,

        Count,
        First = 0
    } ;

class AudioInterface {

protected:
    AudioInterfaceDeviceId interface_id{};

public:
    AudioInterface() noexcept = default;

    void updateInterfaceInfo() noexcept;
    static bool _mac_deviceName(AudioDeviceID mac_device_id, String& out_device_name);
    static uint32_t _mac_channelCount(AudioDeviceID mac_device_id, AudioObjectPropertyScope mac_scope);
};


} // End of namespace Grain

#endif // GrainAudioInterface_hpp
