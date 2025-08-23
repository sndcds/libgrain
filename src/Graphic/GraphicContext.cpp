//
//  GraphicContext.cpp
//
//  Created by Roald Christesen on from 03.05.2016
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "App/App.hpp"
#include "Graphic/GraphicContext.hpp"
#include "Graphic/Graphic.hpp"
#include "Graphic/Font.hpp"
#include "Type/Object.hpp"
#include "Math/Vec2.hpp"
#include "Math/Mat3.hpp"
#include "Color/RGB.hpp"
#include "Color/RGBA.hpp"
#include "Color/Gradient.hpp"
#include "DSP/LUT1.hpp"
#include "String/String.hpp"
#include "2d/Polygon.hpp"
#include "2d/Quadrilateral.hpp"
#include "2d/GraphicPath.hpp"
#include "2d/GraphicPathPoint.hpp"
#include "2d/Line.hpp"
#include "2d/Circle.hpp"
#include "2d/Triangle.hpp"
#include "Bezier/Bezier.hpp"
#include "2d/CatmullRomCurve.hpp"
#include "Math/Math.hpp"
#include "Image/Image.hpp"
#include "Signal/Audio.hpp"
#include "DSP/Freq.hpp"
#include "GUI/Components/Component.hpp"


namespace Grain {

    void _macosView_setContextByComponent(GraphicContext* gc, Component* component);


    GraphicContext::GraphicContext() noexcept {
        _init();
    }


    GraphicContext::GraphicContext(Component* component) noexcept {
        _init();
        #if defined(__APPLE__) && defined(__MACH__)
            _macosView_setContextByComponent(this, component);
        #endif
    }


    GraphicContext::GraphicContext(PDFWriter* pdf_writer) noexcept {
        /* TODO: !!!!!
        if (pdf_writer != nullptr) {

            setCGContext(pdf_writer->m_cg_context);

            m_cg_color_space = CGColorSpaceCreateDeviceRGB();
            // m_cg_color_space = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);  // TODO: An alternative?

            m_flipped_y = true;
            m_width = pdf_writer->m_media_box_pt.width();
            m_height = pdf_writer->m_media_box_pt.height();

            double s = Geometry::convertLength(1, pdf_writer->m_unit, LengthUnit::Point);
            scale(s, -s);
            translate(pdf_writer->bleedLeft(), - pdf_writer->trimBox().height() - pdf_writer->bleedTop());
        }
         */
    }


    GraphicContext::~GraphicContext() noexcept {
        _freeResources();
    }


    void GraphicContext::_init() noexcept {
        m_fill_color.white();
        m_stroke_color.black();
    }


    void GraphicContext::_freeResources() noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            CGColorSpaceRelease(m_cg_color_space);
            m_cg_color_space = nullptr;
        #endif
    }

    void GraphicContext::_freeImage() noexcept {
        GRAIN_RELEASE(m_image);
        m_image = nullptr;
    }

    void GraphicContext::setImage(Image* image) noexcept {
        if (!image) {
            return;
        }
        _freeImage();

        GRAIN_RETAIN(image);
        m_image = image;

        #if defined(__APPLE__) && defined(__MACH__)
            _macos_setImage(image);
        #endif
    }


#if defined(__APPLE__) && defined(__MACH__)
    void GraphicContext::_macos_setImage(Image* image) noexcept {
        if (image != nullptr) {
            GRAIN_RETAIN(image);
            m_image = image;
            image->graphicContext(this);

            m_width = image->width();
            m_height = image->height();

            switch (image->colorModel()) {
                case Color::Model::Lumina:
                case Color::Model::LuminaAlpha:
                    m_cg_color_space = CGColorSpaceCreateDeviceGray();
                    break;

                case Color::Model::CMYK:
                    m_cg_color_space = CGColorSpaceCreateDeviceCMYK();

                case Color::Model::RGB:
                case Color::Model::RGBA:
                default:
                    // m_cg_color_space = CGColorSpaceCreateDeviceRGB();
                    m_cg_color_space = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
                    break;
            }

            if (m_cg_context) {
                CGContextSetAllowsAntialiasing(m_cg_context, true);
                CGContextSetShouldAntialias(m_cg_context, true);

                CGContextSetAllowsFontSmoothing(m_cg_context, true);
                CGContextSetShouldSmoothFonts(m_cg_context, true);

                CGContextSetAllowsFontSubpixelPositioning(m_cg_context, true);
                CGContextSetShouldSubpixelPositionFonts(m_cg_context, true);

                CGContextSetAllowsFontSubpixelQuantization(m_cg_context, false);
                CGContextSetShouldSubpixelQuantizeFonts(m_cg_context, false);

                CGContextSetFillColorSpace(m_cg_context, m_cg_color_space);
                CGContextSetStrokeColorSpace(m_cg_context, m_cg_color_space);
            }
        }
    }
#endif


#if defined(__APPLE__) && defined(__MACH__)
    void GraphicContext::macos_setCGContextByComponent(CGContextRef context, Component* component) noexcept {
        m_cg_context = context;
        m_cg_color_space = CGColorSpaceCreateDeviceRGB();
        // m_cg_color_space = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);  // TODO: An alternative?

        if (component != nullptr) {
            m_flipped_y = component->isFlippedView();
            m_width = component->width();
            m_height = component->height();
            m_component = component;
        }
    }
#endif


    void GraphicContext::setFillColor(float r, float g, float b, float alpha) noexcept {
        m_fill_color.setRGBA(r, g, b, alpha);
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextSetRGBFillColor(m_cg_context, r, g, b, alpha);
        #endif
    }


    void GraphicContext::setFillRGB(const RGB& rgb) noexcept {
        m_fill_color.setRGBA(rgb, 1.0f);
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextSetRGBFillColor(m_cg_context, rgb.m_data[0], rgb.m_data[1], rgb.m_data[2], 1.0f);
        #endif
    }


    void GraphicContext::setFillRGBAndAlpha(const RGB& rgb, float alpha) noexcept {
        m_fill_color.setRGBA(rgb, alpha);
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextSetRGBFillColor(m_cg_context, rgb.m_data[0], rgb.m_data[1], rgb.m_data[2], alpha);
        #endif
    }


    void GraphicContext::setFillRGBA(const RGBA& rgba) noexcept {
        m_fill_color = rgba;
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextSetRGBFillColor(m_cg_context, rgba.m_data[0], rgba.m_data[1], rgba.m_data[2], rgba.m_alpha);
        #endif
    }


    void GraphicContext::setStrokeColor(float r, float g, float b, float alpha) noexcept {
        m_stroke_color.setRGBA(r, g, b, alpha);
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextSetRGBStrokeColor(m_cg_context, r, g, b, alpha);
        #endif
    }


    void GraphicContext::setStrokeRGB(const RGB& rgb) noexcept {
        m_stroke_color.setRGBA(rgb, 1.0f);
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextSetRGBStrokeColor(m_cg_context, rgb.m_data[0], rgb.m_data[1], rgb.m_data[2], 1.0f);
        #endif
    }


    void GraphicContext::setStrokeRGBAndAlpha(const RGB& rgb, float alpha) noexcept {
        m_stroke_color.setRGBA(rgb, alpha);
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextSetRGBStrokeColor(m_cg_context, rgb.m_data[0], rgb.m_data[1], rgb.m_data[2], alpha);
        #endif
    }


    void GraphicContext::setStrokeRGBA(const RGBA& rgba) noexcept {
        m_stroke_color = rgba;
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextSetRGBStrokeColor(m_cg_context, rgba.m_data[0], rgba.m_data[1], rgba.m_data[2], rgba.m_alpha);
        #endif
    }


    void GraphicContext::setTextMatrix(double a, double b, double c, double d, double tx, double ty) const noexcept {
        if (!m_flipped_y) {
            d = -d;
        }
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextSetTextMatrix(m_cg_context, CGAffineTransformMake(a, b, c, d, tx, ty));
        #endif
    }


    void GraphicContext::moveTo(double x, double y) noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        CGContextMoveToPoint(m_cg_context, x, y);
#endif
        m_last_pos.m_x = x;
        m_last_pos.m_y = y;
    }


    void GraphicContext::moveTo(const Vec2d& point) noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        CGContextMoveToPoint(m_cg_context, point.m_x, point.m_y);
#endif
        m_last_pos = point;
    }


    void GraphicContext::lineTo(double x, double y) noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        CGContextAddLineToPoint(m_cg_context, x, y);
#endif
        m_last_pos.m_x = x;
        m_last_pos.m_y = y;
    }


    void GraphicContext::lineTo(double x, double y, bool start_flag) noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        if (start_flag) {
            CGContextMoveToPoint(m_cg_context, x, y);
        }
        else {
            CGContextAddLineToPoint(m_cg_context, x, y);
        }
#endif
        m_last_pos.m_x = x;
        m_last_pos.m_y = y;
    }


    void GraphicContext::lineTo(const Vec2d& point) noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        CGContextAddLineToPoint(m_cg_context, point.m_x, point.m_y);
#endif
        m_last_pos = point;
    }


    void GraphicContext::lineTo(const Vec2d& point, bool start_flag) noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            if (start_flag) {
                CGContextMoveToPoint(m_cg_context, point.m_x, point.m_y);
            }
            else {
                CGContextAddLineToPoint(m_cg_context, point.m_x, point.m_y);
            }
        #endif
        m_last_pos = point;
    }


    void GraphicContext::curveTo(double c1x, double c1y, double c2x, double c2y, double x, double y) noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextAddCurveToPoint(m_cg_context, c1x, c1y, c2x, c2y, x, y);
        #endif
        m_last_pos.m_x = x;
        m_last_pos.m_y = y;
    }


    void GraphicContext::curveTo(const Vec2d& control1, const Vec2d& control2, const Vec2d& point) noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextAddCurveToPoint(m_cg_context, control1.m_x, control1.m_y, control2.m_x, control2.m_y, point.m_x, point.m_y);
        #endif
        m_last_pos = point;
    }


    void GraphicContext::curveTo(double cx, double cy, double x, double y) noexcept {
        double c1x = m_last_pos.m_x + 2.0 / 3.0 * (cx - m_last_pos.m_x);
        double c2x = x + 2.0 / 3.0 * (cx - x);
        double c1y = m_last_pos.m_y + 2.0 / 3.0 * (cy - m_last_pos.m_y);
        double c2y = y + 2.0 / 3.0 * (cy - y);

        #if defined(__APPLE__) && defined(__MACH__)
            CGContextAddCurveToPoint(m_cg_context, c1x, c1y, c2x, c2y, x, y);
        #endif
        m_last_pos.m_x = x;
        m_last_pos.m_y = y;
    }


    void GraphicContext::addPolygon(Polygon* polygon) noexcept {
        if (polygon != nullptr) {
            auto points = polygon->pointsListPtr();

            if (points != nullptr) {
                auto n = static_cast<int32_t>(points->size());

                if (n > 1) {
                    Vec2d* p = points->mutElementPtrAtIndex(0);
                    moveTo(*p);

                    for (int32_t i = 1; i < n; i++) {
                        p = points->mutElementPtrAtIndex(i);
                        lineTo(*p);
                    }

                    if (polygon->isClosed()) {
                        closePath();
                    }
                }
            }
        }
    }


    void GraphicContext::addPath(GraphicPath* path) noexcept {
        if (path != nullptr && path->pointCount() > 1) {

            GraphicPathPoint* prev_point = nullptr;
            // GraphicPathPoint* first_point = nullptr; // Unused

            for (int32_t point_index = 0; point_index < path->pointCount(); point_index++) {

                auto point = path->pointPtrAtIndex(point_index);

                if (point_index == 0) {
                    // first_point = point; // Unused
                    moveTo(point->m_anchor);
                }
                else {
                    if (prev_point->m_right_flag && point->m_left_flag) {
                        curveTo(prev_point->m_right, point->m_left, point->m_anchor);
                    }
                    else if (prev_point->m_right_flag) {
                        curveTo(prev_point->m_right, point->m_anchor, point->m_anchor);
                    }
                    else if (point->m_left_flag) {
                        curveTo(prev_point->m_anchor, point->m_left, point->m_anchor);
                    }
                    else {
                        lineTo(point->m_anchor);
                    }
                }

                prev_point = point;
            }

            if (path->isClosed()) {
                closePath();
            }
        }
    }


    void GraphicContext::addPath(GraphicPath* path, const GraphicPathSplitParam& split_param) noexcept {
        if (path != nullptr) {
            double ts = 0.0;
            double te = 1.0;

            ts = split_param.m_t0;
            for (int32_t i = split_param.m_start_index; i <= split_param.m_end_index; i++) {

                Bezier bezier;
                path->bezierAtIndex(i, bezier);

                if (i == split_param.m_start_index) {
                    ts = split_param.m_t0;
                }
                else {
                    ts = 0.0;
                }

                if (i == split_param.m_end_index) {
                    te = split_param.m_t1;
                }
                else {
                    te = 1.0;
                }

                if (bezier.truncate(ts, te, bezier)) {

                    if (i == split_param.m_start_index) {
                        moveTo(bezier.m_pos[0]);
                    }

                    curveTo(bezier.m_pos[1], bezier.m_pos[2], bezier.m_pos[3]);
                }

                if (path->m_closed) {
                    closePath();
                }
            }
        }
    }


    void GraphicContext::addRectPath(double x, double y, double width, double height) noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextAddRect(m_cg_context, CGRectMake(x, y, width, height));
        #endif
    }


    void GraphicContext::addRectPath(const Rectd& rect) noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextAddRect(m_cg_context, CGRectMake(rect.m_x, rect.m_y, rect.m_width, rect.m_height));
        #endif
    }


    bool GraphicContext::addFramePath(const Rectd& rect, double top, double right, double bottom, double left) noexcept {
        int32_t mode = 0x0;
        if (top > std::numeric_limits<float>::epsilon()) {
            mode |= 0x1;
        }
        if (right > std::numeric_limits<float>::epsilon()) {
            mode |= 0x2;
        }
        if (bottom > std::numeric_limits<float>::epsilon()) {
            mode |= 0x4;
        }
        if (left > std::numeric_limits<float>::epsilon()) {
            mode |= 0x8;
        }

        if (mode == 0x0) {
            return false;
        }

        top = std::min<double>(top, 0.0);
        right = std::min<double>(right, 0.0);
        bottom = std::min<double>(bottom, 0.0);
        left = std::min<double>(left, 0.0);

        double w = rect.m_width;
        double h = rect.m_height;
        double x0 = rect.m_x;
        double x3 = rect.x2();
        double x1 = x0 + left;
        double x2 = x3 - right;

        double y0, y1, y2, y3;
        if (m_flipped_y) {
            y0 = rect.m_y;
            y3 = rect.y2();
            y1 = y0 + top;
            y2 = y3 - bottom;
        }
        else {
            y0 = rect.y2();
            y3 = rect.m_y;
            y1 = y0 - top;
            y2 = y3 + bottom;
        }

        if ((top + bottom) > h || (right + left) > w) {
            moveTo(x0, y0);
            lineTo(x3, y0);
            lineTo(x3, y3);
            lineTo(x0, y3);
            closePath();
        }
        else {
            bool topFlag = false;
            bool rightFlag = false;
            bool bottomFlag = false;
            bool leftFlag = false;

            if (mode == 0x1) {
                topFlag = true;
            }
            else if (mode == 0x2) {
                rightFlag = true;
            }
            else if (mode == 0x3) {
                moveTo(x0, y0);
                lineTo(x3, y0);
                lineTo(x3, y3);
                lineTo(x2, y3);
                lineTo(x2, y1);
                lineTo(x0, y1);
                closePath();
            }
            else if (mode == 0x4) {
                bottomFlag = true;
            }
            else if (mode == 0x5) {
                topFlag = bottomFlag = true;
            }
            else if (mode == 0x6) {
                lineTo(x3, y0);
                lineTo(x3, y3);
                lineTo(x0, y3);
                lineTo(x0, y2);
                lineTo(x2, y2);
                lineTo(x2, y0);
                closePath();
            }
            else if (mode == 0x7) {
                lineTo(x0, y0);
                lineTo(x3, y0);
                lineTo(x3, y3);
                lineTo(x0, y3);
                lineTo(x0, y2);
                lineTo(x2, y2);
                lineTo(x2, y1);
                lineTo(x0, y1);
                closePath();
            }
            else if (mode == 0x8) {
                leftFlag = true;
            }
            else if (mode == 0x9) {
                lineTo(x0, y0);
                lineTo(x3, y0);
                lineTo(x3, y1);
                lineTo(x1, y1);
                lineTo(x1, y3);
                lineTo(x0, y3);
                closePath();
            }
            else if (mode == 0xA) {
                rightFlag = leftFlag = true;
            }
            else if (mode == 0xB) {
                lineTo(x0, y0);
                lineTo(x3, y0);
                lineTo(x3, y3);
                lineTo(x2, y3);
                lineTo(x2, y1);
                lineTo(x1, y1);
                lineTo(x1, y3);
                lineTo(x0, y3);
                closePath();
            }
            else if (mode == 0xC) {
                lineTo(x0, y0);
                lineTo(x1, y0);
                lineTo(x1, y2);
                lineTo(x3, y2);
                lineTo(x3, y3);
                lineTo(x0, y3);
                closePath();
            }
            else if (mode == 0xD) {
                lineTo(x0, y0);
                lineTo(x3, y0);
                lineTo(x3, y1);
                lineTo(x1, y1);
                lineTo(x1, y2);
                lineTo(x3, y2);
                lineTo(x3, y3);
                lineTo(x0, y3);
                closePath();
            }
            else if (mode == 0xE) {
                lineTo(x0, y0);
                lineTo(x1, y0);
                lineTo(x1, y2);
                lineTo(x2, y2);
                lineTo(x2, y0);
                lineTo(x3, y0);
                lineTo(x3, y3);
                lineTo(x0, y3);
                closePath();
            }
            else if (mode == 0xF) {
                moveTo(x0, y0);
                lineTo(x3, y0);
                lineTo(x3, y3);
                lineTo(x0, y3);
                closePath();
                moveTo(x1, y1);
                lineTo(x2, y1);
                lineTo(x2, y2);
                lineTo(x1, y2);
                closePath();
            }

            if (topFlag) {
                moveTo(x0, y0);
                lineTo(x3, y0);
                lineTo(x3, y1);
                lineTo(x3, y1);
                closePath();
            }

            if (rightFlag) {
                moveTo(x2, y0);
                lineTo(x3, y0);
                lineTo(x3, y3);
                lineTo(x2, y3);
                closePath();
            }

            if (bottomFlag) {
                moveTo(x0, y2);
                lineTo(x3, y2);
                lineTo(x3, y3);
                lineTo(x3, y3);
                closePath();
            }

            if (leftFlag) {
                moveTo(x0, y0);
                lineTo(x1, y0);
                lineTo(x1, y3);
                lineTo(x0, y3);
                closePath();
            }
        }

        return true;
    }


    void GraphicContext::addEllipsePath(const Rectd& rect) noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextAddEllipseInRect(m_cg_context, rect.cgRect());
        #endif
    }


    void GraphicContext::addCirclePath(double x, double y, double radius) noexcept {
        double d = radius + radius;
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextAddEllipseInRect(m_cg_context, CGRectMake(x - radius, y - radius, d, d));
        #endif
    }


    void GraphicContext::addCirclePath(const Vec2d& center, double radius) noexcept {
        double d = radius + radius;
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextAddEllipseInRect(m_cg_context, CGRectMake(center.m_x - radius, center.m_y - radius, d, d));
        #endif
    }


    void GraphicContext::addRoundBarPath(double x, double y, double width, double height) noexcept {
        if (std::fabs(width - height) < std::numeric_limits<float>::epsilon()) {
            // Draw as circle
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextAddEllipseInRect(m_cg_context, CGRectMake(x, y, width, height));
            #endif
        }
        else {
            bool horizontal = width > height;
            double radius = horizontal ? height / 2.0 : width / 2.0;
            double a = radius * 0.55228475;

            double x1 = x;
            double x2 = x + radius - a;
            double x3 = x + radius;
            double x6 = x + width;
            double x5 = x6 - radius + a;
            double x4 = x6 - radius;

            double y1 = y;
            double y2 = y + radius - a;
            double y3 = y + radius;
            double y6 = y + height;
            double y5 = y6 - radius + a;
            double y4 = y6 - radius;

            moveTo(x4, y1);
            curveTo(x5, y1, x6, y2, x6, y3);
            if (!horizontal) {
                lineTo(x6, y4);
            }
            curveTo(x6, y5, x5, y6, x4, y6);
            if (horizontal) {
                lineTo(x3, y6);
            }
            curveTo(x2, y6, x1, y5, x1, y4);
            if (!horizontal) {
                lineTo(x1, y3);
            }
            curveTo(x1, y2, x2, y1, x3, y1);
            if (horizontal) {
                lineTo(x4, y1);
            }
            closePath();
        }
    }


    void GraphicContext::addRoundBarPath(const Rectd& rect) noexcept {
        addRoundBarPath(rect.m_x, rect.m_y, rect.m_width, rect.m_height);
    }


    void GraphicContext::addRoundRectPath(double x, double y, double width, double height, double radius) noexcept {
        if (radius > width / 2) {
            radius = width / 2;
        }
        if (radius > height / 2) {
            radius = height / 2;
        }

        double a = radius * 0.3907;
        double b = radius * 0.5944;
        double c = radius * 0.7854;
        double d = radius * 0.9764;

        double x0 = x;
        double x1 = x + radius;
        double x1a = x1 - a;
        double x1b = x1 - b;
        double x1c = x1 - c;
        double x1d = x1 - d;
        double x2 = x + width - radius;
        double x2a = x2 + a;
        double x2b = x2 + b;
        double x2c = x2 + c;
        double x2d = x2 + d;
        double x3 = x + width;

        double y0 = y;
        double y1 = y + radius;
        double y1a = y1 - a;
        double y1b = y1 - b;
        double y1c = y1 - c;
        double y1d = y1 - d;
        double y2 = y + height - radius;
        double y2a = y2 + a;
        double y2b = y2 + b;
        double y2c = y2 + c;
        double y2d = y2 + d;
        double y3 = y + height;

        moveTo(x2, y0);
        curveTo(x2a, y0, x2b, y1d, x2c, y1c);
        curveTo(x2 + d, y1 - b, x3, y1 - a, x3, y1);
        lineTo(x3, y2);
        curveTo(x3, y2a, x2d, y2b, x2c, y2c);
        curveTo(x2b, y2d, x2a, y3, x2, y3);
        lineTo(x1, y3);
        curveTo(x1a, y3, x1b, y2d, x1c, y2c);
        curveTo(x1d, y2b, x0, y2a, x0, y2);
        lineTo(x0, y1);
        curveTo(x0, y1a, x1d, y1b, x1c, y1c);
        curveTo(x1b, y1d, x1a, y0, x1, y0);
        closePath();
    }


    void GraphicContext::addRoundRectPath(const Rectd& rect, double radius) noexcept {
        addRoundRectPath(rect.m_x, rect.m_y, rect.m_width, rect.m_height, radius);
    }


    void GraphicContext::addRoundRectPath(double x, double y, double width, double height, double radius1, double radius2, double radius3, double radius4) noexcept {
        double a = 0.55228475;
        double b = 1.0 - a;

        double x2 = x + width;
        double y2 = y + height;

        if (radius1 < std::numeric_limits<float>::min()) {
            moveTo(x2, y);
        }
        else {
            moveTo(x2 - radius1, y);
            curveTo(x2 - b * radius1, y, x2, y + b * radius1, x2, y + radius1);
        }

        if (radius2 < std::numeric_limits<float>::min()) {
            lineTo(x2, y2);
        }
        else {
            lineTo(x2, y2 - radius2);
            curveTo(x2, y2 - b * radius2, x2 - b * radius2, y2, x2 - radius2, y2);
        }

        if (radius3 < std::numeric_limits<float>::min()) {
            lineTo(x, y2);
        }
        else {
            lineTo(x + radius3, y2);
            curveTo(x + b * radius3, y2, x, y2 - b * radius3, x, y2 - radius3);
        }

        if (radius4 < std::numeric_limits<float>::min()) {
            lineTo(x, y);
        }
        else {
            lineTo(x, y + radius4);
            curveTo(x, y + b * radius4, x + b * radius4, y, x + radius4, y);
        }

        closePath();
    }


    void GraphicContext::addRoundRectPath(const Rectd& rect, double radius1, double radius2, double radius3, double radius4) noexcept {
        addRoundRectPath(rect.m_x, rect.m_y, rect.m_width, rect.m_height, radius1, radius2, radius3, radius4);
    }


    void GraphicContext::addRingPath(const Vec2d& center, double inner_radius, double outer_radius, double angle, double span) noexcept {
        if (span > 0.0) {
            angle = Math::degtorad(angle);
            span = Math::degtorad(span);
            double endAngle = angle + span;

            #if defined(__APPLE__) && defined(__MACH__)
                CGContextAddArc(m_cg_context, center.m_x, center.m_y, outer_radius, angle, endAngle, false);
                CGContextAddArc(m_cg_context, center.m_x, center.m_y, inner_radius, endAngle, angle, true);
            #endif
            closePath();
        }
    }


    void GraphicContext::addTrianglePath(const Triangled& triangle) noexcept {
        moveTo(triangle.m_points[0]);
        lineTo(triangle.m_points[1]);
        lineTo(triangle.m_points[2]);
        closePath();
    }


    void GraphicContext::addTrianglePath(const Vec2d& point1, const Vec2d& point2, const Vec2d& point3) noexcept {
        moveTo(point1);
        lineTo(point2);
        lineTo(point3);
        closePath();
    }


    void GraphicContext::addTrianglePath(double x, double y, double width, double height, Direction direction) noexcept {
        switch (direction) {
            case Direction::LeftToRight:
                moveTo(x + width, y + height * 0.5);
                lineTo(x, y + height);
                lineTo(x, y);
                closePath();
                break;

            case Direction::RightToLeft:
                moveTo(x, y + height * 0.5f);
                lineTo(x + width, y);
                lineTo(x + width, y + height);
                closePath();
                break;

            case Direction::BottomToTop:
                moveTo(x + width * 0.5f, y);
                lineTo(x + width, y + height);
                lineTo(x, y + height);
                closePath();
                break;

            case Direction::TopToBottom:
                moveTo(x + width * 0.5f, y + height);
                lineTo(x, y);
                lineTo(x + width, y);
                closePath();
                break;

            default:
                break;
        }
    }


    void GraphicContext::addTrianglePath(const Rectd& rect, Direction direction) noexcept {
        addTrianglePath(rect.m_x, rect.m_y, rect.m_width, rect.m_height, direction);
    }


    void GraphicContext::addPolygonPath(int32_t point_count, const Vec2d* points) noexcept {
        if (points != nullptr && point_count > 2) {
            const Vec2d* p = points;
            moveTo(*p);
            for (int32_t i = 1; i < point_count; i++) {
                p++;
                lineTo(*p);
            }
            closePath();
        }
    }


    void GraphicContext::addDropPath() noexcept {
        beginPath();
        moveTo(0, 0);
        curveTo(0, 0, 0.052616, 0.0968366, 0.148868, 0.255899);
        curveTo(0.225314, 0.382229, 0.35, 0.532691, 0.35, 0.65);
        curveTo(0.35, 0.843275, 0.193299, 1, 0, 1);
        curveTo(-0.193299, 1, -0.35, 0.843275, -0.35, 0.65);
        curveTo(-0.35, 0.532691, -0.225314, 0.382229, -0.148868, 0.255899);
        curveTo(-0.052616, 0.0968366, 0, 0, 0, 0);
        closePath();
    }


    void GraphicContext::addRightHalfDropPath() noexcept {
        beginPath();
        moveTo(0, 0);
        curveTo(0, 0, 0.052616, 0.0968366, 0.148868, 0.255899);
        curveTo(0.225314, 0.382229, 0.35, 0.532691, 0.35, 0.65);
        curveTo(0.35, 0.843275, 0.193299, 1, 0, 1);
        lineTo(0, 0);
        closePath();
    }


    void GraphicContext::addLeftHalfDropPath() noexcept {
        beginPath();
        moveTo(0, 1);
        curveTo(-0.193299, 1, -0.35, 0.843275, -0.35, 0.65);
        curveTo(-0.35, 0.532691, -0.225314, 0.382229, -0.148868, 0.255899);
        curveTo(-0.052616, 0.0968366, 0, 0, 0, 0);
        moveTo(0, 1);
        closePath();
    }


    void GraphicContext::strokeLine(double x1, double y1, double x2, double y2) noexcept {
        beginPath();
        moveTo(x1, y1);
        lineTo(x2, y2);
        strokePath();
    }


    void GraphicContext::strokeLine(const Vec2d& point1, const Vec2d& point2) noexcept {
        beginPath();
        moveTo(point1);
        lineTo(point2);
        strokePath();
    }


    void GraphicContext::strokeLineXZ(const Vec3d& point1, const Vec3d& point2) noexcept {
        beginPath();
        moveTo(point1.m_x, point1.m_z);
        lineTo(point2.m_x, point2.m_z);
        strokePath();
    }


    void GraphicContext::strokeLineXY(const Vec3d& point1, const Vec3d& point2) noexcept {
        beginPath();
        moveTo(point1.m_x, point1.m_y);
        lineTo(point2.m_x, point2.m_y);
        strokePath();
    }


    void GraphicContext::strokeLine(const Lined& line) noexcept {
        beginPath();
        moveTo(line.m_p1);
        lineTo(line.m_p2);
        strokePath();
    }


    void GraphicContext::strokeHorizontalLine(double x1, double x2, double y) noexcept {
        beginPath();
        moveTo(x1, y);
        lineTo(x2, y);
        strokePath();
    }


    void GraphicContext::strokeVerticalLine(double x, double y1, double y2) noexcept {
        beginPath();
        moveTo(x, y1);
        lineTo(x, y2);
        strokePath();
    }


    void GraphicContext::strokeHorizontalConnection(const Vec2d& start_point, const Vec2d& end_point) noexcept {
        double a = std::fabs(end_point.m_x - start_point.m_x) / 2.0;
        if (a < 8.0) {
            a = 8.0;
        }

        beginPath();
        moveTo(start_point);
        curveTo(start_point.m_x + a, start_point.m_y, end_point.m_x - a, end_point.m_y, end_point.m_x, end_point.m_y);
        strokePath();
    }


    /**
     *  @brief Stroke a Bezier curve.
     */
    void GraphicContext::strokeBezier(const Bezier& bezier) noexcept {
        beginPath();
        moveTo(bezier.m_pos[0]);
        curveTo(bezier.m_pos[1], bezier.m_pos[2], bezier.m_pos[3]);
        strokePath();
    }


    /**
     *  @brief Stroke a Catmull Rom curve.
     */
    void GraphicContext::strokeCatmullRomCurve(const CatmullRomCurve& catmull_rom_curve, int32_t resolution) noexcept {
        if (catmull_rom_curve.pointCount() < 4) {
            return;
        }

        beginPath();

        if (resolution < 1) {
            resolution = catmull_rom_curve.defaultResolution();
        }

        resolution = std::clamp<int32_t>(resolution, 1, 1000000);    // TODO: Is this a good limit?

        Vec2d point;
        catmull_rom_curve.pointOnCurve(0.0, point);
        moveTo(point);
        for (int32_t i = 1; i <= resolution; i++) {
            catmull_rom_curve.pointOnCurve(static_cast<double>(i) / resolution, point);
            lineTo(point);
        }

        strokePath();
    }


    void GraphicContext::strokeCatmullRomCurve(const CatmullRomCurve& catmull_rom_curve, float t_beg, float t_end, int32_t resolution) noexcept {
        if (catmull_rom_curve.pointCount() < 4) {
            return;
        }

        if (t_beg < 0.0) {
            t_beg = 0.0;
        }

        if (t_end > 1.0) {
            t_end = 1.0;
        }

        if (t_beg >= t_end) {
            return;
        }


        beginPath();

        if (resolution < 1) {
            resolution = catmull_rom_curve.defaultResolution();
        }

        resolution = resolution * (t_end - t_beg);
        resolution = std::clamp<int32_t>(resolution, 1, 1000000);    // TODO: Is this a good limit?

        double t = t_beg;
        double t_step = (t_end - t_beg) / resolution;

        Vec2d point;
        catmull_rom_curve.pointOnCurve(t, point);
        moveTo(point);

        for (int32_t i = 0; i <= resolution; i++) {
            t += t_step;
            catmull_rom_curve.pointOnCurve(t, point);
            lineTo(point);
        }

        strokePath();
    }


    void GraphicContext::fillRect(double x, double y, double width, double height) noexcept {
        if (width > 0 && height > 0) {
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextFillRect(m_cg_context,CGRectMake(x, y, width, height));
            #endif
        }
    }


    void GraphicContext::fillRect(const Rectd& rect) noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextFillRect(m_cg_context, rect.cgRect());
        #endif
    }


    void GraphicContext::fillRect(const Rectd& rect, double radius) noexcept {
        if (radius < std::numeric_limits<float>::epsilon()) {
            fillRect(rect);
        }
        else {
            fillRoundRect(rect, radius);
        }
    }


    void GraphicContext::fillRoundRect(double x, double y, double width, double height, double radius) noexcept {
        beginPath();
        addRoundRectPath(x, y, width, height, radius);
        fillPath();
    }


    void GraphicContext::fillRoundRect(const Rectd& rect, double radius) noexcept {
        fillRoundRect(rect.m_x, rect.m_y, rect.m_width, rect.m_height, radius);
    }


    void GraphicContext::fillRoundRect(double x, double y, double width, double height, double radius1, double radius2, double radius3, double radius4) noexcept {
        beginPath();
        addRoundRectPath(x, y, width, height, radius1, radius2, radius3, radius4);
        fillPath();
    }


    void GraphicContext::fillRoundRect(const Rectd& rect, double radius1, double radius2, double radius3, double radius4) noexcept {
        fillRoundRect(rect.m_x, rect.m_y, rect.m_width, rect.m_height, radius1, radius2, radius3, radius4);
    }


    void GraphicContext::fillRoundBar(double x, double y, double width, double height) noexcept {
        beginPath();
        addRoundBarPath(x, y, width, height);
        fillPath();
    }


    void GraphicContext::fillRoundBar(const Rectd& rect) noexcept {
        fillRoundBar(rect.m_x, rect.m_y, rect.m_width, rect.m_height);
    }


    void GraphicContext::fillFrame(const Rectd& rect, double size) noexcept {
        beginPath();
        addRectPath(rect);
        addRectPath(rect.m_x + size, rect.m_y + size, rect.m_width - size * 2, rect.m_height - size * 2);
        fillPathEvenOdd();
    }


    void GraphicContext::fillFrame(const Rectd& rect, double width, double height) noexcept {
        beginPath();
        addRectPath(rect);
        addRectPath(rect.m_x + width, rect.m_y + height, rect.m_width - width * 2, rect.m_height - height * 2);
        fillPathEvenOdd();
    }


    void GraphicContext::fillFrame(const Rectd& rect, double top, double right, double bottom, double left) noexcept {
        beginPath();
        addFramePath(rect, top, right, bottom, left);
        fillPathEvenOdd();
    }


    void GraphicContext::strokeRect(double x, double y, double width, double height) noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextStrokeRect(m_cg_context, CGRectMake(x, y, width, height));
        #endif
    }


    void GraphicContext::strokeRect(const Rectd& rect) noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextStrokeRect(m_cg_context, rect.cgRect());
        #endif
    }


    void GraphicContext::strokeRect(const Rectd& rect, double offset) noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextStrokeRect(m_cg_context, CGRectMake(rect.m_x - offset, rect.m_y - offset, rect.m_width + offset * 2, rect.m_height + offset * 2));
        #endif
    }


    void GraphicContext::strokeRoundBar(double x, double y, double width, double height) noexcept {
        beginPath();
        addRoundBarPath(x, y, width, height);
        strokePath();
    }


    void GraphicContext::strokeRoundBar(const Rectd& rect) noexcept {
        strokeRoundBar(rect.m_x, rect.m_y, rect.m_width, rect.m_height);
    }


    void GraphicContext::strokeRoundRect(double x, double y, double width, double height, double radius) noexcept {
        beginPath();
        addRoundRectPath(x, y, width, height, radius);
        strokePath();
    }


    void GraphicContext::strokeRoundRect(const Rectd& rect, double radius) noexcept {
        strokeRoundRect(rect.m_x, rect.m_y, rect.m_width, rect.m_height, radius);
    }


    void GraphicContext::strokeRoundRect(double x, double y, double width, double height, double radius1, double radius2, double radius3, double radius4) noexcept {
        beginPath();
        addRoundRectPath(x, y, width, height, radius1, radius2, radius3, radius4);
        strokePath();
    }


    void GraphicContext::strokeRoundRect(const Rectd& rect, double radius1, double radius2, double radius3, double radius4) noexcept {
        strokeRoundRect(rect.m_x, rect.m_y, rect.m_width, rect.m_height, radius1, radius2, radius3, radius4);
    }


    void GraphicContext::addQuadrilateralPath(const Vec2d* points) noexcept {
        if (points != nullptr) {
            moveTo(points[0]);
            lineTo(points[1]);
            lineTo(points[2]);
            lineTo(points[3]);
            closePath();
        }
    }


    void GraphicContext::addQuadrilateralPath(const Quadrilateral& quadrilateral) noexcept {
        moveTo(quadrilateral.m_points[0]);
        lineTo(quadrilateral.m_points[1]);
        lineTo(quadrilateral.m_points[2]);
        lineTo(quadrilateral.m_points[3]);
        closePath();
    }


    void GraphicContext::fillQuadrilateral(const Vec2d* points) noexcept {
        if (points != nullptr) {
            beginPath();
            addQuadrilateralPath(points);
            fillPath();
        }
    }


    void GraphicContext::fillQuadrilateral(const Quadrilateral& quadrilateral) noexcept {
        beginPath();
        addQuadrilateralPath(quadrilateral);
        fillPath();
    }


    void GraphicContext::strokeQuadrilateral(const Vec2d* points) noexcept {
        beginPath();
        addQuadrilateralPath(points);
        strokePath();
    }


    void GraphicContext::strokeQuadrilateral(const Quadrilateral& quadrilateral) noexcept {
        beginPath();
        addQuadrilateralPath(quadrilateral);
        strokePath();
    }


    void GraphicContext::fillTriangle(const Rectd& rect, Direction direction) noexcept {
        beginPath();
        addTrianglePath(rect, direction);
        fillPath();
    }


    void GraphicContext::fillPolygon(int32_t point_count, const Vec2d* points) noexcept {
        beginPath();
        addPolygonPath(point_count, points);
        fillPath();
    }


    void GraphicContext::fillEllipse(const Rectd& rect) noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextFillEllipseInRect(m_cg_context, rect.cgRect());
        #endif
    }


    void GraphicContext::fillEllipse(double x, double y, double rh, double rv) noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextFillEllipseInRect(m_cg_context, CGRectMake(x - rh, y - rv, rh * 2, rv * 2));
        #endif
    }


    void GraphicContext::fillEllipse(const Vec2d& center, double rh, double rv) noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextFillEllipseInRect(m_cg_context, CGRectMake(center.m_x - rh, center.m_y - rv, rh * 2, rv * 2));
        #endif
    }


    void GraphicContext::strokeEllipse(const Rectd& rect) noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextStrokeEllipseInRect(m_cg_context, rect.cgRect());
        #endif
    }


    void GraphicContext::strokeEllipse(double x, double y, double rh, double rv) noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextStrokeEllipseInRect(m_cg_context, CGRectMake(x - rh, y - rv, rh * 2, rv * 2));
        #endif
    }


    void GraphicContext::strokeEllipse(const Vec2d& center, double rh, double rv) noexcept {
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextStrokeEllipseInRect(m_cg_context, CGRectMake(center.m_x - rh, center.m_y - rv, rh * 2, rv * 2));
        #endif
    }


    void GraphicContext::fillCircle(const Circled& circle) noexcept {
        fillCircle(circle.m_center, circle.m_radius);
    }


    void GraphicContext::fillCircle(const Rectd& rect) noexcept {
        fillCircle(rect.centerX(), rect.centerY(), rect.shortSide() / 2);
    }


    void GraphicContext::fillCircle(const Rectd& rect, double min_radius, double max_radius) noexcept {
        double radius = std::clamp<double>(rect.shortSide() / 2.0, min_radius, max_radius);
        fillCircle(rect.centerX(), rect.centerY(), radius);
    }


    void GraphicContext::fillCircle(double x, double y, double radius) noexcept {
        if (radius > std::numeric_limits<float>::epsilon()) {
            double diameter = radius + radius;
            #if defined(__APPLE__) && defined(__MACH__)
                CGContextFillEllipseInRect(m_cg_context, CGRectMake(x - radius, y - radius, diameter, diameter));
            #endif
        }
    }


    void GraphicContext::fillCircle(const Vec2d& center, double radius) noexcept {
        fillCircle(center.m_x, center.m_y, radius);
    }


    void GraphicContext::strokeCircle(const Circled& circle) noexcept {
        strokeCircle(circle.m_center, circle.m_radius);
    }


    void GraphicContext::strokeCircle(double x, double y, double radius) noexcept {
        double diameter = radius + radius;
        #if defined(__APPLE__) && defined(__MACH__)
            CGContextStrokeEllipseInRect(m_cg_context, CGRectMake(x - radius, y - radius, diameter, diameter));
        #endif
    }


    void GraphicContext::strokeCircle(const Vec2d& center, double radius) noexcept {
        strokeCircle(center.m_x, center.m_y, radius);
    }


    void GraphicContext::strokeCircle(const Rectd& rect) noexcept {
        strokeCircle(rect.centerX(), rect.centerY(), rect.shortSide() / 2);
    }


    void GraphicContext::strokeCircle(const Rectd& rect, double min_radius, double max_radius) noexcept {
        double radius = std::clamp<double>(rect.shortSide() / 2.0, min_radius, max_radius);
        strokeCircle(rect.centerX(), rect.centerY(), radius);
    }


    void GraphicContext::fillRing(const Vec2d& center, double inner_radius, double outer_radius, double angle, double span) noexcept {
        beginPath();
        addRingPath(center, inner_radius, outer_radius, angle, span);
        fillPath();
    }


    void GraphicContext::fillColorWheel(const Vec2d& center, double outer_radius, double inner_radius) noexcept {

        /* TODO: !!!!!
        Image* color_wheel_image = App::colorWheelImage();

        if (color_wheel_image != nullptr) {

            CGImageRef cg_image = color_wheel_image->macos_cgImageRef();

            if (cg_image != nullptr) {

                CGContextSaveGState(m_cg_context);

                Rectd outerRect(center.m_x - outer_radius, center.m_y - outer_radius, outer_radius * 2, outer_radius * 2);
                Rectd innerRect(outerRect, outer_radius - inner_radius);

                beginPath();
                CGContextAddEllipseInRect(m_cg_context, outerRect.cgRect());
                CGContextAddEllipseInRect(m_cg_context, innerRect.cgRect());
                clipPathEvenOdd();

                CGContextDrawImage(m_cg_context, outerRect.cgRect(), cg_image);

                CGContextRestoreGState(m_cg_context);
            }
        }
         */
    }


    void GraphicContext::fillAudioLocationControl(const Vec2d& center, double radius) noexcept {

        /* TODO !!!!!!
        Image* audio_location_image = App::audioLocationImage();

        if (audio_location_image != nullptr) {

            CGImageRef cg_image = audio_location_image->macos_cgImageRef();

            if (cg_image != nullptr) {

                CGContextSaveGState(m_cg_context);

                Rectd rect(center.m_x - radius, center.m_y - radius, radius * 2, radius * 2);

                beginPath();
                addEllipsePath(rect);
                clipPath();

                CGContextDrawImage(m_cg_context, rect.cgRect(), cg_image);

                CGContextRestoreGState(m_cg_context);
            }
        }
         */
    }

#if defined(__APPLE__) && defined(__MACH__)
    void GraphicContext::drawImage(Image* image, const Rectd& rect, float alpha) noexcept {
        if (image != nullptr && image->hasPixel()) {
            if (CGImageRef cg_image = image->macos_cgImageRef()) {
                CGContextSaveGState(m_cg_context);

                if (m_flipped_y) {
                    CGContextTranslateCTM(m_cg_context, 0, rect.centerY());
                    CGContextScaleCTM(m_cg_context, 1, -1);
                    CGContextTranslateCTM(m_cg_context, 0, -rect.centerY());
                }

                CGContextSetAlpha(m_cg_context, alpha);
                CGContextSetInterpolationQuality(m_cg_context, kCGInterpolationMedium);
                CGContextDrawImage(m_cg_context, rect.cgRect(), cg_image);

                CGContextRestoreGState(m_cg_context);
            }
        }
    }
#else
    void GraphicContext::drawImage(Image* image, const Rectd& rect, float alpha) noexcept {
        // TODO: Implement linux version
    }
#endif


    ErrorCode GraphicContext::drawQuadrilateralImage(Image* image, const Quadrilateral& quadrilateral) noexcept {

        /* TODO !!!!!!

        auto result = ErrorCode::None;

        @autoreleasepool {

        try {

            CGImageRef cg_image = image->macos_cgImageRef();
            CIImage* ci_image = [CIImage imageWithCGImage:cg_image];
            if (!ci_image) {
                throw ErrorCode::Fatal;
            }

            // Create the perspective transform filter.
            CIFilter* perspective_filter = [CIFilter filterWithName:@"CIPerspectiveTransform"];
            if (!perspective_filter) {
                throw ErrorCode::Fatal;
            }

            [perspective_filter setValue:ci_image forKey:kCIInputImageKey];

            auto points = quadrilateral.pointsPtr();

            // Define the quadrilateral points for transformation
            [perspective_filter setValue:[CIVector vectorWithX:points[0].m_x Y:points[0].m_y] forKey:@"inputTopLeft"];
            [perspective_filter setValue:[CIVector vectorWithX:points[1].m_x Y:points[1].m_y] forKey:@"inputTopRight"];
            [perspective_filter setValue:[CIVector vectorWithX:points[2].m_x Y:points[2].m_y] forKey:@"inputBottomRight"];
            [perspective_filter setValue:[CIVector vectorWithX:points[3].m_x Y:points[3].m_y] forKey:@"inputBottomLeft"];

            // Get the output CIImage from the filter.
            CIImage* output_ci_image = [perspective_filter outputImage];
            if (!output_ci_image) {
                throw ErrorCode::Fatal;
            }

            // Create a CIContext for rendering.
            CIContext* ci_context = [CIContext contextWithCGContext:m_cg_context options:nil];

            CGRect target_rect = output_ci_image.extent;

            // Render the CIImage to the CGContextRef within the target rectangle
            [ci_context drawImage:output_ci_image inRect:target_rect fromRect:[output_ci_image extent]];

            // Manual memory cleanup.   // TODO: Check what needs do be released!
            // [ci_image release]; // Not necessary, momory is autoreleased!
            // [perspective_filter release]; // Not necessary, momory is autoreleased!
            // [output_ci_image release]; // Not necessary, momory is autoreleased!
            // [ci_context release]; // Not necessary, momory is autoreleased!

        }
        catch (ErrorCode err) {
            result = err;
        }

        } // End of @autoreleasepool

        */
        return ErrorCode::None;
    }


ErrorCode GraphicContext::drawQuadrilateralImage(Image* image, const Quadrilateral& quadrilateral, float alpha) noexcept {

/* TODO !!!!!!
// TODO: Combine the two methods GraphicContext::drawQuadrilateralImage into one.

// Get CGImage reference and convert it to CIImage
CGImageRef cg_image = image->macos_cgImageRef();
CIImage* ci_image = [CIImage imageWithCGImage:cg_image];
CGImageRelease(cg_image);
if (!ci_image) {
    return ErrorCode::Fatal;
}

// Create the perspective transform filter
CIFilter* perspective_filter = [CIFilter filterWithName:@"CIPerspectiveTransform"];
if (!perspective_filter) {
    return ErrorCode::Fatal;
}
[perspective_filter setValue:ci_image forKey:kCIInputImageKey];

// Define the quadrilateral points for transformation
auto points = quadrilateral.pointsPtr();
[perspective_filter setValue:[CIVector vectorWithX:points[0].m_x Y:points[0].m_y] forKey:@"inputTopLeft"];
[perspective_filter setValue:[CIVector vectorWithX:points[1].m_x Y:points[1].m_y] forKey:@"inputTopRight"];
[perspective_filter setValue:[CIVector vectorWithX:points[2].m_x Y:points[2].m_y] forKey:@"inputBottomRight"];
[perspective_filter setValue:[CIVector vectorWithX:points[3].m_x Y:points[3].m_y] forKey:@"inputBottomLeft"];

// Get the output CIImage from the perspective filter
CIImage* output_ci_image = [perspective_filter outputImage];
if (!output_ci_image) {
    return ErrorCode::Fatal;
}

// Apply alpha transparency using a color generator and compositing filter
CIFilter* alphaFilter = [CIFilter filterWithName:@"CIConstantColorGenerator"];
CIColor* transparentColor = [CIColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:alpha];
[alphaFilter setValue:transparentColor forKey:kCIInputColorKey];

CIFilter* compositeFilter = [CIFilter filterWithName:@"CIMultiplyCompositing"];
[compositeFilter setValue:alphaFilter.outputImage forKey:kCIInputBackgroundImageKey];
[compositeFilter setValue:output_ci_image forKey:kCIInputImageKey];

CIImage* transparent_image = compositeFilter.outputImage;

// Create a CIContext for rendering
CIContext* ci_context = [CIContext contextWithCGContext:m_cg_context options:nil];

CGRect target_rect = transparent_image.extent;

// Render the transparent CIImage to the CGContextRef within the target rectangle
[ci_context drawImage:transparent_image inRect:target_rect fromRect:[output_ci_image extent]];

// Manual memory cleanup.   // TODO: Check what needs do be released!
[ci_image release];
// [perspective_filter release];
// [compositeFilter release];
// [output_ci_image release];
[ci_context release];

 */
        return ErrorCode::None;
    }



void GraphicContext::drawIcon(const Icon* icon, const Rectd& rect, float alpha) noexcept {

    /* TODO !!!!!!

if (icon != nullptr) {

CGContextSaveGState(m_cg_context);

// CGContextTranslateCTM(m_cg_context, 0.0, m_height);
CGContextScaleCTM(m_cg_context, 1.0, -1.0);
CGContextTranslateCTM(m_cg_context, 0.0, - rect.m_y * 2 - rect.m_height);

setAlpha(alpha);
CGContextDrawImage(m_cg_context, rect.cgRect(), icon->_m_cg_image);

CGContextRestoreGState(m_cg_context);
}
     */
}


void GraphicContext::drawIcon(const Icon* icon, const Rectd& rect, const RGB& color, float alpha) noexcept {

    /* TODO !!!!!!
if (icon != nullptr) {

CGContextSaveGState(m_cg_context);

// CGContextTranslateCTM(m_cg_context, 0.0, m_height);
CGContextScaleCTM(m_cg_context, 1, -1);
CGContextTranslateCTM(m_cg_context, 0, -rect.m_height - rect.m_y * 2 );

// CGContextDrawImage(m_cg_context, rect.cgRect(), icon->_m_cg_image);
CGContextClipToMask(m_cg_context, rect.cgRect(), icon->_m_cg_image);
setFillColor(color, alpha);
fillRect(rect);

CGContextRestoreGState(m_cg_context);
}
     */
}


void GraphicContext::drawIconInCircle(const Icon* icon, const Vec2d& center, double radius, const RGB& bg_color, const RGB& icon_color, const RGB& border_color, double border_width, float bg_alpha, float border_alpha, float icon_alpha) noexcept {

/* TODO: ...

Rectd rect(center.m_x - radius, center.m_y - radius, radius * 2, radius * 2);

if (bg_alpha > 0) {

setFillColor(bg_color, bg_alpha);
fillEllipse(rect);

if (border_width > 0) {
rect.inset(border_width * 0.5f);
setStrokeWidth(border_width);
setStrokeColor(border_color, border_alpha);
strokeEllipse(rect);
}
}

if (icon != nullptr) {

drawIcon(icon, icon->getCenteredRect(rect), icon_color, icon_alpha);
}

*/
    }


    void GraphicContext::drawIconInRoundRect(const Icon* icon, const Rectd& rect, double radius1, double radius2, double radius3, double radius4, const RGB& bg_color, const RGB& icon_color, const RGB& border_color, double border_width, float bg_alpha, float border_alpha, float icon_alpha) noexcept {
        Rectd roundRect = rect;
        if (bg_alpha > 0) {

            setFillRGBAndAlpha(bg_color, bg_alpha);
            fillRoundRect(roundRect, radius1, radius2, radius3, radius4);

            if (border_width > 0) {

                roundRect.inset(border_width * 0.5f);
                setStrokeWidth(border_width);
                setStrokeRGBAndAlpha(border_color, border_alpha);
                strokeRoundRect(roundRect, radius1, radius2, radius3, radius4);
            }
        }

        /* TODO !!!!!!
        if (icon != nullptr) {
            drawIcon(icon, icon->centeredRect(rect), icon_color, icon_alpha);
        }
         */
    }


    Rectd GraphicContext::textRect(const String& string, const Font* font) const noexcept {
        return textRect(string.utf8(), font);
    }

#if defined(__APPLE__) && defined(__MACH__)
    Rectd GraphicContext::textRect(const char* text, const Font* font) const noexcept {
        Rectd text_rect;
        text_rect.zero();

        if (text != nullptr && font != nullptr) {

            setTextMatrix(1.0, 0.0, 0.0, -1.0, 0.0, 0.0);

            CTFontRef ct_font = font->ctFont();
            CFDictionaryRef cf_str_attr = CFDictionaryCreate(
                    kCFAllocatorDefault,
                    (const void**)&kCTFontAttributeName,
                    (const void**)&ct_font,
                    1,
                    &kCFTypeDictionaryKeyCallBacks,
                    &kCFTypeDictionaryValueCallBacks);

            CFStringRef cf_str = CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8);
            CFAttributedStringRef cf_attr_str = CFAttributedStringCreate(kCFAllocatorDefault, cf_str, cf_str_attr);

            CTLineRef line = CTLineCreateWithAttributedString(cf_attr_str);

            CGRect line_bounds = CTLineGetImageBounds(line, m_cg_context);
            text_rect.set(0, 0, line_bounds.size.width, line_bounds.size.height);

            CFRelease(cf_str);
            CFRelease(cf_str_attr);
            CFRelease(cf_attr_str);
            CFRelease(line);
        }

        return text_rect;
    }
#else
    Rectd GraphicContext::textRect(const char* text, const Font* font) const noexcept {
        // TODO: Implement linux version
        return Rectd();
    }
#endif


    void GraphicContext::drawText(const String& string, const Vec2d& pos, const Font* font, const RGB& color, float alpha) const noexcept {
        drawText(string.utf8(), pos, font, color, alpha);
    }


#if defined(__APPLE__) && defined(__MACH__)
    void GraphicContext::drawText(const char* text, const Vec2d& pos, const Font* font, const RGB& color, float alpha) const noexcept {
        if (font != nullptr && text != nullptr) {
            setTextMatrix(1.0, 0.0, 0.0, -1.0, 0.0, 0.0);

            CGColorRef cg_text_color = color.createCGColor(alpha);

            CFStringRef keys[] = { kCTFontAttributeName, kCTForegroundColorAttributeName };
            CFTypeRef values[] = { font->ctFont(), cg_text_color };

            CFDictionaryRef cf_str_attr =
                    CFDictionaryCreate(kCFAllocatorDefault, (const void**)&keys,
                                       (const void**)&values, sizeof(keys) / sizeof(keys[0]),
                                       &kCFTypeDictionaryKeyCallBacks,
                                       &kCFTypeDictionaryValueCallBacks);


            CFStringRef cf_str = CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8);
            CFAttributedStringRef cf_attr_str = CFAttributedStringCreate(kCFAllocatorDefault, cf_str, cf_str_attr);


            CTLineRef line = CTLineCreateWithAttributedString(cf_attr_str);

            CGContextSetTextPosition(m_cg_context, pos.m_x, pos.m_y);
            CTLineDraw(line, m_cg_context);

            CFRelease(cf_str);
            CFRelease(cf_str_attr);
            CFRelease(cf_attr_str);
            CFRelease(line);
            CGColorRelease(cg_text_color);
        }
    }
#else
    void GraphicContext::drawText(const char* text, const Vec2d& pos, const Font* font, const RGB& color, float alpha) const noexcept {
        // TODO: Implement linux version
    }
#endif


    void GraphicContext::drawTextInt(int64_t value, const Vec2d& pos, const Font* font, const RGB& color, float alpha) const noexcept {
        char buffer[100];
        std::snprintf(buffer, 100, "%" PRId64, value);
        drawText(buffer, pos, font, color, alpha);
    }


    double GraphicContext::drawTextInRect(const String& string, const Rectd& rect, Alignment alignment, const Font* font, const RGB& color, float alpha) const noexcept {
        return drawTextInRect(string.utf8(), rect, alignment, font, color, alpha);
    }


#if defined(__APPLE__) && defined(__MACH__)
    double GraphicContext::drawTextInRect(const char* text, const Rectd& rect, Alignment alignment, const Font* font, const RGB& color, float alpha) const noexcept {
        if (text == nullptr) {
            return 0.0;
        }

        if (font == nullptr) {
            font = App::uiFont();
        }

        setTextMatrix(1.0, 0.0, 0.0, -1.0, 0.0, 0.0);

        CGColorRef cg_text_color = color.createCGColor(alpha);

        CFStringRef keys[] = { kCTFontAttributeName, kCTForegroundColorAttributeName };
        CFTypeRef values[] = { font->ctFont(), cg_text_color };

        CFDictionaryRef cf_str_attr =
                CFDictionaryCreate(kCFAllocatorDefault,
                                   (const void**)&keys,
                                   (const void**)&values,
                                   2, // sizeof(keys) / sizeof(keys[0]),
                                   &kCFTypeDictionaryKeyCallBacks,
                                   &kCFTypeDictionaryValueCallBacks);


        CFStringRef cf_str = CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8);
        CFAttributedStringRef cf_attr_str = CFAttributedStringCreate(kCFAllocatorDefault, cf_str, cf_str_attr);

        CTLineRef line = CTLineCreateWithAttributedString(cf_attr_str);

        CGFloat ascent, descent, leading;
        double text_width = CTLineGetTypographicBounds(line, &ascent, &descent, &leading);
        double text_x, text_y;

        // Horizontal alignment
        switch (alignment) {
            case Alignment::Center:
            case Alignment::Top:
            case Alignment::Bottom:
                text_x = rect.m_x + rect.m_width / 2 - text_width / 2;
                break;

            case Alignment::Right:
            case Alignment::TopRight:
            case Alignment::BottomRight:
                text_x = rect.m_x + rect.m_width - text_width;
                break;

            default:
                text_x = rect.m_x;
                break;
        }

        // Vertical alignment
        switch (alignment) {
            case Alignment::Center:
            case Alignment::Left:
            case Alignment::Right:
                if (m_flipped_y) {
                    text_y = rect.m_y + rect.m_height / 2 + (ascent - descent) / 2;
                }
                else {
                    text_y = rect.m_y + rect.m_height / 2 - (ascent - descent) / 2;
                }
                break;

            case Alignment::BottomLeft:
            case Alignment::Bottom:
            case Alignment::BottomRight:
                if (m_flipped_y) {
                    text_y = rect.m_y + rect.m_height - descent;
                }
                else {
                    text_y = rect.m_y + descent;
                }
                break;

            default:
                if (m_flipped_y) {
                    text_y = rect.m_y + ascent;
                }
                else {
                    text_y = rect.m_y + rect.m_height - ascent;
                }
                break;
        }

        CGContextSetTextPosition(m_cg_context, text_x, text_y);
        CTLineDraw(line, m_cg_context);

        CGRect line_bounds = CTLineGetImageBounds(line, m_cg_context);

        CFRelease(cf_str);
        CFRelease(cf_str_attr);
        CFRelease(cf_attr_str);
        CFRelease(line);
        CGColorRelease(cg_text_color);

        return line_bounds.size.width;
    }
#else
    double GraphicContext::drawTextInRect(const char* text, const Rectd& rect, Alignment alignment, const Font* font, const RGB& color, float alpha) const noexcept {
        // TODO: Implement linux version
        return 0.0;
    }
#endif


    double GraphicContext::drawTextIntInRect(int64_t value, const Rectd& rect, Alignment alignment, const Font* font, const RGB& color, float alpha) const noexcept {
        char buffer[100];
        std::snprintf(buffer, 100, "%" PRId64, value);
        return drawTextInRect(buffer, rect, alignment, font, color, alpha);
    }


#if defined(__APPLE__) && defined(__MACH__)
    double GraphicContext::drawTextLineByLine(const char* text, const Rectd& bounds_rect, const Rectd& rect, double line_gap, const Font* font, const RGB& color, float alpha) const noexcept {
        constexpr int32_t kMaxLines = 1024;
        // TODO: Refine!
        CGColorRef cg_text_color = color.createCGColor(1.0f);

        CFStringRef keys[] = { kCTFontAttributeName, kCTForegroundColorAttributeName };
        CFTypeRef values[] = { font->ctFont(), cg_text_color };

        CFDictionaryRef cf_str_attr =
                CFDictionaryCreate(kCFAllocatorDefault,
                                   (const void**)&keys,
                                   (const void**)&values,
                                   2, // sizeof(keys) / sizeof(keys[0]),
                                   &kCFTypeDictionaryKeyCallBacks,
                                   &kCFTypeDictionaryValueCallBacks);

        CFStringRef cf_str = CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8);
        CFAttributedStringRef cf_attr_str = CFAttributedStringCreate(kCFAllocatorDefault, cf_str, cf_str_attr);


        CGRect cg_rect = rect.cgRect();

        CTFramesetterRef framesetter = CTFramesetterCreateWithAttributedString(cf_attr_str);
        CGPathRef path = CGPathCreateWithRect(cg_rect, NULL);
        CTFrameRef frame = CTFramesetterCreateFrame(framesetter, CFRangeMake(0, 0), path, NULL);

        CFArrayRef lines = CTFrameGetLines(frame);
        CFIndex line_count = CFArrayGetCount(lines);
        CGSize suggestedSize{};

        if (line_count <= kMaxLines) { // TODO: Implement dynamic allocation if line_count gets to big
            suggestedSize = CTFramesetterSuggestFrameSizeWithConstraints(framesetter, CFRangeMake(0, 0), NULL, cg_rect.size, NULL);
            suggestedSize.height += (line_count - 1) * line_gap;

            if (rect.overlaps(bounds_rect)) {
                CGContextSetTextMatrix(m_cg_context, CGAffineTransformIdentity);
                CGContextTranslateCTM(m_cg_context, 0, CGRectGetHeight(cg_rect));
                CGContextScaleCTM(m_cg_context, 1.0, -1.0);

                CGPoint lineOrigins[kMaxLines];
                CTFrameGetLineOrigins(frame, CFRangeMake(0, 0), lineOrigins);

                for (CFIndex i = 0; i < line_count; i++) {
                    CTLineRef line = (CTLineRef)CFArrayGetValueAtIndex(lines, i);
                    CGPoint lineOrigin = lineOrigins[i];

                    CGContextSetTextPosition(m_cg_context, lineOrigin.x, lineOrigin.y - rect.m_y - i * line_gap);
                    CTLineDraw(line, m_cg_context);
                }
            }
        }

        CFRelease(cf_str);
        CFRelease(cf_str_attr);
        CFRelease(cf_attr_str);
        CGColorRelease(cg_text_color);

        CFRelease(frame);
        CFRelease(path);
        CFRelease(framesetter);

        return suggestedSize.height;
    }
#else
    double GraphicContext::drawTextLineByLine(const char* text, const Rectd& bounds_rect, const Rectd& rect, double line_gap, const Font* font, const RGB& color, float alpha) const noexcept {
        // TODO: Implement linux version
        return 0.0;
    }
#endif


    void GraphicContext::drawDebugText(const char* text, Vec2d& pos, int32_t spacing) noexcept {
        if (text != nullptr) {

            Rectd textRect = this->textRect(text, App::uiFont());
            textRect.m_x = pos.m_x;
            textRect.m_y = pos.m_y;
            textRect.m_width += 10;
            textRect.m_height = std::round(App::uiFont()->lineHeight() * 1.2f);

            setFillRGB(m_debug_bg_color);
            fillRect(textRect);

            textRect.insetLeft(5);
            drawTextInRect(text, textRect, Alignment::Left, App::uiFont(), m_debug_fg_color);

            pos.m_y += textRect.m_height + spacing;
        }
    }


    void GraphicContext::drawDebugBool(const char* label, bool value, Vec2d& pos, int32_t spacing) noexcept {
        if (label != nullptr) {
            drawDebugText(value ? "true" : "false", pos, spacing);
        }
    }


    void GraphicContext::drawDebugInt64(const char* label, int64_t value, Vec2d& pos, int32_t spacing) noexcept {
        if (label != nullptr) {
            char buffer[1024];
            std::snprintf(buffer, 1024, "%s: %" PRId64, label, value);
            drawDebugText(buffer, pos, spacing);
        }
    }


    void GraphicContext::drawDebugDouble(const char* label, double value, Vec2d& pos, int32_t spacing) noexcept {
        if (label != nullptr) {
            char buffer[1024];
            std::snprintf(buffer, 1024, "%s: %f", label, value);
            drawDebugText(buffer, pos, spacing);
        }
    }


    void GraphicContext::addTextPath(const char* text, const Font* font) const noexcept {
        /* TODO: !!!!!! Implement!
        if (text == nullptr || font == nullptr) {
            return;
        }

        NSString* ns_string = [NSString stringWithUTF8String:text];
        int32_t length = (int32_t)ns_string.length;
        if (length < 1) {
            return;
        }

        unichar chars[length];
        CGGlyph glyphs[length];
        CGPoint positions[length];
        CGSize advances[length];
//    Rectd textBounds = getTextBounds(text, font);

        CGRect text_rect = CGRectMake(0, 0, 1000, 1000);

        [ns_string getCharacters:chars range:NSMakeRange(0, length)];
        CTFontRef ct_font = font->ctFont();

        CTFontGetGlyphsForCharacters(ct_font, chars, glyphs, length);
        CTFontGetAdvancesForGlyphs(ct_font, kCTFontOrientationDefault, glyphs, advances, length);

        CGPoint position = CGPointMake(0, 0);
        for (int32_t i = 0; i < length; i++) {
            positions[i] = CGPointMake(position.x, position.y);
            CGSize advance = advances[i];
            position.x += advance.width;
            position.y += advance.height;
        }

        CGRect bounding_box = CTFontGetBoundingRectsForGlyphs(ct_font, kCTFontOrientationDefault, glyphs, NULL, length);

        CGContextTranslateCTM(m_cg_context, 0.0, text_rect.origin.y);
        CGContextTranslateCTM(m_cg_context, 0.0, bounding_box.size.height);
        CGContextScaleCTM(m_cg_context, 1.0, -1.0);
        CGContextTranslateCTM(m_cg_context, 0.0, -text_rect.origin.y);

        for (int32_t i = 0; i < length; i++) {
            CGPoint position = positions[i];
            CGAffineTransform tt = CGAffineTransformMakeTranslation(position.x, position.y);
            CGPathRef path = CTFontCreatePathForGlyph(ct_font, glyphs[i], &tt);
            CGContextAddPath(m_cg_context, path);
            CGPathRelease(path);
        }
         */
    }


    void GraphicContext::clipRect(const Rectd& rect) noexcept {
        addRectPath(rect);
        clipPath();
    }


    void GraphicContext::clipRoundRect(const Rectd& rect, double radius) noexcept {
        beginPath();
        addRoundRectPath(rect, radius);
        clipPath();
    }


    void GraphicContext::clipEllipse(const Rectd& rect) noexcept {
        addEllipsePath(rect);
        clipPath();
    }


    void GraphicContext::clipCircle(double x, double y, double radius) noexcept {
        addCirclePath(x, y, radius);
        clipPath();
    }


    void GraphicContext::clipCircle(const Vec2d& center, double radius) noexcept {
        addCirclePath(center, radius);
        clipPath();
    }


    Rectd GraphicContext::clipBoundsRect() noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        return Rectd(CGContextGetClipBoundingBox(m_cg_context));
#else
        return Rectd();
#endif
    }


    void GraphicContext::affineTransform(const Mat3d& matrix) noexcept {
#if defined(__APPLE__) && defined(__MACH__)
        CGAffineTransform m;
        const double* p = matrix.dataPtr();

        m.a = p[0];
        m.b = p[1];
        m.c = p[3];
        m.d = p[4];
        m.tx = p[6];
        m.ty = p[7];

        CGContextConcatCTM(m_cg_context, m);
#else
#endif
    }


    void GraphicContext::transformToFitRectProportionally(const Rectd& src_rect, const Rectd& dst_rect) noexcept {
        if (src_rect.m_width > std::numeric_limits<float>::epsilon() && src_rect.m_height > std::numeric_limits<float>::epsilon()) {
            double sx = dst_rect.m_width / src_rect.m_width;
            double sy = dst_rect.m_height / src_rect.m_height;
            double s = sx < sy ? sx : sy;

            translate(dst_rect.center());
            scale(s);
            translate(-src_rect.centerX(), -src_rect.centerY());
        }
    }


    void GraphicContext::drawHorizontalKeyboard(int32_t low_pitch, int32_t high_pitch, int32_t marked_pitch, double begin_freq, double end_freq, double min_x, double max_x, double y0, double y1, const RGB& light_color, const RGB& dark_color, const RGB& bg_color, const RGB& mark_color, float alpha) noexcept {
        low_pitch = std::clamp<int16_t>(low_pitch, 1, 255);
        high_pitch = std::clamp<int16_t>(high_pitch, low_pitch, 255);

        save();
        setFillGrayAndAlpha(0, 0.06f);
        setStrokeGrayAndAlpha(0, 0.06f);

        for (int32_t i = low_pitch; i <= high_pitch; i++) {

            double freq = Audio::freqFromPitch(i);
            double lowFreq = Audio::shiftetFreqByCent(freq, -50.0);
            double highFreq = Audio::shiftetFreqByCent(freq, 50.0);
            double x0 = Freq::freqToPos(lowFreq, begin_freq, end_freq, min_x, max_x);
            double x1 = Freq::freqToPos(highFreq, begin_freq, end_freq, min_x, max_x);
            double w = x1 - x0;

            if (i == marked_pitch) {
                setFillRGBAndAlpha(mark_color,  alpha + (1.0f - alpha) / 3);
            }
            else if (Audio::pitchIsBlackKey(i)) {
                setFillRGBAndAlpha(dark_color, alpha);
            }
            else {
                setFillRGBAndAlpha(light_color, alpha);
            }
            fillRect(x0, y0, w, y1 - y0);

            setFillRGB(bg_color);
            fillRect(x0 - w / 24, y0, w / 12, y1 - y0);
        }

        restore();
    }


} // End of namespace Grain
