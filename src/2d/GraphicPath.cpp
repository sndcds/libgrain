//
//  GraphicPath.cpp
//
//  Created by Roald Christesen on from 26.01.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "2d/GraphicPath.hpp"
#include "2d/Superellipse.hpp"
#include "2d/Quadrilateral.hpp"
#include "2d/GraphicPathPoint.hpp"
#include "Bezier/Bezier.hpp"
#include "Graphic/GraphicContext.hpp"
#include "2d/RangeRect.hpp"


namespace Grain {

    GraphicPath::GraphicPath(int32_t point_capacity) noexcept {

        _init(point_capacity);
    }


    GraphicPath::~GraphicPath() noexcept {
    }

    void GraphicPath::_init(int32_t point_capacity) noexcept {

        m_points.reserve(std::max(point_capacity, 5));
        m_closed = false;
        m_must_update = true;
    }


    void GraphicPath::_update() noexcept {

        if (m_must_update) {
            _updateLength(m_bezier_segment_resolution);
            m_must_update = false;
        }
    }


    bool GraphicPath::hasPoints() const noexcept {

        return pointCount() > 0;
    }


    int32_t GraphicPath::pointCount() const noexcept {
        return static_cast<int32_t>(m_points.size());
    }


    int32_t GraphicPath::segmentCount() const noexcept {
        return m_closed ? static_cast<int32_t>(m_points.size()) : static_cast<int32_t>(m_points.size()) - 1;
    }


    double GraphicPath::length() noexcept {

        _update();
        return m_length;
    }


    double GraphicPath::polygonCentroid(Vec2d &out_centroid) const noexcept {

        double area = 0;
        Vec2d c;

        int32_t n = static_cast<int32_t>(m_points.size());
        for (int32_t i = 0; i < n; i++) {
            int j = (i + 1) % n;  // Wrap around to the first vertex
            Vec2d pi = m_points[i].m_anchor;
            Vec2d pj = m_points[j].m_anchor;
            double cross = pi.cross(pj);
            area += cross;
            c += (pi + pj) * cross;
        }

        area *= 0.5;
        out_centroid = c * (1.0 / (6.0 * area));
        return area;
    }


    Vec2d GraphicPath::simplePolygonCentroid() const noexcept {

        Vec2d c;

        for (auto& p : m_points) {
            c += p.anchor();
        }

        return c * (1.0 / m_points.size());
    }


    void GraphicPath::_updateLength(int32_t bezier_resolution) noexcept {

        // TODO: Test! Validate! Check!

        m_length = 0.0;

        if (!hasPoints()) {
            return;
        }

        Bezier bezier;
        const GraphicPathPoint *prev_point = nullptr;

        for (int32_t point_index = 0; point_index < pointCount(); point_index++) {

            auto point = m_points.mutElementPtrAtIndex(point_index);
            bezier.m_pos[0] = point->m_anchor;

            if (prev_point) {
                if (prev_point->m_right_flag && point->m_left_flag) {
                    bezier.m_pos[1] = prev_point->m_right;
                    bezier.m_pos[2] = point->m_left;
                    bezier.m_pos[3] = point->m_anchor;
                }
                else if (prev_point->m_right_flag) {
                    bezier.m_pos[1] = prev_point->m_right;
                    bezier.m_pos[2] = point->m_anchor;
                    bezier.m_pos[3] = point->m_anchor;
                }
                else if (point->m_left_flag) {
                    bezier.m_pos[1] = prev_point->m_anchor;
                    bezier.m_pos[2] = point->m_left;
                    bezier.m_pos[3] = point->m_anchor;
                }
                else {
                    bezier.m_pos[1] = prev_point->m_anchor;
                    bezier.m_pos[2] = point->m_anchor;
                    bezier.m_pos[3] = point->m_anchor;
                }

                point->m_bezier_segment_length = bezier.approximatedCurveLength(bezier_resolution);
                m_length += point->m_bezier_segment_length;
            }

            prev_point = point;
        }
    }


    GraphicPathPoint *GraphicPath::pointPtrAtIndex(int32_t index) noexcept {

        return m_points.mutElementPtrAtIndex(index);
    }


    GraphicPathPoint *GraphicPath::lastPointPtr() noexcept {

        return m_points.mutLastElementPtr();
    }


    Rectd GraphicPath::bounds() noexcept {

        Rectd bounds;
        this->bounds(bounds);
        return bounds;
    }


    bool GraphicPath::bounds(Rectd &out_bounds) noexcept {

        if (pointCount() < 1) {
            out_bounds.zero();
            return false;
        }


        RangeRectd range_rect;

        int32_t n = segmentCount();
        for (int32_t i = 0; i < n; i++) {
            Bezier bezier;
            if (bezierAtIndex(i, bezier)) {
                if (i == 0) {
                    range_rect = bezier.bounds();
                }
                else {
                    range_rect += bezier.bounds();
                }
            }
        }

        out_bounds = range_rect.rect();

        return true;
    }


    bool GraphicPath::bezierAtIndex(int32_t segment_index, Bezier &out_bezier) noexcept {

        int32_t lastIndex = lastPointIndex();
        if (segment_index >= 0 && segment_index < lastIndex) {
            return bezierFromTwoPathPoints(pointPtrAtIndex(segment_index), pointPtrAtIndex(segment_index + 1), out_bezier);
        }
        else if (isClosed() && segment_index == lastIndex) {
            return bezierFromTwoPathPoints(pointPtrAtIndex(lastIndex), pointPtrAtIndex(0), out_bezier);
        }

        return false;
    }


    bool GraphicPath::bezierFromTwoPathPoints(const GraphicPathPoint *p1, const GraphicPathPoint *p2, Bezier &out_bezier) noexcept {

        if (p1 != nullptr && p2 != nullptr) {

            out_bezier.m_pos[0] = p1->m_anchor;

            if (p1->m_right_flag) {
                out_bezier.m_pos[1] = p1->m_right;
            }
            else {
                out_bezier.m_pos[1] = p1->m_anchor;
            }

            if (p2->m_left_flag) {
                out_bezier.m_pos[2] = p2->m_left;
            }
            else {
                out_bezier.m_pos[2] = p2->m_anchor;
            }
            out_bezier.m_pos[3] = p2->m_anchor;

            return true;
        }

        return false;
    }


    void GraphicPath::clear() noexcept {

        m_points.clear();
        m_must_update = true;
    }


    void GraphicPath::addPoint(double x, double y, bool use_left, double lx, double ly, bool use_right, double rx, double ry) noexcept {

        GraphicPathPoint point;

        point.m_anchor.set(x, y);
        point.m_left.set(lx, ly);
        point.m_right.set(rx, ry);
        point.m_left_flag = use_left;
        point.m_right_flag = use_right;

        m_points.push(point);
    }


    void GraphicPath::addPoint(const GraphicPathPoint *point) noexcept {

        if (point != nullptr) {
            addPoint(point->m_anchor.m_x, point->m_anchor.m_y,
                     point->m_left_flag, point->m_left.m_x, point->m_left.m_y,
                     point->m_right_flag, point->m_right.m_x, point->m_right.m_y);
        }
    }


    void GraphicPath::addPoint(const Vec2d &pos) noexcept {

        addPoint(pos.m_x, pos.m_y, false, 0.0, 0.0, false, 0.0, 0.0);
    }


    void GraphicPath::addPoint(double x, double y) noexcept {

        addPoint(x, y, false, 0.0, 0.0, false, 0.0, 0.0);
    }


    void GraphicPath::addPoint(const Vec2d &pos, const Vec2d &left, const Vec2d &right) noexcept {

        addPoint(pos.m_x, pos.m_y, true, left.m_x, left.m_y, true, right.m_x, right.m_y);
    }


    void GraphicPath::addPoint(double x, double y, double lx, double ly, double rx, double ry) noexcept {

        addPoint(x, y, true, lx, ly, true, rx, ry);
    }


    void GraphicPath::addPointLeft(const Vec2d &pos, const Vec2d &left) noexcept {

        addPoint(pos.m_x, pos.m_y, true, left.m_x, left.m_y, false, 0.0, 0.0);
    }


    void GraphicPath::addPointLeft(double x, double y, double lx, double ly) noexcept {

        addPoint(x, y, true, lx, ly, false, 0.0, 0.0);
    }


    void GraphicPath::addPointRight(const Vec2d &pos, const Vec2d &right) noexcept {

        addPoint(pos.m_x, pos.m_y, false, 0.0, 0.0, true, right.m_x, right.m_y);
    }


    void GraphicPath::addPointRight(double x, double y, double rx, double ry) noexcept {

        addPoint(x, y, false, 0.0, 0.0, true, rx, ry);
    }


    void GraphicPath::addPointByAngle(const Vec2d &pos, double angle, double left_length, double right_length) noexcept {

        Vec2d lv(-left_length, 0.0);
        lv.rotate(angle);

        Vec2d rv(right_length, 0.0);
        rv.rotate(angle);

        addPoint(pos.m_x, pos.m_y, pos.m_x + lv.m_x, pos.m_y + lv.m_y, pos.m_x + rv.m_x, pos.m_y + rv.m_y);
    }


    void GraphicPath::addPointByAngle(double x, double y, double angle, double left_length, double right_length) noexcept {

        Vec2d lv(-left_length, 0.0);
        lv.rotate(angle);

        Vec2d rv(right_length, 0.0);
        rv.rotate(angle);

        addPoint(x, y, x + lv.m_x, y + lv.m_y, x + rv.m_x, y + rv.m_y);
    }


    void GraphicPath::addPointByAngle(const Vec2d &pos, double left_angle, double left_length, double right_angle, double right_length) noexcept {

        Vec2d lv(-left_length, 0.0);
        lv.rotate(left_angle);

        Vec2d rv(right_length, 0.0);
        rv.rotate(right_angle);

        addPoint(pos.m_x, pos.m_y, pos.m_x + lv.m_x, pos.m_y + lv.m_y, pos.m_y + rv.m_x, pos.m_y + rv.m_y);
    }


    void GraphicPath::addPointByAngle(double x, double y, double left_angle, double left_length, double right_angle, double right_length) noexcept {

        Vec2d lv(-left_length, 0.0);
        lv.rotate(left_angle);

        Vec2d rv(right_length, 0.0);
        rv.rotate(right_angle);

        addPoint(x, y, x + lv.m_x, y + lv.m_y, x + rv.m_x, y + rv.m_y);
    }


    void GraphicPath::addArcAsBezier(
            const Vec2d &radii,
            double x_axis_rotation,
            bool large_arc_flag,
            bool sweep_flag,
            const Vec2d &end_pos,
            int32_t max_segment_n)
            noexcept {

        constexpr int32_t kMaxSegment_n = 16;
        int32_t point_n = pointCount();

        if (point_n > 0) { // Needs a previous point
            if (max_segment_n > 16) {
                max_segment_n = 16;
            }

            Vec2d start_pos = m_points[point_n - 1].m_anchor;
            Vec2d bezier_pos_array[kMaxSegment_n * 3 + 1];

            int32_t segment_n =
                    Bezier::arcToBezierPosArray(
                            start_pos,
                            radii,
                            x_axis_rotation,
                            large_arc_flag,
                            sweep_flag,
                            end_pos,
                            max_segment_n,
                            bezier_pos_array);

            if (segment_n > 0) {
                m_points[point_n - 1].m_right = bezier_pos_array[1];
                m_points[point_n - 1].m_right_flag = true;
                int32_t pos_index = 3;

                for (int32_t i = 0; i < segment_n - 1; i++) {
                    addPoint(bezier_pos_array[pos_index], bezier_pos_array[pos_index - 1], bezier_pos_array[pos_index + 1]);
                    pos_index += 3;
                }

                addPointLeft(bezier_pos_array[pos_index], bezier_pos_array[pos_index - 1]);
            }
        }
    }


    void GraphicPath::addBezier(const Vec2d &control1_pos, const Vec2d &control2_pos, const Vec2d &end_pos) noexcept {

        // TODO: Check, Verify, Test!

        int32_t point_n = pointCount();
        if (point_n > 0) {
            setLastRight(control1_pos);
            addPointLeft(end_pos, control2_pos);
        }
    }


    void GraphicPath::addQuadraticBezier(const Vec2d &control_pos, const Vec2d &end_pos) noexcept {

        // TODO: Check, Verify, Test!

        int32_t point_n = pointCount();
        if (point_n > 0) {
            Bezier bezier(m_points[point_n - 1].m_anchor, control_pos, end_pos);
            addBezier(bezier.m_pos[1], bezier.m_pos[2], bezier.m_pos[3]);
        }
    }


    void GraphicPath::addSmoothBezier(const Vec2d &control2_pos, const Vec2d &end_pos) noexcept {

        // TODO: Check, Verify, Test!

        int32_t point_n = pointCount();
        if (point_n > 0) {
            auto prev_point = &m_points[point_n - 1];
            if (prev_point->m_left_flag) {
                Vec2d control1_pos = prev_point->m_left.reflectedPoint(prev_point->m_anchor);
                addBezier(control1_pos, control2_pos, end_pos);
            }
            else {
                addBezier(prev_point->m_anchor, control2_pos, end_pos);
            }
        }
    }


    void GraphicPath::addSmoothQuadraticBezier(const Vec2d &end_pos) noexcept {

        // TODO: Check, Verify, Test!

        int32_t point_n = pointCount();
        if (point_n > 0) {
            auto prev_point = &m_points[point_n - 1];
            if (prev_point->m_left_flag) {
                Bezier prev_bezier;
                if (bezierAtIndex(point_n - 2, prev_bezier)) {
                    Vec2d prev_quadratic_control;
                    prev_bezier.approximateQuadraticBezierControlPos(prev_quadratic_control);
                    Vec2d control_pos = prev_quadratic_control.reflectedPoint(prev_point->m_anchor);
                    addQuadraticBezier(control_pos, end_pos);
                }
            }
            else {
                Vec2d control_pos = prev_point->m_anchor + (end_pos - prev_point->m_anchor) * 0.5;
                addQuadraticBezier(control_pos, end_pos);
            }
        }
    }


    void GraphicPath::setLastLeft(const Vec2d &left) noexcept {

        int32_t point_n = pointCount();
        if (point_n > 0) {
            m_points[point_n - 1].m_left = left;
            m_points[point_n - 1].m_left_flag = true;
        }
    }


    void GraphicPath::setLastRight(const Vec2d &right) noexcept {

        int32_t point_n = pointCount();
        if (point_n > 0) {
            m_points[point_n - 1].m_right = right;
            m_points[point_n - 1].m_right_flag = true;
        }
    }


    void GraphicPath::translatePoint(int32_t index, double tx, double ty) noexcept {

        auto point = (GraphicPathPoint*)pointPtrAtIndex(index);
        if (point != nullptr) {
            point->translate(tx, ty);
        }
    }


    void GraphicPath::rotatePoint(int32_t index, double angle) noexcept {

        auto point = (GraphicPathPoint*)pointPtrAtIndex(index);
        if (point != nullptr) {
            point->rotate(angle);
        }
    }


    void GraphicPath::projectToQuadrilateral(const Quadrilateral &quadrilateral, const Mat3d *matrix) noexcept {

        for (auto& point : m_points) {
            point.projectToQuadrilateral(quadrilateral, matrix);
        }
    }


    void GraphicPath::fill(GraphicContext &gc) const noexcept {

        if (pointCount() >= 3) {
            gc.addPath((GraphicPath*)this);
            gc.fillPath();
        }
    }


    void GraphicPath::stroke(GraphicContext &gc) const noexcept {

        if (pointCount() >= 2) {
            gc.addPath((GraphicPath*)this);
            gc.strokePath();
        }
    }


    void GraphicPath::addClip(GraphicContext &gc) const noexcept  {

        if (pointCount() >= 3) {
            gc.addPath((GraphicPath*)this);
            gc.clipPath();
        }
    }


    void GraphicPath::split(double start, double end, GraphicPathSplitParam &out_split_param) {
        m_must_update = true;    // TODO: !!!!
        _update();

        start = std::clamp<double>(start, 0.0, 1.0);
        end = std::clamp<double>(end, start, 1.0);
        if ((end - start) < std::numeric_limits<float>::epsilon()) {
            out_split_param.m_valid = false;
            return;
        }

        out_split_param.m_start = start;
        out_split_param.m_end = end;

        int32_t n  = pointCount();
        double a = start * m_length;
        double b = end * m_length;

        double pos1 = 0.0;
        double pos2 = 0.0;

        int32_t si1 = -1;
        int32_t si2 = -1;

        for (int32_t point_index = 0; point_index < m_points.size(); point_index++) {
            auto point = m_points.elementPtrAtIndex(point_index);

            if (point_index > 0) {
                pos2 += point->m_bezier_segment_length;

                if (si1 < 0) {
                    if ((a >= pos1 && a <= pos2) || a < 0.0) {
                        si1 = point_index - 1;
                        Bezier bezier;
                        if (bezierAtIndex(si1, bezier)) {
                            out_split_param.m_start_index = si1;
                            double l = (pos2 - pos1);
                            out_split_param.m_t0 = l > 0.0 ? (a - pos1) / l : 0.0;
                        }
                    }
                }

                if (si2 < 0) {
                    if ((b >= pos1 && b <= pos2) || b > m_length) {
                        si2 = point_index - 1;
                        Bezier bezier;
                        if (bezierAtIndex(si2, bezier)) {
                            out_split_param.m_end_index = si2;
                            double l = (pos2 - pos1);
                            out_split_param.m_t1 = l > 0.0 ? (b - pos1) / l : 0.0;
                        }
                    }
                }

                pos1 = pos2;
            }
        }

        if (si1 < 0) {
            out_split_param.m_start_index = 0;
            out_split_param.m_t0 = 0.0;

        }

        if (si2 < 0) {
            out_split_param.m_end_index = n - 1;
            out_split_param.m_t1 = 1.0;
        }

        out_split_param.m_start_index = si1;
        out_split_param.m_end_index = si2;

        out_split_param.m_valid = true;
    }


} // End of namespace Grain.
