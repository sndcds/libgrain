//
//  GraphicPath.cpp
//
//  Created by Roald Christesen on from 26.01.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "2d/GraphicPath.hpp"
#include "2d/Quadrilateral.hpp"
#include "2d/GraphicPathPoint.hpp"
#include "2d/RangeRect.hpp"
#include "Bezier/Bezier.hpp"
#include "Graphic/GraphicContext.hpp"


namespace Grain {


GraphicPath::GraphicPath(int32_t point_capacity) noexcept {
    _init(point_capacity);
}


GraphicPath::~GraphicPath() noexcept {
}


void GraphicPath::_init(int32_t point_capacity) noexcept {
    points_.reserve(std::max(point_capacity, 5));
    closed_ = false;
    must_update_ = true;
}


void GraphicPath::_update() noexcept {
    if (must_update_) {
        _updateLength(bezier_segment_resolution_);
        must_update_ = false;
    }
}


bool GraphicPath::hasPoints() const noexcept {
    return pointCount() > 0;
}


int32_t GraphicPath::pointCount() const noexcept {
    return static_cast<int32_t>(points_.size());
}


int32_t GraphicPath::segmentCount() const noexcept {
    return closed_ ? static_cast<int32_t>(points_.size()) : static_cast<int32_t>(points_.size()) - 1;
}


double GraphicPath::length() noexcept {
    _update();
    return length_;
}


double GraphicPath::polygonCentroid(Vec2d& out_centroid) const noexcept {
    double area = 0;
    Vec2d c;

    auto n = static_cast<int32_t>(points_.size());
    for (int32_t i = 0; i < n; i++) {
        int j = (i + 1) % n;  // Wrap around to the first vertex
        Vec2d pi = points_[i].anchor_;
        Vec2d pj = points_[j].anchor_;
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

    for (auto& p : points_) {
        c += p.anchor();
    }

    return c * (1.0 / static_cast<double>(points_.size()));
}


void GraphicPath::_updateLength(int32_t bezier_resolution) noexcept {
    // TODO: Test! Validate! Check!

    length_ = 0.0;

    if (!hasPoints()) {
        return;
    }

    Bezier bezier;
    const GraphicPathPoint *prev_point = nullptr;

    for (int32_t point_index = 0; point_index < pointCount(); point_index++) {
        auto point = points_.mutElementPtrAtIndex(point_index);
        bezier.pos_[0] = point->anchor_;

        if (prev_point) {
            if (prev_point->right_flag_ && point->left_flag_) {
                bezier.pos_[1] = prev_point->right_;
                bezier.pos_[2] = point->left_;
                bezier.pos_[3] = point->anchor_;
            }
            else if (prev_point->right_flag_) {
                bezier.pos_[1] = prev_point->right_;
                bezier.pos_[2] = point->anchor_;
                bezier.pos_[3] = point->anchor_;
            }
            else if (point->left_flag_) {
                bezier.pos_[1] = prev_point->anchor_;
                bezier.pos_[2] = point->left_;
                bezier.pos_[3] = point->anchor_;
            }
            else {
                bezier.pos_[1] = prev_point->anchor_;
                bezier.pos_[2] = point->anchor_;
                bezier.pos_[3] = point->anchor_;
            }

            point->bezier_segment_length_ = bezier.approximatedCurveLength(bezier_resolution);
            length_ += point->bezier_segment_length_;
        }

        prev_point = point;
    }
}


GraphicPathPoint* GraphicPath::pointPtrAtIndex(int32_t index) noexcept {
    return points_.mutElementPtrAtIndex(index);
}


GraphicPathPoint* GraphicPath::lastPointPtr() noexcept {
    return points_.mutLastElementPtr();
}


Rectd GraphicPath::bounds() noexcept {
    Rectd bounds;
    this->bounds(bounds);
    return bounds;
}


bool GraphicPath::bounds(Rectd& out_bounds) noexcept {
    if (pointCount() < 1) {
        out_bounds.zero();
        return false;
    }

    RangeRectd range_rect;
    range_rect.initForMinMaxSearch();

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


bool GraphicPath::bezierAtIndex(int32_t segment_index, Bezier& out_bezier) noexcept {
    int32_t lastIndex = lastPointIndex();
    if (segment_index >= 0 && segment_index < lastIndex) {
        return bezierFromTwoPathPoints(pointPtrAtIndex(segment_index), pointPtrAtIndex(segment_index + 1), out_bezier);
    }
    else if (isClosed() && segment_index == lastIndex) {
        return bezierFromTwoPathPoints(pointPtrAtIndex(lastIndex), pointPtrAtIndex(0), out_bezier);
    }

    return false;
}


bool GraphicPath::bezierFromTwoPathPoints(const GraphicPathPoint* p1, const GraphicPathPoint* p2, Bezier& out_bezier) noexcept {
    if (p1 && p2) {
        out_bezier.pos_[0] = p1->anchor_;

        if (p1->right_flag_) {
            out_bezier.pos_[1] = p1->right_;
        }
        else {
            out_bezier.pos_[1] = p1->anchor_;
        }

        if (p2->left_flag_) {
            out_bezier.pos_[2] = p2->left_;
        }
        else {
            out_bezier.pos_[2] = p2->anchor_;
        }
        out_bezier.pos_[3] = p2->anchor_;

        return true;
    }

    return false;
}


void GraphicPath::clear() noexcept {
    points_.clear();
    must_update_ = true;
}


void GraphicPath::addPoint(double x, double y, bool use_left, double lx, double ly, bool use_right, double rx, double ry) noexcept {
    GraphicPathPoint point;

    point.anchor_.set(x, y);
    point.left_.set(lx, ly);
    point.right_.set(rx, ry);
    point.left_flag_ = use_left;
    point.right_flag_ = use_right;

    points_.push(point);
}


void GraphicPath::addPoint(const GraphicPathPoint* point) noexcept {
    if (point) {
        addPoint(point->anchor_.x_, point->anchor_.y_,
                 point->left_flag_, point->left_.x_, point->left_.y_,
                 point->right_flag_, point->right_.x_, point->right_.y_);
    }
}


void GraphicPath::addPoint(const Vec2d& pos) noexcept {
    addPoint(pos.x_, pos.y_, false, 0.0, 0.0, false, 0.0, 0.0);
}


void GraphicPath::addPoint(double x, double y) noexcept {
    addPoint(x, y, false, 0.0, 0.0, false, 0.0, 0.0);
}


void GraphicPath::addPoint(const Vec2d& pos, const Vec2d& left, const Vec2d& right) noexcept {
    addPoint(pos.x_, pos.y_, true, left.x_, left.y_, true, right.x_, right.y_);
}


void GraphicPath::addPoint(double x, double y, double lx, double ly, double rx, double ry) noexcept {
    addPoint(x, y, true, lx, ly, true, rx, ry);
}


void GraphicPath::addPointLeft(const Vec2d& pos, const Vec2d& left) noexcept {
    addPoint(pos.x_, pos.y_, true, left.x_, left.y_, false, 0.0, 0.0);
}


void GraphicPath::addPointLeft(double x, double y, double lx, double ly) noexcept {
    addPoint(x, y, true, lx, ly, false, 0.0, 0.0);
}


void GraphicPath::addPointRight(const Vec2d& pos, const Vec2d& right) noexcept {
    addPoint(pos.x_, pos.y_, false, 0.0, 0.0, true, right.x_, right.y_);
}


void GraphicPath::addPointRight(double x, double y, double rx, double ry) noexcept {
    addPoint(x, y, false, 0.0, 0.0, true, rx, ry);
}


void GraphicPath::addPointByAngle(const Vec2d& pos, double angle, double left_length, double right_length) noexcept {
    Vec2d lv(-left_length, 0.0);
    lv.rotate(angle);
    Vec2d rv(right_length, 0.0);
    rv.rotate(angle);
    addPoint(pos.x_, pos.y_, pos.x_ + lv.x_, pos.y_ + lv.y_, pos.x_ + rv.x_, pos.y_ + rv.y_);
}


void GraphicPath::addPointByAngle(double x, double y, double angle, double left_length, double right_length) noexcept {
    Vec2d lv(-left_length, 0.0);
    lv.rotate(angle);
    Vec2d rv(right_length, 0.0);
    rv.rotate(angle);
    addPoint(x, y, x + lv.x_, y + lv.y_, x + rv.x_, y + rv.y_);
}


void GraphicPath::addPointByAngle(const Vec2d& pos, double left_angle, double left_length, double right_angle, double right_length) noexcept {
    Vec2d lv(-left_length, 0.0);
    lv.rotate(left_angle);
    Vec2d rv(right_length, 0.0);
    rv.rotate(right_angle);
    addPoint(pos.x_, pos.y_, pos.x_ + lv.x_, pos.y_ + lv.y_, pos.y_ + rv.x_, pos.y_ + rv.y_);
}


void GraphicPath::addPointByAngle(double x, double y, double left_angle, double left_length, double right_angle, double right_length) noexcept {
    Vec2d lv(-left_length, 0.0);
    lv.rotate(left_angle);
    Vec2d rv(right_length, 0.0);
    rv.rotate(right_angle);
    addPoint(x, y, x + lv.x_, y + lv.y_, x + rv.x_, y + rv.y_);
}


void GraphicPath::addArcAsBezier(
        const Vec2d& radii,
        double x_axis_rotation,
        bool large_arc_flag,
        bool sweep_flag,
        const Vec2d& end_pos,
        int32_t max_segment_n
) noexcept
{
    constexpr int32_t kMaxSegment_n = 16;
    int32_t point_n = pointCount();

    if (point_n > 0) { // Needs a previous point
        if (max_segment_n > 16) {
            max_segment_n = 16;
        }

        Vec2d start_pos = points_[point_n - 1].anchor_;
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
            points_[point_n - 1].right_ = bezier_pos_array[1];
            points_[point_n - 1].right_flag_ = true;
            int32_t pos_index = 3;

            for (int32_t i = 0; i < segment_n - 1; i++) {
                addPoint(bezier_pos_array[pos_index], bezier_pos_array[pos_index - 1], bezier_pos_array[pos_index + 1]);
                pos_index += 3;
            }

            addPointLeft(bezier_pos_array[pos_index], bezier_pos_array[pos_index - 1]);
        }
    }
}


void GraphicPath::addBezier(const Vec2d& control1_pos, const Vec2d& control2_pos, const Vec2d& end_pos) noexcept {
    // TODO: Check, Verify, Test!

    int32_t point_n = pointCount();
    if (point_n > 0) {
        setLastRight(control1_pos);
        addPointLeft(end_pos, control2_pos);
    }
}


void GraphicPath::addQuadraticBezier(const Vec2d& control_pos, const Vec2d& end_pos) noexcept {
    // TODO: Check, Verify, Test!

    int32_t point_n = pointCount();
    if (point_n > 0) {
        Bezier bezier(points_[point_n - 1].anchor_, control_pos, end_pos);
        addBezier(bezier.pos_[1], bezier.pos_[2], bezier.pos_[3]);
    }
}


void GraphicPath::addSmoothBezier(const Vec2d& control2_pos, const Vec2d& end_pos) noexcept {
    // TODO: Check, Verify, Test!

    int32_t point_n = pointCount();
    if (point_n > 0) {
        auto prev_point = &points_[point_n - 1];
        if (prev_point->left_flag_) {
            Vec2d control1_pos = prev_point->left_.reflectedPoint(prev_point->anchor_);
            addBezier(control1_pos, control2_pos, end_pos);
        }
        else {
            addBezier(prev_point->anchor_, control2_pos, end_pos);
        }
    }
}


void GraphicPath::addSmoothQuadraticBezier(const Vec2d& end_pos) noexcept {
    // TODO: Check, Verify, Test!

    int32_t point_n = pointCount();
    if (point_n > 0) {
        auto prev_point = &points_[point_n - 1];
        if (prev_point->left_flag_) {
            Bezier prev_bezier;
            if (bezierAtIndex(point_n - 2, prev_bezier)) {
                Vec2d prev_quadratic_control;
                prev_bezier.approximateQuadraticBezierControlPos(prev_quadratic_control);
                Vec2d control_pos = prev_quadratic_control.reflectedPoint(prev_point->anchor_);
                addQuadraticBezier(control_pos, end_pos);
            }
        }
        else {
            Vec2d control_pos = prev_point->anchor_ + (end_pos - prev_point->anchor_) * 0.5;
            addQuadraticBezier(control_pos, end_pos);
        }
    }
}


void GraphicPath::setLastLeft(const Vec2d& left) noexcept {
    int32_t point_n = pointCount();
    if (point_n > 0) {
        points_[point_n - 1].left_ = left;
        points_[point_n - 1].left_flag_ = true;
    }
}


void GraphicPath::setLastRight(const Vec2d& right) noexcept {
    int32_t point_n = pointCount();
    if (point_n > 0) {
        points_[point_n - 1].right_ = right;
        points_[point_n - 1].right_flag_ = true;
    }
}


void GraphicPath::translatePoint(int32_t index, double tx, double ty) noexcept {
    auto point = (GraphicPathPoint*)pointPtrAtIndex(index);
    if (point) {
        point->translate(tx, ty);
    }
}


void GraphicPath::rotatePoint(int32_t index, double angle) noexcept {
    auto point = (GraphicPathPoint*)pointPtrAtIndex(index);
    if (point) {
        point->rotate(angle);
    }
}


void GraphicPath::projectToQuadrilateral(const Quadrilateral& quadrilateral, const Mat3d* matrix) noexcept {
    for (auto& point : points_) {
        point.projectToQuadrilateral(quadrilateral, matrix);
    }
}


void GraphicPath::fill(GraphicContext& gc) const noexcept {
    if (pointCount() >= 3) {
        gc.addPath((GraphicPath*)this);
        gc.fillPath();
    }
}


void GraphicPath::stroke(GraphicContext& gc) const noexcept {
    if (pointCount() >= 2) {
        gc.addPath((GraphicPath*)this);
        gc.strokePath();
    }
}


void GraphicPath::addClip(GraphicContext& gc) const noexcept  {
    if (pointCount() >= 3) {
        gc.addPath((GraphicPath*)this);
        gc.clipPath();
    }
}


void GraphicPath::split(double start, double end, GraphicPathSplitParam& out_split_param) {
    must_update_ = true;    // TODO: !!!!
    _update();

    start = std::clamp<double>(start, 0.0, 1.0);
    end = std::clamp<double>(end, start, 1.0);
    if ((end - start) < std::numeric_limits<float>::epsilon()) {
        out_split_param.valid_ = false;
        return;
    }

    out_split_param.start_ = start;
    out_split_param.end_ = end;

    int32_t n  = pointCount();
    double a = start * length_;
    double b = end * length_;

    double pos1 = 0.0;
    double pos2 = 0.0;

    int32_t si1 = -1;
    int32_t si2 = -1;

    for (int32_t point_index = 0; point_index < points_.size(); point_index++) {
        auto point = points_.elementPtrAtIndex(point_index);

        if (point_index > 0) {
            pos2 += point->bezier_segment_length_;

            if (si1 < 0) {
                if ((a >= pos1 && a <= pos2) || a < 0.0) {
                    si1 = point_index - 1;
                    Bezier bezier;
                    if (bezierAtIndex(si1, bezier)) {
                        out_split_param.start_index_ = si1;
                        double l = (pos2 - pos1);
                        out_split_param.t0_ = l > 0.0 ? (a - pos1) / l : 0.0;
                    }
                }
            }

            if (si2 < 0) {
                if ((b >= pos1 && b <= pos2) || b > length_) {
                    si2 = point_index - 1;
                    Bezier bezier;
                    if (bezierAtIndex(si2, bezier)) {
                        out_split_param.end_index_ = si2;
                        double l = (pos2 - pos1);
                        out_split_param.t1_ = l > 0.0 ? (b - pos1) / l : 0.0;
                    }
                }
            }

            pos1 = pos2;
        }
    }

    if (si1 < 0) {
        out_split_param.start_index_ = 0;
        out_split_param.t0_ = 0.0;

    }

    if (si2 < 0) {
        out_split_param.end_index_ = n - 1;
        out_split_param.t1_ = 1.0;
    }

    out_split_param.start_index_ = si1;
    out_split_param.end_index_ = si2;

    out_split_param.valid_ = true;
}


} // End of namespace Grain
