//
//  Midi.hpp
//
//  Created by Roald Christesen on 02.09.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 02.09.2025

#ifndef GrainMidi_hpp
#define GrainMidi_hpp

#include "Type/Object.hpp"
#include "Type/List.hpp"
#include "Core/Log.hpp"
#include "String/String.hpp"

#if defined(__APPLE__) && defined(__MACH__)
#import <CoreMIDI/CoreMIDI.h>
#include <mach/mach_time.h>
#endif


namespace Grain {

    class MidiClient;
    class MidiIn;
    class MidiOut;


    typedef void (*MidiInCallbackFunc)(MidiIn* midi_in, const uint8_t* data, size_t length);

    /**
     *  @brief The MidiEndpointInfo structure encapsulates detailed information about
     *         a MIDI source or destination.
     */
    class MidiEndpointInfo : public Object {
    public:
        enum class EndpointKind {
            Unknown,
            Input,   ///< A MIDI source (app receives data)
            Output   ///< A MIDI destination (app sends data)
        };

    protected:
        EndpointKind m_kind;        ///< Whether this is an input, output, or unknown.
        String m_name;              ///< The name of the MIDI source.
        String m_display_name;      ///< A user-readable name for display purposes.
        String m_manufacturer;      ///< The manufacturer of the device.
        String m_model;             ///< The model name of the device.
        int32_t m_uid;              ///< A unique identifier for the source.
        int32_t m_device_id;        ///< The identifier for the device that owns this source.
        bool m_offline;             ///< A boolean indicating whether the source is currently offline.
        String m_driver_owner;      ///< The name of the driver managing the device.
        int32_t m_driver_version;   ///< The version of the driver managing the device.
        int32_t m_protocol;         ///< Indicates the MIDI protocol version (e.g., MIDI 1.0 or MIDI 2.0).

    public:
        void log(Log& l);
        const char* kindName() noexcept;

#if defined(__APPLE__) && defined(__MACH__)
        void fill(MIDIEndpointRef endpoint, EndpointKind kind) noexcept;
#endif
    };

    class MidiIn : public Object {
        friend class MidiClient;

    protected:
        String m_name;
#if defined(__APPLE__) && defined(__MACH__)
        MIDIEndpointRef m_endpoint{0};
        MIDIClientRef m_client{0};
        MIDIPortRef m_port{0};
#endif
        MidiInCallbackFunc m_callback;

    public:
        MidiIn(MidiClient* client, const char* port_name, MidiInCallbackFunc callback_func);
        ~MidiIn();

        bool openByUid(int32_t m_uid, MidiInCallbackFunc callback_func) noexcept;
        void close() noexcept;
        const char* name() const noexcept { return m_name.utf8(); }

#if defined(__APPLE__) && defined(__MACH__)
        bool isOpen() const noexcept { return m_endpoint != 0; }
        static void _readProc(const MIDIPacketList* pkt_list, void* read_proc_ref_con, void* src_conn_ref_con);
#endif
    };


    class MidiOut : public Object {
        friend class MidiClient;

    protected:
#if defined(__APPLE__) && defined(__MACH__)
        MIDIEndpointRef m_endpoint{0};
        MIDIClientRef m_client{0};
        MIDIPortRef m_port{0};
#endif

    public:
        MidiOut(MidiClient* client, const char* port_name);
        ~MidiOut();

        bool openByUid(int32_t m_uid) noexcept;
        void close() noexcept;

#if defined(__APPLE__) && defined(__MACH__)
        bool isOpen() const noexcept { return m_endpoint != 0; }
#endif

#if defined(__APPLE__) && defined(__MACH__)
        bool send(const uint8_t* bytes, int32_t length) noexcept {
            if (!m_endpoint || !m_port) {
                std::cerr << "Cannot send MIDI: endpoint or port is invalid\n";
                return false;
            }

            MIDIPacketList packetList;
            MIDIPacket* packet = MIDIPacketListInit(&packetList);
            packet = MIDIPacketListAdd(&packetList, sizeof(packetList), packet, 0, length, bytes);

            OSStatus status = MIDISend(m_port, m_endpoint, &packetList);
            if (status != noErr) {
                std::cerr << "MIDISend failed with status: " << status << "\n";
                return false;
            }
            return true;
        }

        void sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) noexcept {
            uint8_t packetData[3] = { static_cast<uint8_t>(0x90 | (channel & 0x0F)), note, velocity };
            MIDIPacketList packetList;
            MIDIPacket* packet = MIDIPacketListInit(&packetList);
            packet = MIDIPacketListAdd(&packetList, sizeof(packetList), packet, 0, 3, packetData);
            MIDISend(m_port, m_endpoint, &packetList);
        }

        void sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) noexcept {
            uint8_t packetData[3] = { static_cast<uint8_t>(0x80 | (channel & 0x0F)), note, velocity };
            MIDIPacketList packetList;
            MIDIPacket* packet = MIDIPacketListInit(&packetList);
            packet = MIDIPacketListAdd(&packetList, sizeof(packetList), packet, 0, 3, packetData);
            MIDISend(m_port, m_endpoint, &packetList);
        }

        void sendBytesAtAbsoluteTime(const uint8_t* bytes, size_t length, MIDITimeStamp timestamp) noexcept {
            if (!m_port || !m_endpoint || !bytes || length == 0) return;
            MIDIPacketList packetList;
            MIDIPacket* packet = MIDIPacketListInit(&packetList);
            packet = MIDIPacketListAdd(
                    &packetList,
                    sizeof(packetList),
                    packet,
                    timestamp,
                    static_cast<UInt16>(length),
                    bytes);
            if (packet) {
                MIDISend(m_port, m_endpoint, &packetList);
            }
        }
#endif

        void sendNoteOnAtAbsoluteTime(uint8_t channel, uint8_t note, uint8_t velocity, MIDITimeStamp timestamp) noexcept {
            uint8_t data[3] = { static_cast<uint8_t>(0x90 | (channel & 0x0F)), note, velocity };
            sendBytesAtAbsoluteTime(data, 3, timestamp);
        }

        void sendNoteOffAtAbsoluteTime(uint8_t channel, uint8_t note, uint8_t velocity, MIDITimeStamp timestamp) noexcept {
            uint8_t data[3] = { static_cast<uint8_t>(0x80 | (channel & 0x0F)), note, velocity };
            sendBytesAtAbsoluteTime(data, 3, timestamp);
        }
    };


    class MidiClient : public Object {
        friend class MidiIn;
        friend class MidiOut;

    protected:
#if defined(__APPLE__) && defined(__MACH__)
        MIDIClientRef m_client{0};
        MIDIPortRef m_out_port{0};      // Shared output port for all MidiOut objects
#endif

    public:
        MidiClient(const char* name = "GrainMidiClient") {
#if defined(__APPLE__) && defined(__MACH__)
            // Create the CoreMIDI client
            MIDIClientCreate(
                    CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8),
                    nullptr, nullptr, &m_client);
            // Create ONE output port for sending MIDI
            MIDIOutputPortCreate(
                    m_client,
                    CFStringCreateWithCString(NULL, "GrainMidiOutputPort", kCFStringEncodingUTF8),
                    &m_out_port);
#endif
        }

        ~MidiClient() {
#if defined(__APPLE__) && defined(__MACH__)
            if (m_out_port) {
                MIDIPortDispose(m_out_port);
            }
            if (m_client) {
                MIDIClientDispose(m_client);
            }
#endif
        }

#if defined(__APPLE__) && defined(__MACH__)
        MIDIClientRef _acm_client() const noexcept { return m_client; }
        MIDIPortRef _acm_output_port() const noexcept { return m_out_port; }

#endif

        // Factory methods
        MidiIn* createIn(const char* port_name, int32_t uid, MidiInCallbackFunc callback_func);
        MidiOut* createOut(const char* port_name, int32_t uid);
    };


    class Midi : public Object {
    public:
        /**
         *  @brief Get the number of all available MIDI sources on the system.
         */
        static int32_t sourceCount() noexcept;
        static int32_t listSources(Log& l) noexcept;

        /**
         *  @brief Get the number of all available MIDI destinations on the system.
         */
        static int32_t destinationCount() noexcept;
        static int32_t listDestinations(Log& l) noexcept;

#if defined(__APPLE__) && defined(__MACH__)
        static bool _acm_stringProperty(MIDIObjectRef obj, CFStringRef property, String& out_value) noexcept;
        static bool _acm_integerProperty(MIDIObjectRef obj, CFStringRef property, int32_t& out_value) noexcept;
        static bool _acm_booleanProperty(MIDIObjectRef obj, CFStringRef property, bool& out_value) noexcept;
        static bool _acm_endpointName(MIDIEndpointRef endpoint, String& out_name) noexcept;
#endif

        /**
         * Helper to get current mach absolute time
         */
        static uint64_t now() noexcept {
#if defined(__APPLE__) && defined(__MACH__)
            return mach_absolute_time();
#else
            return 0;
#endif
        }

        static uint64_t secToTicks(double seconds) {
#if defined(__APPLE__) && defined(__MACH__)
            mach_timebase_info_data_t timebase;
            mach_timebase_info(&timebase); // get conversion info
            uint64_t ticks = mach_absolute_time(); // current time in ticks
            uint64_t ns = static_cast<uint64_t>(seconds * 1e9); // desired delay in nanoseconds
            return ns * timebase.denom / timebase.numer;
#else
            return 1000000000;
#endif
        }
    };

} // End of namespace Grain

#endif // GrainMidi_hpp