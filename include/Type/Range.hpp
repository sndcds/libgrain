//
//  Range.hpp
//
//  Created by Roald Christesen on from 20.07.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 12.07.2025
//

#ifndef GrainRange_hpp
#define GrainRange_hpp

#include "Type/Fix.hpp"

#include <cstdint>
#include <iostream>


namespace Grain {

    template <class T>
    class Range {
    public:
        Range() noexcept = default;
        Range(T min, T max) noexcept : m_min(min), m_max(max) {}

        [[nodiscard]] virtual const char* className() const noexcept { return "Range"; }

        friend std::ostream& operator << (std::ostream& os, const Range& o) {
            return os << o.m_min << ", " << o.m_max;
        }

        bool operator == (const Range& other) const {
            return m_min == other.m_min && m_max == other.m_max;
        }

        bool operator != (const Range& other) const {
            return m_min != other.m_min || m_max != other.m_max;
        }

        /**
         *  @brief Returns the union of this range and another range.
         *
         *  Combines this range with another range `r` and returns a new `Range`
         *  that spans from the smallest minimum to the largest maximum of both ranges.
         *  This effectively produces the smallest range that fully contains both
         *  `this` and `r`.
         *
         *  @param r The other range to combine with.
         *  @return A new `Range` representing the union (maximal extent) of the two ranges.
         */
         Range operator + (const Range& other) const {
            Range result;
            result.m_min = m_min < other.m_min ? m_min : other.m_min;
            result.m_max = m_max > other.m_max ? m_max : other.m_max;
            return result;
        }

        /**
         *  @brief Expands this range to include another range.
         *
         *  Updates this range so that it spans from the smallest minimum to the largest maximum
         *  of both this and the given range `r`. This effectively computes the union of the two
         *  ranges and stores it in the currentMillis object.
         *
         *  @param r The range to merge into this one.
         *  @return Reference to this updated range.
         */
        Range& operator += (const Range& other) {
            m_min = m_min < other.m_min ? m_min : other.m_min;
            m_max = m_max > other.m_max ? m_max : other.m_max;
            return *this;
        }

        Range operator + (const T v) const {
            Range result;
            result.m_min = m_min < v ? m_min : v;
            result.m_max = m_max > v ? m_max : v;
            return result;
        }

        Range& operator += (const T v) {
            m_min = m_min < v ? m_min : v;
            m_max = m_max > v ? m_max : v;
            return *this;
        }

        [[nodiscard]] T min() const noexcept { return m_min; }
        [[nodiscard]] T max() const noexcept { return m_max; }
        [[nodiscard]] T center() const noexcept { return m_min + (m_max - m_min) / 2; }
        [[nodiscard]] T width() const noexcept { return m_max - m_min; }

        [[nodiscard]] T random() const noexcept;

        void set(T min, T max) noexcept {
            m_min = min; m_max = max;
        }

        void initForMinMaxSearch() noexcept;

        [[nodiscard]] bool contains(const T v) const noexcept {
            return v >= m_min && v <= m_max;
        }

        [[nodiscard]] bool contains(const T v, T tolerance) const noexcept {
            return v >= m_min - tolerance && v <= m_max + tolerance;
        }

        void translate(T t) noexcept {
            m_min += t;
            m_max += t;
        }

        void translateByFactor(T f) noexcept {
            T t = width() * f;
            m_min += t;
            m_max += t;
        }

        void scale(double s) noexcept {
            double center = static_cast<double>(m_min + m_max) / 2.0;
            double half_w = static_cast<double>(m_max - m_min) * s / 2.0;
            m_min = center - half_w;
            m_max = center + half_w;
        }

        void scaleFrom(T pivot, T s) noexcept {
            m_min = pivot + (m_min - pivot) * s;
            m_max = pivot + (m_max - pivot) * s;
        }

        T lerp(double t) noexcept {
            return static_cast<T>(static_cast<double>(m_min) + t * static_cast<double>(m_max - m_min));
        }

    public:
        T m_min = 0;
        T m_max = 0;
    };

    // Standard types
    using Rangei = Range<int32_t>;  ///< 32 bit integer
    using Rangel = Range<int64_t>;  ///< 64 bit integer
    using Rangef = Range<float>;    ///< 32 bit floating point
    using Ranged = Range<double>;   ///< 64 bit floating point

    // Specialized methods
    template <> void Range<int32_t>::initForMinMaxSearch() noexcept;
    template <> void Range<int64_t>::initForMinMaxSearch() noexcept;
    template <> void Range<float>::initForMinMaxSearch() noexcept;
    template <> void Range<double>::initForMinMaxSearch() noexcept;


    class RangeFix {
    public:
        RangeFix() noexcept = default;
        RangeFix(Fix min, Fix max) noexcept : m_min(min), m_max(max) {}

        [[nodiscard]] virtual const char* className() const noexcept { return "RangeFix"; }

        friend std::ostream& operator << (std::ostream& os, const RangeFix& o) {
            os << o.m_min << ", " << o.m_max;
            return os;
        }

        bool operator == (const RangeFix& other) const {
            return m_min == other.m_min && m_max == other.m_max;
        }

        bool operator != (const RangeFix& other) const {
            return m_min != other.m_min || m_max != other.m_max;
        }

        RangeFix operator + (const RangeFix& other) const {
            RangeFix result;
            result.m_min = m_min < other.m_min ? m_min : other.m_min;
            result.m_max = m_max > other.m_max ? m_max : other.m_max;
            return result;
        }

        RangeFix& operator += (const RangeFix& other) {
            m_min = m_min < other.m_min ? m_min : other.m_min;
            m_max = m_max > other.m_max ? m_max : other.m_max;
            return *this;
        }

        RangeFix operator + (const Fix& v) const {
            RangeFix result;
            result.m_min = m_min < v ? m_min : v;
            result.m_max = m_max > v ? m_max : v;
            return result;
        }

        RangeFix& operator += (const Fix& v) {
            m_min = m_min < v ? m_min : v;
            m_max = m_max > v ? m_max : v;
            return *this;
        }

        [[nodiscard]] Fix min() const noexcept { return m_min; }
        [[nodiscard]] Fix max() const noexcept { return m_max; }
        [[nodiscard]] Fix center() const noexcept { return m_min + (m_max - m_min) / 2; }
        [[nodiscard]] Fix width() const noexcept { return m_max - m_min; }

        [[nodiscard]] double random() const noexcept;

        void set(Fix min, Fix max) noexcept { m_min = min; m_max = max; }

        [[nodiscard]] bool contains(const Fix v) const noexcept {
            return v >= m_min && v <= m_max;
        }

        [[nodiscard]] bool contains(const Fix v, Fix tolerance) const noexcept {
            return v >= m_min - tolerance && v <= m_max + tolerance;
        }

        [[nodiscard]] double lerp(double t) noexcept {
            return m_min.asDouble() + t * (m_max.asDouble() - m_min.asDouble());
        }

    public:
        Fix m_min = 0;
        Fix m_max = 0;
    };

} // End of namespace Grain

#endif // GrainRange_hpp
