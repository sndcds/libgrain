//
//  Color.cpp
//
//  Created by Roald Christesen on from 28.06.2012
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#include "Color/Color.hpp"
#include "String/String.hpp"
#include "Type/Type.hpp"


namespace Grain {

    /**
     *  @brief Convert RGB to HSV.
     */
    void Color::rgb_to_hsv(const float* rgb, float* out_hsv) noexcept {

        if (!out_hsv) {
            return;
        }

        if (!rgb) {
            out_hsv[0] = out_hsv[1] = out_hsv[2] = 0.0f;
            return;
        }

        float r = rgb[0];
        float g = rgb[1];
        float b = rgb[2];

        float min = Type::minOf3(r, g, b);
        float max = Type::minOf3(r, g, b);
        float v = max;
        float delta = max - min;
        float h = -1.0f;
        float s = 0.0f;

        if (delta >= std::numeric_limits<float>::epsilon()) {
            s = delta / max;

            if (r == max) { // Between yellow and magenta.
                h = (g - b) / delta;
            }
            else if (g == max) { // Between cyan and yellow.
                h = 2.0f + (b - r) / delta;
            }
            else { // Between magenta and cyan.
                h = 4.0f + (r - g) / delta;
            }

            h /= 6.0f;    // [0, 1]
            if (h < 0.0f) {
                h += 1.0f;
            }
        }

        out_hsv[0] = h;
        out_hsv[1] = s;
        out_hsv[2] = v;
    }


    /**
     *  @brief Convert RGB to HSL.
     */
    void Color::rgb_to_hsl(const float* rgb, float* out_hsl) noexcept {

        if (!out_hsl) {
            return;
        }

        if (!rgb) {
            out_hsl[0] = out_hsl[1] = out_hsl[2] = 0;
            return;
        }

        float r = rgb[0];
        float g = rgb[1];
        float b = rgb[2];

        float h, s, l;
        float min = Type::minOf3(r, g, b);
        float max = Type::maxOf3(r, g, b);

        l = (max + min) / 2.0f;

        if (max == min) {
            // Achromatic
            h = s = 0.0f;
        }
        else {
            float d = max - min;
            s = l > 0.5f ? d / (2.0f - max - min) : d / (max + min);

            if (r > g && r > b) {
                h = (g - b) / d + (g < b ? 6.0f : 0.0f);
            }
            else if (g > r && g > b) {
                h = (b - r) / d + 2.0f;
            }
            else {
                h = (r - g) / d + 4.0f;
            }

            h /= 6.0f;
        }

        out_hsl[0] = h;
        out_hsl[1] = s;
        out_hsl[2] = l;
    }


    /**
     *  @brief Convert RGB to YUV601.
     */
    void Color::rgb_to_yuv601(const float* rgb, float* out_yuv) noexcept {

        if (!out_yuv) {
            return;
        }

        if (!rgb) {
            out_yuv[0] = out_yuv[1] = out_yuv[2] = 0.0f;
            return;
        }

        float r = rgb[0];
        float g = rgb[1];
        float b = rgb[2];

        out_yuv[0] = kLumina601ScaleR * r + kLumina601ScaleG * g + kLumina601ScaleB * b;
        out_yuv[1] = -0.16873f * r - 0.33127f * g + 0.50001f * b;
        out_yuv[2] = 0.50001f * r - 0.4187f * g - 0.08131f * b;
    }


    /**
     *  @brief Convert RGB to YUV709.
     */
    void Color::rgb_to_yuv709(const float* rgb, float* out_yuv) noexcept {

        if (!out_yuv) {
            return;
        }

        if (!rgb) {
            out_yuv[0] = out_yuv[1] = out_yuv[2] = 0.0f;
            return;
        }


        float r = rgb[0];
        float g = rgb[1];
        float b = rgb[2];

        out_yuv[0] = kLumina709ScaleR * r + kLumina709ScaleG * g + kLumina709ScaleB * b;
        out_yuv[1] = -0.114569f * r - 0.385436f * g + 0.500004f * b;
        out_yuv[2] = 0.500004f * r - 0.454162f * g - 0.045842f * b;
    }


    /**
     *  @brief Convert RGB to OKLab.
     */
    void Color::rgb_to_oklab(const float* rgb, float* out_oklab) noexcept {

        if (!out_oklab) {
            return;
        }

        if (!rgb) {
            out_oklab[0] = out_oklab[1] = out_oklab[2] = 0.0f;
            return;
        }

        float r = Color::gamma_to_linear(rgb[0]);
        float g = Color::gamma_to_linear(rgb[1]);
        float b = Color::gamma_to_linear(rgb[2]);

        float l = r * 0.4122214708f + g * 0.5363325363f + b * 0.0514459929f;
        float m = r * 0.2119034982f + g * 0.6806995451f + b * 0.1073969566f;
        float s = r * 0.0883024619f + g * 0.2817188376f + b * 0.6299787005f;

        l = cbrt(l);
        m = cbrt(m);
        s = cbrt(s);

        out_oklab[0] = l * 0.2104542553f + m * 0.7936177850f + s * -0.0040720468f;
        out_oklab[1] = l * 1.9779984951f + m * -2.4285922050f + s * 0.4505937099f;
        out_oklab[2] = l * 0.0259040371f + m * 0.7827717662f + s * -0.8086757660f;
    }


    /**
     *  @brief Convert HSV to RGB.
     */
    void Color::hsv_to_rgb(const float* hsv, float* out_rgb) noexcept {

        if (!out_rgb) {
            return;
        }

        if (!hsv) {
            out_rgb[0] = out_rgb[1] = out_rgb[2] = 0.0f;
            return;
        }

        float h = hsv[0] * 6.0f;
        float s = hsv[1];
        float v = hsv[2];

        if (s != 0.0f) {

            if (h < 0.0f) {
                h += 6.0f;
            }
            else if (h >= 6.0f) {
                h -= 6.0f;
            }

            int32_t i = std::floor(h);
            float f = h - static_cast<float>(i);
            float p = v * (1.0f - s);
            float q = v * (1.0f - s * f);
            float t = v * (1.0f - s * (1.0f - f));

            switch (i) {
                case 0:  out_rgb[0] = v; out_rgb[1] = t; out_rgb[2] = p; break;
                case 1:  out_rgb[0] = q; out_rgb[1] = v; out_rgb[2] = p; break;
                case 2:  out_rgb[0] = p; out_rgb[1] = v; out_rgb[2] = t; break;
                case 3:  out_rgb[0] = p; out_rgb[1] = q; out_rgb[2] = v; break;
                case 4:  out_rgb[0] = t; out_rgb[1] = p; out_rgb[2] = v; break;
                default: out_rgb[0] = v; out_rgb[1] = p; out_rgb[2] = q; break;
            }
        }
        else {
            out_rgb[0] = out_rgb[1] = out_rgb[2] = v;
        }
    }


    /**
     *  @brief Convert HSV to HSL.
     */
    void Color::hsv_to_hsl(const float* hsv, float* out_hsl) noexcept {

        if (!out_hsl) {
            return;
        }

        if (!hsv) {
            out_hsl[0] = out_hsl[1] = out_hsl[2] = 0.0f;
            return;
        }

        // Hue remains the same
        out_hsl[0] = hsv[0];

        float v = hsv[2];
        float s_hsv = hsv[1];

        // Compute Lightness
        float l = v * (1.0f - s_hsv / 2.0f);

        // Compute Saturation (s_hsl)
        float s_hsl = (l == 0.0f || l == 1.0f) ? 0.0f : (v - l) / std::min(l, 1.0f - l);

        out_hsl[1] = s_hsl;
        out_hsl[2] = l;
    }


    inline float _gr_hsl_to_rgb_helper(float p, float q, float t) {

        if (t < 0) t += 1.0f;
        if (t > 1) t -= 1.0f;
        if (t < 1.0f / 6.0f) {
            return p + (q - p) * 6.0f * t;
        }
        if (t < 1.0f / 2.0f) {
            return q;
        }
        if (t < 2.0f / 3.0f) {
            return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
        }

        return p;
    }


/**
 *  @brief Convert HSL to RGB.
 */
    void Color::hsl_to_rgb(const float* hsl, float* out_rgb) noexcept {

        if (!out_rgb) {
            return;
        }

        if (!hsl) {
            out_rgb[0] = out_rgb[1] = out_rgb[2] = 0;
            return;
        }

        float h = hsl[0];
        float s = hsl[1];
        float l = hsl[2];

        if (s <= 0.0f) {
            // Achromatic
            out_rgb[0] = out_rgb[1] = out_rgb[2] = l;
        }
        else {
            float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
            float p = 2.0f * l - q;

            out_rgb[0] = _gr_hsl_to_rgb_helper(p, q, h + 1.0f / 3.0f);
            out_rgb[1] = _gr_hsl_to_rgb_helper(p, q, h);
            out_rgb[2] = _gr_hsl_to_rgb_helper(p, q, h - 1.0f / 3.0f);
        }
    }


    /**
     *  @brief Convert HSL to HSV.
     */
    void Color::hsl_to_hsv(const float* hsl, float* out_hsv) noexcept {

        if (!out_hsv) {
            return;
        }

        if (!hsl) {
            out_hsv[0] = out_hsv[1] = out_hsv[2] = 0.0f;
            return;
        }

        // Hue remains the same
        out_hsv[0] = hsl[0];

        float l = hsl[2];
        float s_hsl = hsl[1];

        // Compute Value.
        float v = l + s_hsl * std::min(l, 1.0f - l);

        // Compute Saturation (s_hsv).
        float s_hsv = (v == 0.0f) ? 0.0f : 2.0f * (1.0f - l / v);

        out_hsv[1] = s_hsv;
        out_hsv[2] = v;
    }


    /**
     *  @brief Convert YUV601 to RGB.
     */
    void Color::yuv601_to_rgb(const float* yuv, float* out_rgb) noexcept {

        if (!out_rgb) {
            return;
        }

        if (!yuv) {
            out_rgb[0] = out_rgb[1] = out_rgb[2] = 0.0f;
            return;
        }

        out_rgb[0] = yuv[0] + 1.4020f * yuv[2];
        out_rgb[1] = yuv[0] - 0.3441f * yuv[1] - 0.7141f * yuv[2];
        out_rgb[2] = yuv[0] + 1.7720f * yuv[1];
    }


    /**
     *  @brief Convert YUV709 to RGB.
     */
    void Color::yuv709_to_rgb(const float* yuv, float* out_rgb) noexcept {

        if (!out_rgb) {
            return;
        }

        if (!yuv) {
            out_rgb[0] = out_rgb[1] = out_rgb[2] = 0.0f;
            return;
        }

        out_rgb[0] = yuv[0] + 1.5748f * yuv[2];
        out_rgb[1] = yuv[0] - 0.1873f * yuv[1] - 0.4681f * yuv[2];
        out_rgb[2] = yuv[0] + 1.8556f * yuv[1];
    }


    /**
     *  @brief Convert Lab to RGB.
     */
    void Color::lab_to_rgb(const float* lab, float* out_rgb) noexcept {

        if (!out_rgb) {
            return;
        }

        if (!lab) {
            out_rgb[0] = out_rgb[1] = out_rgb[2] = 0.0f;
            return;
        }

        float l = lab[0];
        float a = lab[1];
        float b = lab[2];

        float Y = l * (1.0f / 116.0f) + 16.0f / 116.0f;
        float X = a * (1.0f / 500.0f) + Y;
        float Z = b * (-1.0f / 200.0f) + Y;

        X = X > 6.0f / 29.0f ? X * X * X : X * (108.0f / 841.0f) - 432.0f / 24389.0f;
        Y = l > 8.0f ? Y * Y * Y : l * (27.0f / 24389.0f);
        Z = Z > 6.0f / 29.0f ? Z * Z * Z : Z * (108.0f / 841.0f) - 432.0f / 24389.0f;

        // Normalized XYZFile -> linear sRGB.

        out_rgb[0] = X * (1219569.0f / 395920.0f) + Y * (-608687.0f / 395920.0f) + Z * (-107481.0f / 197960.0f);
        out_rgb[1] = X * (-80960619.0f / 87888100.0f) + Y * (82435961.0f / 43944050.0f) + Z * (3976797.0f / 87888100.0f);
        out_rgb[2] = X * (93813.0f / 1774030.0f) + Y * (-180961.0f / 887015.0f) + Z * (107481.0f / 93370.0f);
    }


    /**
     *  @brief Convert OKLab to RGB.
     */
    void Color::oklab_to_rgb(const float* oklab, float* out_rgb) noexcept {

        if (!out_rgb) {
            return;
        }

        if (!oklab) {
            out_rgb[0] = out_rgb[1] = out_rgb[2] = 0;
            return;
        }

        float l = oklab[0] + oklab[1] * 0.3963377774f + oklab[2] * 0.2158037573f;
        float m = oklab[0] + oklab[1] * -0.1055613458f + oklab[2] * -0.0638541728f;
        float s = oklab[0] + oklab[1] * -0.0894841775f + oklab[2] * -1.2914855480f;

        l = l * l * l;
        m = m * m * m;
        s = s * s * s;

        float r = l * 4.0767416621f + m * -3.3077115913f + s * 0.2309699292f;
        float g = l * -1.2684380046f + m * 2.6097574011f + s * -0.3413193965f;
        float b = l * -0.0041960863f + m * -0.7034186147f + s * 1.7076147010f;

        r = std::clamp(r, 0.0f, 1.0f);
        g = std::clamp(g, 0.0f, 1.0f);
        b = std::clamp(b, 0.0f, 1.0f);

        out_rgb[0] = Color::linear_to_gamma(r);
        out_rgb[1] = Color::linear_to_gamma(g);
        out_rgb[2] = Color::linear_to_gamma(b);
    }


    /**
     *  @brief Convert OKLab to OKLCh.
     */
    void Color::oklab_to_oklch(const float* oklab, float* out_oklch) noexcept {

        if (!out_oklch) {
            return;
        }

        if (!oklab) {
            out_oklch[0] = out_oklch[1] = out_oklch[2] = 0.0f;
            return;
        }

        out_oklch[0] = oklab[0];
        float a = oklab[1];
        float b = oklab[2];
        out_oklch[1] = std::sqrt(a * a + b * b);
        out_oklch[2] = std::atan2(b, a) / (2.0f * std::numbers::pi);
    }


    /**
     *  @brief Convert OKLCh to RGB.
     */
    void Color::oklch_to_rgb(const float* oklch, float* out_rgb) noexcept {

        float oklab[3];
        oklch_to_oklab(oklch, oklab);
        oklab_to_rgb(oklab, out_rgb);
    }


    /**
     *  @brief Convert OKLCh to OKLab.
     */
    void Color::oklch_to_oklab(const float* oklch, float* out_oklab) noexcept {

        if (!out_oklab) {
            return;
        }

        if (oklch == nullptr) {
            out_oklab[0] = out_oklab[1] = out_oklab[2] = 0.0f;
            return;
        }

        out_oklab[0] = oklch[0];
        float c = oklch[1];
        float h = oklch[2] * 2.0f * std::numbers::pi;
        out_oklab[1] = c * std::cos(h);
        out_oklab[2] = c * std::sin(h);
    }


    inline float _gr_rgb_to_lab_helper(float c) {

        return c > 216.0f / 24389.0f ? std::pow(c, 1.0f / 3.0f) : c * (841.0f / 108.0f) + (4.0f / 29.0f);
    }


    /**
     *  @brief Convert RGB to Lab.
     */
    void Color::rgb_to_lab(const float* rgb, float* out_lab) noexcept {

        if (out_lab == nullptr) {
            return;
        }

        if (rgb == nullptr) {
            out_lab[0] = out_lab[1] = out_lab[2] = 0.0f;
            return;
        }

        float r = rgb[0];
        float g = rgb[1];
        float b = rgb[2];

        // Linear sRGB -> normalized XYZFile (X,Y,Z are all in 0...1)
        float X = _gr_rgb_to_lab_helper(r * (10135552.0f / 23359437.0f) + g * (8788810.0f / 23359437.0f) + b * (4435075.0f / 23359437.0f));
        float Y = _gr_rgb_to_lab_helper(r * (871024.0f / 4096299.0f) + g * (8788810.0f / 12288897.0f) + b * (887015.0f / 12288897.0f));
        float Z = _gr_rgb_to_lab_helper(r * (158368.0f / 8920923.0f) + g * (8788810.0f / 80288307.0f) + b * (70074185.0f / 80288307.0f));

        // Normalized XYZFile -> Lab
        out_lab[0] = Y * 116.0f - 16.0f;
        out_lab[1] = (X - Y) * 500.0f;
        out_lab[2] = (Y - Z) * 200.0f;
    }


    /**
     *  @brief Convert XYZFile to RGB.
     */
    void Color::xyz_to_rgb(const float* xyz, const Mat3f& matrix, float* out_rgb) noexcept {

        if (out_rgb == nullptr) {
            return;
        }

        if (xyz == nullptr) {
            out_rgb[0] = out_rgb[1] = out_rgb[2] = 0.0f;
            return;
        }

        out_rgb[0] = matrix.m_data[0][0] * xyz[0] + matrix.m_data[0][1] * xyz[1] + matrix.m_data[0][2] * xyz[2];
        out_rgb[1] = matrix.m_data[1][0] * xyz[0] + matrix.m_data[1][1] * xyz[1] + matrix.m_data[1][2] * xyz[2];
        out_rgb[2] = matrix.m_data[2][0] * xyz[0] + matrix.m_data[2][1] * xyz[1] + matrix.m_data[2][2] * xyz[2];
    }


    /**
     *  @brief Convert XYZFile to RGB.
     */
    void Color::xyz_to_rgb(const float* xyz, const float* m, float* out_rgb) noexcept {

        if (out_rgb == nullptr) {
            return;
        }

        if (xyz == nullptr || m == nullptr) {
            out_rgb[0] = out_rgb[1] = out_rgb[2] = 0.0f;
            return;
        }

        out_rgb[0] = m[0] * xyz[0] + m[1] * xyz[1] + m[2] * xyz[2];
        out_rgb[1] = m[3] * xyz[0] + m[4] * xyz[1] + m[5] * xyz[2];
        out_rgb[2] = m[6] * xyz[0] + m[7] * xyz[1] + m[8] * xyz[2];
    }


    /**
     *  @brief Sony SLog2 level to linear conversion.
     */
    float Color::sony_SLog2_to_linear(float v) noexcept {

        if (v >= 0.030001222851889303f) {
            return 219.0f * (std::pow(10.0f, ((v - 0.616596f - 0.03f) / 0.432699f)) - 0.037584f) / 155.0f;
        }
        else {
            return (v - 0.030001222851889303f) / 3.53881278538813f;
        }
    }


    /**
     *  @brief Linear level to Sony SLog2 level conversion.
     */
    float Color::sony_Linear_to_SLog2(float v) noexcept {

        if (v > 0) {
            return (0.432699f * std::log10(155.0f * v / 219.0f + 0.037584f) + 0.616596f) + 0.03f;
        }
        else {
            return v * 3.53881278538813f + 0.030001222851889303f;
        }
    }


    /**
     *  @brief Sony SLog3 level to linear conversion.
     */
    float Color::sony_SLog3_to_Linear(float v) noexcept {

        if (v >= 171.2102946929f / 1023.0f) {
            return std::pow(10.0f, (v * 1023.0f - 420.0f) / 261.5f) * (0.18f + 0.01f) - 0.01f;
        }
        else {
            return (v * 1023.0f - 95.0f) * 0.01125f / (171.2102946929f - 95.0f);
        }
    }


    /**
     *  @brief Linear level to Sony SLog3 level conversion.
     */
    float Color::sony_Linear_to_SLog3(float v) noexcept {

        if (v >= 0.01125f) {
            return (420.0f + std::log10((v + 0.01f) / (0.18f + 0.01f)) * 261.5f) / 1023.0f;
        }
        else {
            return (v * (171.2102946929f - 95.0f) / 0.01125f + 95.0f) / 1023.0f;
        }
    }


    /**
     *  @brief Color component combination for overlay mode.
     */
    float Color::combineOverlay(float b, float f) noexcept {

        return b < 0.5f ? f * b / 0.5f : 1.0f - ((1.0f - f) * (1.0f - b) / 0.5f);
    }


    /**
     *  @brief Color component combination for screen mode.
     */
    float Color::combineScreen(float b, float f) noexcept {

        return 1.0f - (1.0f - f) * (1.0f - b);
    }


    /**
     *  @brief Color component combination for soft light mode.
     */
        float Color::combineSoftLight(float b, float f) noexcept {

        return f <= 0.5f ? (2.0f * f - 1.0f) * (b - b * b) + b : (2.0f * f - 1.0f) * (std::sqrt(b) - b) + b;
    }


    /**
     *  @brief Color component combination for hard light mode.
     */
    float Color::combineHardLight(float b, float f) noexcept {

        return f <= 0.5f ? 2.0f * f * b : 1.0f - 2.0f * (1.0f - f) * (1.0f - b);
    }


    /**
     *  @brief Color component combination for luminance mode.
     */
    float Color::combineLuminance(float r, float g, float b, float l) noexcept {

        return l * r * 0.3f + l * g * 0.59f + l * b * 0.11f;
    }


} // End of namespace Grain
