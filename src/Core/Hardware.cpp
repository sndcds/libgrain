//
//  Hardware.cpp
//
//  Created by Roald Christesen on from 25.08.2012
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Core/Hardware.hpp"
#include "Type/Type.hpp"
#include "Type/ByteOrder.hpp"

#include <sys/socket.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <iostream>
#include <unistd.h>


#if defined(__APPLE__) || defined(__FreeBSD__)
    #include <mach/mach.h>
    #include <net/if_dl.h>
    #include <sys/sysctl.h>
#elif defined(__linux__)
    #include <unistd.h>
    #include <sys/sysinfo.h>
    #include <net/if.h>
    #include <sys/ioctl.h>
    #include <unistd.h>
#endif


namespace Grain {

    /**
     *  @brief Retrieves the IPv4 address of a given network interface.
     *
     *  This function queries the operating system to obtain the IP address assigned
     *  to a specified network interface and stores the result in the provided
     *  buffer.
     *
     *  @param interface_name Name of the network interface (e.g., "eth0").
     *  @param out_ip_addr Pointer to a buffer with space for 4 uint8_t values to
     *                     store the IP address.
     *  @return true if the IP address was successfully retrieved, false otherwise.
     *
     *  @note The output IP address is in host byte order (not network byte order).
     *  @note The caller must ensure that `out_ip_addr` points to a buffer of at
     *        least 4 bytes.
     *  @note If the interface name is invalid or the socket operation fails, the
     *        function returns false.
     */
    bool Hardware::interfaceIPAddr(const char* interface_name, uint8_t* out_ip_addr) noexcept {

        // `out_ip_addr` must be a buffer with space for 4 uint8_t values

        if (out_ip_addr == nullptr) {
            return false;
        }

        Type::clearArray<uint8_t>(out_ip_addr, 4);

        if (interface_name == nullptr) {
            return false;
        }

        if (strlen(interface_name) > IFNAMSIZ) {
            return false;
        }


        int sock;
        uint32_t ip;
        struct ifreq ifr;

        // Determine UDN according to MAC address
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            return false;
        }

        strcpy(ifr.ifr_name, interface_name);
        ifr.ifr_addr.sa_family = AF_INET;

        if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
            close(sock);
            return false;
        }

        ip = ((struct sockaddr_in*) &ifr.ifr_addr)->sin_addr.s_addr;
        ip = ntoh32(ip);

        out_ip_addr [0] = (ip >> 24) & 0xFF;
        out_ip_addr [1] = (ip >> 16) & 0xFF;
        out_ip_addr [2] = (ip >> 8) & 0xFF;
        out_ip_addr [3] = ip & 0xFF;

        close(sock);

        return true;
    }


    /**
     *  @brief Retrieves the MAC address of a specified network interface.
     *
     *  This function uses the system's routing interface (via `sysctl`) to extract
     *  the MAC address associated with a given network interface. The result is stored
     *  in the provided output buffer.
     *
     *  @param interface_name Name of the network interface (e.g., "en0", "eth0").
     *  @param out_mac_addr Pointer to a buffer with space for 6 uint8_t values to
     *                      store the MAC address.
     *  @return true if the MAC address was successfully retrieved; false otherwise.
     *
     *  @note This function is specific to BSD-based systems (e.g., macOS) and uses
     *        system calls like `sysctl` and `if_nametoindex`.
     *  @note The `out_mac_addr` buffer must be preallocated with at least 6 bytes.
     *  @note On failure, no partial MAC address is returned — the buffer is cleared.
     */
#if defined(__APPLE__) && defined(__MACH__)
    bool Hardware::interfaceMacAddr(const char* interface_name, uint8_t* out_mac_addr) noexcept {

        // 'out_mac_addr' must be a buffer with space for 6 uint8_t values
        if (out_mac_addr == nullptr) {
            return false;
        }

        Type::clearArray<uint8_t>(out_mac_addr, 6);

        if (interface_name == nullptr) {
            return false;
        }

        int mgmt_info_base[6];
        char* msg_buffer = nullptr;
        size_t length;
        struct if_msghdr* interface_msg_struct;
        struct sockaddr_dl* socket_struct;
        int err = 0;

        // Setup the management Information Base (mib)
        mgmt_info_base[0] = CTL_NET;        // Request network subsystem
        mgmt_info_base[1] = AF_ROUTE;       // Routing table info
        mgmt_info_base[2] = 0;
        mgmt_info_base[3] = AF_LINK;        // Request link layer information
        mgmt_info_base[4] = NET_RT_IFLIST;  // Request all configured interfaces

        // With all configured interfaces requested, get handle index
        if ((mgmt_info_base[5] = (int)if_nametoindex(interface_name)) == 0) {
            err = 1;  // = @"if_nametoindex failure";
        }
        else {
            // Get the size of the data available (store in len)
            if (sysctl(mgmt_info_base, 6, nullptr, &length, nullptr, 0) < 0) {
                err = 2;  // errorFlag = @"sysctl mgmtInfoBase failure";
            }
            else {
                // Alloc memory based on above call
                if ((msg_buffer = (char*)std::malloc(length)) == nullptr) {
                    err = 3;  // errorFlag = @"buffer allocation failure";
                }
                else {
                    // Get system information, store in buffer
                    if (sysctl(mgmt_info_base, 6, msg_buffer, &length, nullptr, 0) < 0) {
                        err = 4;  // errorFlag = @"sysctl msgBuffer failure";
                    }
                }
            }
        }

        // Before going any further ...
        if (err != 0) {
            return false;
        }

        // Map msgbuffer to interface message structure
        interface_msg_struct = (struct if_msghdr*)msg_buffer;

        // Map to link-level socket structure
        socket_struct = (struct sockaddr_dl*)(interface_msg_struct + 1);

        // Copy link layer address data in socket structure to an array
        memcpy(out_mac_addr, socket_struct->sdl_data + socket_struct->sdl_nlen, 6);

        // Release the buffer memory
        std::free(msg_buffer);

        return true;
    }
#else
    bool Hardware::interfaceMacAddr(const char* interface_name, uint8_t* out_mac_addr) noexcept {
        // TODO: Implement linux version
        return false;
    }
#endif

    void NetworkInterfaceInfo::log(Log& l) const {

        l.header(className());
        l << "m_name: " << m_name << Log::endl;
        l.label("m_ip_addr");
        l.ubyteDecimal((uint8_t*)m_ip_addr, 4, ':');
        l << Log::endl;
        l.label("m_mac_addr");
        l.ubyteDecimal((uint8_t*)m_mac_addr, 6, ':');
        l << Log::endl;
        l--;
    }


    void NetworkInterfaceList::log(Log& l) const {
        l.header(className());
        l << "network interfaces: " << size() << Log::endl;
        l++;
        for (auto& interface : *this) {
            interface->log(l);
        }
        l--;
        l--;
    }


    void NetworkInterfaceList::update() noexcept {

        /* !!!!! macOS Implement!
        SCDynamicStoreRef store_ref = NULL;

        try {
            clear();
            m_status = Status::Undefined;

            SCDynamicStoreRef store_ref = SCDynamicStoreCreate(NULL, (CFStringRef)@"FindCurrentInterfaceIpMac", NULL, NULL);
            CFPropertyListRef global = SCDynamicStoreCopyValue(store_ref, CFSTR("State:/Network/Interface"));
            NSDictionary* primary_interface = [(__bridge NSDictionary*)global valueForKey:@"Interfaces"];

            int32_t interface_count = (int32_t)[primary_interface count];
            if (interface_count > 0) {
                for (NSString* item in primary_interface) {
                    auto network_interface_info = new (std::nothrow) NetworkInterfaceInfo((const char*)[item UTF8String]);
                    Error::throwCheckInstantiation(network_interface_info);
                    Hardware::interfaceIPAddr((char*)[item UTF8String], (uint8_t*)network_interface_info->ipAddrPtr());
                    Hardware::interfaceMacAddr((char*)[item UTF8String], (uint8_t*)network_interface_info->macAddrPtr());
                    pushChangeOwner(network_interface_info);
                }

                m_status = Status::OK;
            }
        }
        catch (ErrorCode err) {
            m_status = Status::UpdateFailed;
        }

        // Cleanup
        if (store_ref != NULL) {
            CFRelease(store_ref);
        }
         */
    }


    const NetworkInterfaceInfo *NetworkInterfaceList::interfaceInfoAtIndex(int32_t index) noexcept {

        if (m_status == Status::OK) {
            return elementAtIndex(index);
        }
        else {
            return nullptr;
        }
    }


    /**
     *  @brief Retrieves a 32-bit integer value from the kernel using a sysctl name.
     *
     *  This function safely queries the kernel for a 32-bit integer using the
     *  `sysctlbyname` API. If the operation fails, the function returns false and
     *  does not modify the output.
     *
     *  @param name Null-terminated string representing the sysctl variable name
     *              (e.g., "hw.ncpu").
     *  @param out_value Reference to a variable where the result will be stored on
     *                   success.
     *  @return true if the value was successfully retrieved, false otherwise.
     */
#if defined(__APPLE__) && defined(__MACH__)
    bool Hardware::kernelInt32(const char* name, int32_t& out_value) noexcept {

        if (name == nullptr) {
            return false;
        }

        size_t len = sizeof(out_value);
        if (sysctlbyname(name, &out_value, &len, nullptr, 0) != 0 || len != sizeof(out_value)) {
            return false;
        }

        return true;
    }
#else
    bool Hardware::kernelInt32(const char* name, int32_t& out_value) noexcept {
        // TODO: Implement linux version
        return false;
    }
#endif

    /**
     * @brief Retrieves a 64-bit integer value from the kernel using a sysctl name.
     *
     * This version performs error checking on the sysctl call. If the call fails,
     * the function returns a fallback value and logs or handles the failure safely.
     *
     * @param name Null-terminated string representing the sysctl variable name
     *             (e.g., "hw.memsize").
     * @param out_value Reference to a variable where the retrieved value will be
     *                  stored on success.
     * @return true if the value was successfully retrieved, false otherwise.
     */
#if defined(__APPLE__) && defined(__MACH__)
    bool Hardware::kernelInt64(const char* name, int64_t& out_value) noexcept {

        if (name == nullptr) {
            return false;
        }

        size_t len = sizeof(out_value);
        if (sysctlbyname(name, &out_value, &len, nullptr, 0) != 0 || len != sizeof(out_value)) {
            return false;
        }

        return true;
    }
#else
    bool Hardware::kernelInt64(const char* name, int64_t& out_value) noexcept {
        // TODO: Implement linux version
        return false;
    }
#endif


    /**
     *  @brief Retrieves a kernel string value using a sysctl name.
     *
     *  Queries the kernel for a string value using `sysctlbyname` and copies the
     *  result into a user-provided buffer. The function allocates a temporary
     *  buffer of the required size and ensures safe copying into the provided
     *  output buffer.
     *
     *  @param name Null-terminated string representing the sysctl variable name
     *              (e.g., "kern.ostype").
     *  @param out_buffer Pointer to a character buffer where the result will be
     *                    stored. Must be preallocated and large enough to hold the
     *                    value.
     *  @param out_buffer_size Size (in bytes) of the output buffer.
     *  @return 0 on success, non-zero on error (e.g., null parameters or sysctl
     *          failure).
     */
#if defined(__APPLE__) && defined(__MACH__)
    int32_t Hardware::kernelString(const char* name, char* out_buffer, int32_t out_buffer_size) noexcept {

        if (name == nullptr || out_buffer == nullptr || out_buffer_size < 1) {
            return -1;
        }

        size_t size = 0;

        // First call to get the required size
        if (sysctlbyname(name, nullptr, &size, nullptr, 0) != 0 || size == 0) {
            return -2;
        }

        char* temp_buffer = static_cast<char*>(std::malloc(size));
        if (temp_buffer == nullptr) {
            return -3;
        }

        if (sysctlbyname(name, temp_buffer, &size, nullptr, 0) != 0) {
            std::free(temp_buffer);
            return -4;
        }

        // Ensure no overflow in the user-provided buffer
        strncpy(out_buffer, temp_buffer, out_buffer_size - 1);
        out_buffer[out_buffer_size - 1] = '\0';  // Ensure null-termination

        std::free(temp_buffer);

        return 0;
    }
#else
    int32_t Hardware::kernelString(const char* name, char* out_buffer, int32_t out_buffer_size) noexcept {
        // TODO: Implement linux version
        return false;
    }
#endif

    double Hardware::loadAverage(int32_t index) noexcept {

        double load[3];
        index = index < 0 ? 0 : index > 2 ? 2 : index;
        if (getloadavg(load, 3) != -1) {
            return load[index];
        }

        return -1.0f;
    }


#if defined(__APPLE__) && defined(__MACH__)
    double Hardware::cpuUsage() noexcept {

        kern_return_t kr;
        task_info_data_t tinfo;
        mach_msg_type_number_t task_info_count;

        task_info_count = TASK_INFO_MAX;
        kr = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)tinfo, &task_info_count);
        if (kr != KERN_SUCCESS) {
            return -1;
        }

        thread_array_t thread_list;
        mach_msg_type_number_t thread_count;

        thread_info_data_t thinfo;
        mach_msg_type_number_t thread_info_count;

        thread_basic_info_t basic_info_th;

        // auto basic_info = (task_basic_info_t)tinfo; // Unused

        // get threads in the task
        kr = task_threads(mach_task_self(), &thread_list, &thread_count);
        if (kr != KERN_SUCCESS) {
            return -1;
        }

        /* Unused
        uint32_t stat_thread = 0; // Mach threads, unused
        if (thread_count > 0) {
            stat_thread += thread_count;
        }
         */

        long tot_sec = 0;
        long tot_usec = 0;
        float tot_cpu = 0;
        int j;

        // For each thread
        for (j = 0; j < (int)thread_count; j++) {

            thread_info_count = THREAD_INFO_MAX;
            kr = thread_info(thread_list[j], THREAD_BASIC_INFO,
                             (thread_info_t)thinfo, &thread_info_count);
            if (kr != KERN_SUCCESS) {
                return -1;
            }

            basic_info_th = (thread_basic_info_t)thinfo;

            if (!(basic_info_th->flags & TH_FLAGS_IDLE)) {
                tot_sec = tot_sec + basic_info_th->user_time.seconds + basic_info_th->system_time.seconds;
                tot_usec = tot_usec + basic_info_th->system_time.microseconds + basic_info_th->system_time.microseconds;
                tot_cpu = tot_cpu + basic_info_th->cpu_usage / static_cast<float>(TH_USAGE_SCALE);
            }

        }

        kr = vm_deallocate(mach_task_self(), (vm_offset_t)thread_list, thread_count * sizeof(thread_t));

        return tot_cpu;
    }
#else
    double Hardware::cpuUsage() noexcept {
        // TODO: Implement linux version
        return 0.0;
    }
#endif


#if defined(__APPLE__) && defined(__MACH__)
    bool Hardware::reportMemorySize(size_t* out_virtual_size, size_t* out_max_size) noexcept {

        struct mach_task_basic_info info;
        mach_msg_type_number_t size = MACH_TASK_BASIC_INFO_COUNT;
        kern_return_t kerr = task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &size);

        if (kerr == KERN_SUCCESS) {
            if (out_virtual_size != nullptr) {
                *out_virtual_size = info.virtual_size;
            }

            if (out_max_size  != nullptr) {
                *out_max_size = info.resident_size_max;
            }

            return true;
        }

        return false;
    }
#else
    bool Hardware::reportMemorySize(size_t* out_virtual_size, size_t* out_max_size) noexcept {
        // TODO: Implement linux version
        return false;
    }
#endif

} // End of namespace Grain
