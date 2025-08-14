//
//  Hardware.hpp
//
//  Created by Roald Christesen on from 25.08.2012
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 12.07.2025
//

#ifndef GrainHardware_hpp
#define GrainHardware_hpp

#include "Grain.hpp"
#include "Type/List.hpp"
#include "Log.hpp"

#if defined(__APPLE__) && defined(__MACH__)
    #include <SystemConfiguration/SystemConfiguration.h>
#endif


namespace Grain {

    /**
     *  @class NetworkInterfaceInfo
     *  @brief Stores information about a network interface, including its name,
     *         IP address, and MAC address.
     *
     *  This class provides functionality to log and clear interface details,
     *  and to access raw pointers to the stored IP and MAC addresses.
     */
    class NetworkInterfaceInfo : Object {
    private:
        char* m_name = nullptr;     ///< Name of the network interface
        uint8_t m_ip_addr[4]{};     ///< IPv4 address of the network interface
        uint8_t m_mac_addr[6]{};    ///< MAC address of the network interface

    public:
        const char* className() const noexcept override { return "NetworkInterfaceInfo"; }
        void log(Log& l) const;

        NetworkInterfaceInfo(const char* name) noexcept {
            m_name = name ? strdup(name) : nullptr;
            clearIpAddr();
            clearMacAddr();
        }

        ~NetworkInterfaceInfo() {
            std::free(m_name);
        }

        [[nodiscard]] const uint8_t* ipAddrPtr() const noexcept { return m_ip_addr; }
        [[nodiscard]] const uint8_t* macAddrPtr() const noexcept { return m_mac_addr; }

        void clearIpAddr() noexcept { for (int32_t i = 0; i < 4; i++) m_ip_addr[i] = 0; }
        void clearMacAddr() noexcept { for (int32_t i = 0; i < 6; i++) m_mac_addr[i] = 0; }
    };


    /**
     *  @class NetworkInterfaceList
     *  @brief Manages a list of network interface information objects.
     *
     *  Inherits from ObjectList and provides functionality to update the list,
     *  log its contents, and access individual interface information objects.
     */
    class NetworkInterfaceList : public ObjectList<NetworkInterfaceInfo*> {
    public:
        enum class Status {
            Undefined = 0,
            UpdateFailed,
            OK
        };

    protected:
        Status m_status = Status::Undefined;

    public:
        const char* className() const noexcept override { return "NetworkInterfaceList"; }
        void log(Log& l) const;

        void update() noexcept;
        [[nodiscard]] const NetworkInterfaceInfo* interfaceInfoAtIndex(int32_t index) noexcept;
    };


    class Hardware {
    public:
        static bool interfaceIPAddr(const char* interface_name, uint8_t* out_ip_addr) noexcept;
        static bool interfaceMacAddr(const char* interface_name, uint8_t* out_mac_addr) noexcept;

        static bool kernelInt32(const char* name, int32_t& out_value) noexcept;
        static bool kernelInt64(const char* name, int64_t& out_value) noexcept;

        static int32_t kernelString(const char* name, char* out_buffer, int32_t out_buffer_size) noexcept;

        [[nodiscard]] static int32_t physicalCores() noexcept {
            int32_t value;
            return kernelInt32("hw.physicalcpu", value) ? value : -1;
        }

        [[nodiscard]] static int32_t logicalCores() noexcept {
            int32_t value;
            return kernelInt32("hw.logicalcpu", value) ? value : -1;
        }
        static int32_t machine(char* out_buffer, int32_t out_buffer_size) noexcept {
            return kernelString("hw.machine", out_buffer, out_buffer_size);
        }
        static int32_t model(char* out_buffer, int32_t out_buffer_size) noexcept {
            return kernelString("hw.model", out_buffer, out_buffer_size);
        }
        [[nodiscard]] static int32_t byteOrder() noexcept {
            int32_t value;
            return kernelInt32("hw.byteorder", value) ? value : -1;
        }
        [[nodiscard]] static int64_t memSize() noexcept {
            int64_t value;
            return kernelInt64("hw.memsize", value) ? value : -1;
        }
        static int32_t osType(char* out_buffer, int32_t out_buffer_size) noexcept {
            return kernelString("kern.ostype", out_buffer, out_buffer_size);
        }
        static int32_t osRelease(char* out_buffer, int32_t out_buffer_size) noexcept {
            return kernelString("kern.osrelease", out_buffer, out_buffer_size);
        }
        [[nodiscard]] static int32_t osRevision() noexcept {
            int32_t value;
            return kernelInt32("kern.osrevision", value) ? value : -1;
        }
        static int32_t kernelVersion(char* out_buffer, int32_t out_buffer_size) noexcept {
            return kernelString("kern.version", out_buffer, out_buffer_size);
        }
        static  int32_t kernelHostName(char* out_buffer, int32_t out_buffer_size) noexcept {
            return kernelString("kern.hostname", out_buffer, out_buffer_size);
        }
        [[nodiscard]] static int32_t kernelHostId() noexcept {
            int32_t value;
            return kernelInt32("kern.hostid", value) ? value : -1;
        }

        [[nodiscard]] static double loadAverage(int32_t index) noexcept;
        [[nodiscard]] static double cpuUsage() noexcept;
        static bool reportMemorySize(size_t* out_virtual_size, size_t* out_max_size = nullptr) noexcept;
    };


} // End of namespace Grain

#endif // GrainHardware_hpp
