//
//  Data.cpp
//
//  Created by Roald Christesen on 27.10.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
#include "Type/Data.hpp"
#include "File/File.hpp"
#include "zlib.h"

namespace Grain {

    uint8_t Base64Data::code2bits(const unsigned char code) {

        if (code >= 'A' && code <= 'Z') { return code - 'A'; }
        if (code >= 'a' && code <= 'z') { return code - 'a' + 0x1a; }
        if (code >= '0' && code <= '9') { return code - '0' + 0x34; }
        if (code == '+') { return 0x3e; }
        if (code == '/') { return 0x3f; }
        if (code == '=') { return 0x0; }

        return 0x80;  // Invalid code
    }


    uint8_t Base64Data::bits2code(const unsigned char bits) {

        if (bits < 0x1a) { return bits + 'A'; }
        if (bits < 0x34) { return (bits - 0x1a) + 'a'; }
        if (bits < 0x3e) { return (bits - 0x34) + '0'; }
        if (bits == 0x3e) { return '+'; }
        if (bits == 0x3f) { return '/'; }

        return 0;  // Invalid bits
    }


    bool Base64Data::_bits2code() {

        for (int32_t i = 0; i < 4; i++) {
            m_codes[i] = bits2code(m_bits[i]);
            if (m_codes[i] == 0) {
                return false;
            }
        }

        return true;
    }


    bool Base64Data::encodeByte(uint8_t byte) {

        switch (m_byte_index) {
            case 0:
                m_bits[0] = byte >> 2;
                m_bits[1] = (byte & 0x3) << 4;
                m_byte_index = 1;
                break;

            case 1:
                m_bits[1] |= byte >> 4;
                m_bits[2] = (byte & 0xf) << 2;
                m_byte_index = 2;
                break;

            case 2:
                m_bits[2] |= byte >> 6;
                m_bits[3] = byte & 0x3f;
                m_byte_index = 0;
                _bits2code();
                return true;
        }

        return false;
    }


    bool Base64Data::encodeFinalize() {

        int32_t n = 0;
        switch (m_byte_index) {
            case 0: break;
            case 1: n = 2; break;
            case 2: n = 3; break;
        }

        for (int32_t i = 0; i < n; i++) {
            m_codes[i] = bits2code(m_bits[i]);
            if (m_codes[i] == 0) {
                return false;
            }
        }
        for (int32_t i = n; i < 4; i++) {
            m_codes[i] = '=';
        }

        return true;
    }


/**
 *  @brief Decode a block of 4 Base64 encoded characters.
 *
 *  This function decodes a block of 4 Base64 encoded characters into up to 3
 *  bytes of data. It handles padding characters (`=`) by reducing the byte
 *  count accordingly.
 *
 *  @param codes Pointer to a buffer with 4 encoded characters. The buffer must
 *               be at least 4 bytes long.
 *  @param[out] out_bytes Pointer to a buffer for the resulting bytes.
 *                        The buffer must be at least 3 bytes long.
 *
 *  @return Number of valid bytes in `out_bytes`, which can be 1, 2, or 3 bytes.
 *          Returns -1 if `codes` is `nullptr` and -2 if `out_bytes` is `nullptr`.
 */
    int32_t Base64Data::decodeBlock(const char *codes, uint8_t *out_bytes) {

        int32_t byte_count = 3;  // Assume 3 bytes by default

        // Check for null pointers.
        if (!codes) {
            return -1;
        }
        if (!out_bytes) {
            return -2;
        }

        // Decode 4 Base64 characters into up to 3 bytes
        for (int32_t i = 0; i < 4; i++) {

            auto c = codes[i];
            auto bits = Base64Data::code2bits(c);  // Map Base64 char to 6-bit value

            switch (i) {
                case 0:
                    out_bytes[0] = bits << 2;  // First 6 bits of the 1st byte
                    break;

                case 1:
                    out_bytes[0] |= bits >> 4;  // Last 2 bits of the 1st byte
                    out_bytes[1] = bits << 4;  // First 4 bits of the 2nd byte
                    break;

                case 2:
                    if (codes[2] != '=') { // Handle padding.
                        out_bytes[1] |= bits >> 2;  // Last 4 bits of the 2nd byte
                        out_bytes[2] = bits << 6;  // First 2 bits of the 3rd byte
                    }
                    else {
                        byte_count--;  // One padding '=' reduces byte count
                    }
                    break;

                case 3:
                    if (codes[3] != '=') {      // Handle padding
                        out_bytes[2] |= bits;   // Remaining 6 bits of the 3rd byte
                    }
                    else {
                        byte_count--;  // Two padding '==' reduce byte count further
                    }
                    break;
            }
        }

        return byte_count;  // Return number of valid bytes
    }


    int32_t Base64Data::_scanBase64Code(uint8_t code) {

        if (code == '=') {
            if (m_base64_size < 1) {
                m_last_err = kErr_PaddingMismatch;
                return m_last_err;
            }

            if (m_padding >= 2) {
                m_last_err = kErr_PaddingMoreThanTwo;
                return m_last_err;
            }

            m_padding++;
            m_base64_size++;
            return kErr_None;
        }

        if (String::charIsWhiteSpace(code)) {
            return kErr_None;
        }

        if (!String::isBase64(code)) {
            m_last_err = kErr_NoBase64Code;
            return m_last_err;
        }

        if (m_padding > 0) {
            m_last_err = kErr_PostPaddingData;
            return m_last_err;
        }

        m_base64_size++;
        return kErr_None;
    }


} // End of namespace Grain
