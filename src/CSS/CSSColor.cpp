//
//  CSSColor.hpp
//
//  Created by Roald Christesen on from 07.01.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "CSS/CSSColor.hpp"
#include "CSS/CSS.hpp"
#include "String/String.hpp"
#include "Color/RGBA.hpp"
#include "Color/HSL.hpp"
#include "Color/OKColor.hpp"


namespace Grain {

    const CSSNamedColor CSSColor::_g_named_css_colors[] = {
            { "transparent", 0x00000000U },
            { "aliceblue", 0xF0F8FFFFu },
            { "antiquewhite", 0xFAEBD7FFu },
            { "aqua", 0x00FFFFFFu },
            { "aquamarine", 0x7FFFD4FFu },
            { "azure", 0xF0FFFFFFu },
            { "beige", 0xF5F5DCFFu },
            { "bisque", 0xFFE4C4FFu },
            { "black", 0x000000FFu },
            { "blanchedalmond", 0xFFEBCDFFu },
            { "blue", 0x0000FFFFu },
            { "blueviolet", 0x8A2BE2FFu },
            { "brown", 0xA52A2AFFu },
            { "burlywood", 0xDEB887FFu },
            { "cadetblue", 0x5F9EA0FFu },
            { "chartreuse", 0x7FFF00FFu },
            { "chocolate", 0xD2691EFFu },
            { "coral", 0xFF7F50FFu },
            { "cornflowerblue", 0x6495EDFFu },
            { "cornsilk", 0xFFF8DCFFu },
            { "crimson", 0xDC143CFFu },
            { "cyan", 0x00FFFFFFu },
            { "darkblue", 0x00008BFFu },
            { "darkcyan", 0x008B8BFFu },
            { "darkgoldenrod", 0xB8860BFFu },
            { "darkgray", 0xA9A9A9FFu },
            { "darkgreen", 0x006400FFu },
            { "darkgrey", 0xA9A9A9FFu },
            { "darkkhaki", 0xBDB76BFFu },
            { "darkmagenta", 0x8B008BFFu },
            { "darkolivegreen", 0x556B2FFFu },
            { "darkorange", 0xFF8C00FFu },
            { "darkorchid", 0x9932CCFFu },
            { "darkred", 0x8B0000FFu },
            { "darksalmon", 0xE9967AFFu },
            { "darkseagreen", 0x8FBC8FFFu },
            { "darkslateblue", 0x483D8BFFu },
            { "darkslategray", 0x2F4F4FFFu },
            { "darkslategrey", 0x2F4F4FFFu },
            { "darkturquoise", 0x00CED1FFu },
            { "darkviolet", 0x9400D3FFu },
            { "deeppink", 0xFF1493FFu },
            { "deepskyblue", 0x00BFFFFFu },
            { "dimgray", 0x696969FFu },
            { "dimgrey", 0x696969FFu },
            { "dodgerblue", 0x1E90FFFFu },
            { "firebrick", 0xB22222FFu },
            { "floralwhite", 0xFFFAF0FFu },
            { "forestgreen", 0x228B22FFu },
            { "fuchsia", 0xFF00FFFFu },
            { "gainsboro", 0xDCDCDCFFu },
            { "ghostwhite", 0xF8F8FFFFu },
            { "gold", 0xFFD700FFu },
            { "goldenrod", 0xDAA520FFu },
            { "gray", 0x808080FFu },
            { "green", 0x008000FFu },
            { "greenyellow", 0xADFF2FFFu },
            { "grey", 0x808080FFu },
            { "honeydew", 0xF0FFF0FFu },
            { "hotpink", 0xFF69B4FFu },
            { "indianred", 0xCD5C5CFFu },
            { "indigo", 0x4B0082FFu },
            { "ivory", 0xFFFFF0FFu },
            { "khaki", 0xF0E68CFFu },
            { "lavender", 0xE6E6FAFFu },
            { "lavenderblush", 0xFFF0F5FFu },
            { "lawngreen", 0x7CFC00FFu },
            { "lemonchiffon", 0xFFFACDFFu },
            { "lightblue", 0xADD8E6FFu },
            { "lightcoral", 0xF08080FFu },
            { "lightcyan", 0xE0FFFFFFu },
            { "lightgoldenrodyellow", 0xFAFAD2FFu },
            { "lightgray", 0xD3D3D3FFu },
            { "lightgreen", 0x90EE90FFu },
            { "lightgrey", 0xD3D3D3FFu },
            { "lightpink", 0xFFB6C1FFu },
            { "lightsalmon", 0xFFA07AFFu },
            { "lightseagreen", 0x20B2AAFFu },
            { "lightskyblue", 0x87CEFAFFu },
            { "lightslategray", 0x778899FFu },
            { "lightslategrey", 0x778899FFu },
            { "lightsteelblue", 0xB0C4DEFFu },
            { "lightyellow", 0xFFFFE0FFu },
            { "lime", 0x00FF00FFu },
            { "limegreen", 0x32CD32FFu },
            { "linen", 0xFAF0E6FFu },
            { "magenta", 0xFF00FFFFu },
            { "maroon", 0x800000FFu },
            { "mediumaquamarine", 0x66CDAAFFu },
            { "mediumblue", 0x0000CDFFu },
            { "mediumorchid", 0xBA55D3FFu },
            { "mediumpurple", 0x9370DBFFu },
            { "mediumseagreen", 0x3CB371FFu },
            { "mediumslateblue", 0x7B68EEFFu },
            { "mediumspringgreen", 0x00FA9AFFu },
            { "mediumturquoise", 0x48D1CCFFu },
            { "mediumvioletred", 0xC71585FFu },
            { "midnightblue", 0x191970FFu },
            { "mintcream", 0xF5FFFAFFu },
            { "mistyrose", 0xFFE4E1FFu },
            { "moccasin", 0xFFE4B5FFu },
            { "navajowhite", 0xFFDEADFFu },
            { "navy", 0x000080FFu },
            { "oldlace", 0xFDF5E6FFu },
            { "olive", 0x808000FFu },
            { "olivedrab", 0x6B8E23FFu },
            { "orange", 0xFFA500FFu },
            { "orangered", 0xFF4500FFu },
            { "orchid", 0xDA70D6FFu },
            { "palegoldenrod", 0xEEE8AAFFu },
            { "palegreen", 0x98FB98FFu },
            { "paleturquoise", 0xAFEEEEFFu },
            { "palevioletred", 0xDB7093FFu },
            { "papayawhip", 0xFFEFD5FFu },
            { "peachpuff", 0xFFDAB9FFu },
            { "peru", 0xCD853FFFu },
            { "pink", 0xFFC0CBFFu },
            { "plum", 0xDDA0DDFFu },
            { "powderblue", 0xB0E0E6FFu },
            { "purple", 0x800080FFu },
            { "red", 0xFF0000FFu },
            { "rosybrown", 0xBC8F8FFFu },
            { "royalblue", 0x4169E1FFu },
            { "saddlebrown", 0x8B4513FFu },
            { "salmon", 0xFA8072FFu },
            { "sandybrown", 0xF4A460FFu },
            { "seagreen", 0x2E8B57FFu },
            { "seashell", 0xFFF5EEFFu },
            { "sienna", 0xA0522DFFu },
            { "silver", 0xC0C0C0FFu },
            { "skyblue", 0x87CEEBFFu },
            { "slateblue", 0x6A5ACDFFu },
            { "slategray", 0x708090FFu },
            { "slategrey", 0x708090FFu },
            { "snow", 0xFFFAFAFFu },
            { "springgreen", 0x00FF7FFFu },
            { "steelblue", 0x4682B4FFu },
            { "tan", 0xD2B48CFFu },
            { "teal", 0x008080FFu },
            { "thistle", 0xD8BFD8FFu },
            { "tomato", 0xFF6347FFu },
            { "turquoise", 0x40E0D0FFu },
            { "violet", 0xEE82EEFFu },
            { "wheat", 0xF5DEB3FFu },
            { "white", 0xFFFFFFFFu },
            { "whitesmoke", 0xF5F5F5FFu },
            { "yellow", 0xFFFF00FFu },
            { "yellowgreen", 0x9ACD32FFu },

            { nullptr, 0x0 } // Sentinel item (end of list).
    };

    const CSSColorFunctionInfo CSSColor::_g_color_function_infos[]  = {
            { "rgb(",   4, CSSColorFunction::RGB, 3, true },
            { "rgba(",  5, CSSColorFunction::RGBA, 4, false },
            { "hsl(",   4, CSSColorFunction::HSL, 3, true },
            { "hsla(",  5, CSSColorFunction::HSLA, 4, false },
            { "hwb(",   4, CSSColorFunction::HWB, 3, true },
            { "cmyk(",  5, CSSColorFunction::CMYK, 4, true },
            { "lab(",   4, CSSColorFunction::Lab, 3, true },
            { "lch(",   4, CSSColorFunction::Lch, 3, true },
            { "color(", 6, CSSColorFunction::Color, 0, true },  // TODO: Check if `m_can_modern_syntax` is correct!
            { "gray(",  5, CSSColorFunction::Gray, 1, true },
            { "oklch(", 6, CSSColorFunction::OKLCh, 3, true },
            { "oklab(", 6, CSSColorFunction::OKLab, 3, true },

            { nullptr, 0, CSSColorFunction::Undefined, 0, false } // Sentinel item (end of list).
    };


    ErrorCode CSSColor::parseColor(const char* css_str) noexcept {
        auto result = ErrorCode::None;

        m_valid = false;

        try {
            if (!css_str) { throw ErrorCode::BadArgs; }

            const char* ptr = css_str;

            // Skip all whitespaces.
            while (String::charIsWhiteSpace(*ptr)) {
                ptr++;
            }

            // Hexadecimal notation.
            if (*ptr == '#') {
                return parseHex(ptr + 1);
            }

            if (parseNamed(ptr)) {
                return ErrorCode::None;
            }

            ErrorCode functional_err;
            parseFunctional(ptr, functional_err);
            if (functional_err != ErrorCode::None) { throw functional_err; }
        }
        catch (ErrorCode err) {
            result = err;
        }

        return result;
    }


    ErrorCode CSSColor::parseHex(const char* str) noexcept {

        const int32_t kMaxHexLength = 8;
        auto result = ErrorCode::None;

        m_valid = false;

        try {
            if (!str) { throw ErrorCode::BadArgs; }

            int32_t index = 0;
            int8_t hex_digits[kMaxHexLength];  // Buffer for eight hex values.

            auto ptr = str;
            while (*ptr != '\0') {
                if (index < kMaxHexLength) {
                    hex_digits[index] = String::valueForHexChar(*ptr);
                    if (hex_digits[index] < 0) { throw ErrorCode::CSSNoneHexLetter; }
                    index++;
                }
                else {
                    throw ErrorCode::CSSToManyDigitsInHexCode;
                }
                ptr++;
            }
            switch (index) {
                case 3:
                    m_rgba.m_data[0] = static_cast<float>((hex_digits[0] << 4) + hex_digits[0]) / 255.0f;
                    m_rgba.m_data[1] = static_cast<float>((hex_digits[1] << 4) + hex_digits[1]) / 255.0f;
                    m_rgba.m_data[2] = static_cast<float>((hex_digits[2] << 4) + hex_digits[2]) / 255.0f;
                    m_rgba.m_alpha = 1.0f;
                    m_valid = true;
                    break;
                case 4:
                    m_rgba.m_data[0] = static_cast<float>((hex_digits[0] << 4) + hex_digits[0]) / 255.0f;
                    m_rgba.m_data[1] = static_cast<float>((hex_digits[1] << 4) + hex_digits[1]) / 255.0f;
                    m_rgba.m_data[2] = static_cast<float>((hex_digits[2] << 4) + hex_digits[2]) / 255.0f;
                    m_rgba.m_alpha = static_cast<float>((hex_digits[3] << 4) + hex_digits[3]) / 255.0f;
                    m_valid = true;
                    break;
                case 6:
                    m_rgba.m_data[0] = static_cast<float>((hex_digits[0] << 4) + hex_digits[1]) / 255.0f;
                    m_rgba.m_data[1] = static_cast<float>((hex_digits[2] << 4) + hex_digits[3]) / 255.0f;
                    m_rgba.m_data[2] = static_cast<float>((hex_digits[4] << 4) + hex_digits[5]) / 255.0f;
                    m_rgba.m_alpha = 1.0f;
                    m_valid = true;
                    break;
                case 8:
                    m_rgba.m_data[0] = static_cast<float>((hex_digits[0] << 4) + hex_digits[1]) / 255.0f;
                    m_rgba.m_data[1] = static_cast<float>((hex_digits[2] << 4) + hex_digits[3]) / 255.0f;
                    m_rgba.m_data[2] = static_cast<float>((hex_digits[4] << 4) + hex_digits[5]) / 255.0f;
                    m_rgba.m_alpha = static_cast<float>((hex_digits[6] << 4) + hex_digits[7]) / 255.0f;
                    m_valid = true;
                    break;
                default:
                    throw ErrorCode::CSSWrongDigitsInHexCode;
            }
        }
        catch (ErrorCode err) {
            m_valid = false;
            result = err;
        }

        return result;
    }


    bool CSSColor::parseNamed(const char* str) noexcept {

        m_valid = false;

        if (str == nullptr) {
            return false;
        }

        auto named_color = CSSColor::_g_named_css_colors;
        while (named_color->m_name != nullptr) {
            if (strcasecmp(str, named_color->m_name) == 0) {
                m_rgba.set32bit(named_color->m_color);
                m_valid = true;
                return true;
            }
            named_color++;
        }

        return false;
    }


    bool CSSColor::parseFunctional(const char* str, ErrorCode& out_err) noexcept {

        m_valid = false;

        out_err = ErrorCode::None;

        bool function_flag = false;
        CSSColorFunction function = CSSColorFunction::Undefined;
        CSSColorFunctionInfo function_info;
        int32_t component_n = 0;

        int32_t index = 0;

        const char* ptr = nullptr;

        while (_g_color_function_infos[index].m_signature != nullptr) {
            auto info = &_g_color_function_infos[index];
            if (strncasecmp(str, info->m_signature, info->m_length) == 0) {
                ptr = str + info->m_length;  // Pointer to the functions content part
                function_info = *info;
                function = info->m_function;
                function_flag = true;
                component_n = info->m_component_n;
                break;
            }
            index++;
        }

        if (!function_flag) {
            out_err = ErrorCode::None;
            return false;  // No function found
        }

        // Look for ending ')'
        auto end_ptr = strchr(ptr, ')');
        if (end_ptr == nullptr) {
            out_err = ErrorCode::CSSClosingBracketMissing;
            return false;
        }

        auto err = parseColorComponents(ptr, static_cast<int32_t>(end_ptr - ptr), component_n);
        if (err != ErrorCode::None) {
            out_err = err;
            return false;
        }

        int32_t must_have_comp_n = component_n;
        if (m_modern_syntax) {
            if (!function_info.m_can_modern_syntax) {
                out_err = ErrorCode::CSSColorFunctionDoesntSupportModernSyntax;
                return false;
            }
            must_have_comp_n++;
        }

        if (must_have_comp_n != m_parsed_comp_n) {
            out_err = ErrorCode::CSSWrongNumberOfValues;
            return false;
        }


        float alpha = 1.0f;
        if (m_modern_syntax) {
            auto alpha_value = &m_comp_values[m_parsed_comp_n - 1];
            if (!alpha_value->isWithoutUnitOrPercentage()) {
                return false;
            }
            alpha = alpha_value->valueAsFloat();
            if (alpha_value->unit() == CSSUnit::Percentage) {
                alpha *= 0.01;
            }
        }


        switch (function) {
            case CSSColorFunction::RGB:
                if (!m_comp_values[0].isColorLevelUnit() ||
                    !m_comp_values[1].isColorLevelUnit() ||
                    !m_comp_values[2].isColorLevelUnit()) {
                    return false;
                }
                m_rgba.set(m_comp_values[0].valueForColorLevel(), m_comp_values[1].valueForColorLevel(),
                           m_comp_values[2].valueForColorLevel());
                break;

            case CSSColorFunction::RGBA:
                if (!m_comp_values[0].isColorLevelUnit() ||
                    !m_comp_values[1].isColorLevelUnit() ||
                    !m_comp_values[2].isColorLevelUnit() ||
                    !m_comp_values[3].isWithoutUnitOrPercentage()) {
                    return false;
                }
                m_rgba.setRGBA(m_comp_values[0].valueForColorLevel(), m_comp_values[1].valueForColorLevel(),
                               m_comp_values[2].valueForColorLevel(), m_comp_values[3].valueAsFloat());
                break;

            case CSSColorFunction::HSL: {
                if (!m_comp_values[0].isAngleUnit() ||
                    !m_comp_values[1].isPercentage() ||
                    !m_comp_values[2].isPercentage()) {
                    return false;
                }
                HSL hsl(
                        m_comp_values[0].valueAsFloat() / 360.0f,
                        m_comp_values[1].valueForColorLevel(),
                        m_comp_values[2].valueForColorLevel());
                m_rgba.setRGB(RGB(hsl));
                break;
            }

            case CSSColorFunction::HSLA: {
                if (!m_comp_values[0].isAngleUnit() ||
                    !m_comp_values[1].isPercentage() ||
                    !m_comp_values[2].isPercentage() ||
                    !m_comp_values[3].isWithoutUnitOrPercentage()) {
                    return false;
                }
                HSL hsl(
                        m_comp_values[0].valueAsFloat() / 360.0f,
                        m_comp_values[1].valueForColorLevel(),
                        m_comp_values[2].valueForColorLevel());
                m_rgba.setRGBA(RGB(hsl), m_comp_values[3].valueForColorLevel());
                break;
            }

            case CSSColorFunction::HWB:
                #pragma message("case CSSColorFunction::HWB must be implemented")
                return false;
                break;

            case CSSColorFunction::CMYK:
                #pragma message("case CSSColorFunction::CMYK must be implemented")
                return false;
                break;

            case CSSColorFunction::Lab:
                #pragma message("case CSSColorFunction::Lab must be implemented")
                return false;
                break;

            case CSSColorFunction::Lch:
                #pragma message("case CSSColorFunction::Lch must be implemented")
                return false;
                break;

            case CSSColorFunction::Color:
                #pragma message("case CSSColorFunction::Color must be implemented")
                return false;
                break;

            case CSSColorFunction::Gray:
                if (!m_comp_values[0].isWithoutUnitOrPercentage()) {
                    return false;
                }
                m_rgba.setGrey(m_comp_values[0].valueForColorLevel());
                break;

            case CSSColorFunction::OKLCh: {
                if (!m_comp_values[0].isWithoutUnitOrPercentage() ||
                    !m_comp_values[1].isWithoutUnitOrPercentage() ||
                    !m_comp_values[2].isAngleUnit()) {
                    return false;
                }
                OKLCh oklch(
                        m_comp_values[0].valueForColorLevel(),
                        m_comp_values[1].valueForColorLevel(),
                        m_comp_values[2].valueForColorLevel() / 360.0f);
                m_rgba.setRGB(RGB(oklch));
                break;
            }

            case CSSColorFunction::OKLab: {
                if (!m_comp_values[0].isWithoutUnitOrPercentage() ||
                    !m_comp_values[1].isWithoutUnitOrPercentage() ||
                    !m_comp_values[2].isWithoutUnitOrPercentage()) {
                    return false;
                }
                OKLab oklab(m_comp_values[0].valueForColorLevel(),
                            m_comp_values[1].valueForColorLevel(),
                            m_comp_values[2].valueForColorLevel());
                m_rgba.setRGB(RGB(oklab));
                break;
            }

            default:
                break;
        }

        if (m_modern_syntax) {
            m_rgba.m_alpha = alpha;
        }

        m_valid = true;

        return true;
    }


    ErrorCode CSSColor::parseColorComponents(const char* str, int32_t str_length, int32_t component_n) noexcept {
        constexpr int32_t kMaxStrLength = 256;
        m_valid = false;
        m_parsed_comp_n = 0;

        if (str_length <= 0) {
            return ErrorCode::BadArgs;
        }

        if (str_length > kMaxStrLength) {
            return ErrorCode::LimitExceeded;
        }

        // Look for slash separator '/', which indicates modern syntax and separates the alpha component at the end.
        auto slash_separator_ptr = strchr(str, '/');
        m_modern_syntax = slash_separator_ptr != nullptr;

        char buffer[kMaxStrLength + 1];
        strncpy(buffer, str, str_length);
        buffer[str_length] = '\0';


        CSSValidator validator;
        if (!validator.checkValueContent(str)) {
            return ErrorCode::CSSWrongCommaDelimiterSequence;
        }

        String::replaceChar(buffer, ',', ' ');
        String::replaceChar(buffer, '/', ' ');

        int32_t index = 0;
        char* ptr = buffer;
        while (ptr != nullptr) {
            auto err = CSS::extractCSSValueFromStr(ptr, m_comp_values[index], &ptr);
            if (err != ErrorCode::None) {
                break;
            }

            // Check if the unit is valid for color components.
            // Valid units are `Absolute` and `Percentage`.
            CSSUnit unit = m_comp_values[index].unit();
            if (unit != CSSUnit::Absolute && unit != CSSUnit::Percentage) {
                return ErrorCode::CSSWrongUnit;
            }

            m_parsed_comp_n++;

            index++;
            if (index > kMaxValueComponents) {
                return ErrorCode::CSSValueOverflow;
            }
        }

        m_valid = true;

        return ErrorCode::None;
    }


    ErrorCode CSSColor::parseColorToRGB(const char* css_str, RGB& out_color) noexcept {

        CSSColor css_color;
        auto err = css_color.parseColor(css_str);
        if (err == ErrorCode::None) {
            out_color = css_color.rgb();
        }
        return err;
    }


    ErrorCode CSSColor::parseColorToRGB(const String& css_string, RGB& out_color) noexcept {

        return parseColorToRGB(css_string.utf8(), out_color);
    }


    ErrorCode CSSColor::parseColorToRGBA(const char* css_str, RGBA& out_color) noexcept {

        CSSColor css_color;
        auto err = css_color.parseColor(css_str);
        if (err == ErrorCode::None) {
            out_color = css_color.rgba();
        }
        return err;
    }


    ErrorCode CSSColor::parseColorToRGBA(const String& css_string, RGBA& out_color) noexcept {

        return parseColorToRGBA(css_string.utf8(), out_color);
    }


} // End of namespace Grain
