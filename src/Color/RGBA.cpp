//
//  RGB.cpp
//
//  Created by Roald Christesen on from 17.04.2016
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Color/RGBA.hpp"
#include "Color/HSL.hpp"
#include "Color/mixbox.h"
#include "String/CSVString.hpp"
#include "CSS/CSS.hpp"
#include "CSS/CSSColor.hpp"


namespace Grain {


    const RGBA RGBA::kBlack = { 0, 0, 0, 1 };
    const RGBA RGBA::kWhite = { 1, 1, 1, 1 };


    /**
     *  @brief Constructs an RGB color object from a CSV string representation.
     *
     *  @param csv A string containing RGBA values separated by commas (e.g., "255,128,64,98").
     *             Each value represents the red, green, and blue components, respectively.
     *
     *  @note If the string does not contain valid RGB values, the resulting color may be undefined
     *        or default to black (0, 0, 0) depending on the implementation.
     */
    RGBA::RGBA(const char* csv) noexcept {

        setByCSV(csv);
    }


    RGBA::RGBA(const String &csv) noexcept {

        setByCSV(csv.utf8());
    }


    void RGBA::setValues(const float* comp) noexcept {

        if (comp != nullptr) {
            m_data[0] = comp[0];
            m_data[1] = comp[1];
            m_data[2] = comp[2];
            m_alpha = comp[3];
        }
    }


    void RGBA::setValues(const float* comp, float scale) noexcept {

        if (comp != nullptr) {
            m_data[0] = comp[0] * scale;
            m_data[1] = comp[1] * scale;
            m_data[2] = comp[2] * scale;
            m_alpha = comp[3] * scale;
        }
    }


    /**
     *  @brief Sets the RGBA color from a comma-separated string.
     *
     *  This function parses a comma-separated string to initialize the RGBA values.
     *  The behavior depends on how many values are provided:
     *  - 1 value: R, G, and B are all set to the value. Alpha is set to 1.
     *  - 2 values: R, G, and B are set to the first value. Alpha is set to the second.
     *  - 3 values: Assigned to R, G, B respectively. Alpha is set to 1.
     *  - 4 values: Assigned to R, G, B, and Alpha respectively.
     *  - 5 values: First four are assigned to R, G, B, and Alpha; the fifth is used to normalize (divide) all four values if it’s greater than std::numeric_limits<float>::epsilon().
     *
     *  @param csv A null-terminated string containing comma-separated numeric values
     *             (e.g., "0.1,0.2,0.3,0.4", "128,64,255,128,255", or just "0.5").
     *  @return The number of values parsed from the string.
     *
     *  @note If @p csv is nullptr, the color is set to black.
     */
    int32_t RGBA::setByCSV(const char* csv) noexcept {

        if (!csv) {
            *this = kBlack;
        }

        CSVLineParser csv_line_parser(csv);
        float values[6];
        int32_t n = 0;
        while (n < 6 && csv_line_parser.next<float>(values[n])) {
            n++;
        }

        if (n == 1) {
            // Grey
            m_data[0] = m_data[1] = m_data[2] = values[0];
            m_alpha = 1.0f;
        }
        else if (n == 2) {
            // Grey with alpha
            m_data[0] = m_data[1] = m_data[2] = values[0];
            m_alpha = values[1];
        }
        else if (n == 3) {
            // RGB
            m_data[0] = values[0];
            m_data[1] = values[1];
            m_data[2] = values[2];
            m_alpha = 1.0f;
        }
        else if (n == 4) {
            // RGBA
            m_data[0] = values[0];
            m_data[1] = values[1];
            m_data[2] = values[2];
            m_alpha = values[3];
        }
        else if (n == 5) {
            // RGBA, scaled
            m_data[0] = values[0];
            m_data[1] = values[1];
            m_data[2] = values[2];
            m_alpha = values[3];
            float max = values[4];
            if (max > std::numeric_limits<float>::epsilon()) {
                float scale = 1.0f / max;
                m_data[0] *= scale;
                m_data[1] *= scale;
                m_data[2] *= scale;
                m_alpha *= scale;
            }
        }

        return n;
    }


    ErrorCode RGBA::setByCSS(const char* css_str) noexcept {

        if (css_str != nullptr) {
            return CSSColor::parseColorToRGBA(css_str, *this);
        }
        else {
            return ErrorCode::NullData;
        }
    }


    uint32_t RGBA::rgba32bit() const noexcept {
        return
            (static_cast<uint32_t>(Type::floatToUInt8(m_data[0])) << 24) +
            (static_cast<uint32_t>(Type::floatToUInt8(m_data[1])) << 16) +
            (static_cast<uint32_t>(Type::floatToUInt8(m_data[2])) << 8) +
            static_cast<uint32_t>(Type::floatToUInt8(m_alpha));
    }


    void RGBA::values(float* out_values) const noexcept {

        if (out_values != nullptr) {
            out_values[0] = m_data[0];
            out_values[1] = m_data[1];
            out_values[2] = m_data[2];
            out_values[3] = m_alpha;
        }
    }


    bool RGBA::isSame(const RGBA &rgba, float tolerance) const noexcept {

        return (std::fabs(m_data[0] - rgba.m_data[0]) <= tolerance &&
                std::fabs(m_data[1] - rgba.m_data[1]) <= tolerance &&
                std::fabs(m_data[2] - rgba.m_data[2]) <= tolerance &&
                std::fabs(m_alpha - rgba.m_alpha) <= tolerance);
    }


    void RGBA::setBlend(const RGBA &other, float t) noexcept {
        float f2 = std::clamp<float>(t, 0.0f, 1.0f);
        float f1 = 1.0f - f2;
        m_data[0] = m_data[0] * f1 + other.m_data[0] * f2;
        m_data[1] = m_data[1] * f1 + other.m_data[1] * f2;
        m_data[2] = m_data[2] * f1 + other.m_data[2] * f2;
        m_alpha = m_alpha * f1 + other.m_alpha * f2;
    }


    void RGBA::setBlend(const RGBA &a, const RGBA &b, float t) noexcept {
        float f2 = std::clamp<float>(t, 0.0f, 1.0f);
        float f1 = 1.0f - f2;
        m_data[0] = a.m_data[0] * f1 + b.m_data[0] * f2;
        m_data[1] = a.m_data[1] * f1 + b.m_data[1] * f2;
        m_data[2] = a.m_data[2] * f1 + b.m_data[2] * f2;
        m_alpha = a.m_alpha * f1 + b.m_alpha * f2;
    }


    void RGBA::mixbox(const RGBA &color1, const RGBA &color2, float t) noexcept {

        mixbox_latent l1, l2, lmix;
        auto c1 = color1.m_data;
        auto c2 = color2.m_data;
        mixbox_float_rgb_to_latent(c1[0], c1[1], c1[2], l1);
        mixbox_float_rgb_to_latent(c2[0], c2[1], c2[2], l2);
        float ti = (1.0f - t);
        for (int i = 0; i < MIXBOX_LATENT_SIZE; i++) {
            lmix[i] = ti * l1[i] + t * l2[i];
        }
        mixbox_latent_to_float_rgb(lmix, &m_data[0], &m_data[1], &m_data[2]);
        m_alpha = color1.m_alpha * (1.0 - t) + color2.m_alpha * t;
    }


    void RGBA::scale(float scale) noexcept {

        m_data[0] *= scale;
        m_data[1] *= scale;
        m_data[2] *= scale;
        m_alpha *= scale;
    }


} // End of namespace Grain
