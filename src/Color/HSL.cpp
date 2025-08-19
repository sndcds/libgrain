//
//  HSL.cpp
//
//  Created by Roald Christesen on from 30.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Color/HSL.hpp"
#include "Color/HSV.hpp"
#include "Color/RGB.hpp"
#include "String/CSVString.hpp"


namespace Grain {

    HSL& HSL::operator = (const RGB& v) {
        set(v);
        return *this;
    }


    void HSL::set(const RGB& rgb) noexcept {
        Color::rgb_to_hsl(rgb.m_data, m_data);
    }


    void HSL::set(const HSV& hsv) noexcept {
        Color::hsv_to_hsl(hsv.m_data, m_data);
    }


    void HSL::set(const YUV& yuv, Color::Space yuv_color_space) noexcept {
        RGB rgb(yuv, yuv_color_space);
        set(rgb);
    }


    void HSL::set(const CIEXYZ& xyz) noexcept {
        RGB rgb(xyz);
        set(rgb);
    }


    void HSL::set(const CIExyY& xyY) noexcept {
        RGB rgb(xyY);
        set(rgb);
    }


    void HSL::setRGB(float r, float g, float b) noexcept {
        set(RGB(r, g, b));
    }


    bool HSL::setByCSV(const char* csv, char delimiter) noexcept {
        if (csv) {
            CSVLineParser csv_line_parser(csv);
            csv_line_parser.setDelimiter(delimiter);
            if (csv_line_parser.values(3, m_data) == 3) {
                return true;
            }
        }
        return false;
    }


} // End of namespace Grain
