//
//  CairoContext.hpp
//
//  Created by Roald Christesen on from 15.08.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Graphic/CairoContext.hpp"
#include "Image/Image.hpp"
#include "Color/Gradient.hpp"
#include "Core/Log.hpp"


namespace Grain {

    CairoContext::CairoContext() noexcept : GraphicContext() {
        _cairoInit();
    }


    CairoContext::~CairoContext() noexcept {
        _cairoFreeResources();
    }


    void CairoContext::_cairoInit() noexcept {
        m_magic = Type::fourcc('c', 'a', 'i', 'r');
    }


    void CairoContext::_cairoFreeResources() noexcept {
        if (m_cairo_cr) {
            cairo_destroy(m_cairo_cr);
            m_cairo_cr = nullptr;
        }
        if (m_cairo_surface) {
            cairo_surface_destroy((::cairo_surface_t*)m_cairo_surface);
            m_cairo_surface = nullptr;
        }
    }


    void CairoContext::_cairoSetFillColor() noexcept {
        cairo_set_source_rgba(
                m_cairo_cr,
                m_fill_color.m_data[0],
                m_fill_color.m_data[1],
                m_fill_color.m_data[2],
                m_fill_color.m_alpha);
    }


    void CairoContext::_cairoSetStrokeColor() noexcept {
        cairo_set_source_rgba(
                m_cairo_cr,
                m_stroke_color.m_data[0],
                m_stroke_color.m_data[1],
                m_stroke_color.m_data[2],
                m_stroke_color.m_alpha);
    }


    void CairoContext::log(Log& l) const noexcept {
        l << "Cairo version: "
            << CAIRO_VERSION_MAJOR << "."
            << CAIRO_VERSION_MINOR << "."
            << CAIRO_VERSION_MICRO
            << l.endl;

        cairo_status_t s = cairo_status(m_cairo_cr);
        if (s != CAIRO_STATUS_SUCCESS) {
            l << "Cairo error: " << cairo_status_to_string(s) << l.endl;
        }
        if (!m_cairo_cr) {
            l << "Cairo error: m_cairo_cr is nullptr!" << l.endl;
            return;
        }
        if (!m_cairo_surface) {
            l << "Cairo error: m_cairo_surface is nullptr!" << l.endl;
            return;
        }

        if (cairo_surface_status((::cairo_surface_t *) m_cairo_surface) != CAIRO_STATUS_SUCCESS) {
            l << "Cairo surface error: "
                << cairo_status_to_string(cairo_surface_status((::cairo_surface_t *) m_cairo_surface))
                << l.endl;
        }

        if (cairo_status(m_cairo_cr) != CAIRO_STATUS_SUCCESS) {
            l << "Cairo context error: "
                << cairo_status_to_string(cairo_status(m_cairo_cr))
                << l.endl;
        }
    }


    void CairoContext::setImage(Image* image) noexcept {
        GraphicContext::setImage(image);

        if (image->colorModel() == Color::Model::RGBA && image->isFloat()) {
            _cairoFreeResources();

            m_cairo_surface = cairo_image_surface_create_for_data(
                    image->mutPixelDataPtr(),
                    CAIRO_FORMAT_RGBA128F, // Cairo's RGBA floating point pixel format
                    image->width(),
                    image->height(),
                    image->bytesPerRow());
            m_cairo_cr = cairo_create((::cairo_surface_t*)m_cairo_surface);
        }
    }


    bool CairoContext::isValid() noexcept {
        if (!m_cairo_cr || !m_cairo_surface) {
            return false;
        }
        return true;
    }


    void CairoContext::save() noexcept {
        cairo_save(m_cairo_cr);
        m_state_depth++;
    }

    void CairoContext::restore() noexcept {
        if (m_state_depth > 0) {
            cairo_restore(m_cairo_cr);
            m_state_depth--;
        }
    }


    void CairoContext::setAlpha(float alpha) noexcept {
        m_alpha = alpha;
    }


    void CairoContext::setFillColor(float r, float g, float b, float alpha) noexcept {
        m_fill_color.setRGBA(r, g, b, alpha);
    }


    void CairoContext::setStrokeColor(float r, float g, float b, float alpha) noexcept {
        m_stroke_color.setRGBA(r, g, b, alpha);
    }


    void CairoContext::setStrokeWidth(double width) noexcept {
        cairo_set_line_width(m_cairo_cr, width);
    }


    void CairoContext::setStrokeMiterLimit(double limit) noexcept {
        cairo_set_miter_limit(m_cairo_cr, limit);
    }


    void CairoContext::setStrokeJoinStyle(StrokeJoinStyle join) noexcept {
        cairo_line_join_t cj = CAIRO_LINE_JOIN_MITER;
        switch (join) {
            case StrokeJoinStyle::Miter:
                cj = CAIRO_LINE_JOIN_MITER;
                break;
            case StrokeJoinStyle::Round:
                cj = CAIRO_LINE_JOIN_ROUND;
                break;
            case StrokeJoinStyle::Bevel:
                cj = CAIRO_LINE_JOIN_BEVEL;
                break;
        }
        cairo_set_line_join(m_cairo_cr, cj);
    }


    void CairoContext::setStrokeCapStyle(StrokeCapStyle cap) noexcept {
        cairo_line_cap_t cc = CAIRO_LINE_CAP_BUTT;
        switch (cap) {
            case StrokeCapStyle::Butt:
                cc = CAIRO_LINE_CAP_BUTT;
                break;
            case StrokeCapStyle::Round:
                cc = CAIRO_LINE_CAP_ROUND;
                break;
            case StrokeCapStyle::Square:
                cc = CAIRO_LINE_CAP_SQUARE;
                break;
        }
        cairo_set_line_cap(m_cairo_cr, cc);
    }


    void CairoContext::setStrokeDash(double dash_length, double gap_length) noexcept {
        if (dash_length <= 0.0 && gap_length <= 0.0) {
            cairo_set_dash(m_cairo_cr, nullptr, 0, 0); // solid
        }
        else {
            double dashes[2] = { dash_length, gap_length };
            cairo_set_dash(m_cairo_cr, dashes, 2, 0.0);
        }
    }


    void CairoContext::setStrokeDash(int32_t array_length, const double* array, double scale) noexcept {
        if (array_length <= 0 || array == nullptr) {
            cairo_set_dash(m_cairo_cr, nullptr, 0, 0); // solid
        }
        else {
            if (array_length > 32) array_length = 32;
            double dashes[32];
            for (int32_t i = 0; i < array_length; i++) {
                dashes[i] = array[i] * scale;
            }
            cairo_set_dash(m_cairo_cr, dashes, array_length, 0.0);
        }
    }


    void CairoContext::setStrokeSolid() noexcept {
        cairo_set_dash(m_cairo_cr, nullptr, 0, 0);
    }


    void CairoContext::setBlendMode(BlendMode blend_mode) noexcept {
        static cairo_operator_t cairo_blend_modes[] = {
                CAIRO_OPERATOR_OVER,           // Normal
                CAIRO_OPERATOR_MULTIPLY,       // Multiply
                CAIRO_OPERATOR_SCREEN,         // Screen
                CAIRO_OPERATOR_OVERLAY,        // Overlay
                CAIRO_OPERATOR_DARKEN,         // Darken
                CAIRO_OPERATOR_LIGHTEN,        // Lighten
                CAIRO_OPERATOR_COLOR_DODGE,    // Color Dodge (since Cairo 1.10+)
                CAIRO_OPERATOR_COLOR_BURN,     // Color Burn
                CAIRO_OPERATOR_SOFT_LIGHT,     // Soft Light
                CAIRO_OPERATOR_HARD_LIGHT,     // Hard Light
                CAIRO_OPERATOR_DIFFERENCE,     // Difference
                CAIRO_OPERATOR_EXCLUSION,      // Exclusion
                // Cairo has *no direct support* for Hue/Saturation/Color/Luminosity
                // you might need custom shaders/effects for those
                CAIRO_OPERATOR_CLEAR,          // Clear
                CAIRO_OPERATOR_SOURCE,         // Copy
                CAIRO_OPERATOR_IN,             // Source In
                CAIRO_OPERATOR_OUT,            // Source Out
                CAIRO_OPERATOR_ATOP,           // Source Atop
                CAIRO_OPERATOR_DEST_OVER,      // Dest Over
                CAIRO_OPERATOR_DEST_IN,        // Dest In
                CAIRO_OPERATOR_DEST_OUT,       // Dest Out
                CAIRO_OPERATOR_DEST_ATOP,      // Dest Atop
                CAIRO_OPERATOR_XOR,            // XOR
                CAIRO_OPERATOR_DARKEN,         // Plus Darker (approx)
                CAIRO_OPERATOR_LIGHTEN         // Plus Lighter (approx)
        };

        int idx = static_cast<int>(blend_mode);
        if (idx < 0 || idx >= static_cast<int>(BlendMode::Last)) {
            idx = static_cast<int>(BlendMode::Normal);
        }

        cairo_set_operator(m_cairo_cr, cairo_blend_modes[idx]);
    }


    void CairoContext::setBlendModeNormal() noexcept {
        cairo_set_operator(m_cairo_cr, CAIRO_OPERATOR_OVER);
    }


    void CairoContext::setBlendModeMultiply() noexcept {
        cairo_set_operator(m_cairo_cr, CAIRO_OPERATOR_MULTIPLY);
    }


    void CairoContext::enableAliasing() noexcept {
        /* Implement! */
    }


    void CairoContext::disableAliasing() noexcept {
        /* Implement! */
    }


    void CairoContext::enableFontSmoothing() noexcept {
        /* Implement! */
    }


    void CairoContext::disableFontSmoothing() noexcept {
        /* Implement! */
    }


    void CairoContext::enableFontSubpixelQuantization() noexcept {
        /* Implement! */
    }


    void CairoContext::disableFontSubpixelQuantization() noexcept {
        /* Implement! */
    }


    void CairoContext::setTextMatrix(double a, double b, double c, double d, double tx, double ty) noexcept {
        cairo_matrix_t matrix;
        cairo_matrix_init(&matrix, a, b, c, d, tx, ty);
        cairo_set_font_matrix(m_cairo_cr, &matrix);
    }


    void CairoContext::beginPath() noexcept {
        cairo_new_path(m_cairo_cr);
    }


    void CairoContext::moveTo(double x, double y) noexcept {
        cairo_move_to(m_cairo_cr, x, y);
    }


    void CairoContext::lineTo(double x, double y) noexcept {
        cairo_line_to(m_cairo_cr, x, y);
        m_last_pos.m_x = x;
        m_last_pos.m_y = y;
    }


    void CairoContext::lineTo(double x, double y, bool start_flag) noexcept {
        if (start_flag) {
            cairo_move_to(m_cairo_cr, x, y);
        }
        else {
            cairo_line_to(m_cairo_cr, x, y);
        }
        m_last_pos.m_x = x;
        m_last_pos.m_y = y;
    }


    void CairoContext::curveTo(double c1x, double c1y, double c2x, double c2y, double x, double y) noexcept {
        cairo_curve_to(m_cairo_cr, c1x, c1y, c2x, c2y, x, y);
        m_last_pos.m_x = x;
        m_last_pos.m_y = y;
    }

    void CairoContext::curveTo(const Vec2d& control1, const Vec2d& control2, const Vec2d& point) noexcept {
        cairo_curve_to(m_cairo_cr, control1.m_x, control1.m_y, control2.m_x, control2.m_y, point.m_x, point.m_y);
        m_last_pos = point;
    }

    void CairoContext::curveTo(double cx, double cy, double x, double y) noexcept {
        double c1x = m_last_pos.m_x + 2.0 / 3.0 * (cx - m_last_pos.m_x);
        double c2x = x + 2.0 / 3.0 * (cx - x);
        double c1y = m_last_pos.m_y + 2.0 / 3.0 * (cy - m_last_pos.m_y);
        double c2y = y + 2.0 / 3.0 * (cy - y);

        cairo_curve_to(m_cairo_cr, c1x, c1y, c2x, c2y, x, y);
        m_last_pos.m_x = x;
        m_last_pos.m_y = y;
    }


    void CairoContext::closePath() noexcept {
        cairo_close_path(m_cairo_cr);
    }


    void CairoContext::fillPath() noexcept {
        _cairoSetFillColor();
        cairo_fill(m_cairo_cr);  // fills and clears path
    }


    void CairoContext::fillPathEvenOdd() noexcept {
        _cairoSetFillColor();
        cairo_set_fill_rule(m_cairo_cr, CAIRO_FILL_RULE_EVEN_ODD);
        cairo_fill(m_cairo_cr);
        // (optional) restore to default rule if you mix with non-even-odd:
        cairo_set_fill_rule(m_cairo_cr, CAIRO_FILL_RULE_WINDING);
    }


    void CairoContext::strokePath() noexcept {
        _cairoSetStrokeColor();
        cairo_stroke(m_cairo_cr);  // strokes and clears path
    }


    void CairoContext::drawPath() noexcept {
        _cairoSetFillColor();
        cairo_fill_preserve(m_cairo_cr);   // fill but keep path
        _cairoSetStrokeColor();
        cairo_stroke(m_cairo_cr);          // stroke, then clear path
    }


    void CairoContext::addRectPath(double x, double y, double width, double height) noexcept {
        cairo_rectangle(m_cairo_cr, x, y, width, height);
    }


    void CairoContext::addEllipsePath(const Rectd& rect) noexcept {
        // Approximate ellipse with cairo_scale + circle
        double cx = rect.m_x + rect.m_width / 2.0;
        double cy = rect.m_y + rect.m_height / 2.0;
        double rx = rect.m_width / 2.0;
        double ry = rect.m_height / 2.0;

        cairo_save(m_cairo_cr);
        cairo_translate(m_cairo_cr, cx, cy);
        cairo_scale(m_cairo_cr, rx, ry);
        cairo_arc(m_cairo_cr, 0.0, 0.0, 1.0, 0.0, 2.0 * M_PI);
        cairo_restore(m_cairo_cr);
    }


    void CairoContext::addCirclePath(double x, double y, double radius) noexcept {
        cairo_arc(m_cairo_cr, x, y, radius, 0.0, 2.0 * M_PI);
    }


    void CairoContext::addRingPath(
            const Vec2d& center,
            double inner_radius,
            double outer_radius,
            double angle_degrees,
            double span_degrees) noexcept
    {
        if (span_degrees <= 0.0) return;

        double angle = angle_degrees * M_PI / 180.0;
        double span  = span_degrees * M_PI / 180.0;
        double endAngle = angle + span;

        // Outer arc (counter-clockwise by default)
        cairo_arc(m_cairo_cr, center.m_x, center.m_y, outer_radius, angle, endAngle);

        // Inner arc in reverse (clockwise)
        cairo_arc_negative(m_cairo_cr, center.m_x, center.m_y, inner_radius, endAngle, angle);

        // Close the path to make a proper ring
        cairo_close_path(m_cairo_cr);
    }


    void CairoContext::fillRect(double x, double y, double width, double height) noexcept {
        if (width > 0 && height > 0) {
            _cairoSetFillColor();
            cairo_rectangle(m_cairo_cr, x, y, width, height);
            cairo_fill(m_cairo_cr);
        }
    }


    void CairoContext::strokeRect(double x, double y, double width, double height) noexcept {
        if (width > 0 && height > 0) {
            _cairoSetStrokeColor();
            cairo_rectangle(m_cairo_cr, x, y, width, height);
            cairo_stroke(m_cairo_cr);
        }
    }


    void CairoContext::fillEllipse(double x, double y, double rh, double rv) noexcept {
        if (rh <= 0.0 || rv <= 0.0) return;

        _cairoSetFillColor();
        cairo_save(m_cairo_cr);
        cairo_translate(m_cairo_cr, x, y);
        cairo_scale(m_cairo_cr, rh, rv);
        cairo_arc(m_cairo_cr, 0.0, 0.0, 1.0, 0.0, 2.0 * M_PI);
        cairo_restore(m_cairo_cr);

        cairo_fill(m_cairo_cr);
    }


    void CairoContext::strokeEllipse(double x, double y, double rh, double rv) noexcept {
        if (rh <= 0.0 || rv <= 0.0) return;
        _cairoSetStrokeColor();
        cairo_save(m_cairo_cr);
        cairo_translate(m_cairo_cr, x, y);
        cairo_scale(m_cairo_cr, rh, rv);
        cairo_arc(m_cairo_cr, 0.0, 0.0, 1.0, 0.0, 2.0 * std::numbers::pi);
        cairo_restore(m_cairo_cr);
        cairo_stroke(m_cairo_cr);
    }


    void CairoContext::fillCircle(double x, double y, double radius) noexcept {
        if (radius <= std::numeric_limits<float>::epsilon()) return;
        _cairoSetFillColor();
        cairo_arc(m_cairo_cr, x, y, radius, 0.0, 2.0 * std::numbers::pi);
        cairo_fill(m_cairo_cr);
    }


    void CairoContext::strokeCircle(double x, double y, double radius) noexcept {
        if (radius <= std::numeric_limits<float>::epsilon()) return;
        _cairoSetStrokeColor();
        cairo_arc(m_cairo_cr, x, y, radius, 0.0, 2.0 * std::numbers::pi);
        cairo_fill(m_cairo_cr);
    }


    void CairoContext::drawGradient(
            Gradient* gradient,
            const Vec2d& start_pos,
            const Vec2d& end_pos,
            bool draw_before,
            bool draw_after) noexcept {

        if (!gradient || gradient->stopCount() <= 1) return;

        // update gradient stops or colors
        gradient->update(this);

        // Create a Cairo linear gradient pattern
        cairo_pattern_t* pattern = cairo_pattern_create_linear(
                start_pos.m_x, start_pos.m_y,
                end_pos.m_x, end_pos.m_y
        );

        // Add gradient stops
        for (int i = 0; i < gradient->stopCount(); ++i) {
            auto stop = gradient->stopPtrAtIndex(i); // assume stop has offset [0,1] and color
            cairo_pattern_add_color_stop_rgba(
                    pattern,
                    stop->pos(),
                    stop->color(0).red(),
                    stop->color(0).green(),
                    stop->color(0).blue(),
                    stop->color(0).alpha());
        }

        // Extend options
        if (draw_before && draw_after) {
            cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REFLECT);
        }
        else if (draw_before) {
            cairo_pattern_set_extend(pattern, CAIRO_EXTEND_PAD);
        }
        else if (draw_after) {
            cairo_pattern_set_extend(pattern, CAIRO_EXTEND_PAD);
        }
        else {
            cairo_pattern_set_extend(pattern, CAIRO_EXTEND_NONE);
        }

        // Fill the current path with the gradient
        cairo_set_source(m_cairo_cr, pattern);
        cairo_fill(m_cairo_cr);

        cairo_pattern_destroy(pattern);
    }


    void CairoContext::drawRadialGradient(
            Gradient* gradient,
            const Vec2d& center,
            double radius,
            bool draw_before,
            bool draw_after) noexcept {

        if (!gradient || gradient->stopCount() <= 1) return;

        gradient->updateLUT();  // update lookup table if needed
        gradient->update(this);

        // Create a Cairo radial gradient pattern
        // Inner circle radius = 0 (like CGContext), outer circle radius = radius
        cairo_pattern_t* pattern = cairo_pattern_create_radial(
                center.m_x, center.m_y, 0.0,  // inner circle
                center.m_x, center.m_y, radius // outer circle
        );

        // Add gradient stops
        for (int i = 0; i < gradient->stopCount(); ++i) {
            const auto& stop = gradient->stopPtrAtIndex(i); // offset [0,1], RGBA
            cairo_pattern_add_color_stop_rgba(
                    pattern,
                    stop->pos(),
                    stop->color(0).red(),
                    stop->color(0).green(),
                    stop->color(0).blue(),
                    stop->color(0).alpha());
        }

        // Extend options (before/after)
        if (draw_before && draw_after) {
            cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REFLECT);
        }
        else if (draw_before) {
            cairo_pattern_set_extend(pattern, CAIRO_EXTEND_PAD);
        }
        else if (draw_after) {
            cairo_pattern_set_extend(pattern, CAIRO_EXTEND_PAD);
        }
        else {
            cairo_pattern_set_extend(pattern, CAIRO_EXTEND_NONE);
        }

        // Fill the current path with the gradient
        cairo_set_source(m_cairo_cr, pattern);
        cairo_fill(m_cairo_cr);

        cairo_pattern_destroy(pattern);
    }


    void CairoContext::drawImage(Image* image, const Rectd& rect, float alpha) noexcept {
        /* Implement! */
    }


    ErrorCode CairoContext::drawQuadrilateralImage(Image* image, const Quadrilateral& quadrilateral) noexcept {
        /* Implement! */
        return ErrorCode::Unknown;
    }


    ErrorCode CairoContext::drawQuadrilateralImage(Image* image, const Quadrilateral& quadrilateral, float alpha) noexcept {
        /* Implement! */
        return ErrorCode::Unknown;
    }


    void CairoContext::drawIcon(const Icon* icon, const Rectd& rect, float alpha) noexcept {
        /* Implement! */
    }


    void CairoContext::drawIcon(const Icon* icon, const Rectd& rect, const RGB& color, float alpha) noexcept {
        /* Implement! */
    }


    void CairoContext::drawIconInCircle(const Icon* icon, const Vec2d& center, double radius, const RGB& bg_color, const RGB& icon_color, const RGB& border_color, double border_width, float bg_alpha, float border_alpha, float icon_alpha) noexcept {
        /* Implement! */
    }


    void CairoContext::translate(double tx, double ty) noexcept {
        cairo_translate(m_cairo_cr, tx, ty);
    }


    void CairoContext::scale(double sx, double sy) noexcept {
        cairo_scale(m_cairo_cr, sx, sy);
    }


    void CairoContext::rotate(double angle_degrees) noexcept {
        double angle_radians = angle_degrees * M_PI / 180.0;
        cairo_rotate(m_cairo_cr, angle_radians);
    }


    void CairoContext::affineTransform(const Mat3d& matrix) noexcept {
        const double* p = matrix.dataPtr();
        cairo_matrix_t m;
        m.xx = p[0];
        m.yx = p[1];
        m.xy = p[3];
        m.yy = p[4];
        m.x0 = p[6];
        m.y0 = p[7];
        cairo_transform(m_cairo_cr, &m);
    }
}