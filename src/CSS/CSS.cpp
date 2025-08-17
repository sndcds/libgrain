//
//  CSS.hpp
//
//  Created by Roald Christesen on from 31.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "CSS/CSS.hpp"
#include "String/String.hpp"


namespace Grain {


    const CSSUnitInfo CSS::_g_css_unit_infos[] = {

            { "undefined", 9, CSSUnit::Undefined, CSSUnitContext::Undefined },
            { "absolute", 8, CSSUnit::Absolute, CSSUnitContext::Absolute },

            { "mm", 2, CSSUnit::Millimeter, CSSUnitContext::Absolute },
            { "cm", 2, CSSUnit::Centimeter, CSSUnitContext::Absolute },
            { "q", 1, CSSUnit::QuarterMillimeter, CSSUnitContext::Absolute },
            { "in", 2, CSSUnit::Inch, CSSUnitContext::Absolute },
            { "px", 2, CSSUnit::Pixel, CSSUnitContext::Absolute },
            { "pt", 2, CSSUnit::Point, CSSUnitContext::Absolute },
            { "pc", 2, CSSUnit::Pica, CSSUnitContext::Absolute },

            // Font-Relative Units
            { "em", 2, CSSUnit::Relative_em, CSSUnitContext::Relative },  // Relative to the font size of the parent element
            { "rem", 3, CSSUnit::Relative_rem, CSSUnitContext::Relative }, // Relative to the font size of the root element
            { "ex", 2, CSSUnit::Relative_ex, CSSUnitContext::Relative },  // Relative to the height of the letter "x"
            { "ch", 2, CSSUnit::Relative_ch, CSSUnitContext::Relative },  // Relative to the width of the "0" (zero) character
            { "lh", 2, CSSUnit::Relative_lh, CSSUnitContext::Relative },  // Relative to the line-height of the element
            { "rlh", 3, CSSUnit::Relative_rlh, CSSUnitContext::Relative },  // Relative to the line-height of the root element

            // Viewport-Relative Units
            { "vw", 2, CSSUnit::Viewport_vw, CSSUnitContext::Relative },  // 1% of the viewport's width
            { "vh", 2, CSSUnit::Viewport_vh, CSSUnitContext::Relative },  // 1% of the viewport's height
            { "vmin", 4, CSSUnit::Viewport_vmin, CSSUnitContext::Relative },  // 1% of the smaller of the viewport's width or height
            { "vmax", 4, CSSUnit::Viewport_vmax, CSSUnitContext::Relative },  // 1% of the larger of the viewport's width or height
            { "svw", 3, CSSUnit::Viewport_svw, CSSUnitContext::Relative },  // 1% of the small viewport's width (introduced in CSS4)
            { "svh", 3, CSSUnit::Viewport_svh, CSSUnitContext::Relative },  // 1% of the small viewport's height
            { "lvw", 3, CSSUnit::Viewport_lvw, CSSUnitContext::Relative },  // 1% of the large viewport's width
            { "lvh", 3, CSSUnit::Viewport_lvh, CSSUnitContext::Relative },  // 1% of the large viewport's height
            { "dvw", 3, CSSUnit::Viewport_dvw, CSSUnitContext::Relative },  // 1% of the dynamic viewport's width (accounts for UI changes)
            { "dvh", 3, CSSUnit::Viewport_dvh, CSSUnitContext::Relative },  // 1% of the dynamic viewport's height

            // Container-Relative Units (CSS Containment)
            { "cqw", 3, CSSUnit::Container_cqw, CSSUnitContext::Relative },  // 1% of the container’s inline size
            { "cqh", 3, CSSUnit::Container_cqh, CSSUnitContext::Relative },  // 1% of the container’s block size
            { "cqmin", 5, CSSUnit::Container_cqmin, CSSUnitContext::Relative },  // 1% of the smaller of the container's dimensions
            { "cqmax", 5, CSSUnit::Container_cqmax, CSSUnitContext::Relative },  // 1% of the larger of the container's dimensions

            // Time Units
            { "s", 1, CSSUnit::Time_s, CSSUnitContext::Time },  // Seconds
            { "ms", 2, CSSUnit::Time_ms, CSSUnitContext::Time },  // Milliseconds

            // Angle Units
            { "deg", 3, CSSUnit::Angle_deg, CSSUnitContext::Angle },  // Degrees (360° in a circle)
            { "grad", 4, CSSUnit::Angle_grad, CSSUnitContext::Angle },  // Gradians (400 grads in a circle)
            { "rad", 3, CSSUnit::Angle_rad, CSSUnitContext::Angle },  // Radians (2π radians in a circle)
            { "turn", 4, CSSUnit::Angle_turn, CSSUnitContext::Angle },  // Turns (1 turn = 360°)

            // Frequency Units
            { "Hz", 2, CSSUnit::Frequency_Hz, CSSUnitContext::Frequency },  // Hertz
            { "kHz", 3, CSSUnit::Frequency_kHz, CSSUnitContext::Frequency },  // Kilohertz

            // Resolution Units
            { "dpi", 3, CSSUnit::Resolution_dpi, CSSUnitContext::Resolution },  // Dots per inch
            { "dpcm", 4, CSSUnit::Resolution_dpcm, CSSUnitContext::Resolution },  // Dots per centimeter
            { "dppx", 4, CSSUnit::Resolution_dppx, CSSUnitContext::Resolution },  // Dots per pixel (1dppx = 96dpi)

            // Percentage.
            { "%", 1, CSSUnit::Percentage, CSSUnitContext::Percentage },  // Percentage

            { "", 0, CSSUnit::Undefined, CSSUnitContext::Undefined }  // Sentinel item (end of list)
    };


    const char *CSSValue::unitName() const noexcept {

        return CSS::unitName(m_unit);
    }


    double CSSValue::valueAsDoubleConsiderPercentage() const noexcept {

        return m_unit == CSSUnit::Percentage ? m_value.asDouble() * 0.01 : m_value.asDouble();
    }


    double CSSValue::valueForColorLevel() const noexcept {

        float value = m_value.asFloat();
        if (m_unit == CSSUnit::Percentage) {
            return value * 0.01f;
        }
        else if (m_is_float) {
            return value;
        }
        else {
            return value / 255;
        }
    }


    double CSSValue::valueForAngleDegree() const noexcept {

        switch (m_unit) {
            case CSSUnit::Absolute:
            case CSSUnit::Angle_deg:
                return m_value.asFloat();

            case CSSUnit::Angle_grad:
                return m_value.asFloat() / 400 * 360;

            case CSSUnit::Angle_rad:
                return m_value.asFloat() / (std::numbers::pi * 2.0) * 360;

            case CSSUnit::Angle_turn:
                return m_value.asFloat() * 360;

            default:
                return m_value.asFloat();
        }
    }


    /**
     *  @brief Get value in unit of pixels.
     */
    double CSSValue::valueSVGPixel(double dpi) const noexcept {

        double value = valueAsDouble();

        switch (m_unit) {

            case CSSUnit::Millimeter:
                return value * 10 / 2.54 * dpi;

            case CSSUnit::Centimeter:
                return value / 2.54 * dpi;

            case CSSUnit::QuarterMillimeter:
                return value * 10 / 2.54 / 4 * dpi;

            case CSSUnit::Inch:
                return value * dpi;

            case CSSUnit::Point:
                return value * dpi / 72;

            case CSSUnit::Pica:
                return value * dpi / 6;

            case CSSUnit::Absolute:
            case CSSUnit::Pixel:
            default:
                return value;
        }
    }


    /**
     *  @brief Checks a C-string for values and delimiters.
     */
    bool CSSValidator::checkValueContent(const char *str) noexcept {

        if (str == nullptr) {
            return false;
        }

        // Init properties
        m_value_n = 0;
        m_comma_n = 0;
        m_slash_n = 0;
        m_pre_slash_value_n = 0;

        bool delimiter_flag = false;
        bool space_flag = false;
        bool comma_flag = false;
        bool slash_flag = false;
        int32_t value_length = 0;

        auto ptr = str;

        while (*ptr == ' ') {
            ptr++;
        }

        while (*ptr != '\0' && *ptr != ')') {

            bool c_space = *ptr == ' ';
            bool c_comma = *ptr == ',';
            bool c_slash = *ptr == '/';
            if (c_space || c_comma || c_slash) {
                if (!delimiter_flag) {
                    if (value_length < 1) {
                        return false;
                    }
                    m_value_n++;
                    value_length = 0;
                    delimiter_flag = true;
                }
                if (c_comma || c_slash) {
                    if (comma_flag || slash_flag) {
                        return false;
                    }
                    if (c_comma) {
                        comma_flag = true;
                        m_comma_n++;
                    }
                    if (c_slash) {
                        slash_flag = true;
                        if (m_pre_slash_value_n == 0) {
                            m_pre_slash_value_n = m_value_n;
                        }
                        m_slash_n++;
                    }
                }
            }
            else {
                value_length++;
                delimiter_flag = false;
                comma_flag = false;
                space_flag = false;
                slash_flag = false;
            }

            ptr++;
        }

        if (value_length > 0) {
            m_value_n++;
        }

        if (comma_flag || slash_flag) {
            return false;
        }

        return true;
    }


    CSS::CSS() noexcept {
    }


    CSS::~CSS() noexcept {
    }


    /**
     *  @brief Extract numeric values from a C-string as floats.
     *
     *  This function parses a C-string to extract up to `values_size` numeric values,
     *  storing them as floats in the `out_values` array. The function also uses a
     *  buffer to temporarily store segments of the input string during parsing.
     *
     *  @param[in] str          The input C-string to parse. Must be null-terminated.
     *  @param[out] buffer      A temporary character buffer used during parsing. This
     *                          buffer should have at least `buffer_size` capacity.
     *  @param[in] buffer_size  The size of the `buffer`. Must be large enough to store
     *                          segments of the input string during parsing.
     *  @param[in] closing_char If != '\0', this char is needed at the end of `str`.
     *  @param[in] values_size  The maximum number of numeric values to extract.
     *                          This determines the size of the `out_values` array.
     *  @param[out] out_values  An array where extracted float values will be stored.
     *                          Must have a capacity of at least `values_size`.
     *
     *  @return int32_t The number of numeric values successfully extracted and stored
     *                  in `out_values`.
     *
     *  @note
     *  - The function stops parsing either when `values_size` numbers are extracted
     *    or when the input string is fully processed.
     *  - Non-numeric characters in the input string are ignored, except when they
     *    terminate a numeric sequence.
     *  - If the buffer is insufficient for parsing segments, the function may fail.
     *
     *  @warning
     *  - Ensure `buffer` and `out_values` are properly allocated with sufficient size
     *    before calling this function. Passing invalid pointers or insufficient sizes
     *    may result in undefined behavior.
     *  - This function does not handle locale-specific number formatting (e.g., commas
     *    as decimal separators).
     *
     *  @example
     *  @code
     *  char input[] = "rgba(255, 50%, 64, 0.5)";
     *  char temp_buffer[32];
     *  float values[4];
     *  int32_t num_values = CSS::extractValuesFromStr(input, temp_buffer, 32, ')', 4, values);
     *
     *  // Result: values = {255.0, 128.0, 64.0, 0.5}, num_values = 4
     *  @endcode
     */
    int32_t CSS::extractValuesFromStr(const char *str, char *buffer, int32_t buffer_size, char closing_char, int32_t values_size, float *out_values) {

        static constexpr int32_t kMaxValuesCount = 128;

        if (!str) { throw ErrorCode::CSSInternalError; }
        if (!buffer) { throw ErrorCode::CSSInternalMemoryError; }
        if (!out_values) { throw ErrorCode::CSSInternalMemoryError; }

        if (buffer_size < 1) { throw ErrorCode::CSSInternalError; }
        if (values_size > kMaxValuesCount) { throw ErrorCode::CSSInternalError; }

        if (*str == '\0') {
            return 0;
        }

        const char *end_ptr = nullptr;

        if (closing_char != '\0') {
            end_ptr = strchr(str, ')');
            if (!end_ptr) { throw ErrorCode::CSSClosingBracketMissing; }
        }
        else {
            end_ptr = str + strlen(str);
        }

        auto length = end_ptr - str;

        if (length < 1) { throw ErrorCode::CSSContentMissing; }
        if (length >= buffer_size) { throw ErrorCode::CSSContentToBig; }

        std::strncpy(buffer, str, length); // Copy up to `buffer_size` - 1 characters.
        buffer[length] = '\0'; // Ensure null-termination.

        // Replace all commas with spaces, which now separates the values.
        String::replaceChar(buffer, ',', ' ');
        String::replaceChar(buffer, '/', ' ');


        // Find all value pointers.

        const char *value_ptr[kMaxValuesCount]; // Pointer to start of values in `buffer`.

        char *b_ptr = buffer;
        bool prev_space_flag = false;
        int32_t value_index = 0;
        value_ptr[0] = b_ptr;
        for (int32_t i = 0; i < length; i++) {
            bool space_flag = *b_ptr == ' ';
            if (space_flag) {
                if (!prev_space_flag) {
                    *b_ptr = '\0';
                    value_index++;
                    if (value_index >= values_size) { throw ErrorCode::CSSValueStorageOverflow; }
                }
            }
            else {
                if (prev_space_flag) {
                    value_ptr[value_index] = b_ptr;
                }
            }
            prev_space_flag = space_flag;
            b_ptr++;
        }


        // Convert the values from str to floats

        int32_t value_n = value_index + 1;
        for (int32_t i = 0; i < value_n; i++) {
            auto value_str_length = static_cast<int32_t>(strlen(value_ptr[i]));
            bool percentage_flag = (value_ptr[i][value_str_length - 1] == '%');
            out_values[i] = String::parseDoubleWithDotOrComma(value_ptr[i]);
            if (percentage_flag) {
                out_values[i] *= 0.01;
            }
        }

        return value_n;
    }


    /**
     *  @brief Extracts a numerical value and its corresponding unit from a CSS-like string.
     *
     *  This function scans a C-string for a numeric value followed by a unit (e.g., "px", "em", "%").
     *  It then stores the extracted value as a double and the unit in the provided `CSSUnit` reference.
     *  The function can handle typical CSS units for lengths and sizes, such as pixels, percentage, etc.
     *
     *  @param str       A pointer to a C-style string containing a number followed by an optional  unit.
     *  @param out_value A reference to a `CSSValue` obejct that will hold the resulting value with unit.
     *
     *  @return An `ErrorCode` indicating the success or a failure.
     *
     *  @note This function assumes the input string contains a valid number followed by a unit with no spaces.
     *        It is not intended to handle more complex CSS syntax or malformed strings.
     *
     *  Example:
     *  @code
     *  double value;
     *  CSSUnit unit;
     *  int32_t result = CSS::extractValueAndUnit("12px", value, unit);
     *  if (result > 0) {
     *      // Process value and unit ...
     *  }
     *  @endcode
     */
    ErrorCode CSS::extractCSSValueFromStr(const char *str, CSSValue &out_value, char **next_value_ptr) noexcept {

        auto result = ErrorCode::None;

        try {
            if (!str) { throw ErrorCode::CSSInternalError; }

            // Skip all whitespaces.
            auto ptr = str;
            while (String::charIsWhiteSpace(*ptr)) {
                ptr++;
            }

            auto value_beg_ptr = ptr;

            char first_c = *value_beg_ptr;
            if (first_c == '\0') { throw ErrorCode::CSSInvalidFormat; }

            // First char must be a legal letter for a numeric value
            if (first_c != '-' && first_c != '+' && !(first_c >= '0' && first_c <= '9')) {
                throw ErrorCode::CSSNumberParseError;
            }


            bool has_dot = false;
            ptr++;
            for (int32_t i = 0; ; i++) {
                if (*ptr == '.') {
                    if (has_dot) {
                        throw ErrorCode::CSSNumberParseError;
                    }
                    has_dot = true;
                }
                else if (*ptr < '0' || *ptr > '9') {
                    break;
                }
                ptr++;
            }

            out_value.setIsFloat(has_dot);


            auto unit_beg_ptr = ptr;

            if (next_value_ptr != nullptr) {
                char *end_ptr = (char*)strchr(value_beg_ptr, ' ');
                *next_value_ptr = end_ptr != nullptr ? end_ptr + 1 : nullptr;
            }


            // Find the unit if any characters immediately follow the last digit of the numeric value,
            // excluding CSS separator characters: ' ', ',', or '/'

            auto css_unit_info = _g_css_unit_infos;
            bool unit_flag = false;
            CSSUnit css_unit = CSSUnit::Absolute;
            if (!_css_isDelimiter(*unit_beg_ptr)) {
                while (css_unit_info->m_unit_str[0] != '\0') {  // Check for sentinel.
                    if (_css_strcmp(unit_beg_ptr, css_unit_info->m_unit_str)) {
                        css_unit = css_unit_info->m_unit;
                        unit_flag = true;
                        break;
                    }
                    css_unit_info++;
                }
                if (!unit_flag) { throw ErrorCode::CSSUnkownUnit; }  // No unit found
            }

            out_value.setDouble(String::parseDoubleWithDotOrComma(value_beg_ptr), css_unit);
        }
        catch (ErrorCode err) {
            out_value.undef();
            result = err;
        }

        return result;
    }


    const char *CSS::unitName(CSSUnit unit) noexcept {

        static const char *undefined_name = "unknown";

        if (unit >= CSSUnit::First && unit <= CSSUnit::Last) {
            return _g_css_unit_infos[static_cast<int32_t>(unit)].m_unit_str;
        }
        else {
            return undefined_name;
        }
    }


    bool CSS::_css_strcmp(const char *a, const char *b) noexcept {

        if (a == nullptr || b == nullptr) {
            return false;
        }

        while (true) {
            if (_css_isDelimiter(*a) && _css_isDelimiter(*b)) {
                return true;
            }
            if (*a != *b) {
                return false;
            }
            a++;
            b++;
        }
    }


    /**
     *  @brief Checks if a character is a CSS delimiter.
     *
     *  This utility function determines whether the given character is a delimiter
     *  commonly used in CSS syntax. Delimiters are used to separate tokens or values
     *  in CSS properties.
     *
     *  @param c The character to check.
     *  @return true if the character is a CSS delimiter ('\0', ' ', ',', '/', or ')'),
     *          false otherwise.
     */
    bool CSS::_css_isDelimiter(const char c) noexcept {

        return c == '\0' || c == ' ' || c == ',' || c == '/' || c == ')';
    }


    /**
     *  @brief Checks if there are characters for values inbetween commas in a string.
     */
    bool CSS::_css_check_comma_delimiters(const char *str) noexcept {

        if (str == nullptr) {
            return false;
        }

        auto ptr = str;
        bool comma_flag = false;
        bool slash_flag = false;
        int32_t slash_n = 0;                    // Number of slashes.
        int32_t pre_slash_content_n = 0;        // Number of content/value before the first slash.
        int32_t content_n = 0;                  // Number of content/value parts.
        int32_t curr_content_length = 0;
        int32_t prev_content_length = 0;
        while (*ptr != '\0') {

            if (*ptr == ',') {
                if (comma_flag) {
                    return false;
                }
                if (prev_content_length < 1) {
                    return false;
                }
                comma_flag = true;
                if (curr_content_length > 0) {
                    content_n++;
                    prev_content_length = curr_content_length;
                    curr_content_length = 0;
                }
            }
            else {
                if (*ptr != ' ' && *ptr != '/') {
                    // A character which is not a delimiter would be some content/value.
                    curr_content_length++;
                    comma_flag = false;  // The previous comma isn't relevant anymore.
                }
                else {
                    if (*ptr == '/') {
                        slash_n++;
                        if (pre_slash_content_n == 0) {
                            pre_slash_content_n = content_n;
                        }
                        slash_flag = true;
                    }

                    if (curr_content_length > 0) {
                        content_n++;
                        prev_content_length = curr_content_length;
                        curr_content_length = 0;
                    }
                }
            }

            ptr++;
        }

        return true;
    }


} // End of namespace Grain
