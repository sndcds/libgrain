//
//  Midi.cpp
//
//  Created by Roald Christesen on 02.09.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 02.09.2025

#include "Midi/Midi.hpp"


#if defined(__APPLE__) && defined(__MACH__)

namespace Grain {

    void MidiEndpointInfo::log(Log& l) {
        l << "MidiSourceInfo: " << m_name << l.endl;
        l++;
        l << "m_kind: " << kindName() << l.endl;
        l << "m_manufacturer: " << m_manufacturer << l.endl;
        l << "m_model: " << m_model << l.endl;
        l << "m_display_name: " << m_display_name << l.endl;
        l << "m_uid: " << m_uid << l.endl;
        l << "m_device_id: " << m_device_id << l.endl;
        l << "m_offline: " << m_offline << l.endl;
        l << "m_driver_owner: " << m_driver_owner << l.endl;
        l << "m_driver_version: " << m_driver_version << l.endl;
        l << "m_protocol: " << m_protocol << l.endl;
        l--;
    }

    const char* MidiEndpointInfo::kindName() noexcept {
        switch (m_kind) {
            case EndpointKind::Input: return "Input";
            case EndpointKind::Output: return "Output";
            default: return "Unknown";
        }
    }

#if defined(__APPLE__) && defined(__MACH__)
    void MidiEndpointInfo::fill(MIDIEndpointRef endpoint, EndpointKind kind) noexcept {
        m_kind = kind;

        // Basic string & integer properties
        Midi::_acm_stringProperty(endpoint, kMIDIPropertyName, m_name);
        Midi::_acm_stringProperty(endpoint, kMIDIPropertyDisplayName, m_display_name);
        Midi::_acm_stringProperty(endpoint, kMIDIPropertyManufacturer, m_manufacturer);
        Midi::_acm_stringProperty(endpoint, kMIDIPropertyModel, m_model);
        Midi::_acm_stringProperty(endpoint, kMIDIPropertyDriverOwner, m_driver_owner);

        Midi::_acm_integerProperty(endpoint, kMIDIPropertyUniqueID, m_uid);
        Midi::_acm_integerProperty(endpoint, kMIDIPropertyDriverVersion, m_driver_version);
        Midi::_acm_integerProperty(endpoint, kMIDIPropertyProtocolID, m_protocol);

        Midi::_acm_booleanProperty(endpoint, kMIDIPropertyOffline, m_offline);

        // Get the device for this endpoint
        MIDIEntityRef entity;
        OSStatus status = MIDIEndpointGetEntity(endpoint, &entity);
        if (status == noErr && entity != 0) {
            MIDIDeviceRef device;
            status = MIDIEntityGetDevice(entity, &device);
            if (status == noErr && device != 0) {
                // device_id = the device's UID
                Midi::_acm_integerProperty(device, kMIDIPropertyUniqueID, m_device_id);
            }
        }
    }
#endif

    MidiIn::MidiIn(MidiClient* client, const char* port_name, MidiInCallbackFunc callback_func) {
        m_client = client->m_client;
        CFStringRef cfPortName = CFStringCreateWithCString(NULL, port_name, kCFStringEncodingUTF8);
        OSStatus status = MIDIInputPortCreate(
                m_client,
                cfPortName,
                _readProc,      // static callback
                this,           // refCon = this MidiIn instance
                &m_port
        );
        CFRelease(cfPortName);

        if (status != noErr) {
            m_port = 0;
            // handle error (log or throw)
        }

        m_callback = callback_func;
    }

    MidiIn::~MidiIn() {
        if (m_port) {
            MIDIPortDispose(m_port);
        }
    }


    bool MidiIn::openByUid(int32_t uid, MidiInCallbackFunc callback_func) noexcept {
        ItemCount count = MIDIGetNumberOfSources();
        for (ItemCount i = 0; i < count; ++i) {
            MIDIEndpointRef endpoint = MIDIGetSource(i);
            if (endpoint == 0) continue;

            SInt32 endpoint_uid = 0;
            OSStatus status = MIDIObjectGetIntegerProperty(endpoint, kMIDIPropertyUniqueID, &endpoint_uid);
            if (status != noErr) continue;

            if (endpoint_uid == uid) {
                Midi::_acm_endpointName(endpoint, m_name);

                m_endpoint = endpoint;
                m_callback = callback_func;

                // Attach this endpoint to our input port
                status = MIDIPortConnectSource(m_port, m_endpoint, this);
                if (status != noErr) {
                    std::cerr << "MIDIPortConnectSource failed: " << status << std::endl;
                    return false;
                }
                return true;
            }
        }
        return false; // not found
    }


    void MidiIn::_readProc(const MIDIPacketList* pkt_list, void* read_proc_ref_con, void* src_conn_ref_con) {
        auto midi_in = static_cast<MidiIn*>(read_proc_ref_con);
        if (!midi_in || !midi_in->m_callback) return;

        const MIDIPacket* packet = &pkt_list->packet[0];
        for (int i = 0; i < pkt_list->numPackets; ++i) {
            midi_in->m_callback(midi_in, packet->data, packet->length);
            packet = MIDIPacketNext(packet);
        }
    }


    MidiOut::MidiOut(MidiClient* client, const char* port_name) {

    }

    MidiOut::~MidiOut() {
        if (m_port) {
            MIDIPortDispose(m_port);
        }
    }


#if defined(__APPLE__) && defined(__MACH__)
    bool MidiOut::openByUid(int32_t uid) noexcept {
        // Walk through all MIDI destinations
        ItemCount count = MIDIGetNumberOfDestinations();
        for (ItemCount i = 0; i < count; ++i) {
            MIDIEndpointRef endpoint = MIDIGetDestination(i);
            if (endpoint == 0) continue;

            SInt32 endpoint_uid = 0;
            OSStatus status = MIDIObjectGetIntegerProperty(endpoint, kMIDIPropertyUniqueID, &endpoint_uid);
            if (status != noErr) continue;

            if (endpoint_uid == uid) {
                m_endpoint = endpoint; // store the endpoint in your MidiOut class
                return true;
            }
        }
        return false; // not found
    }
#endif

    MidiIn* MidiClient::createIn(const char* port_name, int32_t uid, MidiInCallbackFunc callback_func) {
        std::cout << "MidiClient::createIn(): " << port_name << ", uid: " << uid << std::endl;
        auto in = new (std::nothrow) MidiIn(this, port_name, callback_func);
        std::cout << "in: " << (long)in << std::endl;
        if (!in) return nullptr;
        std::cout << "openByUid()" << std::endl;
        if (!in->openByUid(uid, callback_func)) {
            std::cout << "failed!" << std::endl;
            delete in;
            return nullptr;
        }
        std::cout << "ok" << std::endl;
        return in;
    }


    MidiOut* MidiClient::createOut(const char* port_name, int32_t uid) {
        std::cout << "MidiClient::createOut(): " << port_name << ", uid: " << uid << std::endl;
        auto out = new (std::nothrow) MidiOut(this, port_name);
        if (out) {
            std::cout << "openByUid(): " << uid << std::endl;
            auto ok = out->openByUid(uid);
            std::cout << "ok:" << ok << std::endl;
            out->m_port = m_out_port;
        }
        return out;
    }


    int32_t Midi::sourceCount() noexcept {
        return (int32_t)MIDIGetNumberOfSources();
    }


    int32_t Midi::destinationCount() noexcept {
        return (int32_t)MIDIGetNumberOfDestinations();
    }


    int32_t Midi::listSources(Log& l) noexcept {
        int32_t count = sourceCount();
        l << "MIDI Sources (" << count << "):" << l.endl;

        for (int32_t i = 0; i < count; ++i) {
            MIDIEndpointRef endpoint = MIDIGetSource(i);
            if (endpoint == 0)
                continue;

            MidiEndpointInfo info;
            info.fill(endpoint, MidiEndpointInfo::EndpointKind::Input);

            l << "Source #" << i << ":" << l.endl;
            l++ ;
            info.log(l);
            l-- ;
        }
        return count;
    }


    int32_t Midi::listDestinations(Log& l) noexcept {
        int32_t count = destinationCount();
        l << "MIDI Destinations (" << count << "):" << l.endl;

        for (int32_t i = 0; i < count; ++i) {
            MIDIEndpointRef endpoint = MIDIGetDestination(i);
            if (endpoint == 0)
                continue;

            MidiEndpointInfo info;
            info.fill(endpoint, MidiEndpointInfo::EndpointKind::Output);

            l << "Destination #" << i << ":" << l.endl;
            l++ ;
            info.log(l);
            l-- ;
        }
        return count;
    }


    /**
     *  @brief Retrieves a string property from a CoreMIDI object.
     *
     *  @param obj The CoreMIDI object reference.
     *  @param property The CoreMIDI string property to fetch (e.g., kMIDIPropertyName).
     *  @param out_value A reference to the String getting the value.
     *  @return `true` if successful, `false` otherwise.
     */
    bool Midi::_acm_stringProperty(MIDIObjectRef obj, CFStringRef property, String& out_value) noexcept {
        CFStringRef cf_string = nullptr;
        if (MIDIObjectGetStringProperty(obj, property, &cf_string) == noErr && cf_string != nullptr) {
            char buffer[128];
            CFStringGetCString(cf_string, buffer, sizeof(buffer), kCFStringEncodingUTF8);
            CFRelease(cf_string);
            out_value = buffer;
            return true;
        }
        else {
            out_value = "Unknown";
            return false;
        }
    }


    /**
     *  @brief Retrieves an integer property from a CoreMIDI object.
     *
     *  @param obj The CoreMIDI object reference.
     *  @param property The CoreMIDI string property to fetch (e.g., kMIDIPropertyUniqueID).
     *  @param out_value A reference to the variable getting the value.
     *  @return `true` if successful, `false` otherwise.
     */
    bool Midi::_acm_integerProperty(MIDIObjectRef obj, CFStringRef property, int32_t& out_value) noexcept {
        if (MIDIObjectGetIntegerProperty(obj, property, &out_value) == noErr) {
            return true;
        }
        else {
            out_value = INT32_MIN;
            return false;
        }
    }


    /**
     *  @brief Retrieves a boolean property from a CoreMIDI object.
     *
     *  @param obj The CoreMIDI object reference.
     *  @param property The CoreMIDI string property to fetch (e.g., kMIDIPropertyOffline).
     *  @param out_value A reference to the variable getting the value.
     *  @return `true` if successful, `false` otherwise.
     */
    bool Midi::_acm_booleanProperty(MIDIObjectRef obj, CFStringRef property, bool& out_value) noexcept {
        int32_t value;
        bool result = _acm_integerProperty(obj, property, value);
        out_value = value != 0;
        return result;
    }


    bool Midi::_acm_endpointName(MIDIEndpointRef endpoint, String& out_name) noexcept {
        if (!endpoint) {
            return false;
        }
        CFStringRef str;
        SInt32 intVal;
        if (MIDIObjectGetStringProperty(endpoint, kMIDIPropertyName, &str) == noErr) {
            char buf[256];
            if (CFStringGetCString(str, buf, sizeof(buf), kCFStringEncodingUTF8)) {
                out_name = buf;
            }
            CFRelease(str);
            return true;
        }
        return false;
    }

} // End of namespace Grain

#endif
