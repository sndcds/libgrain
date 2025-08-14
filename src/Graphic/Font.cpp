//
//  Font.cpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//


#include "Graphic/Font.hpp"
#include "App/App.hpp"
#include "String/String.hpp"
#include "String/CSVString.hpp"


namespace Grain {


    Font::Font(float size) noexcept : Object() {
        set(nullptr, size); // System font with defined size
    }


    Font::Font(const String& name, float size) noexcept : Object() {
        set(name, size);
    }


    Font::Font(const char* name, float size) noexcept : Object() {
        set(name, size);
    }


    Font::Font(const Font* font) noexcept : Object() {
        if (font != nullptr) {
            set(font->fontNameUtf8(), font->size());
        }
    }


    Font::Font(const Font* font, float size) noexcept : Object() {
        if (font != nullptr) {
            set(font->fontNameUtf8(), size);
        }
    }


    Font::Font(const String& csv) noexcept : Object() {
        CSVLineParser csv_line_parser(csv);

        String fontName = "System";
        Fix fontSize = 13;

        csv_line_parser.nextString(fontName);
        csv_line_parser.nextFix(fontSize);

        set(fontName, fontSize.asFloat());
    }


    Font::~Font() noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        if (m_ct_font != nullptr) {
            CFRelease(m_ct_font);
        }
#endif
    }


    void Font::set(const String& name, float size) noexcept {
        set(name.utf8(), size);
    }


#if defined(__APPLE__) && defined(__MACH__)
    void Font::set(const char* name, float size) noexcept {
        m_font_name.clear();
        m_display_name.clear();

        if (m_ct_font) {
            CFRelease(m_ct_font);
            m_ct_font = nullptr;
        }

        m_font_size = size;

        if (name == nullptr || strcasecmp(name, "System") == 0) {
            // Use system font
            m_font_name.clear();
            m_ct_font = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, size, nullptr);
            m_font_name.set(CTFontCopyDisplayName(m_ct_font));
        }
        else {
            m_font_name.set(name);
            CFStringRef ct_font_name = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
            m_ct_font = CTFontCreateWithName(ct_font_name, size, NULL);
            CFRelease(ct_font_name);
        }

        if (m_ct_font != nullptr) {

            // Get the display name of the font
            CFStringRef display_name = CTFontCopyDisplayName(m_ct_font);
            m_display_name.set(display_name);
            CFRelease(display_name);

            m_ascent = CTFontGetAscent(m_ct_font);
            m_descent = CTFontGetDescent(m_ct_font);
            m_x_height = CTFontGetXHeight(m_ct_font);
            m_cap_height = CTFontGetCapHeight(m_ct_font);
            m_leading = CTFontGetLeading(m_ct_font);
            // CGAffineTransform matrix = CTFontGetMatrix(m_ct_font);
            // m_italic_angle = std::atan2(matrix.b, matrix.a) * (180.0 / std::numbers::pi);
            m_italic_angle = CTFontGetSlantAngle(m_ct_font);
            m_underline_position = CTFontGetUnderlinePosition(m_ct_font);
            m_underline_thickness = CTFontGetUnderlineThickness(m_ct_font);

            m_units_per_em = static_cast<int32_t>(CTFontGetUnitsPerEm(m_ct_font));
            m_glyph_count = (int32_t)CTFontGetGlyphCount(m_ct_font);

            CGRect ct_bbox = CTFontGetBoundingBox(m_ct_font);
            m_bounding_box.set(ct_bbox.origin.x, ct_bbox.origin.y, ct_bbox.size.width, ct_bbox.size.height);
        }

        m_cell_height = std::max(m_ascent + m_descent, std::numeric_limits<double>::epsilon()); // Avoid division by 0
        m_line_height = m_ascent + m_descent + m_leading;

        m_is_valid = std::fabs(m_ascent) >= 0.00001f; // TODO: Is there a better way?
    }
#else
    void Font::set(const char* name, float size) noexcept {
        // TODO: Implement linux version
    }
#endif


#if defined(__APPLE__) && defined(__MACH__)
    double Font::glyphAdvanceWidth(const char* symbol, int32_t length) const noexcept {
        if (symbol == nullptr || symbol[0] == '\0' || length == 0) {
            return 0.0;
        }

        CFStringRef cf_str = nil;
        if (length < 0 ) {
            cf_str = CFStringCreateWithCString(nullptr, symbol, kCFStringEncodingUTF8);
        }
        else {
            cf_str = CFStringCreateWithBytes(nullptr, reinterpret_cast<const UInt8*>(symbol), length, kCFStringEncodingUTF8, false);
        }
        if (cf_str == nullptr) {
            return 0.0;
        }

        // Ensure the CFString has at least one character
        if (CFStringGetLength(cf_str) == 0) {
            CFRelease(cf_str);
            return 0.0;
        }

        // Get the first UniChar from the CFString
        UniChar uni_char = 0;
        if (CFStringGetLength(cf_str) > 0) {
            uni_char = CFStringGetCharacterAtIndex(cf_str, 0);
        }

        // Release the CFString
        CFRelease(cf_str);

        // Convert the character to a glyph
        CGGlyph glyph;
        if (!CTFontGetGlyphsForCharacters(m_ct_font, &uni_char, &glyph, 1)) {
            return 0.0; // Failed to get glyph
        }

        // Get the advance width of the glyph
        CGSize advance;
        CTFontGetAdvancesForGlyphs(m_ct_font, kCTFontOrientationHorizontal, &glyph, &advance, 1);

        return advance.width;
    }
#else
    double Font::glyphAdvanceWidth(const char* symbol, int32_t length) const noexcept {
        // TODO: Implement linux version
        return 0.0;
    }
#endif

    /**
     *  @brief Get the dimensions (width and height) of a rendered C-string using a specific font.
     *
     *  @param str Pointer to the C-string.
     *  @param byte_length Byte length to use from the C-string. If `byte_length` is
     *         less than 0, the function uses all bytes until the null-terminator is reached.
     *  @return The dimensions (width and height) of the rendered C-string.
     */
#if defined(__APPLE__) && defined(__MACH__)
    Dimensiond Font::textDimension(const char* str, int32_t byte_length) const noexcept {

        // Create an attributed string with a single white space character
        CFStringRef cf_str = nil;
        if (byte_length < 0 ) {
            cf_str = CFStringCreateWithCString(nullptr, str, kCFStringEncodingUTF8);
        }
        else {
            cf_str = CFStringCreateWithBytes(nullptr, reinterpret_cast<const UInt8*>(str), byte_length, kCFStringEncodingUTF8, false);
        }

        CFMutableAttributedStringRef attrString = CFAttributedStringCreateMutable(kCFAllocatorDefault, 0);
        CFAttributedStringReplaceString(attrString, CFRangeMake(0, 0), cf_str);

        // Apply the font attribute
        CFAttributedStringSetAttribute(attrString, CFRangeMake(0, CFStringGetLength(cf_str)), kCTFontAttributeName, m_ct_font);

        // Create a CTLine from the attributed string
        CTLineRef line = CTLineCreateWithAttributedString(attrString);

        // Get the width of the white space character
        double width = CTLineGetTypographicBounds(line, NULL, NULL, NULL);

        // Get the ascent, descent, and leading of the line
        CGFloat ascent = 0;
        CGFloat descent = 0;
        CGFloat leading = 0;
        CTLineGetTypographicBounds(line, &ascent, &descent, &leading);

        // Calculate the total height
        CGFloat height = ascent + descent + leading;

        // Cleanup
        CFRelease(line);
        CFRelease(attrString);
        CFRelease(cf_str);

        return Dimensiond(width, height);
    }
#else
    Dimensiond Font::textDimension(const char* str, int32_t byte_length) const noexcept {
        // TODO: Implement linux version
        return Dimensiond();
    }
#endif


    /**
     *  @brief Previous version of `Font::textDimension()`.
     *
     *  May be used in another context.
     *  In here for historical reasons.
     *  May be deprecated later.
     *
     *  Problem is, that this method doesn´t get the with for single white spaces.
     */
#if defined(__APPLE__) && defined(__MACH__)
    Dimensiond Font::macos_textDimensionUsingCTFramesetter(const char* str, int32_t byte_length) const noexcept {
        if (str == nullptr || byte_length == 0 || m_ct_font == nullptr) {
            return Dimensiond(0, 0);
        }

        CFStringRef cf_str = nil;
        if (byte_length < 0 ) {
            cf_str = CFStringCreateWithCString(nullptr, str, kCFStringEncodingUTF8);
        }
        else {
            cf_str = CFStringCreateWithBytes(nullptr, reinterpret_cast<const UInt8*>(str), byte_length, kCFStringEncodingUTF8, false);
        }

        CFMutableDictionaryRef attributes = CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

        CFDictionaryAddValue(attributes, kCTFontAttributeName, m_ct_font);

        CFAttributedStringRef attributed_string = CFAttributedStringCreate(nullptr, cf_str, attributes);
        CTFramesetterRef framesetter = CTFramesetterCreateWithAttributedString(attributed_string);

        CFRange fit_range;
        CGSize size = CTFramesetterSuggestFrameSizeWithConstraints(framesetter, CFRangeMake(0, CFAttributedStringGetLength(attributed_string)), nullptr, CGSizeMake(CGFLOAT_MAX, CGFLOAT_MAX), &fit_range);


        // Release all Core Foundation objects
        CFRelease(framesetter);
        CFRelease(attributed_string);
        CFRelease(attributes);
        CFRelease(cf_str);

        return Dimensiond(size.width, size.height);
    }
#endif


#if defined(__APPLE__) && defined(__MACH__)
    int32_t Font::charIndexAtX(const char* str, double x, double& in_out_delta, double& out_cursor_x) const noexcept {
        int32_t cursor_index = -1;

        if (str != nullptr && str[0] != '\0') {

            CFStringRef cf_str = CFStringCreateWithCString(NULL, str, kCFStringEncodingUTF8);
            CFMutableAttributedStringRef cf_attr_str = CFAttributedStringCreateMutable(kCFAllocatorDefault, 0);
            CFAttributedStringReplaceString(cf_attr_str, CFRangeMake(0, 0), cf_str);

            CFRange range = CFRangeMake(0, CFAttributedStringGetLength(cf_attr_str));
            if (range.length > 0) {
                CFAttributedStringSetAttribute(cf_attr_str, range, kCTFontAttributeName, m_ct_font);
            }

            CTLineRef line = CTLineCreateWithAttributedString(cf_attr_str);

            double curr_cx = 0.0;
            for (int32_t i = 0; i <= range.length; i++) {
                double cx = CTLineGetOffsetForStringIndex(line, i, NULL);
                double delta = std::fabs(x - cx);
                if (delta < in_out_delta) {
                    in_out_delta = delta;
                    curr_cx = cx;
                    cursor_index = i;
                }
            }

            out_cursor_x = curr_cx;

            // Release all Core Foundation objects
            CFRelease(cf_str);
            CFRelease(cf_attr_str);
            CFRelease(line);
        }

        return cursor_index;
    }
#else
    int32_t Font::charIndexAtX(const char* str, double x, double& in_out_delta, double& out_cursor_x) const noexcept {
        // TODO: Implement linux version
        return 0;
    }
#endif


    int32_t Font::advancesForText(const String& text, int32_t max_length, Vec2d* out_advances) noexcept {
        return advancesForText(text.utf8(), max_length, out_advances);
    }


#if defined(__APPLE__) && defined(__MACH__)
    int32_t Font::advancesForText(const char* text, int32_t max_length, Vec2d* out_advances) noexcept {
        int32_t result = -1;

        if (text && out_advances && max_length > 0) {

            int32_t char_count = 0;

            // Create a CFStringRef from a UTF-8 encoded C-string
            CFStringRef cf_string = CFStringCreateWithCString(nullptr, text, kCFStringEncodingUTF8);
            if (cf_string != nullptr) {
                // Get the character count
                char_count = (int32_t)CFStringGetLength(cf_string);
                CFRelease(cf_string);
            }

            if (char_count > max_length) {
                return -1;
            }

            UniChar chars[char_count];
            CGGlyph glyphs[char_count];
            CGSize advances[char_count];
            CTFontRef ct_font = this->ctFont();

            CTFontGetGlyphsForCharacters(ct_font, chars, glyphs, char_count);
            CTFontGetAdvancesForGlyphs(ct_font, kCTFontOrientationDefault, glyphs, advances, char_count);

            for (int32_t i = 0; i < char_count; i++) {
                out_advances[i].m_x = (double)(advances[i].width);
                out_advances[i].m_y = (double)(advances[i].height);
            }

            result = char_count;
        }

        return result;
    }
#else
    int32_t Font::advancesForText(const char* text, int32_t max_length, Vec2d* out_advances) noexcept {
        // TODO: Implement linux version
        return 0;
    }
#endif


    FontAttributes* Font::buildAttributes(const RGB& color, float alpha) const noexcept {

        return new(std::nothrow) FontAttributes(this, color, alpha);
    }


/* TODO: macOS ...
NSDictionary* Font::buildNSFontAttributes(const Font& font, const GrRGB& color, float alpha) noexcept {

    NSFont* nsFont = font.getNSFont();
    NSColor* nsColor = color.createNSColor(alpha);
    NSDictionary* attributes = [[NSDictionary alloc] initWithObjectsAndKeys:
                                nsFont, NSFontAttributeName,
                                nsColor, NSForegroundColorAttributeName,
                                nil];

    [nsColor release];
    return attributes;
}
*/


    FontAttributes::FontAttributes(const Font& font, const RGB& color, float alpha) noexcept {

        m_ascent = font.m_ascent;
        m_descent = font.m_descent;
        m_x_height = font.m_x_height;
        m_cap_height = font.m_cap_height;
        m_leading = font.m_leading;
        m_cell_height = font.m_cell_height;
        m_line_height = font.m_line_height;
        m_italic_angle = font.m_italic_angle;
        m_underline_position = font.m_underline_position;
        m_underline_thickness = font.m_underline_thickness;

        m_color = color;
        m_alpha = alpha;

        /* TODO: !
        NSColor* nsColor = color.createNSColor(alpha);

        mNSAttributes = [[NSDictionary alloc] initWithObjectsAndKeys:
                         font.getNSFont(), NSFontAttributeName,
                         nsColor, NSForegroundColorAttributeName,
                         nil];

        [nsColor release];
         */
    }


    FontAttributes::~FontAttributes() noexcept {
        /* TODO: !
        if (mNSAttributes != nil)
            [mNSAttributes release];
         */
    }


} // End of namespace Grain
