//
//  Fix.cpp
//
//  Created by Roald Christesen on 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Type/Fix.hpp"
#include "Type/Type.hpp"
#include <String/String.hpp>
#include "Core/Log.hpp"

#include <cinttypes>    // For use of macros like PRId64 in printf()


namespace Grain {

    Fix::Fix(const String& string) noexcept {
        setStr(string.utf8());
    }


    void Fix::log(Log& l) const {
        char buffer[kMaxStrLength];
        toStr(buffer, kMaxStrLength);
        l << buffer;
    }


    /*
     *  Implementation of assignment operator.
     */
    Fix& Fix::operator = (const String& string) {
        setStr(string.utf8());
        return *this;
    }


    /*
     *  Implementation of assignment operator.
     */
    Fix& Fix::operator = (const String* string) {
        if (string) {
            setStr(string->utf8());
        }
        else {
            m_raw_value = 0;
        }
        return *this;
    }


    int64_t Fix::asInt64(int32_t precision) const noexcept {
        static const int64_t f[] = {
            1000000000L,
            100000000L,
            10000000L,
            1000000L,
            100000L,
            10000L,
            1000L,
            100L,
            10L,
            1L
        };

        if (precision < 0) {
            precision = 0;
        }
        else if (precision > 9) {
            precision = 9;
        }

        return m_raw_value / f[precision];
    }


    bool Fix::set(const Fix& value) noexcept {
        if (m_raw_value != value.m_raw_value) {
            m_raw_value = value.m_raw_value;
            return true;
        }
        else {
            return false;
        }
    }


    bool Fix::set(int32_t i, uint32_t f) noexcept {
        int64_t remembered = m_raw_value;
        m_raw_value = static_cast<int64_t>(i) * Fix::kFrcScale + f;
        return m_raw_value != remembered;
    }


    bool Fix::set(const Fix& value, const Fix& min, const Fix& max, int32_t precision) noexcept {
        Fix new_value = value;
        new_value.clamp(min, max);
        new_value.setPrecision(precision);
        return set(new_value);
    }


    bool Fix::setInt32(int32_t value) noexcept {
        int64_t remembered = m_raw_value;
        m_raw_value = static_cast<int64_t>(value) * Fix::kFrcScale;
        return m_raw_value != remembered;
    }


    bool Fix::setInt64(int64_t value) noexcept {
        int64_t remembered = m_raw_value;
        m_raw_value = value * Fix::kFrcScale;
        return m_raw_value != remembered;
    }


    bool Fix::setInt64(int64_t value, int32_t precision) noexcept {
        static const int64_t f[] = {
            1L,
            10L,
            100L,
            1000L,
            10000L,
            100000L,
            1000000L,
            10000000L,
            100000000L,
            1000000000L
        };

        int64_t remembered = m_raw_value;

        if (precision < 0) {
            precision = 0;
        }
        else if (precision > 9) {
            precision = 9;
        }

        m_raw_value = value * Fix::kFrcScale / f[precision];
        return m_raw_value != remembered;
    }


    bool Fix::setFloat(float value) noexcept {
        int64_t remembered = m_raw_value;
        m_raw_value = _fixFromDouble(value);
        return m_raw_value != remembered;
    }


    bool Fix::setDouble(double value) noexcept {
        int64_t remembered = m_raw_value;
        m_raw_value = _fixFromDouble(value);
        return m_raw_value != remembered;
    }


    bool Fix::setDoubleDefinedPrecision(double value, uint32_t precision) noexcept {
        static const double fa[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };
        static const int64_t fb[] = { 1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1 };

        if (precision > 9) {
            precision = 9;
        }

        int64_t remembered = m_raw_value;
        m_raw_value = static_cast<int64_t>(std::round(value * fa[precision]) * fb[precision]);

        return m_raw_value != remembered;
    }


    bool Fix::setFraction(int32_t dividend) noexcept {
        int64_t remembered = m_raw_value;

        switch (dividend) {
            case 1: m_raw_value = 1000000000L; break;
            case 2: m_raw_value = 500000000L; break;
            case 3: m_raw_value = 333333333L; break;
            case 4: m_raw_value = 250000000L; break;
            case 5: m_raw_value = 200000000L; break;
            case 6: m_raw_value = 166666667L; break;
            case 8: m_raw_value = 125000000L; break;
            case 9: m_raw_value = 111111111L; break;
            case 10: m_raw_value = 100000000L; break;
            case 11: m_raw_value = 90909091L; break;
            case 12: m_raw_value = 83333333L; break;
            case 15: m_raw_value = 66666667L; break;
            case 16: m_raw_value = 62500000L; break;
            case 18: m_raw_value = 55555556L; break;
            case 20: m_raw_value = 50000000L; break;
            case 25: m_raw_value = 40000000L; break;
            case 50: m_raw_value = 20000000L; break;
            case 100: m_raw_value = 10000000L; break;
            case 1000: m_raw_value = 1000000L; break;
            case 10000: m_raw_value = 100000L; break;
            case 100000: m_raw_value = 10000L; break;
            case 1000000: m_raw_value = 1000L; break;
            case 10000000: m_raw_value = 100L; break;
            case 100000000: m_raw_value = 10L; break;
            case 1000000000: m_raw_value = 1L; break;
            default:
                m_raw_value = _fixFromDouble(1.0 / dividend);
                break;
        }
        return m_raw_value != remembered;
    }


    bool Fix::setStr(const char* str) noexcept {
        static const int64_t frc_scale[] = {
            0L, 100000000L, 10000000L, 1000000L, 100000L,
            10000L, 1000L, 100L, 10L, 1L
        };

        if (!str) {
            return false;
        }

        int64_t remembered = m_raw_value;
        m_raw_value = 0L;

        // Skip leading whitespace
        while (*str && String::charIsWhiteSpace(*str)) {
            ++str;
        }

        // Handle optional '+' or '-' sign
        bool negative = false;
        if (*str == '-') {
            negative = true;
            ++str;
        }
        else if (*str == '+') {
            ++str;
        }

        int64_t int_value = 0L;
        int64_t frc_value = 0L;
        int32_t frc_n = 0;
        bool decimal_part = false;

        // Parse integer part
        while (*str) {
            char c = *str;

            if (String::charIsWhiteSpace(c)) {
                // skip
            }
            else if (c >= '0' && c <= '9') {
                int_value = int_value * 10 + (c - '0');
            }
            else if (c == '.' || c == ',') {
                decimal_part = true;
                ++str;
                break;
            }
            else {
                break;
            }
            ++str;
        }

        // Parse fractional part
        if (decimal_part) {
            int extra_digit = -1;

            while (*str) {
                char c = *str;
                if (c >= '0' && c <= '9') {
                    if (frc_n < 9) {
                        frc_value = frc_value * 10 + (c - '0');
                        ++frc_n;
                    }
                    else if (extra_digit == -1) {
                        // 10th digit (used for rounding)
                        extra_digit = c - '0';
                    }
                    else {
                        // Ignore further digits
                    }
                }
                else {
                    break;
                }
                ++str;
            }

            // Round up if 10th digit is 5 or greater
            if (extra_digit >= 5) {
                ++frc_value;
                // Handle overflow, e.g. 0.999999999 + 1 → 1.000000000
                if (frc_value >= Fix::kFrcScale) {
                    frc_value = 0;
                    int_value += 1;
                }
            }
        }

        // Combine and apply sign
        m_raw_value = int_value * Fix::kFrcScale + frc_value * frc_scale[frc_n];
        if (negative) {
            m_raw_value = -m_raw_value;
        }

        return m_raw_value != remembered;
    }


    void Fix::setPrecision(int32_t precision) noexcept {
        static const int64_t scale [] = {
            1000000000L,
            100000000L,
            10000000L,
            1000000L,
            100000L,
            10000L,
            1000L,
            100L,
            10L
        };

        if (precision < Fix::kDecPrecision) {
            if (precision < 0) {
                precision = 0;
            }
            m_raw_value = (m_raw_value / scale[precision]) * scale[precision];
        }
    }


    void Fix::flip(const Fix min, const Fix max) noexcept {
        if (m_raw_value > max.m_raw_value) {
            m_raw_value = min.m_raw_value + (m_raw_value - max.m_raw_value);
        }
        else if (m_raw_value < min.m_raw_value) {
            m_raw_value = max.m_raw_value - (min.m_raw_value - m_raw_value);
        }
    }


    void Fix::clamp(const Fix& min, const Fix& max) noexcept {
        if (m_raw_value < min.m_raw_value) {
            m_raw_value = min.m_raw_value;
        }
        else if (m_raw_value > max.m_raw_value) {
            m_raw_value = max.m_raw_value;
        }
    }


    void Fix::fixValueToStr(int64_t fix_value, char* out_str, int32_t max_length, int32_t precision) noexcept {
        static const int64_t f[] = {
            1000000000L,
            100000000L,
            10000000L,
            1000000L,
            100000L,
            10000L,
            1000L,
            100L,
            10L,
            1L,
            0L
        };

        if (!out_str || max_length <= 0) return;

        // Clamp precision
        if (precision < 0) precision = 0;
        if (precision > kDecPrecision) precision = kDecPrecision;


        bool negative = fix_value < 0L;
        if (negative) {
            fix_value = -fix_value;
        }
        // std::cout << "........... presicion: " << precision << ", fix_value: " << fix_value << ", negative: " << negative << std::endl;

        for (int32_t pass = 0; pass < 2; pass++) {

            int64_t int_part = fix_value / kFrcScale;
            int64_t real_part = fix_value % kFrcScale;

            // std::cout << "    pass: " << pass << ", int: " << int_part << ", real: " << real_part << std::endl;
            char* d = out_str;
            if (negative) {
                *d++ = '-';
            }

            std::snprintf(d, 20, "%" PRId64, int_part);
            d += Type::decimalDigitsUInt64(int_part);


            *d++ = '.';
            std::snprintf(d, 20, "%09" PRId64, real_part);
            d += precision;
            // std::cout << "    d[0]: " << d[0] << std::endl;
            if (pass == 0 && precision < kDecPrecision && *d >= '5') {
                fix_value += 5 * f[precision + 1];
                // std::cout << "    fix_value: " << fix_value << std::endl;
                // std::cout << "    out_str: " << out_str << std::endl;
            }
            else {
                *d = '\0';
                d--;
                while (*d == '0') {
                    d--;
                }
                if (*d == '.') {
                    *d = '\0';
                }
                else {
                    d[1] = '\0';
                }

                // std::cout << "    out_str: " << out_str << ", done!" << std::endl;
                return;
            }
        }
    }


    void Fix::toStr(char* out_str, int32_t max_length, int32_t precision) const noexcept {
        fixValueToStr(m_raw_value, out_str, max_length, precision);
    }


    void Fix::toString(String& out_string, int32_t precision) const noexcept {
        char buffer[kStrBufferSize];
        toStr(buffer, kStrBufferSize, precision);
        out_string.set(buffer);
    }


    void Fix::limitCircular(const Fix& min, const Fix& max) noexcept {
        int64_t range = max.m_raw_value - min.m_raw_value;
        m_raw_value = (m_raw_value - min.m_raw_value) % range + min.m_raw_value;
        if (m_raw_value < min.m_raw_value) {
            m_raw_value += range;
        }
    }


}  // End of namespace Grain
