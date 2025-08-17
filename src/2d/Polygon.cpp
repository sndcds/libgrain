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

        m_points.reserve(point_capacity);
        m_closed = false;
        m_must_update = true;
    }


    void Polygon::_update() noexcept {

        if (m_must_update == true) {
            _updateLength();
            m_must_update = false;
        }
    }


    void Polygon::_updateLength() noexcept {

        m_length = 0.0;

        if (pointCount() >= 2) {

            bool first_point_flag = true;
            Vec2d first_point;
            Vec2d prev_point;

            for (auto& point : m_points) {
                if (first_point_flag) {
                    first_point = point;
                    first_point_flag = false;
                }
                else {
                    m_length += point.distance(prev_point);
                }

                prev_point = point;
            }

            if (m_closed == true) {
                m_length += prev_point.distance(first_point);
            }
        }
    }


    int32_t Polygon::segmentCount() const noexcept {

        return m_closed == true ? static_cast<int32_t>(m_points.size()) : static_cast<int32_t>(m_points.size()) - 1;
    }


    bool Polygon::pointAtIndex(int32_t index, Vec2d& out_point) noexcept {

        return m_points.elementAtIndex(index, out_point);
    }


    Vec2d* Polygon::mutPointPtrAtIndex(int32_t index) noexcept {

        return m_points.mutElementPtrAtIndex(index);
    }


    Vec2d* Polygon::mutLastPointPtr() noexcept {

        return m_points.mutLastElementPtr();
    }


    double Polygon::length() noexcept {

        _update();
        return m_length;
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
            for (auto& point : m_points) {
                out_bounds.add(point);
            }

            return true;
        }
    }


    void Polygon::clear() noexcept {

        m_points.clear();
        m_must_update = true;
    }


    bool Polygon::setCapacity(int32_t capacity) noexcept {

        return m_points.reserve(capacity);
    }


    void Polygon::addPoint(double x, double y) noexcept {

        addPoint(Vec2d(x, y));
    }


    void Polygon::addPoint(const Vec2d& pos) noexcept {

        m_points.push(pos);
        m_must_update = true;
    }


    bool Polygon::generateEvenlyDistributedPolygon(int32_t point_count, Polygon& out_polygon) noexcept {

        if (point_count < 2) {
            return false;
        }
        else {
            double length = this->length();
            double step = length / (point_count - 1);

            auto p1 = m_points.elementAtIndex(0);
            auto p2 = m_points.elementAtIndex(1);

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

                    if (m_closed == true) {
                        if (src_point_index < src_point_count) {
                            p2 = m_points.elementAtIndex(src_point_index);
                        }
                        else if (src_point_index == src_point_count) {
                            p2 = m_points.elementAtIndex(0);
                        }
                        else {
                            break;
                        }
                    }
                    else {
                        if (src_point_index < src_point_count) {
                            p2 = m_points.elementAtIndex(src_point_index);
                        }
                        else {
                            break;
                        }
                    }
                }
            }

            out_polygon.setClosed(m_closed);

            if (m_closed == false) {
                out_polygon.addPoint(p2);
            }

            return true;
        }
    }


    bool Polygon::point(double t, Vec2d& out_point) noexcept {

        if (m_points.size() < 2) {
            return false;
        }

        if (t <= 0.0) {
            out_point = m_points.elementAtIndex(0);
            return true;
        }
        else if (t >= 1.0) {
            if (m_closed == true) {
                out_point = m_points.elementAtIndex(0);
            }
            else {
                out_point = m_points.lastElement();
            }
            return true;
        }


        double length = this->length();
        double step = t * length;

        auto p1 = m_points.elementAtIndex(0);
        auto p2 = m_points.elementAtIndex(1);

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

                if (m_closed == true) {
                    if (src_point_index < src_point_count) {
                        p2 = m_points.elementAtIndex(src_point_index);
                    }
                    else if (src_point_index == src_point_count) {
                        p2 = m_points.elementAtIndex(0);
                    }
                    else {
                        break;
                    }
                }
                else {
                    if (src_point_index < src_point_count) {
                        p2 = m_points.elementAtIndex(src_point_index);
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
