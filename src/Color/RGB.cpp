//
//  RGB.cpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Color/RGB.hpp"
#include "Type/Type.hpp"
#include "String/String.hpp"
#include "String/CSVString.hpp"
#include "Math/Mat3.hpp"
#include "Math/Math.hpp"
#include "Geometry.hpp"

#include "Color/RGBA.hpp"
#include "Color/HSV.hpp"
#include "Color/HSL.hpp"
#include "Color/CIEXYZ.hpp"
#include "Color/CIExyY.hpp"
#include "Color/YUV.hpp"
#include "Color/OKColor.hpp"
#include "Color/LMS.hpp"
#include "Color/Color.hpp"
#include "Color/NamedColor.hpp"
#include "Color/CDL.hpp"
#include "DSP/LUT1.hpp"

#include "Color/mixbox.h"


namespace Grain {

    const RGB RGB::kBlack(0.0f, 0.0f, 0.0f);
    const RGB RGB::kWhite(1.0f, 1.0f, 1.0f);
    const RGB RGB::kRed(1.0f, 0.0f, 0.0f);
    const RGB RGB::kGreen(0.0f, 1.0f, 0.0f);
    const RGB RGB::kBlue(0.0f, 0.0f, 1.0f);
    const RGB RGB::kCyan(0.0f, 1.0f, 1.0f);
    const RGB RGB::kMagenta(1.0f, 0.0f, 1.0f);
    const RGB RGB::kYellow(1.0f, 1.0f, 0.0f);
    const RGB RGB::kMixboxCadmiumYellow(0.996f, 0.925f, 0.0f);
    const RGB RGB::kMixboxHansaYellow(0.996f, 0.925f, 0.0f);
    const RGB RGB::kMixboxCadmiumOrange(1.0f, 0.412f, 0.0f);
    const RGB RGB::kMixboxCadmiumRed(1.0f, 0.153f, 0.008f);
    const RGB RGB::kMixboxQuinacridoneMagenta(0.502f, 0.008f, 0.18f);
    const RGB RGB::kMixboxCobaltViolet(0.306f, 0, 0.259f);
    const RGB RGB::kMixboxUltramarineBlue(0.098f, 0, 0.349f);
    const RGB RGB::kMixboxCobaltBlue(0, 0.129f, 0.522f);
    const RGB RGB::kMixboxPhthaloBlue(0.051f, 0.106f, 0.267f);
    const RGB RGB::kMixboxPhthaloGreen(0, 0.235f, 0.196f);
    const RGB RGB::kMixboxPermanentGreen(0.027f, 0.427f, 0.086f);
    const RGB RGB::kMixboxSapGreen(0.42f, 0.58f, 0.016f);
    const RGB RGB::kMixboxBurntSienna(0.482f, 0.282f, 0);


    /**
     *  @brief Constructor for a grayscale color.
     *
     *  Creates an `RGB` object where all color channels (red, green, blue) are set
     *  to the provided `value`, resulting in a grayscale color.
     *
     *  @param value The intensity of the grayscale color, where `0` represents
     *               black, `1` represents white, and intermediate values represent
     *               shades of gray.
     */
    RGB::RGB(float value) noexcept : m_data { value, value, value } {

    }


    /**
     *  @brief Constructor for an RGB color with specified channel values.
     *
     *  Initializes an `RGB` object with individual red, green, and blue channel
     *  values.
     *
     *  @param r The intensity of the red channel, where `0` represents no red and
     *           `1` represents full red.
     *  @param g The intensity of the green channel, where `0` represents no green
     *           and `1` represents full green.
     *  @param b The intensity of the blue channel, where `0` represents no blue
     *            and `1` represents full blue.
     *
     *  @note Each channel value is expected to be in the range `[0, 1]`. Values
     *        outside this range may lead to undefined behavior or clamping,
     *        depending on the implementation.
     */
    RGB::RGB(float r, float g, float b) noexcept : m_data { r, g, b } {

    }


    /**
     *  @brief Constructor for an RGB color with integer channel values and a
     *         specified maximum.
     *
     *  Initializes an `RGB` object with red, green, and blue channel values
     *  provided as integers. The `max` parameter specifies the maximum possible
     *  value for the input channels, which is used to normalize the values into the
     *  range `[0, 1]`.
     *
     *  @param r The intensity of the red channel as an integer, ranging from `0`
     *           to `max`.
     *  @param g The intensity of the green channel as an integer, ranging from `0`
     *           to `max`.
     *  @param b The intensity of the blue channel as an integer, ranging from `0`
     *           to `max`.
     *  @param max The maximum possible value for any of the input channels. This
     *             value must be greater than `0`.
     *
     *  @note Each input channel is normalized to a floating-point value in the
     *        range `[0, 1]` by dividing the channel value by `max`. For example,
     *        if `max` is `255`, and `r` is `128`, the red channel will be
     *        normalized to `0.5`.
     *
     *  @note If `max` is `0`, the resulting color will be white.
     */
    RGB::RGB(int32_t r, int32_t g, int32_t b, int32_t max) noexcept {
        if (max == 0) {
            m_data[0] = m_data[1] = m_data[2] = 1.0f;
        }
        else {
            float f = 1.0f / static_cast<float>(max);
            m_data[0] = f * static_cast<float>(r);
            m_data[1] = f * static_cast<float>(g);
            m_data[2] = f * static_cast<float>(b);
        }
    }


    /**
     *  @brief Constructs an RGB color from a pointer to an array of three float
     *         values.
     *
     *  This constructor initializes the RGB color from a float array of size 3.
     *  Each component (red, green, blue) is copied from the provided array,
     *  assuming the order is: red = values[0], green = values[1], blue = values[2].
     *
     *  @param values Pointer to an array of at least 3 float values representing
     *                RGB components. If the pointer is nullptr, the color remains
     *                uninitialized.
     *
     *  @note The constructor does not perform bounds checking on the input values.
     *        It is the caller's responsibility to ensure that `values` points to at
     *        least three valid float elements.
     *
     *  @see RGB(float r, float g, float b)
     */
    RGB::RGB(const float *values) noexcept {
        if (values != nullptr) {
            m_data[0] = values[0];
            m_data[1] = values[1];
            m_data[2] = values[2];
        }
    }


    /**
     *  @brief Constructor for creating a scaled copy of an existing RGB color.
     *
     *  Initializes an `RGB` object by copying the values from another `RGB` object
     *  and scaling its red, green, and blue channel intensities by a specified
     *  factor.
     *
     *  @param rgb The source `RGB` object whose channel values will be copied and
     *             scaled.
     *  @param scale The scaling factor to apply to the red, green, and blue channel
     *               values. The resulting channel values will be calculated as:
     *               `scaled_value = original_value * scale`.
     *
     *  @note The resulting channel values are not clamped to any range. If the
     *        scaled values exceed the expected range `[0, 1]`, additional handling
     *        (e.g., clamping) may be required depending on the context.
     */
    RGB::RGB(const RGB& rgb, float scale) noexcept {
        m_data[0] = rgb.m_data[0] * scale;
        m_data[1] = rgb.m_data[1] * scale;
        m_data[2] = rgb.m_data[2] * scale;
    }


    /**
     *  @brief Constructor for blending two RGB colors.
     *
     *  Initializes an `RGB` object by blending two source colors, `a` and `b`,
     *  based on a specified blend factor. The resulting color is computed as a
     *  linear interpolation between the two input colors.
     *
     *  @param a The first source `RGB` color, contributing to the blend
     *           proportionally to `(1 - t)`.
     *  @param b The second source `RGB` color, contributing to the blend
     *           proportionally to `t`.
     *  @param t The blending factor, where:
     *           - `0.0` results in the new color being identical to `a`,
     *           - `1.0` results in the new color being identical to `b`,
     *           - Intermediate values blend the colors proportionally.
     *
     *  @note The `t` factor will be clamped to range `[0.0, 1.0]`.
     */
    RGB::RGB(const RGB& a, const RGB& b, float t) noexcept {
        setBlend(a, b, t);
    }


    /**
     *  @brief Constructor for converting an HSV color to an RGB color.
     *
     *  Initializes an `RGB` object by converting the given HSV color representation
     *  (hue, saturation, value) into the RGB color space.
     *
     *  @param hsv The source `HSV` color to be converted. It is expected to contain.
     *
     *  @note The conversion assumes that the `HSV` values are normalized within
     *        their typical ranges:
     *        - `h` in **turns** (wrapped to `[0.0, 1.0)` if println of bounds).
     *        - `s` and `v` in the range `[0, 1]`.
     *
     *  @warning If the input `HSV` values are outside their expected ranges, the
     *           behavior of this constructor is undefined unless clamping or
     *           wrapping is implemented.
     *
     *  @see HSV
     */
    RGB::RGB(const HSV& hsv) noexcept {
        Color::hsv_to_rgb(hsv.m_data, m_data);
    }


    /**
     *  @brief Constructor for converting an HSL color to an RGB color.
     */
    RGB::RGB(const HSL& hsl) noexcept {
        Color::hsl_to_rgb(hsl.m_data, m_data);
    }


    /**
     *  @brief Constructor for converting an YUV color to an RGB color.
     */
    RGB::RGB(const YUV& yuv, Color::Space yuv_color_space) noexcept {
        setYUV(yuv, yuv_color_space);
    }


    /**
     *  @brief Constructor for converting an CIEXYZ color to an RGB color.
     */
    RGB::RGB(const CIEXYZ& xyz) noexcept {
        setXYZ(xyz);
    }


    /**
     *  @brief Constructor for converting an CIExyY color to an RGB color.
     */
    RGB::RGB(const CIExyY& xyY) noexcept {
        setXyY(xyY);
    }


    /**
     *  @brief Constructor for converting an OKLab color to an RGB color.
     */
    RGB::RGB(const OKLab& oklab) noexcept {
        Color::oklab_to_rgb(oklab.m_data, m_data);
    }


    /**
     *  @brief Constructor for converting an OKLCh color to an RGB color.
     */
    RGB::RGB(const OKLCh& oklch) noexcept {
        Color::oklch_to_rgb(oklch.m_data, m_data);
    }


    /**
     *  @brief Constructor for converting a LMS color to an RGB color.
     */
    RGB::RGB(const LMS& lms) noexcept {
        m_data[0] =  4.0767416621f * lms.m_data[0] - 3.3077115913f * lms.m_data[1] + 0.2309699292f * lms.m_data[2];
        m_data[1] = -1.2684380046f * lms.m_data[0] + 2.6097574011f * lms.m_data[1] - 0.3413193965f * lms.m_data[2];
        m_data[2] = -0.0041960863f * lms.m_data[0] - 0.7034186147f * lms.m_data[1] + 1.7076147010f * lms.m_data[2];
    }


    /**
     *  @brief Constructs an RGB color object from a 24-bit color.
     */
    RGB::RGB(uint32_t value) noexcept {
        set24bit(value);
    }


    /**
     *  Constructs an RGB color object from a `GretagMacbethColor`.
     */
    RGB::RGB(Color::GretagMacbethColor gretag_macbeth_color) noexcept {
        setValues(NamedColor::gretagMacbethColorDataPtr(gretag_macbeth_color));
    }


    /**
     *  @brief Constructs an RGB color object from a `CrayolaColor`.
     */
    RGB::RGB(Color::CrayolaColor crayola_color) noexcept {
        setValues(NamedColor::crayolaColorDataPtr(crayola_color));
    }


    /**
     *  @brief Constructs an RGB color object from a CSV string representation.
     *
     *  @param csv A string containing RGB values separated by commas (e.g., "255,128,64").
     *             Each value represents the red, green, and blue components, respectively.
     *
     *  @note If the string does not contain valid RGB values, the resulting color may be undefined
     *        or default to black (0, 0, 0) depending on the implementation.
     */
    RGB::RGB(const char* csv) noexcept {
        setByCSV(csv);
    }


    RGB::RGB(const String& csv) noexcept {
        setByCSV(csv.utf8());
    }


    RGB& RGB::operator = (const HSV& v) {
        Color::hsv_to_rgb(v.m_data, m_data);
        return *this;
    }


    RGB& RGB::operator = (const HSL& v) {
        Color::hsl_to_rgb(v.m_data, m_data);
        return *this;
    }


    RGB& RGB::operator = (const OKLCh& v) {
        Color::oklch_to_rgb(v.m_data, m_data);
        return *this;
    }


    RGB& RGB::operator = (const OKLab& v) {
        Color::oklab_to_rgb(v.m_data, m_data);
        return *this;
    }


    RGB& RGB::operator = (Color::GretagMacbethColor v) {
        *this = RGB(v);
        return *this;
    }


    RGB& RGB::operator = (Color::CrayolaColor v) {
        *this = RGB(v);
        return *this;
    }


    uint32_t RGB::rgb24bit() const noexcept {
        return
            (static_cast<uint32_t>(Type::floatToUInt8(m_data[0])) << 16) +
            (static_cast<uint32_t>(Type::floatToUInt8(m_data[1])) << 8) +
            static_cast<uint32_t>(Type::floatToUInt8(m_data[2]));
    }


    void RGB::values(float* out_values) const noexcept {
        if (out_values != nullptr) {
            out_values[0] = m_data[0];
            out_values[1] = m_data[1];
            out_values[2] = m_data[2];
        }
    }


    void RGB::hexString(String& out_string, bool upper_case, bool c_style) const noexcept {
        static const char* formatStrings[] = { "%06x", "%06X", "0x%06x", "0x%06X" };
        out_string.setFormatted(16, formatStrings[upper_case | (c_style << 1)], rgb24bit());
    }


    float RGB::lumina(Color::Space yuv_color_space) const noexcept {
        return yuv_color_space == Color::Space::Rec601 ? lumina601() : lumina709();
    }


    float RGB::lumina601() const noexcept {
        return Color::kLumina601ScaleR * m_data[0] + Color::kLumina601ScaleG * m_data[1] + Color::kLumina601ScaleB * m_data[2];
    }


    float RGB::lumina709() const noexcept {
        return Color::kLumina709ScaleR * m_data[0] + Color::kLumina709ScaleG * m_data[1] + Color::kLumina709ScaleB * m_data[2];
    }


    float RGB::hsvValue() const noexcept {
        return Type::maxOf3(m_data[0], m_data[1], m_data[2]);
    }


    Vec2f RGB::CIExy() const noexcept {
        return CIEXYZ(*this).CIExy();
    }


    float RGB::uvAngle() const noexcept {
        YUV yuv(*this);
        Vec2f va(0.0, 0.0);
        Vec2f vb(0.0, 1.0);
        Vec2f vc(yuv.m_data[1], yuv.m_data[2]);
        float angle = va.angle(vb, vc);
        return vc.m_x > 0 ? -angle : angle;
    }


    float RGB::distance(const RGB& rgb) const noexcept {
        float dr = rgb.m_data[0] - m_data[0];
        float dg = rgb.m_data[1] - m_data[1];
        float db = rgb.m_data[2] - m_data[2];
        return std::sqrt(dr * dr + dg * dg + db * db);
    }


    float RGB::perceptualDistance(const RGB& rgb) const noexcept {
        float dr = rgb.m_data[0] - m_data[0];
        float dg = rgb.m_data[1] - m_data[1];
        float db = rgb.m_data[2] - m_data[2];
        return static_cast<float>(std::sqrt(0.299 * dr * dr + 0.587 * dg * dg + 0.114 * db * db));
    }


    bool RGB::isDark() const noexcept {
        return lumina() < 0.6f;
    }


    bool RGB::isSame(const RGB& rgb, float tolerance) const noexcept {
        return (std::fabs(m_data[0] - rgb.m_data[0]) <= tolerance &&
                std::fabs(m_data[1] - rgb.m_data[1]) <= tolerance &&
                std::fabs(m_data[2] - rgb.m_data[2]) <= tolerance);
    }


    void RGB::set24bit(uint32_t value) noexcept {
        m_data[0] = static_cast<float>((value >> 16) & 0xFF) / 255.0f;
        m_data[1] = static_cast<float>((value >> 8) & 0xFF) / 255.0f;
        m_data[2] = static_cast<float>(value & 0xFF) / 255.0f;
    }


    void RGB::setUInt8(uint8_t r, uint8_t g, uint8_t b) noexcept {
        m_data[0] = static_cast<float>(r) / 255.0f;
        m_data[1] = static_cast<float>(g) / 255.0f;
        m_data[2] = static_cast<float>(b) / 255.0f;
    }


    void RGB::setValues(const float* values) noexcept {
        if (values != nullptr) {
            m_data[0] = values[0];
            m_data[1] = values[1];
            m_data[2] = values[2];
        }
    }


    void RGB::setValues(const float* values, float scale) noexcept {
        if (values != nullptr) {
            m_data[0] = values[0] * scale;
            m_data[1] = values[1] * scale;
            m_data[2] = values[2] * scale;
        }
    }


    bool RGB::setSystemAndValues(const char* system_name, float v1, float v2, float v3, float v4) noexcept {
        if (strcmp(system_name, "rgb") == 0) {
            set(v1, v2, v3);
            return true;
        }
        else if (strcmp(system_name, "hsv") == 0) {
            setHSV(v1, v2, v3);
            return true;
        }
        else if (strcmp(system_name, "oklch") == 0) {
            setOKLCh(v1, v2, v3);
            return true;
        }
        return false;
    }


    void RGB::setIntRGB(int32_t r, int32_t g, int32_t b, int32_t max) noexcept {
        if (max != 0) {
            float f = 1.0f / static_cast<float>(max);
            m_data[0] = f * static_cast<float>(r);
            m_data[1] = f * static_cast<float>(g);
            m_data[2] = f * static_cast<float>(b);
        }
    }


    void RGB::setHSV(float h, float s, float v) noexcept {
        float hsv[3] = { h, s, v };
        Color::hsv_to_rgb(hsv, m_data);
    }


    void RGB::setYUV(const YUV& yuv, Color::Space yuv_color_space) noexcept {
        switch (yuv_color_space) {
            case Color::Space::Rec601:
                setYUV601(yuv);
                break;

            case Color::Space::Rec709:
            default:
                setYUV709(yuv);
                break;
        }
    }


    void RGB::setYUV601(const YUV& yuv) noexcept {
        Color::yuv601_to_rgb(yuv.dataPtr(), m_data);
    }


    void RGB::setYUV709(const YUV& yuv) noexcept {
        Color::yuv709_to_rgb(yuv.dataPtr(), m_data);
    }


    void RGB::setXYZ(const CIEXYZ& xyz) noexcept {
        float r =  3.2406f * xyz.m_data[0] - 1.5372f * xyz.m_data[1] - 0.4986f * xyz.m_data[2];
        float g = -0.9689f * xyz.m_data[0] + 1.8758f * xyz.m_data[1] + 0.0415f * xyz.m_data[2];
        float b =  0.0557f * xyz.m_data[0] - 0.2040f * xyz.m_data[1] + 1.0570f * xyz.m_data[2];
        m_data[0] = std::clamp<float>(Color::linear_to_gamma(r), 0.0f, 1.0f);
        m_data[1] = std::clamp<float>(Color::linear_to_gamma(g), 0.0f, 1.0f);
        m_data[2] = std::clamp<float>(Color::linear_to_gamma(b), 0.0f, 1.0f);
    }


    void RGB::setXyY(const CIExyY& xyY) noexcept {
        CIEXYZ xyz(xyY);
        setXYZ(xyz);
    }


    void RGB::setCIExy(const Vec2f& xy) noexcept {
        CIEXYZ xyz(xy.m_x, xy.m_y, 1);
        setXYZ(xyz);
    }


    void RGB::setOKLab(const OKLab& oklab) noexcept {
        Color::oklab_to_rgb(oklab.m_data, m_data);
    }


    void RGB::setOKLCh(const OKLCh& oklch) noexcept {
        setOKLab(OKLab(oklch));
    }


    void RGB::setOKLCh(float l, float c, float h) noexcept {
        setOKLCh(OKLCh(l, c, h));
    }


    // TODO: Support the different skin types Skin types
    void RGB::setSkinColor(Color::SkinType skin_type, float value) noexcept {
        float min = 140.0f / 255;
        float max = 180.0f / 255;
        float r_scale = 1.5f;
        float g_scale = 1.15f;
        value = std::clamp<float>(value, 0.0f, 1.0f);
        m_data[2] = value * max + (1 - value) * min;
        m_data[0] = m_data[2] * r_scale;
        m_data[1] = m_data[2] * g_scale;
    }


    void RGB::setKelvin(float temperature) noexcept {
        CIEXYZ xyz;
        xyz.setKelvin(temperature);
        setXYZ(xyz);
        clamp();
    }


    /**
     *  @brief Sets the RGB color from a comma-separated string.
     *
     *  This function parses a comma-separated string to initialize the RGB values.
     *  Depending on the number of values in the input string:
     *  - 1 value: All RGB components are set to that single value.
     *  - 3 values: Values are assigned directly to R, G, B.
     *  - 4 values: First three values are assigned to R, G, B, and the fourth is
     *    used to normalize them (R/G/B are divided by the fou4thrth value if it's
     *    greater than std::numeric_limits<float>::epsilon()).
     *
     *  @param csv A null-terminated string containing comma-separated numeric
     *             values (e.g., "0.1,0.2,0.3", "128,64,255,255", or just "0.5").
     *  @return The number of values parsed and used from the string.
     *
     *  @note If csv is nullptr, the color is set to black.
     */
    int32_t RGB::setByCSV(const char* csv) noexcept {
        if (!csv) {
            *this = kBlack;
            return -1;
        }

        CSVLineParser csv_line_parser(csv);

        float values[5];
        int32_t n = 0;
        while (n < 5 && csv_line_parser.next<float>(values[n])) {
            n++;
        }

        if (n == 1) {
            // Grey
            m_data[0] = m_data[1] = m_data[2] = values[0];
        }
        else if (n == 3) {
            // RGB
            m_data[0] = values[0];
            m_data[1] = values[1];
            m_data[2] = values[2];
        }
        else if (n == 4) {
            // RGB, scaled
            m_data[0] = values[0];
            m_data[1] = values[1];
            m_data[2] = values[2];
            float max = values[3];
            if (max > std::numeric_limits<float>::epsilon()) {
                float scale = 1.0f / max;
                m_data[0] *= scale;
                m_data[1] *= scale;
                m_data[2] *= scale;
            }
        }

        return n;
    }


    void RGB::setPosColor(const Vec3d& pos) noexcept {
        Vec3d loc = pos.posToLoc();
        setByPosOnCircle(static_cast<float>(loc.m_x), static_cast<float>(loc.m_y));
    }


    void RGB::setByPosOnCircle(float angle, float distance) noexcept {
        float hsv[3];
        hsv[0] = static_cast<float>(Geometry::normalizeAngle(angle + 45)) / 360.0f;
        hsv[1] = std::pow(std::clamp<float>(distance, 0.0f, 1.0f), 1.6f);
        hsv[2] = 1.0f;
        Color::hsv_to_rgb(hsv, m_data);
    }


    void RGB::random() noexcept {
        m_data[0] = Random::next();
        m_data[1] = Random::next();
        m_data[2] = Random::next();
    }


    void RGB::random(float min, float max) noexcept {
        m_data[0] = Random::next(min, max);
        m_data[1] = Random::next(min, max);
        m_data[2] = Random::next(min, max);
    }


    void RGB::randomGrey() noexcept {
        m_data[0] = m_data[1] = m_data[2] = Random::next();
    }


    void RGB::randomGrey(float min, float max) noexcept {
        m_data[0] = m_data[1] = m_data[2] = Random::next(min, max);
    }


    void RGB::randomHSV(float min_h, float max_h, float min_s, float max_s, float min_v, float max_v) noexcept {
        HSV hsv(Random::next(min_h, max_h), Random::next(min_s, max_s), Random::next(min_v, max_v));
        *this = hsv;
    }


    void RGB::normalize() noexcept {
        float max = Type::maxOf3(m_data[0], m_data[1], m_data[2]);
        if (max > std::numeric_limits<float>::min()) {
            float f = 1.0f / max;
            m_data[0] *= f;
            m_data[1] *= f;
            m_data[2] *= f;
        }
    }


    void RGB::clamp() noexcept {
        clamp(0.0f, 1.0f);
    }


    void RGB::clamp(float max) noexcept {
        if (m_data[0] > max) m_data[0] = max;
        if (m_data[1] > max) m_data[1] = max;
        if (m_data[2] > max) m_data[2] = max;
    }


    void RGB::clamp(float min, float max) noexcept {
        if (m_data[0] < min) m_data[0] = min; else if (m_data[0] > max) m_data[0] = max;
        if (m_data[1] < min) m_data[1] = min; else if (m_data[1] > max) m_data[1] = max;
        if (m_data[2] < min) m_data[2] = min; else if (m_data[2] > max) m_data[2] = max;
    }


    void RGB::clamp(const RGB& min, const RGB& max) noexcept {
        clampMin(min);
        clampMax(max);
    }


    void RGB::clampMin(const RGB& min) noexcept {
        if (m_data[0] < min.m_data[0]) m_data[0] = min.m_data[0];
        if (m_data[1] < min.m_data[1]) m_data[1] = min.m_data[1];
        if (m_data[2] < min.m_data[2]) m_data[2] = min.m_data[2];
    }


    void RGB::clampMax(const RGB& max) noexcept {
        if (m_data[0] > max.m_data[0]) m_data[0] = max.m_data[0];
        if (m_data[1] > max.m_data[1]) m_data[1] = max.m_data[1];
        if (m_data[2] > max.m_data[2]) m_data[2] = max.m_data[2];
    }


    void RGB::invert() noexcept {
        m_data[0] = 1.0f - m_data[0];
        m_data[1] = 1.0f - m_data[1];
        m_data[2] = 1.0f - m_data[2];
    }


    void RGB::rotateHue(float angle) noexcept {
        HSV hsv(*this);
        hsv.setHue(hsv.m_data[0] + angle / 360);
        *this = hsv;
    }


    void RGB::scale(float scale) noexcept {
        m_data[0] *= scale;
        m_data[1] *= scale;
        m_data[2] *= scale;
    }


    void RGB::scaleValue(float scale) noexcept {
        HSV hsv(*this);
        hsv.m_data[2] *= scale;
        *this = hsv;
    }


    void RGB::applyCDL(const CDL& cdl) noexcept {
        CDL_RGB cdl_rgb;
        cdl.buildCDL_RGB(cdl_rgb);
        applyCDL(cdl_rgb);
    }


    void RGB::applyCDL(const CDL_RGB& cdl_rgb) noexcept {
        m_data[0] = (std::pow(m_data[0], cdl_rgb.m_gamma_rgb.m_data[0]) - cdl_rgb.m_shift1_rgb.m_data[0]) * cdl_rgb.m_gain_rgb.m_data[0] + cdl_rgb.m_shift2_rgb.m_data[0];
        m_data[1] = (std::pow(m_data[1], cdl_rgb.m_gamma_rgb.m_data[1]) - cdl_rgb.m_shift1_rgb.m_data[1]) * cdl_rgb.m_gain_rgb.m_data[1] + cdl_rgb.m_shift2_rgb.m_data[1];
        m_data[2] = (std::pow(m_data[2], cdl_rgb.m_gamma_rgb.m_data[2]) - cdl_rgb.m_shift1_rgb.m_data[2]) * cdl_rgb.m_gain_rgb.m_data[2] + cdl_rgb.m_shift2_rgb.m_data[2];
    }


    void RGB::applyRGBLUT(const LUT1& red_lut, const LUT1& green_lut, const LUT1& blue_lut) noexcept {
        m_data[0] = red_lut.lookup(m_data[0]);
        m_data[1] = green_lut.lookup(m_data[1]);
        m_data[2] = blue_lut.lookup(m_data[2]);
    }


    void RGB::applyPow(float e) noexcept {
        if (m_data[0] >= std::numeric_limits<float>::epsilon()) {
            m_data[0] = std::pow(m_data[0], e);
        }
        if (m_data[1] >= std::numeric_limits<float>::epsilon()) {
            m_data[1] = std::pow(m_data[1], e);
        }
        if (m_data[2] >= std::numeric_limits<float>::epsilon()) {
            m_data[2] = std::pow(m_data[2], e);
        }
    }


    void RGB::linearTosRGB() noexcept {
        m_data[0] = Color::linear_to_gamma(m_data[0]);
        m_data[1] = Color::linear_to_gamma(m_data[1]);
        m_data[2] = Color::linear_to_gamma(m_data[2]);
    }


    void RGB::sRGBToLinear() noexcept {
        m_data[0] = Color::linear_to_gamma(m_data[0]);
        m_data[1] = Color::linear_to_gamma(m_data[1]);
        m_data[2] = Color::linear_to_gamma(m_data[2]);
    }


    void RGB::sonySLog2ToLinear() noexcept {
        m_data[0] = Color::sony_SLog2_to_linear(m_data[0]);
        m_data[1] = Color::sony_SLog2_to_linear(m_data[1]);
        m_data[2] = Color::sony_SLog2_to_linear(m_data[2]);
    }


    void RGB::sonySLog3ToLinear() noexcept {
        m_data[0] = Color::sony_SLog3_to_Linear(m_data[0]);
        m_data[1] = Color::sony_SLog3_to_Linear(m_data[1]);
        m_data[2] = Color::sony_SLog3_to_Linear(m_data[2]);
    }


    void RGB::sonyLinearToSLog2() noexcept {
        m_data[0] = Color::sony_Linear_to_SLog2(m_data[0]);
        m_data[1] = Color::sony_Linear_to_SLog2(m_data[1]);
        m_data[2] = Color::sony_Linear_to_SLog2(m_data[2]);
    }


    void RGB::sonyLinearToSLog3() noexcept {
        m_data[0] = Color::sony_Linear_to_SLog3(m_data[0]);
        m_data[1] = Color::sony_Linear_to_SLog3(m_data[1]);
        m_data[2] = Color::sony_Linear_to_SLog3(m_data[2]);
    }


    void RGB::transform(const Mat3f& matrix) noexcept {
        matrix.transform3(m_data, m_data);
    }


    void RGB::transform(const Mat3f& matrix, RGB& out_rgb) const noexcept {
        matrix.transform3(m_data, out_rgb.m_data);
    }


    void RGB::transform(const Mat3f& matrix, CIEXYZ& out_xyz) const noexcept {
        RGB srgb = *this;
        srgb.sRGBToLinear();
        matrix.transform3(m_data, out_xyz.mutDataPtr());
    }


    RGB RGB::blend(const RGB& rgb, float t) const noexcept {
        if (t < 0.0f) t = 0.0f;
        else if (t > 1.0f) t = 1.0f;
        return rgb * t + *this * (1.0f - t);
    }


    void RGB::setBlend(const RGB& other, float t) noexcept {
        if (t < 0.0f) t = 0.0f;
        else if (t > 1.0f) t = 1.0f;
        float t_inv = 1.0f - t;
        m_data[0] = m_data[0] * t_inv + other.m_data[0] * t;
        m_data[1] = m_data[1] * t_inv + other.m_data[1] * t;
        m_data[2] = m_data[2] * t_inv + other.m_data[2] * t;
    }


    void RGB::setBlend(const RGB& a, const RGB& b, float t) noexcept {
        if (t < 0.0f) t = 0.0f;
        else if (t > 1.0f) t = 1.0f;
        float t_inv = 1.0f - t;
        m_data[0] = a.m_data[0] * t_inv + b.m_data[0] * t;
        m_data[1] = a.m_data[1] * t_inv + b.m_data[1] * t;
        m_data[2] = a.m_data[2] * t_inv + b.m_data[2] * t;
    }


    void RGB::setBlend(const RGB& a, const RGB& b, const RGB& c, float t) noexcept {
        if (t < 0.0f) t = 0.0f;
        else if (t > 1.0f) t = 1.0f;
        t >= 0.5f ? setBlend(b, c, t * 2 - 1) : setBlend(a, b, t * 2);
    }


    void RGB::setBlendWhite(float t) noexcept {
        if (t < 0.0f) t = 0.0f;
        else if (t > 1.0f) t = 1.0f;
        float t_inv = 1.0f - t;
        m_data[0] = m_data[0] * t_inv + t;
        m_data[1] = m_data[1] * t_inv + t;
        m_data[2] = m_data[2] * t_inv + t;
    }


    void RGB::setBlendBlack(float t) noexcept {
        if (t < 0.0f) t = 0.0f;
        else if (t > 1.0f) t = 1.0f;
        float t_inv = 1.0f - t;
        m_data[0] *= t_inv;
        m_data[1] *= t_inv;
        m_data[2] *= t_inv;
    }


    void RGB::mixbox(const RGB& color1, const RGB& color2, float t) noexcept {
        mixbox_latent l1, l2, lmix;
        auto c1 = color1.m_data;
        auto c2 = color2.m_data;
        mixbox_float_rgb_to_latent(c1[0], c1[1], c1[2], l1);
        mixbox_float_rgb_to_latent(c2[0], c2[1], c2[2], l2);
        float t_inv = (1.0f - t);
        for (int i = 0; i < MIXBOX_LATENT_SIZE; i++) {
            lmix[i] = t_inv * l1[i] + t * l2[i];
        }
        mixbox_latent_to_float_rgb(lmix, &m_data[0], &m_data[1], &m_data[2]);
    }


    void RGB::mixbox3(const RGB& color1, const RGB& color2, const RGB& color3, float f1, float f2, float f3) noexcept {
        mixbox_latent l1, l2, l3, lmix;
        auto c1 = color1.m_data;
        auto c2 = color2.m_data;
        auto c3 = color3.m_data;
        mixbox_float_rgb_to_latent(c1[0], c1[1], c1[2], l1);
        mixbox_float_rgb_to_latent(c2[0], c2[1], c2[2], l2);
        mixbox_float_rgb_to_latent(c3[0], c3[1], c3[2], l3);

        for (int i = 0; i < MIXBOX_LATENT_SIZE; i++) {
            lmix[i] = f1 * l1[i] + f2 * l2[i] + f3 * l3[i];
        }

        mixbox_latent_to_float_rgb(lmix, &m_data[0], &m_data[1], &m_data[2]);
    }


    RGBCombineFunc RGB::rgbCombineFunc(Color::CombineMode combine_mode) noexcept {
        switch (combine_mode) {
            case Color::CombineMode::Normal: return combineNormal;
            case Color::CombineMode::Add: return combineAdd;
            case Color::CombineMode::Subtract: return combineSubtract;
            case Color::CombineMode::Multiply: return combineMultiply;
            case Color::CombineMode::Screen: return combineScreen;
            case Color::CombineMode::Overlay: return combineOverlay;
            case Color::CombineMode::SoftLight: return combineSoftLight;
            case Color::CombineMode::HardLight: return combineHardLight;
            case Color::CombineMode::Hue: return combineHue;
            case Color::CombineMode::Color: return combineColor;
            case Color::CombineMode::Luminosity: return combineLuminosity;
        }

        return combineNormal;
    }


    void RGB::combineNormal(RGB& a, const RGB& b) noexcept {
        a.m_data[0] = b.m_data[0];
        a.m_data[1] = b.m_data[1];
        a.m_data[2] = b.m_data[2];
    }


    void RGB::combineAdd(RGB& a, const RGB& b) noexcept {
        a.m_data[0] += b.m_data[0];
        a.m_data[1] += b.m_data[1];
        a.m_data[2] += b.m_data[2];
    }


    void RGB::combineSubtract(RGB& a, const RGB& b) noexcept {
        a.m_data[0] -= b.m_data[0];
        a.m_data[1] -= b.m_data[1];
        a.m_data[2] -= b.m_data[2];
    }


    void RGB::combineMultiply(RGB& a, const RGB& b) noexcept {
        a.m_data[0] *= b.m_data[0];
        a.m_data[1] *= b.m_data[1];
        a.m_data[2] *= b.m_data[2];
    }


    void RGB::combineScreen(RGB& a, const RGB& b) noexcept {
        a.m_data[0] = Color::combineScreen(a.m_data[0], b.m_data[0]);
        a.m_data[1] = Color::combineScreen(a.m_data[1], b.m_data[1]);
        a.m_data[2] = Color::combineScreen(a.m_data[2], b.m_data[2]);
    }


    void RGB::combineOverlay(RGB& a, const RGB& b) noexcept {
        a.m_data[0] = Color::combineOverlay(a.m_data[0], b.m_data[0]);
        a.m_data[1] = Color::combineOverlay(a.m_data[1], b.m_data[1]);
        a.m_data[2] = Color::combineOverlay(a.m_data[2], b.m_data[2]);
    }


    void RGB::combineSoftLight(RGB& a, const RGB& b) noexcept {
        a.m_data[0] = Color::combineSoftLight(a.m_data[0], b.m_data[0]);
        a.m_data[1] = Color::combineSoftLight(a.m_data[1], b.m_data[1]);
        a.m_data[2] = Color::combineSoftLight(a.m_data[2], b.m_data[2]);
    }


    void RGB::combineHardLight(RGB& a, const RGB& b) noexcept {
        a.m_data[0] = Color::combineHardLight(a.m_data[0], b.m_data[0]);
        a.m_data[1] = Color::combineHardLight(a.m_data[1], b.m_data[1]);
        a.m_data[2] = Color::combineHardLight(a.m_data[2], b.m_data[2]);
    }


    void RGB::combineHue(RGB& a, const RGB& b) noexcept {
        HSV hvs_a(a);
        HSV hsv_b(b);
        hvs_a.m_data[0] = hsv_b.m_data[0];
        a = hvs_a;
    }


    void RGB::combineColor(RGB& a, const RGB& b) noexcept {
        HSV hvs_a(a);
        HSV hsv_b(b);
        hvs_a.m_data[0] = hsv_b.m_data[0];
        hvs_a.m_data[1] = hsv_b.m_data[1];
        a = hvs_a;
    }


    void RGB::combineLuminosity(RGB& a, const RGB& b) noexcept {
        HSV hvs_a(a);
        HSV hsv_b(b);
        hvs_a.m_data[2] = hsv_b.m_data[2];
        a = hvs_a;
    }


    void RGB::readFromMem(const float* ptr) noexcept {
        if (ptr != nullptr) {
            m_data[0] = ptr[0];
            m_data[1] = ptr[1];
            m_data[2] = ptr[2];
        }
    }


    void RGB::writeToMem(float*ptr) const noexcept {
        if (ptr != nullptr) {
            ptr[0] = m_data[0];
            ptr[1] = m_data[1];
            ptr[2] = m_data[2];
        }
    }


    void RGB::writeToMemUInt8(uint8_t* ptr) const noexcept {
        if (ptr != nullptr) {
            ptr[0] = Type::floatToUInt8(m_data[0]);
            ptr[1] = Type::floatToUInt8(m_data[1]);
            ptr[2] = Type::floatToUInt8(m_data[2]);
        }
    }


    void RGB::writeToMemUInt16(uint16_t* ptr) const noexcept {
        if (ptr != nullptr) {
            ptr[0] = Type::floatToUInt16(m_data[0]);
            ptr[1] = Type::floatToUInt16(m_data[1]);
            ptr[2] = Type::floatToUInt16(m_data[2]);
        }
    }


    RGB RGB::uiTextColor(bool enabled) const noexcept {
        RGB color = isDark() ? RGB::kWhite : RGB::kBlack;
        color.setBlend(*this, enabled ? 0.3f : 0.8f);
        return color;
    }


    RGB RGB::statusColor(bool selected, bool highlighted, const RGB& bg_color, const RGB& fg_color) noexcept{
        RGB color = bg_color;
        if (highlighted) {
            color.setBlend(fg_color, 0.5f);
            return color;
        }
        if (selected) {
            color = fg_color;
        }

        return color;
    }


#if defined(__APPLE__) && defined(__MACH__)

    CGColorRef RGB::createCGColor(float alpha) const noexcept {
        return CGColorCreateGenericRGB(m_data[0], m_data[1], m_data[2], alpha);
    }

#endif


} // End of namespace Grain
