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

        float sum = xyz.m_data[0] + xyz.m_data[1] + xyz.m_data[2];
        if (sum == 0.0f) {
            m_pos.m_x = 0.0f;
            m_pos.m_y = 0.0f;
        }
        else {
            m_pos.m_x = xyz.m_data[0] / sum;
            m_pos.m_y = xyz.m_data[1] / sum;
        }

        m_y = xyz.m_data[1];
    }


    void CIExyY::rotate(const Vec2f& pivot, float deg) noexcept {

        m_pos.rotate(pivot, deg);
    }


} // End of namespace Grain.
