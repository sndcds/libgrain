//
//  Polygon.hpp
//
//  Created by Roald Christesen on from 06.03.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "2d/Polygon.hpp"
#include "Graphic/GraphicContext.hpp"
#include "2d/RangeRect.hpp"


namespace Grain {

Polygon::Polygon(int32_t point_capacity) noexcept {
    _init(point_capacity);
}


Polygon::~Polygon() noexcept {
}


void Polygon::_init(int32_t point_capacity) noexcept {
    points_.reserve(point_capacity);
    closed_ = false;
    must_update_ = true;
}


void Polygon::_update() noexcept {
    if (must_update_) {
        _updateLength();
        must_update_ = false;
    }
}


void Polygon::_updateLength() noexcept {
    length_ = 0.0;

    if (pointCount() >= 2) {
        bool first_point_flag = true;
        Vec2d first_point;
        Vec2d prev_point;

        for (auto& point : points_) {
            if (first_point_flag) {
                first_point = point;
                first_point_flag = false;
            }
            else {
                length_ += point.distance(prev_point);
            }

            prev_point = point;
        }

        if (closed_) {
            length_ += prev_point.distance(first_point);
        }
    }
}


int32_t Polygon::segmentCount() const noexcept {
    return closed_ ? static_cast<int32_t>(points_.size()) : static_cast<int32_t>(points_.size()) - 1;
}


bool Polygon::pointAtIndex(int32_t index, Vec2d& out_point) noexcept {
    return points_.elementAtIndex(index, out_point);
}


Vec2d* Polygon::mutPointPtrAtIndex(int32_t index) noexcept {
    return points_.mutElementPtrAtIndex(index);
}


Vec2d* Polygon::mutLastPointPtr() noexcept {
    return points_.mutLastElementPtr();
}


double Polygon::length() noexcept {
    _update();
    return length_;
}


RangeRectd Polygon::bounds() noexcept {
    RangeRectd bounds;
    this->bounds(bounds);
    return bounds;
}


bool Polygon::bounds(RangeRectd& out_bounds) noexcept {
    out_bounds.initForMinMaxSearch();

    if (pointCount() < 1) {
        return false;
    }
    else {
        for (auto& point : points_) {
            out_bounds.add(point);
        }

        return true;
    }
}


void Polygon::clear() noexcept {
    points_.clear();
    must_update_ = true;
}


bool Polygon::setCapacity(int32_t capacity) noexcept {
    return points_.reserve(capacity);
}


void Polygon::addPoint(double x, double y) noexcept {
    addPoint(Vec2d(x, y));
}


void Polygon::addPoint(const Vec2d& pos) noexcept {
    points_.push(pos);
    must_update_ = true;
}


bool Polygon::generateEvenlyDistributedPolygon(int32_t point_count, Polygon& out_polygon) noexcept {
    if (point_count < 2) {
        return false;
    }
    else {
        double length = this->length();
        double step = length / (point_count - 1);

        auto p1 = points_.elementAtIndex(0);
        auto p2 = points_.elementAtIndex(1);

        out_polygon.clear();
        out_polygon.setCapacity(point_count);
        out_polygon.addPoint(p1);

        int32_t src_point_count = pointCount();
        int32_t src_point_index = 1;
        double step_rest = step;

        while (true) {
            double l = p1.distance(p2);
            Vec2d dir = p2 - p1;
            dir.normalize();

            if (step_rest <= l) {
                p1 += dir * step_rest;
                out_polygon.addPoint(p1);

                if (out_polygon.pointCount() >= (point_count - 1)) {
                    break;
                }

                step_rest = step;
            }
            else {
                step_rest -= l;
                p1 = p2;
                src_point_index++;

                if (closed_) {
                    if (src_point_index < src_point_count) {
                        p2 = points_.elementAtIndex(src_point_index);
                    }
                    else if (src_point_index == src_point_count) {
                        p2 = points_.elementAtIndex(0);
                    }
                    else {
                        break;
                    }
                }
                else {
                    if (src_point_index < src_point_count) {
                        p2 = points_.elementAtIndex(src_point_index);
                    }
                    else {
                        break;
                    }
                }
            }
        }

        out_polygon.setClosed(closed_);

        if (!closed_) {
            out_polygon.addPoint(p2);
        }

        return true;
    }
}


bool Polygon::point(double t, Vec2d& out_point) noexcept {
    if (points_.size() < 2) {
        return false;
    }

    if (t <= 0.0) {
        out_point = points_.elementAtIndex(0);
        return true;
    }
    else if (t >= 1.0) {
        if (closed_) {
            out_point = points_.elementAtIndex(0);
        }
        else {
            out_point = points_.lastElement();
        }
        return true;
    }

    double length = this->length();
    double step = t * length;

    auto p1 = points_.elementAtIndex(0);
    auto p2 = points_.elementAtIndex(1);

    int32_t src_point_count = pointCount();
    int32_t src_point_index = 1;
    double step_rest = step;

    while (true) {
        double l = p1.distance(p2);
        Vec2d dir = p2 - p1;
        dir.normalize();

        if (step_rest <= l) {
            p1 += dir * step_rest;
            out_point = p1;
            return true;
        }
        else {
            step_rest -= l;
            p1 = p2;
            src_point_index++;

            if (closed_) {
                if (src_point_index < src_point_count) {
                    p2 = points_.elementAtIndex(src_point_index);
                }
                else if (src_point_index == src_point_count) {
                    p2 = points_.elementAtIndex(0);
                }
                else {
                    break;
                }
            }
            else {
                if (src_point_index < src_point_count) {
                    p2 = points_.elementAtIndex(src_point_index);
                }
                else {
                    break;
                }
            }
        }
    }

    return true;
}


void Polygon::fill(GraphicContext& gc) noexcept {
    if (pointCount() > 2) {
        gc.addPolygon(this);
        gc.fillPath();
    }
}


void Polygon::stroke(GraphicContext& gc) noexcept {
    if (pointCount() > 1) {
        gc.addPolygon(this);
        gc.strokePath();
    }
}


void Polygon::addClip(GraphicContext& gc) noexcept  {
    if (pointCount() > 2) {
        gc.addPolygon(this);
        gc.clipPath();
    }
}


}  // End of namespace Grain
