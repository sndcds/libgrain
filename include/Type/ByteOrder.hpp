//
//  NToH.hpp
//
//  Created by Roald Christesen on 20.08.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 20.08.2025
//

#ifndef GrainNToH_hpp
#define GrainNToH_hpp

#if defined(_WIN32) || defined(_WIN64)
    #include <winsock2.h>
    #define BYTE_ORDER_LITTLE_ENDIAN 1234
    #define BYTE_ORDER_BIG_ENDIAN    4321
    #define SYSTEM_BYTE_ORDER BYTE_ORDER_LITTLE_ENDIAN
#elif defined(__APPLE__)
    #include <arpa/inet.h>
    #include <libkern/OSByteOrder.h>
    #define BYTE_ORDER_LITTLE_ENDIAN 1234
    #define BYTE_ORDER_BIG_ENDIAN    4321
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define SYSTEM_BYTE_ORDER BYTE_ORDER_LITTLE_ENDIAN
    #else
        #define SYSTEM_BYTE_ORDER BYTE_ORDER_BIG_ENDIAN
    #endif
#else // Linux and other Unix
    #include <arpa/inet.h>
    #include <endian.h>
    #define BYTE_ORDER_LITTLE_ENDIAN __LITTLE_ENDIAN
    #define BYTE_ORDER_BIG_ENDIAN    __BIG_ENDIAN
    #define SYSTEM_BYTE_ORDER        __BYTE_ORDER
#endif


namespace Grain {

    // 16-bit
    inline uint16_t ntoh16(uint16_t x) { return ntohs(x); }

    inline uint16_t hton16(uint16_t x) { return htons(x); }

    // 32-bit
    inline uint32_t ntoh32(uint32_t x) { return ntohl(x); }

    inline uint32_t hton32(uint32_t x) { return htonl(x); }

    // 64-bit
    inline uint64_t ntoh64(uint64_t x) {
        #if defined(_WIN32) || defined(_WIN64)
            uint32_t high = ntohl(static_cast<uint32_t>(x >> 32));
            uint32_t low  = ntohl(static_cast<uint32_t>(x & 0xFFFFFFFFULL));
            return (static_cast<uint64_t>(low) << 32) | high;
        #elif defined(__APPLE__)
            return OSSwapBigToHostInt64(x);
        #elif SYSTEM_BYTE_ORDER == BYTE_ORDER_LITTLE_ENDIAN
            return ((uint64_t)ntohl(x & 0xFFFFFFFFULL) << 32) | ntohl(x >> 32);
        #else // big endian
            return x;
        #endif
    }

    inline uint64_t hton64(uint64_t x) {
        #if defined(_WIN32) || defined(_WIN64)
            return ntoh64(x);
        #elif defined(__APPLE__)
            return OSSwapHostToBigInt64(x);
        #elif SYSTEM_BYTE_ORDER == BYTE_ORDER_LITTLE_ENDIAN
            return ((uint64_t)htonl(x & 0xFFFFFFFFULL) << 32) | htonl(x >> 32);
        #else
            return x;
        #endif
    }
}

#endif