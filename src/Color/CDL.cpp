//
//  CDL.cpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Color/CDL.hpp"
#include "Color/HSV.hpp"


namespace Grain {

    void CDL::init() noexcept {

        m_black_point = 0.063f;
        m_lift = 0.0f;
        m_gamma = 2.0f;
        m_gain = 2.0f;
        m_lift_hsv.set(0.0f, 0.0f, 0.0f);
        m_gamma_hsv.set(0.0f, 0.0f, 0.5f);
        m_gain_hsv.set(0.0f, 0.0f, 0.5f);
    }


    void CDL::buildCDL_RGB(CDL_RGB& out_cdl_rgb) const noexcept {

        // Lift.

        out_cdl_rgb.m_lift_rgb = RGB(m_lift_hsv);
        out_cdl_rgb.m_lift_rgb += m_lift;


        // Gamma.

        HSV gamma_inv;
        gamma_inv.m_data[0] = m_gamma_hsv.m_data[0] + 0.5f;
        if (gamma_inv.m_data[0] > 1.0f) {
            gamma_inv.m_data[0] -= 1.0f;
        }

        gamma_inv.m_data[1] = m_gamma_hsv.m_data[1];
        gamma_inv.m_data[2] = 1.0f - m_gamma_hsv.m_data[2];
        gamma_inv.m_data[2] = std::clamp<float>(gamma_inv.m_data[2], 0.0001f, 1.0f);

        out_cdl_rgb.m_gamma_rgb = RGB(gamma_inv);

        out_cdl_rgb.m_gamma_rgb *= m_gamma;
        out_cdl_rgb.m_gamma_rgb.clamp(0.0001f, 1000.0f);


        // Gain.

        out_cdl_rgb.m_gain_rgb = RGB(m_gain_hsv);
        out_cdl_rgb.m_gain_rgb *= m_gain;


        // Shift values, necessary for black point.

        out_cdl_rgb.m_shift1_rgb.m_data[0] = m_black_point - out_cdl_rgb.m_lift_rgb.m_data[0];
        out_cdl_rgb.m_shift1_rgb.m_data[1] = m_black_point - out_cdl_rgb.m_lift_rgb.m_data[1];
        out_cdl_rgb.m_shift1_rgb.m_data[2] = m_black_point - out_cdl_rgb.m_lift_rgb.m_data[2];

        out_cdl_rgb.m_shift2_rgb.m_data[0] = out_cdl_rgb.m_lift_rgb.m_data[0] + out_cdl_rgb.m_shift1_rgb.m_data[0];
        out_cdl_rgb.m_shift2_rgb.m_data[1] = out_cdl_rgb.m_lift_rgb.m_data[1] + out_cdl_rgb.m_shift1_rgb.m_data[1];
        out_cdl_rgb.m_shift2_rgb.m_data[2] = out_cdl_rgb.m_lift_rgb.m_data[2] + out_cdl_rgb.m_shift1_rgb.m_data[2];
    }


} // End of namespace Grain
