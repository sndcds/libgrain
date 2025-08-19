//
//  GrType.cpp
//
//  Created by Roald Christesen on 06.05.14.
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Type/Type.hpp"


namespace Grain {

    /**
     *  @brief Overload operator << for struct `Rational`
     */
    std::ostream& operator << (std::ostream& os, const Rational& r) {
        if (r.m_den == 0) {
            os << "NaN";
        }
        else {
            os << r.m_num << "/" << r.m_den << " (" << r.asDouble() << ")";
        }
        return os;
    }


    /**
     *  @brief Overload operator << for struct `URational`
     */
    std::ostream& operator << (std::ostream& os, const URational& r) {
        if (r.m_den == 0) {
            os << "NaN";
        }
        else {
            os << r.m_num << "/" << r.m_den << " (" << r.asDouble() << ")";
        }
        return os;
    }


    /**
     *  @brief Function to convert a hex string to a 32-bit integer.
     *
     *  @param hex A string with at least 8 characters. The characters represent the
     *             nibbles from left (high bit) to right (low bits).
     *  @param endianess Specifies whether the input is in little-endian or
     *                   big-endian format.
     *  @param out_value Stores the resulting value.
     *  @return ErrorCode Success, NullData, InvalidLength, or UnexpectedData.
     */
    ErrorCode Type::hexToUint32(const char* hex, Endianess endianess, uint32_t& out_value) noexcept {

        uint32_t result = 0;

        if (!hex) {
            return ErrorCode::NullData;
        }

        for (int i = 0; i < 8; ++i) {

            char c = hex[i];
            uint32_t value = 0;

            // Convert character to its integer equivalent.
            if (c >= '0' && c <= '9') {
                value = c - '0';
            }
            else if (c >= 'A' && c <= 'F') {
                value = c - 'A' + 10;
            }
            else if (c >= 'a' && c <= 'f') {
                value = c - 'a' + 10;
            }
            else {
                return ErrorCode::UnexpectedData;
            }

            result = (result << 4) | value;
        }

        if constexpr (std::endian::native == std::endian::little) {
            if (endianess == Endianess::Big) {
                out_value = ((result & 0xff000000) >> 24) |
                            ((result & 0x00ff0000) >> 8)  |
                            ((result & 0x0000ff00) << 8)  |
                            ((result & 0x000000ff) << 24);
            }
        }
        else if constexpr (std::endian::native == std::endian::big) {
            if (endianess == Endianess::Little) {
                out_value = ((result & 0xff000000) >> 24) |
                            ((result & 0x00ff0000) >> 8) |
                            ((result & 0x0000ff00) << 8) |
                            ((result & 0x000000ff) << 24);
            }
        }

        return ErrorCode::None;
    }

    TypeDescription TypeInfo::g_type_description_table[] = {
        { DataType::Undefined, 0,  0, false, false, "Undefined" },
        { DataType::Bool,      1,  1, false, true,  "Bool" },
        { DataType::Char,      1,  8, false, true,  "Char" },
        { DataType::Int8,      1,  8, false, true,  "Int8" },
        { DataType::Int16,     2, 16, false, true,  "Int16" },
        { DataType::Int32,     4, 32, false, true,  "Int32" },
        { DataType::Int64,     8, 64, false, true,  "Int64" },
        { DataType::UInt8,     1,  8, false, true,  "UInt8" },
        { DataType::UInt16,    2, 16, false, true,  "UInt16" },
        { DataType::UInt32,    4, 32, false, true,  "UInt32" },
        { DataType::UInt64,    8, 64, false, true,  "UInt64" },
        { DataType::Float,     4, 32, true,  false, "Float" },
        { DataType::Double,    8, 64, true,  false, "Double" },
        { DataType::FourCC,    4, 32, false, true,  "FourCC" },
        { DataType::Fix,       8, 64, true,  false, "Fix" }
    };


} // End of namespace Grain
