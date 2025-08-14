//
//  Flags.cpp
//
//  Created by Roald Christesen on 11.10.2019
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Type/Flags.hpp"
#include "String/String.hpp"
#include "Core/Log.hpp"


namespace Grain {

    /**
     *  @brief Sets the flag bits from a hexadecimal string.
     *
     *  Parses the input C-string as a hexadecimal number and updates the internal
     *  bits.
     *
     *  @param str A pointer to a null-terminated string representing a hexadecimal
     *             number. If `nullptr`, the bits are set to 0.
     *  @return `true` if the internal bits changed, `false` if they stayed the same.
     *
     *  @note The input string should contain a valid hexadecimal value.
     *  @note No error checking is performed for invalid strings.
     */
    bool Flags::set(const char* str) noexcept {

        if (String::isValidHexString(str) == false) {
            return false;
        }
        else {
            uint32_t bits = str != nullptr ? static_cast<uint32_t>(strtoul(str, NULL, 16)) : 0x0;
            bool result = bits != m_bits;
            m_bits = bits;
            return result;
        }
    }


    /**
     *  @brief Converts the internal bit flags into a human-readable binary string.
     *
     *  This function writes the 32 bits of the `Flags` object into the provided
     *  output string as a sequence of '0' and '1' characters. A space character is
     *  inserted after every 8 bits, except after the last group. The resulting
     *  string is null-terminated.
     *
     *  Example output for a flag value: `11110000 00001111 10101010 01010101`
     *
     *  @param[out] out_str A pointer to a character array where the resulting
     *              string will be written. Must have enough space for at least 35
     *              characters (32 bits + 3 spaces + null terminator).
     *  @note If `out_str` is `nullptr`, the function does nothing.
     *  @note No dynamic memory is allocated. The caller is responsible for
     *        providing a valid buffer.
     */
    void Flags::toStr(char* out_str) const noexcept {

        if (out_str != nullptr) {
            int32_t j = 0;
            int32_t k = 0;

            char p = '7';
            for (int32_t i = 31; i >= 0; i--) {
                out_str[j++] = (m_bits & (0x1 << i) ? p : '.');
                if (k < 3 && (i % 8) == 0) {
                    out_str[j++] = ' ';
                    k++;
                }
                p--;
                if (p < '0') {
                    p = '7';
                }
            }

            out_str[j] = 0;
        }
    }


    /**
     *  @brief Converts the internal bit flags into a human-readable binary string
     *         and stores it in a String.
     *
     *  This function generates a binary string representation of the `Flags` object
     *  (similar to `toStr`) and assigns the result to the provided `String` object.
     *
     *  @param[out] out_string A reference to a `String` object where the resulting
     *                         binary string will be stored.
     *  @note The output will contain 32 '0'/'1' characters with spaces inserted
     *        after every 8 bits for better readability.
     *  @note No dynamic memory allocation occurs inside this method itself, but the
     *        `out_string` object may perform a dynamic reallocation internally when
     *        assigned the new string data.
     */
    void Flags::toString(String& out_string) const noexcept {

        char buffer[kFlagsStrLength];
        toStr(buffer);
        out_string = buffer;
    }


} // End of namespace Grain
