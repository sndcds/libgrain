//
//  RGBRamp.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

//
//  GrRGBRamp.cpp
//  GrainLib
//
//  Created by Roald Christesen on 25.01.24.
//

#include "Color/RGBRamp.hpp"


namespace Grain {


    RGBRamp::RGBRamp() {
    }


    void RGBRamp::dedaultLocationRamp() noexcept {

        // TODO: Initialize with other values!

        addSpot(RGB(229, 199, 79, 255));
        addSpot(RGB(205, 95, 85, 255));
        addSpot(RGB(37, 61, 167, 255));
        addSpot(RGB(28, 130, 145, 255));
    }


    bool RGBRamp::spotColorAtIndex(int32_t index, RGB& out_color) const noexcept {

        if (index >= 0 && index < m_spot_count) {
            out_color = m_spots_colors[index];
            return true;
        }
        else {
            return false;
        }
    }


    void RGBRamp::addSpot(const RGB& color) noexcept {

        if (m_spot_count < kMaxSpotCount) {
            m_spots_colors[m_spot_count] = color;
            m_spot_count++;
        }
    }


    void RGBRamp::setSpotColorAtIndex(int32_t index, const RGB& color) noexcept {

        if (index >= 0 && index < m_spot_count) {
            m_spots_colors[index] = color;
        }
    }


    void RGBRamp::lookup(float t, RGB& out_color) const noexcept {

        if (m_spot_count == 1) {
            out_color = m_spots_colors[0];
        }
        else if (m_spot_count > 1) {

            int32_t n = m_spot_count;
            int32_t m = n - 1;
            float d = t;

            if (d <= 0.0f) {
                out_color = m_spots_colors[0];
                return;
            }
            else if (d >= 1.0f) {
                out_color = m_spots_colors[m_spot_count - 1];
                return;
            }

            int32_t i = static_cast<int32_t>(d * m);
            if (i > m) {
                i = m;
            }

            int32_t j = i + 1;
            if (j > m) {
                j = m;
            }

            float b = (d * m) - i;
            out_color.setBlend(m_spots_colors[i], m_spots_colors[j], b);
        }
    }


    void RGBRamp::lookup(float t, float* out_values) const noexcept {

        if (out_values != nullptr) {
            RGB rgb;
            lookup(t, rgb);
            out_values[0] = rgb.m_data[0];
            out_values[1] = rgb.m_data[1];
            out_values[2] = rgb.m_data[2];
        }
    }


    void RGBRamp::lookupRing(float t, RGB& out_color) const noexcept {

        float d = fmod(t, 1.0f);
        int32_t n = m_spot_count;

        if (d < 0.0f) {
            d += 1.0f;
        }

        int32_t i = static_cast<int32_t>(d * n);
        if (i >= n) {
            i -= n;
        }

        int32_t j = i + 1;
        if (j >= n) {
            j -= n;
        }

        float b = (d * n) - i;
        out_color.setBlend(m_spots_colors[i], m_spots_colors[j], b);
    }


    void RGBRamp::lookupRing(float t, float* out_values) const noexcept {

        if (out_values != nullptr) {
            RGB rgb;
            lookupRing(t, rgb);
            out_values[0] = rgb.m_data[0];
            out_values[1] = rgb.m_data[1];
            out_values[2] = rgb.m_data[2];
        }
    }


} // End of namespace Grain.
