//
//  Font.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 27.07.2025
//

#ifndef GrainFont_hpp
#define GrainFont_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"
#include "Color/RGB.hpp"
#include "2d/Rect.hpp"
#include "2d/Dimension.hpp"
#include "String/String.hpp"

#if defined(__APPLE__) && defined(__MACH__)
    #include <CoreText/CoreText.h>
#endif


namespace Grain {

    class FontAttributes;

    class Font : public Object {

    public:
        String m_font_name;
        String m_display_name;
        float m_font_size = 0.0f;

        bool m_is_valid = false;

        double m_ascent = 0.0;
        double m_descent = 0.0;
        double m_x_height = 0.0;
        double m_cap_height = 0.0;
        double m_leading = 0.0;
        double m_cell_height = 0.0;
        double m_line_height = 0.0;
        double m_italic_angle = 0.0;
        double m_underline_position = 0.0;
        double m_underline_thickness = 0.0;

        int32_t m_units_per_em = 0;
        int32_t m_glyph_count = 0;
        Rectd m_bounding_box{};

        #if defined(__APPLE__) && defined(__MACH__)
            CTFontRef m_ct_font = nullptr;    ///! CoreText Font reference
        #endif

    public:
        Font(float size) noexcept;
        Font(const char* name, float size) noexcept;
        Font(const String& name, float size) noexcept;
        Font(const Font* font) noexcept;
        Font(const Font* font, float size) noexcept;
        Font(const String& csv) noexcept;
        ~Font() noexcept;

        [[nodiscard]] const char* className() const noexcept override { return "Font"; }

        friend std::ostream& operator << (std::ostream& os, const Font& o) {
            os << "Font: " << o.m_font_name << std::endl;
            os << "  ascent: " << o.ascent() << std::endl;
            os << "  descent: " << o.descent() << std::endl;
            os << "  x height: " << o.xHeight() << std::endl;
            os << "  cap height: " << o.capHeight() << std::endl;
            os << "  leading: " << o.leading() << std::endl;
            os << "  cell height: " << o.m_cell_height << std::endl;
            os << "  line height: " << o.lineHeight() << std::endl;
            os << "  italic angle: " << o.m_italic_angle << std::endl;
            os << "  underline position: " << o.m_underline_position << std::endl;
            os << "  underline thickness: " << o.m_underline_thickness << std::endl;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const Font* o) {
            o == nullptr ? os << "Font nullptr" : os << *o;
            return os;
        }

        [[nodiscard]] const char* fontNameUtf8() const noexcept { return m_font_name.utf8(); }
        [[nodiscard]] float size() const noexcept { return m_font_size; }

        #if defined(__APPLE__) && defined(__MACH__)
            CTFontRef ctFont() const noexcept { return m_ct_font; }
            // NSFont* nsFont() const noexcept { return (__bridge NSFont*)m_ct_font; }
        #endif

        [[nodiscard]] bool isValid() const noexcept { return m_is_valid; }

        void set(const String& name, float size) noexcept;
        void set(const char* name, float size) noexcept;

        [[nodiscard]] double ascent() const noexcept { return m_ascent; }
        [[nodiscard]] double descent() const noexcept { return m_descent; }
        [[nodiscard]] double xHeight() const noexcept { return m_x_height; }
        [[nodiscard]] double capHeight() const noexcept { return m_cap_height; }
        [[nodiscard]] double leading() const noexcept { return m_leading; }
        [[nodiscard]] double lineHeight() const noexcept { return m_line_height; }

        [[nodiscard]] double ascentFraction() const noexcept {
            return m_ascent / m_cell_height;
        }

        [[nodiscard]] double capPosition(double reference_height = 1.0) const noexcept {
            return (m_ascent - m_cap_height) / m_cell_height * reference_height;
        }

        [[nodiscard]] double xHeightPosition(double reference_height = 1.0) const noexcept {
            return (m_ascent - m_x_height) / m_cell_height * reference_height;
        }

        [[nodiscard]] double baselinePosition(double reference_height = 1.0) const noexcept {
            return m_ascent / m_cell_height * reference_height;
        }

        [[nodiscard]] double centerPosition(double reference_height = 1.0) const noexcept {
            return ((m_ascent - m_cap_height) + m_cap_height / 2) / m_cell_height * reference_height;
        }


        [[nodiscard]] double glyphAdvanceWidth(const char* symbol, int32_t length = -1) const noexcept;
        [[nodiscard]] Dimensiond textDimension(const char* str, int32_t byte_length = -1) const noexcept;

#if defined(__APPLE__) && defined(__MACH__)
        [[nodiscard]] Dimensiond macos_textDimensionUsingCTFramesetter(const char* str, int32_t byte_length = -1) const noexcept;
#endif
        [[nodiscard]] int32_t charIndexAtX(const char* str, double x, double &in_out_delta, double& out_cursor_x) const noexcept;

        [[nodiscard]] int32_t advancesForText(const char* text, int32_t max_length, Vec2d* out_advances) noexcept;
        [[nodiscard]] int32_t advancesForText(const String& text, int32_t max_length, Vec2d* out_advances) noexcept;

        [[nodiscard]] FontAttributes* buildAttributes(const RGB& color, float alpha = 1.0f) const noexcept;

        /* TODO: macOS ...
        static NSDictionary* buildNSFontAttributes(const Font &font, const RGB& color, float alpha) noexcept;
         */
    };


    class FontAttributes {

        friend class Font;

    public:
        double m_ascent = 0.0;
        double m_descent = 0.0;
        double m_x_height = 0.0;
        double m_cap_height = 0.0;
        double m_leading = 0.0;
        double m_cell_height = 0.0;
        double m_line_height = 0.0;
        double m_italic_angle = 0.0;
        double m_underline_position = 0.0;
        double m_underline_thickness = 0.0;

        RGB m_color;
        float m_alpha = 1;

        /* TODO: macOS ...
        NSDictionary* mNSAttributes = nil;
         */


    public:
        FontAttributes(const Font& font, const RGB& color, float alpha = 1.0f) noexcept;
        ~FontAttributes() noexcept;

        /* TODO: macOS ...
        NSDictionary* getNSAttributes() const noexcept { return mNSAttributes; };
         */

        double ascentFraction() const noexcept { return m_ascent / m_cell_height; }

        double capPosition(double reference_height = 1.0) const noexcept {
            return (m_ascent - m_cap_height) / m_cell_height * reference_height;
        }

        double xHeightPosition(double reference_height = 1.0) const noexcept {
            return (m_ascent - m_x_height) / m_cell_height * reference_height;
        }

        double baselinePosition(double reference_height = 1.0) const noexcept {
            return m_ascent / m_cell_height * reference_height;
        }

        double centerPosition(double reference_height = 1.0) const noexcept {
            return ((m_ascent - m_cap_height) + m_cap_height / 2) / m_cell_height * reference_height;
        }
    };


} // End of namespace Grain

#endif // GrainFont_hpp
