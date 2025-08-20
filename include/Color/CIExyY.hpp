//
//  CIExyY.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//


#ifndef GrainCIExyY_hpp
#define GrainCIExyY_hpp

#include "Grain.hpp"
#include "Color/Color.hpp"
#include "Math/Vec2.hpp"


namespace Grain {


    class RGB;
    class CIEXYZ;


/**
 *  @brief CIE xyY Color.
 *
 *  The CIE xyY color space represents colors using three components:
 *  - x: The chromaticity coordinate along the red-green axis.
 *  - y: The chromaticity coordinate along the blue-yellow axis.
 *  - Y: The luminance or brightness of the color.
 *
 *  CIE xyY is used to describe color perception independently of luminance. It provides a compact
 *  representation for color analysis and allows color transformations between different color spaces.
 */
    class CIExyY {

    public:
        Vec2f m_pos{};
        float m_y = 0.0f;

    public:
        CIExyY() noexcept {}
        CIExyY(float x, float y, float Y) noexcept : m_pos(x, y), m_y(Y) {}
        CIExyY(const RGB& rgb) noexcept;
        CIExyY(const CIEXYZ &xyz) noexcept;

        ~CIExyY() noexcept {}

        virtual const char *className() const noexcept { return "CIExyY"; }

        friend std::ostream& operator << (std::ostream &os, const CIExyY *o) {
            o == nullptr ? os << "CIExyY nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream &os, const CIExyY &o) {
            os << o.m_pos << ", " << o.m_y;
            return os;
        }


        bool operator == (const CIExyY &v) const {
            return m_pos.m_x == v.m_pos.m_x && m_pos.m_y == v.m_pos.m_y && m_y == v.m_y;
        }

        bool operator != (const CIExyY &v) const {
            return m_pos.m_x != v.m_pos.m_x || m_pos.m_y != v.m_pos.m_y || m_y == v.m_y;
        }


        Vec2f pos() const noexcept { return m_pos; }
        float yValue() const noexcept { return m_y; }

        void setPos(Vec2f &pos) noexcept { m_pos = pos; }
        void setY(float y) noexcept { m_y = y; }

        void set(float x, float y, float Y) noexcept { m_pos.m_x = x; m_pos.m_y = y; m_y = Y; }
        void set(const RGB& rgb) noexcept;
        void set(const CIEXYZ &xyz) noexcept;

        void translate(float tx, float ty) noexcept { m_pos.m_x += tx; m_pos.m_y += ty; }
        void rotate(const Vec2f &pivot, float deg) noexcept;

        CIExyY blend(const CIExyY &xyz, float t) noexcept {
            if (t < 0.0f) t = 0.0f; else if (t > 1.0f) t = 1.0f;
            float ti = 1.0f - t;
            CIExyY result;
            result.m_pos.m_x = m_pos.m_x * ti + xyz.m_pos.m_x * t;
            result.m_pos.m_y = m_pos.m_y * ti + xyz.m_pos.m_y * t;
            result.m_y = m_y * ti + xyz.m_y * t;
            return result;
        }
    };


} // End of namespace Grain

#endif // GrainCIExyY_hpp
