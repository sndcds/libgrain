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
        black_point_ = 0.063f;
        lift_ = 0.0f;
        gamma_ = 2.0f;
        gain_ = 2.0f;
        lift_hsv_.set(0.0f, 0.0f, 0.0f);
        gamma_hsv_.set(0.0f, 0.0f, 0.5f);
        gain_hsv_.set(0.0f, 0.0f, 0.5f);
    }


    void CDL::buildCDL_RGB(CDL_RGB& out_cdl_rgb) const noexcept {
        // Lift
        out_cdl_rgb.lift_rgb_ = RGB(lift_hsv_);
        out_cdl_rgb.lift_rgb_ += lift_;

        // Gamma
        HSV gamma_inv;
        gamma_inv.data_[0] = gamma_hsv_.data_[0] + 0.5f;
        if (gamma_inv.data_[0] > 1.0f) {
            gamma_inv.data_[0] -= 1.0f;
        }

        gamma_inv.data_[1] = gamma_hsv_.data_[1];
        gamma_inv.data_[2] = 1.0f - gamma_hsv_.data_[2];
        gamma_inv.data_[2] = std::clamp<float>(gamma_inv.data_[2], 0.0001f, 1.0f);

        out_cdl_rgb.gamma_rgb_ = RGB(gamma_inv);

        out_cdl_rgb.gamma_rgb_ *= gamma_;
        out_cdl_rgb.gamma_rgb_.clamp(0.0001f, 1000.0f);

        // Gain
        out_cdl_rgb.gain_rgb_ = RGB(gain_hsv_);
        out_cdl_rgb.gain_rgb_ *= gain_;

        // Shift values, necessary for black point
        out_cdl_rgb.shift1_rgb_.data_[0] = black_point_ - out_cdl_rgb.lift_rgb_.data_[0];
        out_cdl_rgb.shift1_rgb_.data_[1] = black_point_ - out_cdl_rgb.lift_rgb_.data_[1];
        out_cdl_rgb.shift1_rgb_.data_[2] = black_point_ - out_cdl_rgb.lift_rgb_.data_[2];

        out_cdl_rgb.shift2_rgb_.data_[0] = out_cdl_rgb.lift_rgb_.data_[0] + out_cdl_rgb.shift1_rgb_.data_[0];
        out_cdl_rgb.shift2_rgb_.data_[1] = out_cdl_rgb.lift_rgb_.data_[1] + out_cdl_rgb.shift1_rgb_.data_[1];
        out_cdl_rgb.shift2_rgb_.data_[2] = out_cdl_rgb.lift_rgb_.data_[2] + out_cdl_rgb.shift1_rgb_.data_[2];
    }


} // End of namespace Grain
