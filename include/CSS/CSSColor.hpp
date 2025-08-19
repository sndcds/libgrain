//
//  CSSColor.hpp
//
//  Created by Roald Christesen on from 07.01.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 12.07.2025
//

#ifndef GrainCSSColor_hpp
#define GrainCSSColor_hpp

#include "Grain.hpp"
#include "CSS/CSS.hpp"
#include "Color/RGBA.hpp"


namespace Grain {


    class RGB;
    class RGBA;


    /**
     *  @struct CSSNamedColor
     *  @brief Represents a named CSS color and its corresponding value as an unsigned 32-bit hex value.
     *
     *  This structure is used to associate a CSS named color (e.g., "red", "blue") with its numerical representation.
     */
    struct CSSNamedColor {
        const char* m_name; ///< The name of the color as a null-terminated string, e.g. Example: `"red"`, `"blue"`, `"chartreuse"`.
        uint32_t m_color;   ///< The numerical value of the color, typically encoded as an integer in a format such as 0xRRGGBB (hexadecimal representation), e.g. `0xFF000000` for red or `0x0000FF00` for blue.
    };


    /**
     *  @struct CSSColorFunctionInfo
     *  @brief Represents information for CSS color functions, like `rgb()`, `rgba()` etc.
     *
     *  This structure is used to store metadata about the supported CSS color functions,
     *  including their signature, length, mode, and number of components.
     */
    struct CSSColorFunctionInfo {
        const char* m_signature;    ///< The system signature string, which represents the beginning of the color definition, including the opening bracket.
        int32_t m_length;           ///< The length of the system signature string.
        CSSColorFunction m_function;///< The color function.
        int32_t m_component_n;      ///< The number of color components required for the system.
        bool m_can_modern_syntax;   ///< Flag indicating wether the function supports modern CSS color syntax.
    };


    /**
     *  @class CSSColor
     *
     *  A utility class for parsing, manipulating, and converting colors
     *  according to the CSS Color Module Level 4 specification.
     *
     *  This class supports common color formats such as:
     *  - Named colors (e.g., "red", "blue").
     *  - Hexadecimal colors (e.g., "#ff0000", "#f00").
     *  - Functional color syntax (e.g., `rgb()`, `rgba()`, `hsl()`, `hwb()`).
     *  - Modern color spaces introduced in Level 4 (e.g., `lab()`, `lch()`, `color()`).
     *
     *  Features:
     *  - Parsing of CSS color strings into color component values.
     *  - Validation of color strings for correctness.
     *
     *  @see https://www.w3.org/TR/css-color-4/
     */
    class CSSColor : public Object {

    public:
        enum {
            kMaxValueComponents = 6
        };

    protected:
        RGBA m_rgba;
        bool m_modern_syntax = false;
        bool m_valid = false;
        CSSValue m_comp_values[kMaxValueComponents];
        int32_t m_parsed_comp_n = 0;

        // Globals.

        static const CSSNamedColor _g_named_css_colors[];
        static const CSSColorFunctionInfo _g_color_function_infos[];

    public:
        CSSColor() noexcept = default;
        ~CSSColor() noexcept = default;

        ErrorCode parseColor(const char* css_str) noexcept;
        ErrorCode parseHex(const char* str) noexcept;
        bool parseNamed(const char* str) noexcept;
        bool parseFunctional(const char* str, ErrorCode& out_err) noexcept;
        ErrorCode parseColorComponents(const char* str, int32_t str_length, int32_t component_n) noexcept;

        bool isValid() const noexcept { return m_valid; }
        bool usesModernSyntax() const noexcept { return m_modern_syntax; }
        RGBA rgba() const noexcept { return m_rgba; }
        RGB rgb() const noexcept { return m_rgba; }

        static ErrorCode parseColorToRGB(const char* css_str, RGB& out_color) noexcept;
        static ErrorCode parseColorToRGB(const String& css_string, RGB& out_color) noexcept;
        static ErrorCode parseColorToRGBA(const char* css_str, RGBA& out_color) noexcept;
        static ErrorCode parseColorToRGBA(const String& css_string, RGBA& out_color) noexcept;
    };


} // End of namespace Grain

#endif // GrainCSSColor_hpp
