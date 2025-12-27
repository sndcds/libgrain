//
//  CIExyY.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
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
    float y_ = 0.0f;

public:
    CIExyY() noexcept = default;
    CIExyY(float x, float y, float Y) noexcept : m_pos(x, y), y_(Y) {}
    explicit CIExyY(const RGB& rgb) noexcept;
    explicit CIExyY(const CIEXYZ& xyz) noexcept;

    ~CIExyY() noexcept = default;

    [[nodiscard]] virtual const char *className() const noexcept { return "CIExyY"; }

    friend std::ostream& operator << (std::ostream& os, const CIExyY *o) {
        o == nullptr ? os << "CIExyY nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const CIExyY& o) {
        os << o.m_pos << ", " << o.y_;
        return os;
    }

    bool operator == (const CIExyY& v) const {
        return m_pos.x_ == v.m_pos.x_ && m_pos.y_ == v.m_pos.y_ && y_ == v.y_;
    }

    bool operator != (const CIExyY& v) const {
        return m_pos.x_ != v.m_pos.x_ || m_pos.y_ != v.m_pos.y_ || y_ == v.y_;
    }

    [[nodiscard]] Vec2f pos() const noexcept { return m_pos; }
    [[nodiscard]] float yValue() const noexcept { return y_; }

    void setPos(Vec2f& pos) noexcept { m_pos = pos; }
    void setY(float y) noexcept { y_ = y; }

    void set(float x, float y, float Y) noexcept { m_pos.x_ = x; m_pos.y_ = y; y_ = Y; }
    void set(const RGB& rgb) noexcept;
    void set(const CIEXYZ& xyz) noexcept;

    void translate(float tx, float ty) noexcept { m_pos.x_ += tx; m_pos.y_ += ty; }
    void rotate(const Vec2f& pivot, float deg) noexcept;

    CIExyY blend(const CIExyY& xyz, float t) const noexcept {
        if (t < 0.0f) t = 0.0f; else if (t > 1.0f) t = 1.0f;
        float ti = 1.0f - t;
        CIExyY result;
        result.m_pos.x_ = m_pos.x_ * ti + xyz.m_pos.x_ * t;
        result.m_pos.y_ = m_pos.y_ * ti + xyz.m_pos.y_ * t;
        result.y_ = y_ * ti + xyz.y_ * t;
        return result;
    }
};


} // End of namespace Grain

#endif // GrainCIExyY_hpp
