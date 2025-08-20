//
//  Type.hpp
//
//  Created by Roald Christesen on 06.05.2014
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 20.08.2025
//

#ifndef GrainType_hpp
#define GrainType_hpp

#include "Grain.hpp"


namespace Grain {

    /**
     *  @brief A simple class representing a rational number (fraction).
     *
     *  This class stores a rational number as a numerator and denominator,
     *  and provides a method to convert it to a floating-point value.
     */
    class Rational {
    public:
        Rational() : m_num(0), m_den(0) {}

        /**
         *  @brief Constructs a rational number with the given numerator and
         *         denominator.
         *
         *  @param num The numerator.
         *  @param den The denominator.
         */
        Rational(int32_t num, uint32_t den) : m_num(num), m_den(den) {}

        /**
         *  @brief Converts the rational number to a double-precision floating-point
         *         number.
         *
         *  @return The double representation of the rational number. Returns NaN if
         *          the denominator is zero.
         */
        [[nodiscard]] double asDouble() const noexcept {
            return m_den == 0 ? NAN : static_cast<double>(m_num) / m_den;
        }

        friend std::ostream& operator << (std::ostream& os, const Rational& r);

    protected:
        int32_t m_num;      ///< Numerator
        uint32_t m_den;     ///< Denominator
    };


    /**
     *  @brief A class representing an unsigned rational number (fraction).
     *
     *  This class stores a rational number using only unsigned 32-bit integers
     *  for both the numerator and denominator. It provides a method to convert
     *  the rational number to a double-precision floating-point value.
     */
    class URational {
    public:
        URational() : m_num(0), m_den(0) {}

        /**
         *  @brief Constructs a rational number with the given numerator and
         *         denominator.
         *
         *  @param num The numerator.
         *  @param den The denominator.
         */
        URational(uint32_t num, uint32_t den) : m_num(num), m_den(den) {}

        /**
         *  @brief Converts the rational number to a double-precision floating-point
         *         number.
         *
         *  @return The double representation of the rational number. Returns NaN if
         *          the denominator is zero.
         */
        [[nodiscard]] double asDouble() const noexcept {
            return m_den == 0 ? NAN : static_cast<double>(m_num) / m_den;
        }

        friend std::ostream& operator << (std::ostream& os, const URational& r);

    protected:
        uint32_t m_num;     ///< Numerator
        uint32_t m_den;     ///< Denominator
    };


    class Type {
    public:
        /**
         *  @brief Type constants, typically used in files.
         *  @note Value must be in range of int16_t.
         */
        enum {
            kType_Undefined = -1,
            kType_Int8 = 0,
            kType_Int16 = 1,
            kType_Int32 = 2,
            kType_Int64 = 3,
            kType_UInt8 = 4,
            kType_UInt16 = 5,
            kType_UInt32 = 6,
            kType_UInt64 = 7,
            kType_Float = 8,
            kType_Double = 9,
            kType_Fix = 10,
            kType_Vec2i = 11,
            kType_Vec2l = 12,
            kType_Vec2f = 13,
            kType_Vec2d = 14,
            kType_Vec3i = 15,
            kType_Vec3l = 16,
            kType_Vec3f = 17,
            kType_Vec3d = 18,

            kType_Count,
            kType_First = 0,
            kType_Last = kType_Count - 1
        };


        /**
         *  @brief Returns the string name of a type based on its integer identifier.
         *
         *  This function maps a given type identifier to a corresponding human-readable
         *  string name (e.g., "int32", "float", "Vec3f", etc.). If the identifier falls
         *  outside the known range, it returns "undefined" or "unknown" appropriately.
         *
         *  @param type The type identifier as a 16-bit integer.
         *  @return A constant character pointer to the name of the type.
         *
         *  @note The valid range of type identifiers is assumed to be between
         *        `kType_First` and `kType_Last`. The special value `kType_Undefined`
         *        is also handled explicitly.
         */
        [[nodiscard]] static const char* typeName(int16_t type) {
            static const char* names[] = {
                "int8", "int16", "int32", "int64",
                "uint8", "uint16", "uint32", "uint64",
                "float", "double", "Fix",
                "Vec2i", "Vec2l", "Vec2f", "Vec2d",
                "Vec3i", "Vec3l", "Vec3f", "Vec3d"
            };

            if (type >= kType_First && type <= kType_Last) {
                return names[type];
            }
            else if (type == kType_Undefined) {
                return "undefined";
            }
            else {
                return "unknown";
            }
        }

        /**
         *  @brief Returns the sign of a numeric value.
         *
         *  This templated function works with any arithmetic type (int, float, double, etc.).
         *
         *  @tparam T Any arithmetic type.
         *  @param v The input value.
         *  @return -1 if v < 0, 1 if v > 0, 0 if v == 0.
         */
        template <typename T>
        [[nodiscard]] static inline constexpr typename std::enable_if<std::is_arithmetic<T>::value, int>::type
        sign(T v) {
            return (T(0) < v) - (v < T(0));
        }

        /**
         *  @brief Checks whether a given integer is a power of two.
         *
         *  This templated static function determines if a given integral value is a power of two.
         *  A number is considered a power of two if it has exactly one bit set in its binary representation.
         *
         *  The function works for any integral type (signed or unsigned), such as:
         *  - `int8_t`, `int16_t`, `int32_t`, `int64_t`
         *  - `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t`
         *
         *  For signed types, non-positive values return `false`.
         *  The check is performed using a bitwise operation: `(value & (value - 1)) == 0`.
         *
         *  @tparam T An integral type.
         *  @param value The value to test.
         *  @return `true` if the value is a power of two, `false` otherwise.
         *
         *  @note This is a templated function and constrained to integral types only.
         */
        template<typename T>
        [[nodiscard]] static bool isPowerOfTwo(T value) noexcept {
            static_assert(std::is_integral_v<T>, "isPowerOfTwo requires an integral type.");

            if constexpr (std::is_signed_v<T>) {
                if (value <= 0) return false;
            }
            else {
                if (value == 0) return false;
            }

            return (value & (value - 1)) == 0;
        }

        /**
         *  @brief Compares two floating-point values for approximate equality.
         *
         *  This method checks if the absolute difference between two double values
         *  is within a specified tolerance (epsilon). Useful for comparing floating-point
         *  results where exact equality is unreliable due to precision errors.
         *
         *  @param lhs The left-hand side value to compare.
         *  @param rhs The right-hand side value to compare.
         *  @param epsilon The tolerance threshold for the comparison (default: 1e-12).
         *  @return true if the values are approximately equal within epsilon, false otherwise.
         */
         static bool approx(double lhs, double rhs, double epsilon = 1e-12) noexcept {
            return std::fabs(lhs - rhs) <= epsilon;
        }

        [[nodiscard]] static int32_t decimalDigitsInt32(int32_t v) {
            if (v < 0) v = -v;
            if (v < 10) return 1;
            if (v < 100) return 2;
            if (v < 1000) return 3;
            if (v < 10000) return 4;
            if (v < 100000) return 5;
            if (v < 1000000) return 6;
            if (v < 10000000) return 7;
            if (v < 100000000) return 8;
            if (v < 1000000000) return 9;
            return 10;
        }

        [[nodiscard]] static int32_t decimalDigitsUInt32(uint32_t v) {
            if (v < 10U) return 1;
            if (v < 100U) return 2;
            if (v < 1000U) return 3;
            if (v < 10000U) return 4;
            if (v < 100000U) return 5;
            if (v < 1000000U) return 6;
            if (v < 10000000U) return 7;
            if (v < 100000000U) return 8;
            if (v < 1000000000U) return 9;
            return 10;
        }

        [[nodiscard]] static int32_t decimalDigitsInt64(int64_t v) {
            if (v < 0) v = -v;
            if (v < 10LL) return 1;
            if (v < 100LL) return 2;
            if (v < 1000LL) return 3;
            if (v < 10000LL) return 4;
            if (v < 100000LL) return 5;
            if (v < 1000000LL) return 6;
            if (v < 10000000LL) return 7;
            if (v < 100000000LL) return 8;
            if (v < 1000000000LL) return 9;
            if (v < 10000000000LL) return 10;
            if (v < 100000000000LL) return 11;
            if (v < 1000000000000LL) return 12;
            if (v < 10000000000000LL) return 13;
            if (v < 100000000000000LL) return 14;
            if (v < 1000000000000000LL) return 15;
            if (v < 10000000000000000LL) return 16;
            if (v < 100000000000000000LL) return 17;
            if (v < 1000000000000000000LL) return 18;
            return 19;
        }

        [[nodiscard]] static int32_t decimalDigitsUInt64(uint64_t v) {
            if (v < 10ULL) return 1;
            if (v < 100ULL) return 2;
            if (v < 1000ULL) return 3;
            if (v < 10000ULL) return 4;
            if (v < 100000ULL) return 5;
            if (v < 1000000ULL) return 6;
            if (v < 10000000ULL) return 7;
            if (v < 100000000ULL) return 8;
            if (v < 1000000000ULL) return 9;
            if (v < 10000000000ULL) return 10;
            if (v < 100000000000ULL) return 11;
            if (v < 1000000000000ULL) return 12;
            if (v < 10000000000000ULL) return 13;
            if (v < 100000000000000ULL) return 14;
            if (v < 1000000000000000ULL) return 15;
            if (v < 10000000000000000ULL) return 16;
            if (v < 100000000000000000ULL) return 17;
            if (v < 1000000000000000000ULL) return 18;
            if (v < 10000000000000000000ULL) return 19;
            return 20;
        }

        /**
         *  @brief Copies a block of memory from one array to another.
         *
         *  Performs a simple memory copy of `count` elements from source array `s` to
         *  destination array `d`. The function checks that both pointers are non-null
         *  and not equal before copying.
         *
         *  @tparam T The type of the elements to copy.
         *  @param d Pointer to the destination array.
         *  @param s Pointer to the source array.
         *  @param count Number of elements to copy.
         */
        template <typename T>
        static void copy(T* d, const T* s, int64_t count) noexcept {
            if (d && s && d != s) {
                memcpy(d, s, count * sizeof(T));
            }
        }

        /**
         *  @brief Copies elements from a source buffer to a destination buffer with custom strides.
         *
         *  Performs bounds checking before copying. If the requested range exceeds bounds,
         *  only the valid portion will be copied.
         *
         *  @tparam T Trivially copyable type.
         *  @param dst Destination buffer.
         *  @param src Source buffer.
         *  @param dst_offset Offset into the destination buffer.
         *  @param src_offset Offset into the source buffer.
         *  @param dst_stride Stride between elements in the destination buffer.
         *  @param src_stride Stride between elements in the source buffer.
         *  @param dst_capacity Size of the destination buffer in elements.
         *  @param src_capacity Size of the source buffer in elements.
         *  @param count Number of elements to copy.
         *  @return Number of elements successfully copied (≥ 0), or a negative error code:
         *          -1 = null destination pointer
         *          -2 = null source pointer
         *          -3 = negative count
         *          -4 = non-positive destination stride
         *          -5 = non-positive source stride
         */
        template <typename T>
        static int64_t copyStrided(
            T* dst,
            const T* src,
            int64_t dst_offset,
            int64_t src_offset,
            int32_t dst_stride,
            int32_t src_stride,
            int64_t dst_capacity,
            int64_t src_capacity,
            int64_t count
        ) noexcept {
            static_assert(std::is_trivially_copyable<T>::value, "copy<T>() requires trivially copyable types");

            if (!dst) return -1; // Null destination
            if (!src) return -2; // Null source
            if (dst_offset < 0) return -3; // Invalid destination offset
            if (src_offset < 0) return -4; // Invalid destination offset
            if (dst_offset >= dst_capacity) return -5; // Invalid destination offset
            if (src_offset >= src_capacity) return -6; // Invalid destination offset
            if (dst_stride <= 0) return -7; // Invalid destination stride
            if (src_stride <= 0) return -8; // Invalid source stride
            if (count < 0) return -9; // Negative count

            if (count == 0) return 0;

            // Compute accessed index ranges
            int64_t dst_first = dst_offset;
            int64_t dst_last  = dst_offset + (count - 1) * dst_stride;
            int64_t src_first = src_offset;
            int64_t src_last  = src_offset + (count - 1) * src_stride;

            int64_t dst_valid_max = std::min<int64_t>(dst_capacity - 1, dst_last);
            int64_t src_valid_max = std::min<int64_t>(src_capacity - 1, src_last);

            // Compute valid steps (number of safe elements)
            int64_t dst_steps = std::max<int64_t>(0, (dst_valid_max - dst_first) / dst_stride + 1);
            int64_t src_steps = std::max<int64_t>(0, (src_valid_max - src_first) / src_stride + 1);

            int64_t valid_count = std::min<int64_t>(count, std::min<int64_t>(dst_steps, src_steps));

            if (valid_count <= 0) return 0;

            int64_t skipped = count - valid_count;
            dst += dst_offset;
            src += src_offset;

            for (int64_t i = 0; i < valid_count; ++i) {
                *dst = *src;
                dst += dst_stride;
                src += src_stride;
            }

            return valid_count;
        }

        /**
         *  @brief Conditionally updates a variable if the new value differs.
         *
         *  This function assigns the given value to the variable only if the new
         *  value is different from the currentMillis one. It helps avoid unnecessary
         *  assignments, which can be useful for change detection or optimization.
         *
         *  @tparam T Type of the variable. Must support comparison via operator !=
         *            and copy assignment.
         *  @param var Reference to the variable to update.
         *  @param value The new value to assign.
         *  @return true if the variable was updated (value differed).
         *  @return false if the variable was already equal to the given value.
         *
         *  @note This function is noexcept assuming comparison and assignment
         *        for type T are noexcept.
         */
        template<typename T>
        static inline bool setIfChanged(T& var, const T& value) noexcept {
            static_assert(std::is_fundamental_v<T>, "setIfChanged requires fundamental types.");
            if (value != var) {
                var = value;
                return true;
            }
            return false;
        }

        /**
         * @brief Returns the minimum or maximum of two or three numeric values.
         *
         * These constexpr templated functions compute the smallest or largest of two or
         * three values of any arithmetic type (e.g., `int`, `float`, `double`, etc.).
         *
         * @tparam T An arithmetic type (integral or floating point).
         * @param a First value.
         * @param b Second value.
         * @param c (optional) Third value for the 3-value variants.
         * @return The smallest/largest value among the inputs.
         *
         * @note Requires that the type supports `<` or `>` comparison.
         */
        template <typename T>
        [[nodiscard]] static inline constexpr T minOf(T a, T b) noexcept {
            static_assert(std::is_arithmetic_v<T>, "minOf requires a numeric type");
            return a < b ? a : b;
        }

        template <typename T>
        [[nodiscard]] static inline constexpr T minOf3(T a, T b, T c) noexcept {
            return minOf(minOf(a, b), c);
        }

        template <typename T>
        [[nodiscard]] static inline constexpr T maxOf(T a, T b) noexcept {
            static_assert(std::is_arithmetic_v<T>, "maxOf requires a numeric type");
            return a > b ? a : b;
        }

        template <typename T>
        [[nodiscard]] static inline constexpr T maxOf3(T a, T b, T c) noexcept {
            return maxOf(maxOf(a, b), c);
        }

        /**
         *  @brief Finds the minimum value in an array.
         *
         *  @param ptr Pointer to the array of values.
         *  @param count Number of elements in the array.
         *  @return The minimum value in the array, or the highest possible
         *          value if the input is invalid.
         */
        template <typename T>
        [[nodiscard]] static T minOfArray(const T* ptr, int64_t count) noexcept {
            static_assert(std::is_arithmetic_v<T>, "minOfArray requires a numeric type");

            T min = std::numeric_limits<T>::max();
            if (ptr && count > 0) {
                while (count--) {
                    if (*ptr < min) {
                        min = *ptr;
                    }
                    ++ptr;
                }
            }
            return min;
        }

        /**
         *  @brief Finds the maximum value in an array of numeric values.
         *
         *  @tparam T An arithmetic type (e.g., int, float, double).
         *  @param ptr Pointer to the array of values.
         *  @param count Number of elements in the array.
         *  @return The maximum value in the array, or the lowest possible
         *          value of type T if the input is invalid.
         */
        template <typename T>
        [[nodiscard]] static T maxOfArray(const T* ptr, int64_t count) noexcept {
            static_assert(std::is_arithmetic_v<T>, "maxOfArray requires a numeric type");

            T max = std::numeric_limits<T>::lowest();
            if (ptr && count > 0) {
                while (count--) {
                    if (*ptr > max) {
                        max = *ptr;
                    }
                    ++ptr;
                }
            }
            return max;
        }

        /**
         *  @brief Computes the maximum absolute value in an array of numeric values.
         *
         *  @tparam T An arithmetic type (e.g., int, float, double).
         *  @param ptr Pointer to the array of values.
         *  @param count Number of elements in the array.
         *  @return The maximum absolute value in the array, or T{0} if input is invalid.
         */
        template <typename T>
        [[nodiscard]] static T absMaxOfArray(const T* ptr, int64_t count) noexcept {
            static_assert(std::is_arithmetic_v<T>, "absMaxOfArray requires a numeric type");

            if (!ptr || count == 0) {
                return T{0};
            }

            T max = T{0};
            while (count--) {
                T v = *ptr++;
                if (v < T{0}) {
                    v = -v;
                }
                if (v > max) {
                    max = v;
                }
            }
            return max;
        }

        /**
         *  @brief Scales each element in an array by a constant factor.
         *
         *  @tparam T An arithmetic type (e.g., float, double, int).
         *  @param ptr Pointer to the array of values to be scaled.
         *  @param count Number of elements in the array.
         *  @param scale Scalar value by which each element will be multiplied.
         *
         *  @note If `ptr` is `nullptr` or `count` is zero, no operation is performed.
         */
        template <typename T>
        static void scaleArray(T* ptr, int64_t count, T scale) noexcept {
            static_assert(std::is_arithmetic_v<T>, "scaleArray requires an arithmetic type");

            if (ptr && count > 0) {
                while (count--) {
                    *ptr++ *= scale;
                }
            }
        }

        /**
         *  @brief Normalizes an array to the [0, 1] range.
         *
         *  @tparam T Must be float or double.
         *  @param ptr Pointer to the array.
         *  @param count Number of elements.
         *
         *  @note If all values are equal or range is zero, the function does nothing.
         */
        template <typename T>
        static void normalizeArrayToUnitRange(T* ptr, int64_t count) noexcept {
            static_assert(std::is_same_v<T, float> || std::is_same_v<T, double>,
                    "normalizeArrayToUnitRange requires float or double");

            if (!ptr || count <= 0) return;

            T min = minOfArray(ptr, static_cast<std::size_t>(count));
            T max = maxOfArray(ptr, static_cast<std::size_t>(count));

            T range = max - min;
            if (range == T(0)) return;  // Avoid divide-by-zero

            for (int64_t i = 0; i < count; ++i) {
                ptr[i] = (ptr[i] - min) / range;
            }
        }

        /**
         *  @brief Scales each element in a data array by the corresponding value in a factors array.
         *
         *  @tparam T An arithmetic type (e.g., float, double).
         *  @param dst Pointer to the data array to be scaled.
         *  @param factors Pointer to the array of scaleFrom factors.
         *  @param count Number of elements in both arrays.
         *
         *  @note If either `dst` or `factors` is `nullptr`, or if `count` is zero,
         *        the function performs no operation.
         */
        template <typename T>
        static void scaleArray(T* ptr, const T* factors, int64_t count) noexcept {
            static_assert(std::is_arithmetic_v<T>, "scaleArray requires an arithmetic type");

            if (ptr && factors) {
                for (int64_t i = 0; i < count; i++) {
                    *ptr++ *= *factors++;
                }
            }
        }

        /**
         *  @brief Clears an array by setting all its elements' bytes to zero.
         *
         *  This function zeroes out the memory occupied by an array of trivially constructible
         *  elements (e.g., integers, floats, plain structs) by using `std::memset`.
         *
         *  @tparam T The element type of the array. Must be a trivial type.
         *  @param ptr Pointer to the first element of the array.
         *  @param count Number of elements to clear (not bytes).
         *
         *  @note If `ptr` is `nullptr` or `count` is zero, the function does nothing.
         *  @warning This performs a raw byte-level zeroing. For non-trivial types,
         *           use appropriate constructors or initialization mechanisms instead.
         */
        template <typename T>
        static void clearArray(T* ptr, int64_t count) noexcept {
            static_assert(std::is_trivial_v<T>, "clearArray requires a trivial type");

            if (ptr && count > 0) {
                std::memset(ptr, 0, count * sizeof(T));
            }
        }

        /**
         *  @brief Fills elements in a strided array with a given value.
         *
         *  @tparam T Element type (arithmetic or bool).
         *  @param ptr Pointer to the array start.
         *  @param offset Starting index offset in the array.
         *  @param stride Stride (step) between filled elements.
         *  @param count Number of elements to fill.
         *  @param capacity Total capacity of the array (to check bounds).
         *  @param value Value to assign.
         *
         *  @note If `ptr` is nullptr, `count` is zero, or bounds are exceeded, no operation is performed.
         */
        template <typename T>
        static void fillStridedArray(
            T* ptr,
            int64_t offset,
            int32_t stride,
            int64_t count,
            int64_t capacity,
            T value
        ) noexcept {
            static_assert(std::is_arithmetic_v<T> || std::is_same_v<T, bool>,
                          "fillStridedArray requires an arithmetic or bool type");

            if (!ptr || capacity < 1 || count < 1 || stride < 1) return;

            // Adjust count if it would go out of bounds
            int64_t max_index = offset + (count - 1) * stride;
            if (max_index >= capacity) {
                if (stride == 0) {
                    // stride=0 means repeatedly fill the same element, so only 1 fill possible
                    count = 1;
                }
                else if (offset >= capacity) {
                    // offset itself out of bounds, do nothing
                    return;
                }
                else {
                    // calculate max valid count that fits into capacity
                    count = (capacity - 1 - offset) / stride + 1;
                }
            }

            for (int64_t i = 0; i < count; ++i) {
                ptr[offset] = value;
                offset += stride;
            }
        }

        /**
         *  @brief Reverses the elements of an array in place.
         *
         *  This templated function reverses the order of elements in the array,
         *  swapping the first with the last, the second with the second-last, and so on.
         *
         *  @tparam T The type of elements in the array.
         *  @param ptr Pointer to the array to be reversed.
         *  @param count Number of elements in the array.
         *
         *  @note If `ptr` is `nullptr` or `count` is less than or equal to 1, the function does nothing.
         */
        template <typename T>
        static void flipArray(T* ptr, int64_t count) noexcept {
            if (!ptr || count <= 1) return;

            T* left = ptr;
            T* right = ptr + count - 1;

            while (left < right) {
                std::swap(*left, *right);
                ++left;
                --right;
            }
        }

        /**
         * @brief Swap two variables.
         *
         * @param[in,out] a The first variable.
         * @param[in,out] b The second variable.
         */
        template <typename T>
        static bool swapIfGreater(T& a, T& b) noexcept {
            if (a > b) {
                std::swap(a, b);
                return true;
            }
            return false;
        }

        [[nodiscard]] static constexpr int16_t swapBytesInt16(int16_t value) noexcept {
            auto uvalue = static_cast<uint16_t>(value);
            uvalue = (uvalue << 8) | (uvalue >> 8);
            return static_cast<int16_t>(uvalue);
        }

        [[nodiscard]] static constexpr uint16_t swapBytesUInt16(uint16_t value) noexcept {
            return (value << 8) | (value >> 8);
        }

        [[nodiscard]] static constexpr int32_t swapBytesInt32(int32_t value) noexcept {
            auto uvalue = static_cast<uint32_t>(value);
            uvalue = ((uvalue & 0x000000FFu) << 24) |
                     ((uvalue & 0x0000FF00u) << 8)  |
                     ((uvalue & 0x00FF0000u) >> 8)  |
                     ((uvalue & 0xFF000000u) >> 24);
            return static_cast<int32_t>(uvalue);
        }

        [[nodiscard]] static constexpr uint32_t swapBytesUInt32(uint32_t value) noexcept {
            return ((value & 0x000000FFu) << 24) |
                   ((value & 0x0000FF00u) << 8)  |
                   ((value & 0x00FF0000u) >> 8)  |
                   ((value & 0xFF000000u) >> 24);
        }

        [[nodiscard]] static constexpr int64_t swapBytesInt64(int64_t value) noexcept {
            auto uvalue = static_cast<uint64_t>(value);
            uvalue = ((uvalue & 0x00000000000000FFull) << 56) |
                     ((uvalue & 0x000000000000FF00ull) << 40) |
                     ((uvalue & 0x0000000000FF0000ull) << 24) |
                     ((uvalue & 0x00000000FF000000ull) << 8)  |
                     ((uvalue & 0x000000FF00000000ull) >> 8)  |
                     ((uvalue & 0x0000FF0000000000ull) >> 24) |
                     ((uvalue & 0x00FF000000000000ull) >> 40) |
                     ((uvalue & 0xFF00000000000000ull) >> 56);
            return static_cast<int64_t>(uvalue);
        }

        [[nodiscard]] static constexpr uint64_t swapBytesUInt64(uint64_t value) noexcept {
            return ((value & 0x00000000000000FFull) << 56) |
                   ((value & 0x000000000000FF00ull) << 40) |
                   ((value & 0x0000000000FF0000ull) << 24) |
                   ((value & 0x00000000FF000000ull) << 8)  |
                   ((value & 0x000000FF00000000ull) >> 8)  |
                   ((value & 0x0000FF0000000000ull) >> 24) |
                   ((value & 0x00FF000000000000ull) >> 40) |
                   ((value & 0xFF00000000000000ull) >> 56);
        }

        /**
         *  @brief Reverses the bit order of the low nibble (4 bits) of an 8-bit unsigned integer.
         *
         *  This function reflects the least significant 4 bits of the input value.
         *  For example, the binary value `0b0110` (6) becomes `0b0110` (6),
         *  and `0b1101` (13) becomes `0b1011` (11).
         *
         *  @param value An 8-bit unsigned integer whose low nibble will be reflected.
         *  @return A new 8-bit unsigned integer with the low nibble bits reversed;
         *          the upper nibble (high 4 bits) is set to zero.
         */
        [[nodiscard]] static constexpr uint8_t reflectLowNibble(uint8_t value) noexcept {
            return (value & 0xF0) | // Keep upper nibble unchanged
                   (((value & 0x1) << 3) |
                    ((value & 0x2) << 1) |
                    ((value & 0x4) >> 1) |
                    ((value & 0x8) >> 3));
        }

        /**
         *  @brief Converts a little-endian encoded 16-bit unsigned integer to a
         *         normalized float.
         *
         *  This function interprets a 16-bit unsigned integer as little-endian and
         *  converts it to a floating-point value in the range [0.0, 1.0], dividing
         *  by `std::numeric_limits<uint16_t>::max()`.
         *
         *  @param value The 16-bit unsigned integer in little-endian byte order.
         *  @return A float in the range [0.0, 1.0].
         */
        [[nodiscard]] static constexpr float litteEndianUInt16ToFloat(uint16_t value) noexcept {
            return static_cast<float>((value >> 8) | ((value & 0xFF) << 8)) / std::numeric_limits<uint16_t>::max();
        }

        /**
         *  @brief Converts a normalized float in the range [0.0, 1.0] to a 16-bit
         *         little-endian unsigned integer.
         *
         *  This function clamps the input float to [0.0, 1.0], scales it to the
         *  `uint16_t` range, and encodes the result in little-endian byte order.
         *
         *  @param value A float in the range [0.0, 1.0].
         *  @return A 16-bit unsigned integer with its bytes arranged in
         *          little-endian order.
         */
        [[nodiscard]] static constexpr uint16_t floatToLitteEndianUInt16(float value) noexcept {
            if (value < 0.0f) {
                value = 0.0f;
            }
            else if (value > 1.0f) {
                value = 1.0f;
            }
            auto i = static_cast<uint16_t>(value * std::numeric_limits<uint16_t>::max());
            return (i >> 8) | ((i & 0xFF) << 8);
        }

        /**
         * @brief Clamps the elements of an array to a specified range.
         *
         * This function iterates through an array and ensures that each element is
         * within the provided minimum and maximum bounds. If an element is less
         * than the specified minimum, it is set to the minimum value; if it is
         * greater than the maximum, it is set to the maximum value.
         *
         * @tparam T The type of the array elements. It must support comparison
         *           with `min` and `max`, and assignment.
         *
         * @param data Pointer to the array of values to be clamped.
         * @param length The number of elements in the array.
         * @param min The minimum value that elements can take.
         * @param max The maximum value that elements can take.
         *
         * @note If the `data` pointer is `nullptr`, the function does nothing.
         */
        template <typename T>
        static void clampData(T* data, int64_t length, const T& min, const T& max) {
            if (data) {
                for (int64_t i = 0; i < length; ++i) {
                    data[i] = std::clamp(data[i], min, max);
                }
            }
        }

        [[nodiscard]] static constexpr fourcc_t fourcc(char c1, char c2, char c3, char c4) {
            return (static_cast<fourcc_t>(static_cast<unsigned char>(c1)) << 24) |
                   (static_cast<fourcc_t>(static_cast<unsigned char>(c2)) << 16) |
                   (static_cast<fourcc_t>(static_cast<unsigned char>(c3)) << 8)  |
                   (static_cast<fourcc_t>(static_cast<unsigned char>(c4)));
        }

        [[nodiscard]] static fourcc_t fourCCFromStr(const char* str) noexcept {
            if (!str || !str[0] || !str[1] || !str[2] || !str[3]) {
                return 0;
            }
            return (static_cast<fourcc_t>(static_cast<unsigned char>(str[0])) << 24) |
                   (static_cast<fourcc_t>(static_cast<unsigned char>(str[1])) << 16) |
                   (static_cast<fourcc_t>(static_cast<unsigned char>(str[2])) << 8)  |
                   (static_cast<fourcc_t>(static_cast<unsigned char>(str[3])));
        }

        static const char* fourCCToStr(fourcc_t value, char* out_str) noexcept {
            if (!out_str) return nullptr;

            // Always extract by shifting — no aliasing issues, works on any endianness
            for (int i = 0; i < 4; ++i) {
                unsigned char ch = static_cast<unsigned char>((value >> (8 * (3 - i))) & 0xFF);
                out_str[i] = (ch >= 32) ? static_cast<char>(ch) : ' ';
            }
            out_str[4] = '\0';
            return out_str;
        }

        /**
         *  @brief Adjusts the bounds for copying data between arrays.
         *
         *  This function validates and adjusts the source and destination offsets for
         *  copying `length` elements from a source array to a destination array,
         *  ensuring no out-of-bounds memory access occurs.
         *
         *  @tparam T Signed integral type (e.g., int32_t, int64_t).
         *  @param length          Number of elements requested to copy.
         *  @param src_size        Total number of elements in the source array.
         *  @param src_offset      Starting offset in the source array.
         *  @param dst_size        Total number of elements in the destination array.
         *  @param dst_offset      Starting offset in the destination array.
         *  @param out_src_offset  Output adjusted source offset (clamped to bounds).
         *  @param out_dst_offset  Output adjusted destination offset (clamped to bounds).
         *
         *  @return Adjusted copy length (may be less than requested), or 0 if invalid.
         */
        template <typename T>
        static T computeValidCopyRegion(
                T length, T src_size, T src_offset, T dst_size, T dst_offset, T& out_src_offset, T& out_dst_offset
                ) noexcept {
            static_assert(std::is_integral<T>::value, "T must be an integral type");
            static_assert(std::is_signed<T>::value, "T must be signed");

            out_src_offset = out_dst_offset = 0;

            if (length < 1 || src_size < 1 || dst_size < 1 ||
                dst_offset >= dst_size || (dst_offset + length - 1) < 0) {
                return 0;
            }

            if (src_offset < 0) {
                T n = -src_offset;
                length -= n;
                dst_offset += n;
                src_offset = 0;
                if (length < 1) return 0;
            }

            if (src_offset + length > src_size) {
                length = src_size - src_offset;
                if (length < 1) return 0;
            }

            if (dst_offset < 0) {
                T n = -dst_offset;
                length -= n;
                src_offset += n;
                dst_offset = 0;
                if (length < 1) return 0;
            }

            if (dst_offset + length > dst_size) {
                length = dst_size - dst_offset;
                if (length < 1) return 0;
            }

            out_src_offset = src_offset;
            out_dst_offset = dst_offset;
            return length;
        }

        template <typename T>
        static bool isValidCopyRegion(
                T length, T src_size, T src_offset, T dst_size, T dst_offset
        ) noexcept {
            static_assert(std::is_integral<T>::value, "T must be an integral type");
            static_assert(std::is_signed<T>::value, "T must be signed");
            T new_src_offset;
            T out_dst_offset;
            return computeValidCopyRegion<T>(length, src_size, src_offset, dst_size, dst_offset, new_src_offset, out_dst_offset) > 0;
        }

            /**
             *  @brief Checks if a value is within a specified range (inclusive).
             *
             *  This function determines whether the value `a` lies within the range
             *  defined by `b` and `c`. The range is inclusive, meaning `a` may equal
             *  either `b` or `c`. It also handles cases where the range is reversed
             *  (i.e., `b > c`).
             *
             *  @tparam T A type that supports relational operators (`<`, `<=`, `>=`).
             *  @param a The value to check.
             *  @param b One bound of the range.
             *  @param c The other bound of the range.
             *
             *  @return `true` if `a` is within the range `[b, c]` or `[c, b]`; `false` otherwise.
             */
        template <typename T>
        [[nodiscard]] static inline bool isValueInRange(T a, T b, T c) {
            return a >= std::min<T>(b, c) && a <= std::max<T>(b, c);
        }

        /**
         *  @brief Wraps an integer value within a specified inclusive range.
         *
         *  This function wraps the input `value` into the inclusive range `[min, max]`.
         *  If the value exceeds the maximum, it wraps around to the minimum, and vice versa.
         *  The behavior is similar to modular arithmetic applied over a custom range.
         *
         *  @tparam T An integral type (e.g., int32_t, int64_t).
         *  @param value The value to wrap.
         *  @param min The lower bound of the range (inclusive).
         *  @param max The upper bound of the range (inclusive).
         *  @return The wrapped value within the specified range `[min, max]`.
         */
        template <typename T>
        [[nodiscard]] static inline T wrappedValue(
                std::type_identity_t<T> value,
                std::type_identity_t<T> min,
                std::type_identity_t<T> max) noexcept
        {
            static_assert(std::is_arithmetic_v<T>, "wrappedValue requires an arithmetic type");

            if (min > max) return min;

            if constexpr (std::is_integral_v<T>) {
                const T range = max - min + 1;
                if (range <= 0) return min;

                if constexpr (std::is_unsigned_v<T>) {
                    int64_t diff = static_cast<int64_t>(value) - static_cast<int64_t>(min);
                    int64_t wrapped = (diff % range + range) % range; // ensure non-negative modulo
                    return static_cast<T>(min + wrapped);
                } else {
                    T t = (value - min) % range;
                    if (t < 0) t += range;
                    return t + min;
                }
            }
            else if constexpr (std::is_floating_point_v<T>) {
                const T range = max - min;
                if (range <= T(0)) return min;

                T t = std::fmod(value - min, range);
                if (t < T(0)) t += range;
                return t + min;
            }
        }

        template <typename T>
        [[nodiscard]] static inline T wrappedBipolarPI(
                typename std::type_identity<T>::type value) noexcept {
            static_assert(std::is_floating_point<T>::value, "wrappedBipolarPI requires a floating-point type");
            constexpr T range = static_cast<T>(std::numbers::pi * 2);
            T t = std::fmod(value - static_cast<T>(-std::numbers::pi), range);
            if (t < T(0)) t += range;
            return t + static_cast<T>(-std::numbers::pi);
        }

        /**
         * @brief Wraps a floating-point value within the range [0.0, 1.0].
         *
         * This function wraps a floating-point value `value` within the range [0.0, 1.0],
         * using modular arithmetic. Works for both float and double types.
         *
         * @tparam T Must be a floating-point type (`float`, `double`, or `long double`).
         * @param value The value to wrap.
         * @return The wrapped value within the range [0.0, 1.0].
         */
        template <typename T>
        [[nodiscard]] static inline T wrappedUnit(T value) noexcept {
            static_assert(std::is_floating_point<T>::value, "wrappedUnit requires a floating-point type");
            T t = std::fmod(value, static_cast<T>(1.0));
            return t < static_cast<T>(0.0) ? t + static_cast<T>(1.0) : t;
        }

        /**
         * @brief Convert a normalized float [0.0, 1.0] to an 8-bit unsigned integer [0, 255].
         *
         * Values below 0.0 are clamped to 0; above 1.0 are clamped to 255.
         * The multiplier (255.996f) ensures rounding covers the full range.
         */
        [[nodiscard]] static inline uint8_t floatToUInt8(float value) noexcept {
            return value <= 0.0f ? 0 : value >= 1.0f ? 255 : static_cast<uint8_t>(value * 255.996f);
        }

        /**
         * @brief Convert a normalized float [0.0, 1.0] to a 16-bit unsigned integer [0, 65535].
         */
        [[nodiscard]] static inline uint16_t floatToUInt16(float value) noexcept {
            return value <= 0.0f ? 0 : value >= 1.0f ? 65535 : static_cast<uint16_t>(value * 65534.976f);
        }

        /**
         * @brief Convert a normalized float [0.0, 1.0] to a 24-bit unsigned integer [0, 16777215].
         */
        [[nodiscard]] static inline uint32_t floatToUInt24(float value) noexcept {
            return value <= 0.0f ? 0 : value >= 1.0f ? 16777215 : static_cast<uint32_t>(value * 16776953.856f);
        }

        [[nodiscard]] static uint64_t kilobytesToBytes(uint64_t kilobytes) noexcept {
            return kilobytes * 1024ULL;
        }
        [[nodiscard]] static uint64_t megabytesToBytes(uint64_t megabytes) noexcept {
            return megabytes * 1048576ULL; // 1024*1024
        }
        [[nodiscard]] static uint64_t gigabytesToBytes(uint64_t gigabytes) noexcept {
            return gigabytes * 1073741824ULL; // 1024^3
        }
        [[nodiscard]] static uint64_t terabytesToBytes(uint64_t terabytes) noexcept {
            return terabytes * 1099511627776ULL; // 1024^4
        }
        [[nodiscard]] static uint64_t petabytesToBytes(uint64_t petabytes) noexcept {
            return petabytes * 1125899906842624ULL; // 1024^5
        }
        [[nodiscard]] static double bytesToKilobytes(uint64_t bytes) noexcept {
            return static_cast<double>(bytes) / 1024.0;
        }
        [[nodiscard]] static double bytesToMegabytes(uint64_t bytes) noexcept {
            return static_cast<double>(bytes) / 1048576.0;
        }
        [[nodiscard]] static double bytesToGigabytes(uint64_t bytes) noexcept {
            return static_cast<double>(bytes) / 1073741824.0;
        }
        [[nodiscard]] static double bytesToTerabytes(uint64_t bytes) noexcept {
            return static_cast<double>(bytes) / 1099511627776.0;
        }
        [[nodiscard]] static double bytesToPetabytes(uint64_t bytes) noexcept {
            return static_cast<double>(bytes) / 1125899906842624.0;
        }

        static ErrorCode hexToUint32(const char* hex, Endianess endianess, uint32_t& out_value) noexcept;

        [[nodiscard]] static const char* boolToYesNoStr(bool v) noexcept { return v ? "yes" : "no"; }
        [[nodiscard]] static const char* boolToTrueFalseStr(bool v) noexcept { return v ? "true" : "false"; }
    };


    /**
     *  @struct TypeDescription
     *  @brief A structure that describes a data type.
     *
     *  The `TypeDescription` structure holds information about a specific type,
     *  including its size in bytes and bits, whether it is a floating-point or
     *  integer type, and its name.
     */
    struct TypeDescription {
        DataType m_type;         ///< The type classification (e.g., Int32, Float)
        int32_t m_bytes;            ///< The size of the type in bytes
        int32_t m_bits;             ///< The size of the type in bits
        bool m_floating_point_flag; ///< Flag indicating whether the type is a floating-point type
        bool m_integer_flag;        ///< Flag indicating whether the type is an integer type
        const char* m_name;         ///< The name of the type as a string
    };

    /**
     *  @brief Returns the name of the class for a given type.
     *
     *  This function template returns the name of the class (or type) of the template parameter `T`.
     *  It checks the type of `T` and returns the corresponding string name. For example, if `T` is
     *  `int32_t`, it will return `"int32_t"`, or `"float"` for a `float` type. If the type is not recognized,
     *  it returns `"unknown"`.
     *
     *  @tparam T The type whose class name is being returned.
     *
     *  @return The name of the type as a string.
     */
    template <typename T>
    [[nodiscard]] static const char* typeClassName() {
        if (std::is_same<T, int8_t>::value) { return "int8_t"; }
        else if (std::is_same<T, int16_t>::value) { return "int16_t"; }
        else if (std::is_same<T, int32_t>::value) { return "int32_t"; }
        else if (std::is_same<T, int64_t>::value) { return "int64_t"; }
        else if (std::is_same<T, uint8_t>::value) { return "uint8_t"; }
        else if (std::is_same<T, uint16_t>::value) { return "uint16_t"; }
        else if (std::is_same<T, uint32_t>::value) { return "uint32_t"; }
        else if (std::is_same<T, uint64_t>::value) { return "uint64_t"; }
        else if (std::is_same<T, float>::value) { return "float"; }
        else if (std::is_same<T, double>::value) { return "double"; }
        else if (std::is_pointer<T>::value) { return "pointer"; }
        else { return "unknown"; }
    }


    /**
     *  @brief Data types.
     *
     *  @note Changes here must reflect in methods using this enum.
     */
    class TypeInfo {
    protected:
        /**
         *  @brief The total number of supported types.
         *
         *  This constant defines the number of different types supported by `TypeInfo`.
         */
        enum { kTypeCount = 15 };

        /**
         *  @brief A static table of type descriptions.
         *
         *  This table holds `TypeDescription` objects for each supported type,
         *  which describe their size, bit count, classification, and name.
         */
        static TypeDescription g_type_description_table[kTypeCount];

    public:
        /**
        * @brief Checks whether the given type is valid.
        */
        [[nodiscard]] static bool isValid(DataType t) noexcept {
            auto index = static_cast<int>(t);
            return index >= 0 && index < kTypeCount;
        }

        /**
     * @brief Gets the byte size of the given type.
     */
        [[nodiscard]] static int32_t byteSize(DataType t) noexcept {
            int index = static_cast<int>(t);
            return isValid(t) ? g_type_description_table[index].m_bytes : 0;
        }

        /**
         * @brief Gets the bit count of the given type.
         */
        [[nodiscard]] static int32_t bitCount(DataType t) noexcept {
            int index = static_cast<int>(t);
            return isValid(t) ? g_type_description_table[index].m_bits : 0;
        }

        /**
         * @brief Checks whether the given type is an integer type.
         */
        [[nodiscard]] static bool isInteger(DataType t) noexcept {
            int index = static_cast<int>(t);
            return isValid(t) ? g_type_description_table[index].m_integer_flag : false;
        }

        /**
         * @brief Checks whether the given type is a floating-point type.
         */
        [[nodiscard]] static bool isFloatingPoint(DataType t) noexcept {
            int index = static_cast<int>(t);
            return isValid(t) ? g_type_description_table[index].m_floating_point_flag : false;
        }

        /**
         * @brief Gets the name of the given type.
         */
        [[nodiscard]] static const char* name(DataType t) noexcept {
            int index = static_cast<int>(t);
            if (isValid(t)) {
                return g_type_description_table[index].m_name;
            }
            return g_type_description_table[static_cast<int>(DataType::Undefined)].m_name;
        }

        /**
         * @brief Gets the type corresponding to the given name.
         */
        [[nodiscard]] static DataType typeByName(const char* name) noexcept {
            if (!name) {
                return DataType::Undefined;
            }

            for (int i = 0; i < kTypeCount; ++i) {
                if (std::strcmp(name, g_type_description_table[i].m_name) == 0) {
                    return g_type_description_table[i].m_type;
                }
            }

            return DataType::Undefined;
        }
    };


} // End of namespace Grain

#endif // GrainType_hpp
