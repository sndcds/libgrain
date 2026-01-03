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
        if (comp) {
            data_[0] = comp[0];
            data_[1] = comp[1];
            data_[2] = comp[2];
            alpha_ = comp[3];
        }
    }


    void RGBA::setValues(const float* comp, float scale) noexcept {
        if (comp) {
            data_[0] = comp[0] * scale;
            data_[1] = comp[1] * scale;
            data_[2] = comp[2] * scale;
            alpha_ = comp[3] * scale;
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
     *  @note If `csv` is nullptr, the color is set to black.
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
            data_[0] = data_[1] = data_[2] = values[0];
            alpha_ = 1.0f;
        }
        else if (n == 2) {
            // Grey with alpha
            data_[0] = data_[1] = data_[2] = values[0];
            alpha_ = values[1];
        }
        else if (n == 3) {
            // RGB
            data_[0] = values[0];
            data_[1] = values[1];
            data_[2] = values[2];
            alpha_ = 1.0f;
        }
        else if (n == 4) {
            // RGBA
            data_[0] = values[0];
            data_[1] = values[1];
            data_[2] = values[2];
            alpha_ = values[3];
        }
        else if (n == 5) {
            // RGBA, scaled
            data_[0] = values[0];
            data_[1] = values[1];
            data_[2] = values[2];
            alpha_ = values[3];
            float max = values[4];
            if (max > std::numeric_limits<float>::epsilon()) {
                float scale = 1.0f / max;
                data_[0] *= scale;
                data_[1] *= scale;
                data_[2] *= scale;
                alpha_ *= scale;
            }
        }

        return n;
    }


    ErrorCode RGBA::setByCSS(const char* css_str) noexcept {
        if (css_str) {
            return CSSColor::parseColorToRGBA(css_str, *this);
        }
        else {
            return ErrorCode::NullData;
        }
    }


    uint32_t RGBA::rgba32bit() const noexcept {
        return
            (static_cast<uint32_t>(Type::floatToUInt8(data_[0])) << 24) +
            (static_cast<uint32_t>(Type::floatToUInt8(data_[1])) << 16) +
            (static_cast<uint32_t>(Type::floatToUInt8(data_[2])) << 8) +
            static_cast<uint32_t>(Type::floatToUInt8(alpha_));
    }


    void RGBA::values(float* out_values) const noexcept {
        if (out_values) {
            out_values[0] = data_[0];
            out_values[1] = data_[1];
            out_values[2] = data_[2];
            out_values[3] = alpha_;
        }
    }


    bool RGBA::isSame(const RGBA &rgba, float tolerance) const noexcept {

        return (std::fabs(data_[0] - rgba.data_[0]) <= tolerance &&
                std::fabs(data_[1] - rgba.data_[1]) <= tolerance &&
                std::fabs(data_[2] - rgba.data_[2]) <= tolerance &&
                std::fabs(alpha_ - rgba.alpha_) <= tolerance);
    }


    void RGBA::setBlend(const RGBA &other, float t) noexcept {
        float f2 = std::clamp<float>(t, 0.0f, 1.0f);
        float f1 = 1.0f - f2;
        data_[0] = data_[0] * f1 + other.data_[0] * f2;
        data_[1] = data_[1] * f1 + other.data_[1] * f2;
        data_[2] = data_[2] * f1 + other.data_[2] * f2;
        alpha_ = alpha_ * f1 + other.alpha_ * f2;
    }


    void RGBA::setBlend(const RGBA &a, const RGBA &b, float t) noexcept {
        float f2 = std::clamp<float>(t, 0.0f, 1.0f);
        float f1 = 1.0f - f2;
        data_[0] = a.data_[0] * f1 + b.data_[0] * f2;
        data_[1] = a.data_[1] * f1 + b.data_[1] * f2;
        data_[2] = a.data_[2] * f1 + b.data_[2] * f2;
        alpha_ = a.alpha_ * f1 + b.alpha_ * f2;
    }


    void RGBA::mixbox(const RGBA &color1, const RGBA &color2, float t) noexcept {

        mixbox_latent l1, l2, lmix;
        auto c1 = color1.data_;
        auto c2 = color2.data_;
        mixbox_float_rgb_to_latent(c1[0], c1[1], c1[2], l1);
        mixbox_float_rgb_to_latent(c2[0], c2[1], c2[2], l2);
        float ti = (1.0f - t);
        for (int i = 0; i < MIXBOX_LATENT_SIZE; i++) {
            lmix[i] = ti * l1[i] + t * l2[i];
        }
        mixbox_latent_to_float_rgb(lmix, &data_[0], &data_[1], &data_[2]);
        alpha_ = color1.alpha_ * (1.0 - t) + color2.alpha_ * t;
    }


    void RGBA::scale(float scale) noexcept {

        data_[0] *= scale;
        data_[1] *= scale;
        data_[2] *= scale;
        alpha_ *= scale;
    }


} // End of namespace Grain
