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

    GraphicContext::GraphicContext() noexcept {
        _init();
    }


    GraphicContext::GraphicContext(Component* component) noexcept {
        _init();
    }


    GraphicContext::GraphicContext(PDFWriter* pdf_writer) noexcept {
        /* TODO: !!!!!
        if (pdf_writer) {

            setCGContext(pdf_writer->m_cg_context);

            m_cg_color_space = CGColorSpaceCreateDeviceRGB();
            // m_cg_color_space = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);  // TODO: An alternative?

            m_flipped_y = true;
            width_ = pdf_writer->m_media_box_pt.width();
            height_ = pdf_writer->m_media_box_pt.height();

            double s = Geometry::convertLength(1, pdf_writer->unit_, LengthUnit::Point);
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
    }

    void GraphicContext::addPolygon(Polygon* polygon) noexcept {
        if (polygon) {
            auto points = polygon->pointsListPtr();

            if (points) {
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
        if (path && path->pointCount() > 1) {
            GraphicPathPoint* prev_point = nullptr;
            // GraphicPathPoint* first_point = nullptr; // Unused

            for (int32_t point_index = 0; point_index < path->pointCount(); point_index++) {
                auto point = path->pointPtrAtIndex(point_index);

                if (point_index == 0) {
                    // first_point = point; // Unused
                    moveTo(point->anchor_);
                }
                else {
                    if (prev_point->right_flag_ && point->left_flag_) {
                        curveTo(prev_point->right_, point->left_, point->anchor_);
                    }
                    else if (prev_point->right_flag_) {
                        curveTo(prev_point->right_, point->anchor_, point->anchor_);
                    }
                    else if (point->left_flag_) {
                        curveTo(prev_point->anchor_, point->left_, point->anchor_);
                    }
                    else {
                        lineTo(point->anchor_);
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
        if (path) {
            double ts = 0.0;
            double te = 1.0;

            ts = split_param.t0_;
            for (int32_t i = split_param.start_index_; i <= split_param.end_index_; i++) {

                Bezier bezier;
                path->bezierAtIndex(i, bezier);

                if (i == split_param.start_index_) {
                    ts = split_param.t0_;
                }
                else {
                    ts = 0.0;
                }

                if (i == split_param.end_index_) {
                    te = split_param.t1_;
                }
                else {
                    te = 1.0;
                }

                if (bezier.truncate(ts, te, bezier)) {

                    if (i == split_param.start_index_) {
                        moveTo(bezier.pos_[0]);
                    }

                    curveTo(bezier.pos_[1], bezier.pos_[2], bezier.pos_[3]);
                }

                if (path->closed_) {
                    closePath();
                }
            }
        }
    }

    void GraphicContext::addRectPath(double x, double y, double width, double height) noexcept {
        moveTo(x, y);
        lineTo(x + width, y);
        lineTo(x + width, y + height);
        lineTo(x, y + height);
        closePath();
    }

    bool GraphicContext::addFramePath(
            const Rectd& rect,
            double top, double right, double bottom, double left) noexcept {

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

        double w = rect.width_;
        double h = rect.height_;
        double x0 = rect.x_;
        double x3 = rect.x2();
        double x1 = x0 + left;
        double x2 = x3 - right;

        double y0, y1, y2, y3;
        if (m_flipped_y) {
            y0 = rect.y_;
            y3 = rect.y2();
            y1 = y0 + top;
            y2 = y3 - bottom;
        }
        else {
            y0 = rect.y2();
            y3 = rect.y_;
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

    void GraphicContext::addRoundBarPath(double x, double y, double width, double height) noexcept {
        if (std::fabs(width - height) < std::numeric_limits<float>::epsilon()) {
            // Draw as circle
            addCirclePath(x + width / 2.0, y + height / 2.0, width / 2.0);
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

    void GraphicContext::addRoundRectPath(
            double x, double y,
            double width, double height,
            double radius) noexcept {

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

    void GraphicContext::addRoundRectPath(
            double x, double y, double width, double height,
            double radius1, double radius2, double radius3, double radius4) noexcept {

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

    void GraphicContext::addTrianglePath(const Triangled& triangle) noexcept {
        moveTo(triangle.points_[0]);
        lineTo(triangle.points_[1]);
        lineTo(triangle.points_[2]);
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
        addTrianglePath(rect.x_, rect.y_, rect.width_, rect.height_, direction);
    }

    void GraphicContext::addPolygonPath(int32_t point_count, const Vec2d* points) noexcept {
        if (points && point_count > 2) {
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
        moveTo(point1.x_, point1.z_);
        lineTo(point2.x_, point2.z_);
        strokePath();
    }

    void GraphicContext::strokeLineXY(const Vec3d& point1, const Vec3d& point2) noexcept {
        beginPath();
        moveTo(point1.x_, point1.y_);
        lineTo(point2.x_, point2.y_);
        strokePath();
    }

    void GraphicContext::strokeLine(const Lined& line) noexcept {
        beginPath();
        moveTo(line.p1_);
        lineTo(line.p2_);
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
        double a = std::fabs(end_point.x_ - start_point.x_) / 2.0;
        if (a < 8.0) {
            a = 8.0;
        }

        beginPath();
        moveTo(start_point);
        curveTo(start_point.x_ + a, start_point.y_, end_point.x_ - a, end_point.y_, end_point.x_, end_point.y_);
        strokePath();
    }

    /**
     *  @brief Stroke a Bezier curve.
     */
    void GraphicContext::strokeBezier(const Bezier& bezier) noexcept {
        beginPath();
        moveTo(bezier.pos_[0]);
        curveTo(bezier.pos_[1], bezier.pos_[2], bezier.pos_[3]);
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

    void GraphicContext::fillRect(const Rectd& rect) noexcept {
        fillRect(rect.x_, rect.y_, rect.width_, rect.height_);
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
        fillRoundRect(rect.x_, rect.y_, rect.width_, rect.height_, radius);
    }

    void GraphicContext::fillRoundRect(double x, double y, double width, double height, double radius1, double radius2, double radius3, double radius4) noexcept {
        beginPath();
        addRoundRectPath(x, y, width, height, radius1, radius2, radius3, radius4);
        fillPath();
    }

    void GraphicContext::fillRoundRect(const Rectd& rect, double radius1, double radius2, double radius3, double radius4) noexcept {
        fillRoundRect(rect.x_, rect.y_, rect.width_, rect.height_, radius1, radius2, radius3, radius4);
    }

    void GraphicContext::fillRoundBar(double x, double y, double width, double height) noexcept {
        beginPath();
        addRoundBarPath(x, y, width, height);
        fillPath();
    }

    void GraphicContext::fillRoundBar(const Rectd& rect) noexcept {
        fillRoundBar(rect.x_, rect.y_, rect.width_, rect.height_);
    }

    void GraphicContext::fillFrame(const Rectd& rect, double size) noexcept {
        beginPath();
        addRectPath(rect);
        addRectPath(rect.x_ + size, rect.y_ + size, rect.width_ - size * 2, rect.height_ - size * 2);
        fillPathEvenOdd();
    }

    void GraphicContext::fillFrame(const Rectd& rect, double width, double height) noexcept {
        beginPath();
        addRectPath(rect);
        addRectPath(rect.x_ + width, rect.y_ + height, rect.width_ - width * 2, rect.height_ - height * 2);
        fillPathEvenOdd();
    }

    void GraphicContext::fillFrame(const Rectd& rect, double top, double right, double bottom, double left) noexcept {
        beginPath();
        addFramePath(rect, top, right, bottom, left);
        fillPathEvenOdd();
    }

    void GraphicContext::strokeRoundBar(double x, double y, double width, double height) noexcept {
        beginPath();
        addRoundBarPath(x, y, width, height);
        strokePath();
    }

    void GraphicContext::strokeRoundBar(const Rectd& rect) noexcept {
        strokeRoundBar(rect.x_, rect.y_, rect.width_, rect.height_);
    }

    void GraphicContext::strokeRoundRect(double x, double y, double width, double height, double radius) noexcept {
        beginPath();
        addRoundRectPath(x, y, width, height, radius);
        strokePath();
    }

    void GraphicContext::strokeRoundRect(double x, double y, double width, double height, double radius1, double radius2, double radius3, double radius4) noexcept {
        beginPath();
        addRoundRectPath(x, y, width, height, radius1, radius2, radius3, radius4);
        strokePath();
    }

    void GraphicContext::strokeRoundRect(const Rectd& rect, double radius1, double radius2, double radius3, double radius4) noexcept {
        strokeRoundRect(rect.x_, rect.y_, rect.width_, rect.height_, radius1, radius2, radius3, radius4);
    }

    void GraphicContext::addQuadrilateralPath(const Vec2d* points) noexcept {
        if (points) {
            moveTo(points[0]);
            lineTo(points[1]);
            lineTo(points[2]);
            lineTo(points[3]);
            closePath();
        }
    }

    void GraphicContext::addQuadrilateralPath(const Quadrilateral& quadrilateral) noexcept {
        moveTo(quadrilateral.points_[0]);
        lineTo(quadrilateral.points_[1]);
        lineTo(quadrilateral.points_[2]);
        lineTo(quadrilateral.points_[3]);
        closePath();
    }

    void GraphicContext::fillQuadrilateral(const Vec2d* points) noexcept {
        if (points) {
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
        fillEllipse(rect.centerX(), rect.centerY(), rect.width_ / 2.0, rect.height_ / 2.0);
    }

    void GraphicContext::fillEllipse(const Vec2d& center, double rh, double rv) noexcept {
        fillEllipse(center.x_, center.y_, rh, rv);
    }

    void GraphicContext::strokeEllipse(const Rectd& rect) noexcept {
        strokeEllipse(rect.centerX(), rect.centerY(), rect.width_ / 2.0, rect.height_ / 2.0);
    }

    void GraphicContext::strokeEllipse(const Vec2d& center, double rh, double rv) noexcept {
        strokeEllipse(center.x_, center.y_, rh, rv);
    }

    void GraphicContext::fillCircle(const Circled& circle) noexcept {
        fillCircle(circle.center_, circle.radius_);
    }

    void GraphicContext::fillCircle(const Rectd& rect) noexcept {
        fillCircle(rect.centerX(), rect.centerY(), rect.shortSide() / 2);
    }

    void GraphicContext::fillCircle(const Rectd& rect, double min_radius, double max_radius) noexcept {
        double radius = std::clamp<double>(rect.shortSide() / 2.0, min_radius, max_radius);
        fillCircle(rect.centerX(), rect.centerY(), radius);
    }

    void GraphicContext::fillCircle(const Vec2d& center, double radius) noexcept {
        fillCircle(center.x_, center.y_, radius);
    }

    void GraphicContext::strokeCircle(const Rectd& rect) noexcept {
        strokeCircle(rect.centerX(), rect.centerY(), rect.shortSide() / 2);
    }

    void GraphicContext::strokeCircle(const Circled& circle) noexcept {
        strokeCircle(circle.center_, circle.radius_);
    }

    void GraphicContext::strokeCircle(const Rectd& rect, double min_radius, double max_radius) noexcept {
        double radius = std::clamp<double>(rect.shortSide() / 2.0, min_radius, max_radius);
        strokeCircle(rect.centerX(), rect.centerY(), radius);
    }

    void GraphicContext::strokeCircle(const Vec2d& center, double radius) noexcept {
        strokeCircle(center.x_, center.y_, radius);
    }

    void GraphicContext::fillRing(const Vec2d& center, double inner_radius, double outer_radius, double angle, double span) noexcept {
        beginPath();
        addRingPath(center, inner_radius, outer_radius, angle, span);
        fillPath();
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

        if (icon) {
            // drawIcon(icon, icon->centeredRect(rect), icon_color, icon_alpha); TODO: !!!!
        }
    }

    Rectd GraphicContext::textRect(const String& string, const Font* font) noexcept {
        return textRect(string.utf8(), font);
    }

    void GraphicContext::drawText(const String& string, const Vec2d& pos, const Font* font, const RGB& color, float alpha) noexcept {
        drawText(string.utf8(), pos, font, color, alpha);
    }

    void GraphicContext::drawTextInt(int64_t value, const Vec2d& pos, const Font* font, const RGB& color, float alpha) noexcept {
        char buffer[100];
        std::snprintf(buffer, 100, "%" PRId64, value);
        drawText(buffer, pos, font, color, alpha);
    }

    double GraphicContext::drawTextInRect(const String& string, const Rectd& rect, Alignment alignment, const Font* font, const RGB& color, float alpha) noexcept {
        return drawTextInRect(string.utf8(), rect, alignment, font, color, alpha);
    }


    double GraphicContext::drawTextIntInRect(int64_t value, const Rectd& rect, Alignment alignment, const Font* font, const RGB& color, float alpha) noexcept {
        char buffer[100];
        std::snprintf(buffer, 100, "%" PRId64, value);
        return drawTextInRect(buffer, rect, alignment, font, color, alpha);
    }

    void GraphicContext::drawDebugText(const char* text, Vec2d& pos, int32_t spacing) noexcept {
        if (text) {
            Rectd textRect = this->textRect(text, App::uiFont());
            textRect.x_ = pos.x_;
            textRect.y_ = pos.y_;
            textRect.width_ += 10;
            textRect.height_ = std::round(App::uiFont()->lineHeight() * 1.2f);

            setFillRGB(m_debug_bg_color);
            fillRect(textRect);

            textRect.insetLeft(5);
            drawTextInRect(text, textRect, Alignment::Left, App::uiFont(), m_debug_fg_color);

            pos.y_ += textRect.height_ + spacing;
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

    void GraphicContext::rotateAroundPivot(const Vec2d& pivot, double angle) noexcept {
        translate(pivot);
        rotate(angle);
        translate(-pivot);
    }

    void GraphicContext::transformToFitRectProportionally(const Rectd& src_rect, const Rectd& dst_rect) noexcept {
        if (src_rect.width_ > std::numeric_limits<float>::epsilon() && src_rect.height_ > std::numeric_limits<float>::epsilon()) {
            double sx = dst_rect.width_ / src_rect.width_;
            double sy = dst_rect.height_ / src_rect.height_;
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
