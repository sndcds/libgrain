//
//  RectEdges.hpp
//
//  Created by Roald Christesen on 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 12.07.2025
//

#ifndef GrainRectEdges_hpp
#define GrainRectEdges_hpp

#include <cstdint>
#include <iostream>


namespace Grain {

    /**
     *  @brief A templated class representing rect edge dimensions (top, right, bottom, left).
     *
     *  The RectEdges class provides a convenient way to define, access, and manipulate rect edge sizes
     *  (e.g., for UI margins, padding, layout spacing, etc.) with support for uniform and directional initialization.
     *
     *  @tparam T The numeric type used for the edge values (e.g., int, float, double).
     */
    template <class T>
    class RectEdges {
    public:
        RectEdges() noexcept = default;
        explicit RectEdges(T top, T right, T bottom, T left) noexcept : m_top(top), m_right(right), m_bottom(bottom), m_left(left) {}
        explicit RectEdges(T horizontal, T vertical) noexcept : m_top(vertical), m_right(horizontal), m_bottom(vertical), m_left(horizontal) {}
        explicit RectEdges(T size) noexcept : m_top(size), m_right(size), m_bottom(size), m_left(size) {}
        virtual ~RectEdges() = default;

        friend std::ostream& operator << (std::ostream& os, const RectEdges& o) {
            return os << o.m_top << ", " << o.m_right << ", " << o.m_bottom << ", " << o.m_left;
        }


        [[nodiscard]] T left() const noexcept { return m_left; }
        [[nodiscard]] T right() const noexcept { return m_right; }
        [[nodiscard]] T top() const noexcept { return m_top; }
        [[nodiscard]] T bottom() const noexcept { return m_bottom; }
        [[nodiscard]] T width() const noexcept { return m_left + m_right; }
        [[nodiscard]] T height() const noexcept { return m_top + m_bottom; }

        void set(T size) noexcept {
            m_top = m_right = m_bottom = m_left = size;
        }

        void set(T vertical, T horizontal) noexcept {
            m_top = m_bottom = vertical;
            m_right = m_left = horizontal;
        }

        void set(T top, T right, T bottom, T left) noexcept {
            m_top = top;
            m_right = right;
            m_bottom = bottom;
            m_left = left;
        }

        bool set(T* values, int32_t n) noexcept {
            if (values != nullptr) {
                if (n == 1) {
                    m_top = m_right = m_bottom = m_left = values[0];
                    return true;
                }
                else if (n == 2) {
                    m_top = m_bottom = values[0];
                    m_right = m_left = values[1];
                    return true;
                }
                else if (n == 4) {
                    m_top = values[0];
                    m_right = values[1];
                    m_bottom = values[2];
                    m_left = values[3];
                    return true;
                }
            }
            return false;
        }

    public:
        T m_top{};
        T m_right{};
        T m_bottom{};
        T m_left{};
    };


    // Standard types.
    using RectEdgesi = RectEdges<int32_t>;    ///< 32 bit integer.
    using RectEdgesl = RectEdges<int64_t>;    ///< 64 bit integer.
    using RectEdgesf = RectEdges<float>;      ///< 32 bit floating point.
    using RectEdgesd = RectEdges<double>;     ///< 64 bit floating point.


} // End of namespace Grain

#endif // GrainRectEdges_hpp
