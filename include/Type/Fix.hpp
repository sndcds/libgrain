//
//  Fix.hpp
//
//  Created by Roald Christesen on 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 11.07.2025
//

#ifndef GrainFix_hpp
#define GrainFix_hpp

#include <stdio.h>
#include <iostream>
#include <cstdint>
#include <cmath>
#include <string.h>


namespace Grain {

    class String;
    class Log;


    /**
     *  @class Fix
     *  @brief Real number type class with a precision of 9 decimal places.
     *
     *  Fix is a class that represents very precise real numbers (fixed-point math,
     *  not floating point), useful when you need high precision and no floating
     *  point rounding errors. Maximum range is -999,999,999 to 999,999,999
     *  with precision of 9 decimals.
     */
    class Fix {
        friend class Vec2Fix;
        friend class Vec3Fix;

    public:
        Fix() noexcept = default;

        // Integral constructor
        template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
        Fix(T v) noexcept {
            // std::cout << "Fix(T integral): " << v << std::endl;
            m_raw_value = static_cast<int64_t>(v) * kFrcScale;
        }

        // Floating-point constructor
        template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
        Fix(T v) noexcept {
            // std::cout << "Fix(T float): " << v << std::endl;
            m_raw_value = _fixFromDouble(v);
        }

        explicit Fix(int32_t i, uint32_t f) noexcept { m_raw_value = static_cast<int64_t>(i) * Fix::kFrcScale + f; }
        explicit Fix(const char* str) noexcept { setStr(str); }
        explicit Fix(const String& string) noexcept;

        ~Fix() noexcept = default;

        friend std::ostream& operator << (std::ostream& os, const Fix& o) {
            char buffer[kMaxStrLength];
            o.toStr(buffer, kMaxStrLength);
            return os << buffer;
        }

        void log(Log& l) const;


        explicit operator bool() const { return m_raw_value != 0; }

        Fix& operator = (const Fix& other) { m_raw_value = other.m_raw_value; return *this; }
        Fix& operator = (size_t v) { m_raw_value = static_cast<int64_t>(v) * Fix::kFrcScale; return *this; }
        Fix& operator = (int32_t v) { m_raw_value = static_cast<int64_t>(v) * Fix::kFrcScale; return *this; }
        Fix& operator = (int64_t v) { m_raw_value = v * Fix::kFrcScale; return *this; }
        Fix& operator = (float v) { m_raw_value = _fixFromDouble(v); return *this; }
        Fix& operator = (double v) { m_raw_value = _fixFromDouble(v); return *this; }
        Fix& operator = (const String& string);
        Fix& operator = (const String* string);

        bool operator == (const Fix& other) const { return m_raw_value == other.m_raw_value; }
        bool operator == (int32_t v) const { return m_raw_value == _fixFromInt64(v); }

        bool operator != (const Fix& other) const { return m_raw_value != other.m_raw_value; }
        bool operator != (int32_t v) const { return m_raw_value != _fixFromInt64(v); }

        bool operator > (const Fix& other) const { return m_raw_value > other.m_raw_value; }
        bool operator > (int32_t v) const { return m_raw_value > _fixFromInt64(v); }
        bool operator > (float v) const { return m_raw_value > _fixFromDouble(v); }
        bool operator > (double v) const { return m_raw_value > _fixFromDouble(v); }

        bool operator >= (const Fix& other) const { return m_raw_value >= other.m_raw_value; }
        bool operator >= (int32_t v) const { return m_raw_value >= _fixFromInt64(v); }
        bool operator >= (float v) const { return m_raw_value >= _fixFromDouble(v); }
        bool operator >= (double v) const { return m_raw_value >= _fixFromDouble(v); }

        bool operator < (const Fix& other) const { return m_raw_value < other.m_raw_value; }
        bool operator < (int32_t v) const { return m_raw_value < _fixFromInt64(v); }
        bool operator < (float v) const { return m_raw_value < _fixFromDouble(v); }
        bool operator < (double v) const { return m_raw_value < _fixFromDouble(v); }

        bool operator <= (const Fix& other) const { return m_raw_value <= other.m_raw_value; }
        bool operator <= (int32_t v) const { return m_raw_value <= _fixFromInt64(v); }
        bool operator <= (float v) const { return m_raw_value <= _fixFromDouble(v); }
        bool operator <= (double v) const { return m_raw_value <= _fixFromDouble(v); }

        Fix operator - () const { Fix result; result.m_raw_value = -m_raw_value; return result; }

        Fix operator + (const Fix& other) const {
            Fix result;
            result.m_raw_value = m_raw_value + other.m_raw_value;
            return result;
        }

        Fix operator - (const Fix& other) const {
            Fix result;
            result.m_raw_value = m_raw_value - other.m_raw_value;
            return result;
        }

        Fix operator * (const Fix& other) const {
            double a = static_cast<double>(m_raw_value) * Fix::kInvScale;
            double b = static_cast<double>(other.m_raw_value) * Fix::kInvScale;
            Fix result;
            result.m_raw_value = _fixFromDouble(a * b);
            return result;
        }

        Fix operator * (int32_t v) const {
            Fix result;
            result.m_raw_value = m_raw_value * v;
            return result;
        }

        Fix operator * (double v) const {
            Fix result;
            result.m_raw_value = _fixFromDouble((static_cast<double>(m_raw_value) * Fix::kInvScale) * v);
            return result;
        }

        Fix operator / (const Fix& other) const {
            double a = static_cast<double>(m_raw_value) * Fix::kInvScale;
            double b = static_cast<double>(other.m_raw_value) * Fix::kInvScale;
            Fix result;
            result.m_raw_value = (b != 0.0) ? _fixFromDouble(a / b) : 0;
            return result;
        }

        Fix operator / (int32_t v) const {
            Fix result;
            result.m_raw_value = m_raw_value / v;
            return result;
        }

        Fix operator / (double v) const {
            Fix result;
            result.m_raw_value = _fixFromDouble((static_cast<double>(m_raw_value) * Fix::kInvScale) / v);
            return result;
        }

        Fix& operator += (const Fix& other) { m_raw_value += other.m_raw_value; return *this; }
        Fix& operator += (int32_t v) { m_raw_value += _fixFromInt64(v); return *this; }
        Fix& operator += (float v) { m_raw_value += _fixFromDouble(v); return *this; }
        Fix& operator += (double v) { m_raw_value += _fixFromDouble(v); return *this; }

        Fix& operator -= (const Fix& other) { m_raw_value -= other.m_raw_value; return *this; }
        Fix& operator -= (int32_t v) { m_raw_value -= _fixFromInt64(v); return *this; }
        Fix& operator -= (float v) { m_raw_value -= _fixFromDouble(v); return *this; }
        Fix& operator -= (double v) { m_raw_value -= _fixFromDouble(v); return *this; }

        Fix& operator *= (const Fix& other) { *this = *this * other; return *this; }
        Fix& operator *= (int32_t v) { m_raw_value *= v; return *this; }
        Fix& operator *= (float v) { m_raw_value = _fixFromDouble(asDouble() * v); return *this; }
        Fix& operator *= (double v) { m_raw_value = _fixFromDouble(asDouble() * v); return *this; }

        Fix& operator /= (const Fix& other) { *this = *this / other; return *this; }
        Fix& operator /= (int32_t v) { m_raw_value /= v; return *this; }
        Fix& operator /= (float v) { m_raw_value = _fixFromDouble(asDouble() / v); return *this; }
        Fix& operator /= (double v) { m_raw_value = _fixFromDouble(asDouble() / v); return *this; }


        [[nodiscard]] inline int64_t raw() const noexcept { return m_raw_value; }
        [[nodiscard]] inline const int64_t* rawValuePtr() const noexcept { return &m_raw_value; }
        [[nodiscard]] inline int64_t* mutRawValuePtr() noexcept { return &m_raw_value; }

        [[nodiscard]] inline bool isPositive() const noexcept { return m_raw_value > 0; }
        [[nodiscard]] inline bool isNegative() const noexcept { return m_raw_value < 0; }
        [[nodiscard]] inline bool isZero() const noexcept { return m_raw_value != 0; }


        /**
         *  @brief Checks if the value is an integer, meaning it contains no
         *         fractional part.
         *
         *  This function is used to determine if a numerical value represents a
         *  whole number without any fractional components (i.g., nothing after the
         *  decimal point).
         */
        [[nodiscard]] inline bool isInt() const noexcept { return std::abs(m_raw_value % 1000000000) == 0; }
        [[nodiscard]] inline bool isFloat() const noexcept { return std::abs(m_raw_value % 1000000000) != 0; }

        [[nodiscard]] bool asBool() const noexcept { return asInt32() != 0; }
        [[nodiscard]] inline int32_t asInt32() const noexcept {
            return static_cast<int32_t>(std::round(static_cast<double>(m_raw_value) * Fix::kInvScale));
        }
        [[nodiscard]] inline int64_t asInt64() const noexcept {
            return static_cast<int64_t>(std::round(static_cast<double>(m_raw_value) * Fix::kInvScale));
        }
        [[nodiscard]] inline float asFloat() const noexcept { return static_cast<float>(m_raw_value) * Fix::kInvScale; }
        [[nodiscard]] inline double asDouble() const noexcept { return static_cast<double>(m_raw_value) * Fix::kInvScale; }
        [[nodiscard]] int64_t asInt64(int32_t precision) const noexcept;

        inline void setRaw(int64_t raw) noexcept { m_raw_value = raw; }
        inline void setToMin() noexcept { m_raw_value = kMinValue; }
        inline void setToMax() noexcept { m_raw_value = kMaxValue; }

        bool set(const Fix& value) noexcept;
        bool set(int32_t i, uint32_t f) noexcept;
        bool set(const Fix& value, const Fix& min, const Fix& max, int32_t precision) noexcept;
        bool setInt32(int32_t value) noexcept;
        bool setInt64(int64_t value) noexcept;
        bool setInt64(int64_t value, int32_t precision) noexcept;
        bool setFloat(float value) noexcept;
        bool setDouble(double value) noexcept;
        bool setDoubleDefinedPrecision(double value, uint32_t precision) noexcept;
        bool setFraction(int32_t dividend) noexcept;
        bool setStr(const char* str) noexcept;
        void setPrecision(int32_t precision) noexcept;


        [[nodiscard]] Fix abs() const noexcept {
            return Fix::fromValue(m_raw_value >= 0 ? m_raw_value : -m_raw_value);
        }

        [[nodiscard]] Fix floor() const noexcept {
            int64_t divided = m_raw_value / kFrcScale;
            Fix f;
            f.m_raw_value = divided * kFrcScale;
            return f;
        }

        [[nodiscard]] Fix ceil() const noexcept {
            int64_t divided = m_raw_value / kFrcScale;
            if (m_raw_value % kFrcScale != 0) {
                divided += 1;
            }
            Fix f;
            f.m_raw_value = divided * kFrcScale;
            return f;
        }

        [[nodiscard]] Fix round() const noexcept {
            int64_t divided = (m_raw_value + (kFrcScale / 2)) / kFrcScale;
            Fix f;
            f.m_raw_value = divided * kFrcScale;
            return f;
        }

        [[nodiscard]] Fix sqrt() const noexcept {
            if (m_raw_value <= 0) {
                return Fix(0);
            }
            else {
                return Fix(std::sqrt(asDouble()));
            }
        }


        inline void invalidate() noexcept { m_raw_value = 0x7FFFFFFFFFFFFFFFL; }
        inline void negate() noexcept { m_raw_value = -m_raw_value; }
        void flip(const Fix min, const Fix max) noexcept;
        void clamp(const Fix& min, const Fix& max) noexcept;

        static void fixValueToStr(int64_t fix_value, char* out_str, int32_t max_length, int32_t precision = Fix::kDecPrecision) noexcept;
        void toStr(char* out_str, int32_t max_length, int32_t precision = Fix::kDecPrecision) const noexcept;
        void toString(String& out_string, int32_t precision = Fix::kDecPrecision) const noexcept;

        void limitCircular(const Fix& min, const Fix& max) noexcept;

        [[nodiscard]] static inline Fix minOf(const Fix& a, const Fix& b) { return a < b ? a : b; }
        [[nodiscard]] static inline Fix maxOf(const Fix& a, const Fix& b) { return a > b ? a : b; }

        [[nodiscard]] static Fix remap(const Fix& imin, const Fix& imax, const Fix& omin, const Fix& omax, const Fix& v) noexcept {
            return imax != imin ? ((v - imin) / (imax - imin)) * (omax - omin) + omin : omin;
        }

        [[nodiscard]] static inline Fix fromValue(int64_t raw_value) noexcept { Fix f; f.m_raw_value = raw_value; return f; }
        [[nodiscard]] static inline int64_t _fixFromFloat(float v) {
            return (static_cast<int64_t>(static_cast<double>(v) * 1000000000.0));
        }
        [[nodiscard]] static inline int64_t _fixFromDouble(double v) {
            return (static_cast<int64_t>(v * 1000000000.0));
            // constexpr double scaleFrom = 1e9;
            // return static_cast<int64_t>(std::round(v * scaleFrom));
        }
        [[nodiscard]] static inline int64_t _fixFromInt64(int64_t v) {
            return (static_cast<int64_t>(v * 1000000000));
        }

    public:
        static constexpr int64_t kMaxValue = 999999999000000000L;
        static constexpr int64_t kMinValue = -999999999000000000L;
        static constexpr double kInvScale = 0.000000001L;
        static constexpr int64_t kFrcScale = 1000000000;
        static constexpr int64_t kDecPrecision = 9;     ///< Number of decimal digits
        static constexpr int64_t kMaxStrLength = 21;    ///< Maximum length of str representation incl. EOS

    public:
        enum {
            kStrBufferSize = 40
        };

    protected:
        int64_t m_raw_value = 0L;
    };


} // End of namespace Grain

#endif // GrFix_hpp
