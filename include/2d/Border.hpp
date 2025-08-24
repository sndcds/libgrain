//
//  Border.hpp
//
//  Created by Roald Christesen on from 24.01.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 24.08.2025
//

#ifndef GrainBorder_hpp
#define GrainBorder_hpp

#include <cstdint>
#include <iostream>


namespace Grain {

    template <class T>
    class Border {

    public:
        T m_top = 0;
        T m_right = 0;
        T m_bottom = 0;
        T m_left = 0;

    public:
        Border() = default;

        explicit Border(T top, T right, T bottom, T left) :
            m_top(top), m_right(right), m_bottom(bottom), m_left(left) {}

        explicit Border(T horizontal, T vertical) :
            m_top(vertical), m_right(horizontal), m_bottom(vertical), m_left(horizontal) {}

        explicit Border(T size) :
            m_top(size), m_right(size), m_bottom(size), m_left(size) {}

        friend std::ostream& operator << (std::ostream& os, const Border& o) {
            os << o.m_top << ", " << o.m_right << ", " << o.m_bottom << ", " << o.m_left;
            return os;
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
    };


    // Standard types
    using Borderi = Border<int32_t>;    ///< 32 bit integer
    using Borderl = Border<int64_t>;    ///< 64 bit integer
    using Borderf = Border<float>;      ///< 32 bit floating point
    using Borderd = Border<double>;     ///< 64 bit floating point


} // End of namespace Grain

#endif // GrainBorder_hpp
