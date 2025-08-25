//
//  WKBParser.cpp
//
//  Created by Roald Christesen on from 26.02.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 24.08.2025
//

#include "Geo/WKBParser.hpp"


namespace Grain {

    void WKBParser::setBinaryData(const uint8_t* data, int32_t size) noexcept {
        m_binary_mode = true;
        m_binary_data = data;
        m_binary_ptr = data;
        m_binary_size = size;

        if (m_binary_data) {
            if (*m_binary_ptr++ == 1) {
                m_little_endian = true;
            }
            m_type = (WKBType)readInt();
        }
    }


    void WKBParser::setTextData(const char* data, int32_t size) noexcept {
        m_text_data = data;
        m_read_pos = 0;

        if (m_text_data) {
            m_text_length = (int32_t)strlen(data);

            if (m_text_data[0] == '\\' && m_text_data[1] == 'x') {
                m_read_pos += 2;
            }

            if (m_text_data[m_read_pos + 1] == '1') {
                m_little_endian = true;
            }

            m_read_pos += 2;
        }

        m_type = (WKBType)readInt();
    }


    uint8_t WKBParser::readNibble() {
        if (m_read_pos >= m_text_length) {
            throw Grain::ErrorCode::IndexOutOfRange;
        }

        char c = m_text_data[m_read_pos++];

        if (c >= '0' && c <= '9') {
            return (uint8_t)(c - '0');
        }
        else if (c >= 'a' && c <= 'f') {
            return (uint8_t)(c - 'a' + 10);
        }
        else if (c >= 'A' && c <= 'F') {
            return (uint8_t)(c - 'A' + 10);
        }
        else {
            throw Grain::ErrorCode::FormatMismatch;
        }
    }

    uint8_t WKBParser::readByte() {
        if (m_binary_mode) {
            return* m_binary_ptr++;
        }
        else {
            uint8_t result = readNibble() << 4;
            result |= readNibble();
            return result;
        }
    }


    uint32_t WKBParser::readInt() {
        uint32_t result;

        if (m_binary_mode) {
            if (m_little_endian) {
                result = m_binary_ptr[3]; result <<= 8;
                result |= m_binary_ptr[2]; result <<= 8;
                result |= m_binary_ptr[1]; result <<= 8;
                result |= m_binary_ptr[0];
            }
            else {
                result = m_binary_ptr[0]; result <<= 8;
                result |= m_binary_ptr[1]; result <<= 8;
                result |= m_binary_ptr[2]; result <<= 8;
                result |= m_binary_ptr[3];
            }
            m_binary_ptr += 4;
        }
        else {
            uint8_t* p = (uint8_t*)&result;
            if (m_little_endian == true) {
                for (int i = 0; i < 4; i++) {
                    p[i] = readByte();
                }
            }
            else {
                for (int i = 0; i < 4; i++) {
                    p[3 - i] = readByte();
                }
            }
        }

        return result;
    }


    double WKBParser::readDouble() {
        double result;
        uint8_t* p = (uint8_t*)&result;

        if (m_binary_mode) {
            if (m_little_endian) {
                uint64_t* temp = (uint64_t*)&result;
                *temp = m_binary_ptr[7]; *temp <<= 8;
                *temp |= m_binary_ptr[6]; *temp <<= 8;
                *temp |= m_binary_ptr[5]; *temp <<= 8;
                *temp |= m_binary_ptr[4]; *temp <<= 8;
                *temp |= m_binary_ptr[3]; *temp <<= 8;
                *temp |= m_binary_ptr[2]; *temp <<= 8;
                *temp |= m_binary_ptr[1]; *temp <<= 8;
                *temp |= m_binary_ptr[0];
            }
            else {
                uint64_t* temp = (uint64_t*)&result;
                *temp = m_binary_ptr[0]; *temp <<= 8;
                *temp |= m_binary_ptr[1]; *temp <<= 8;
                *temp |= m_binary_ptr[2]; *temp <<= 8;
                *temp |= m_binary_ptr[3]; *temp <<= 8;
                *temp |= m_binary_ptr[4]; *temp <<= 8;
                *temp |= m_binary_ptr[5]; *temp <<= 8;
                *temp |= m_binary_ptr[6]; *temp <<= 8;
                *temp |= m_binary_ptr[7];
            }
            m_binary_ptr += 8;
        }
        else {
            if (m_little_endian == true) {
                for (int i = 0; i < 8; i++) {
                    p[i] = readByte();
                }
            }
            else {
                for (int i = 0; i < 8; i++) {
                    p[7 - i] = readByte();
                }
            }
        }

        return result;
    }


    void WKBParser::readVec2(Vec2d& out_vec) {
        out_vec.m_x = readDouble();
        out_vec.m_y = readDouble();
    }


    void WKBParser::skipBytes(uint32_t n) {
        m_binary_ptr += n;
    }


} // End of namespace Grain.
