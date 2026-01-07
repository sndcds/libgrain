//
//  HSV.cpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Color/HSV.hpp"
#include "Color/RGB.hpp"
#include "Color/HSL.hpp"
#include "String/CSVString.hpp"


namespace Grain {

    HSV& HSV::operator = (const RGB& v) {

        set(v);
        return *this;
    }


    void HSV::set(const RGB& rgb) noexcept {

        Color::rgb_to_hsv(rgb.data_, data_);
    }


    void HSV::set(const HSL& hsl) noexcept {

        Color::hsl_to_hsv(hsl.m_data, data_);
    }


    void HSV::set(const YUV& yuv, Color::Space yuv_color_space) noexcept {

        RGB rgb(yuv, yuv_color_space);
        set(rgb);
    }


    void HSV::set(const CIEXYZ& xyz) noexcept {

        RGB rgb(xyz);
        set(rgb);
    }


    void HSV::set(const CIExyY& xyY) noexcept {

        RGB rgb(xyY);
        set(rgb);
    }


    void HSV::setRGB(float r, float g, float b) noexcept {

        set(RGB(r, g, b));
    }


    void HSV::set(const char* csv) noexcept {

        if (csv) {
            CSVLineParser csv_line_parser(csv);
            csv_line_parser.values(3, data_);
        }
        else {
            set(0, 0, 0);
        }
    }


} // End of namespace Grain
