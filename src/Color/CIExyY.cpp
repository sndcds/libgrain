//
//  CIExyY.cpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Color/CIExyY.hpp"
#include "Color/CIEXYZ.hpp"
#include "Color/RGB.hpp"


namespace Grain {


CIExyY::CIExyY(const RGB& rgb) noexcept {
    set(rgb);
}


CIExyY::CIExyY(const CIEXYZ& xyz) noexcept {
    set(xyz);
}


void CIExyY::set(const RGB& rgb) noexcept{
    CIEXYZ xyz(rgb);
    set(xyz);
}


void CIExyY::set(const CIEXYZ& xyz) noexcept {
    float sum = xyz.data_[0] + xyz.data_[1] + xyz.data_[2];
    if (sum == 0.0f) {
        m_pos.x_ = 0.0f;
        m_pos.y_ = 0.0f;
    }
    else {
        m_pos.x_ = xyz.data_[0] / sum;
        m_pos.y_ = xyz.data_[1] / sum;
    }

    y_ = xyz.data_[1];
}


void CIExyY::rotate(const Vec2f& pivot, float deg) noexcept {
    m_pos.rotate(pivot, deg);
}


} // End of namespace Grain.
