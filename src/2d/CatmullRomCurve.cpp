//
//  CatmullRomCurve.hpp
//
//  Created by Roald Christesen on from 30.07.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

//
//  GrCatmullRomCurve.cpp
//  GrainLib
//
//  Created by Roald Christesen on 26.01.24.
//

#include "2d/CatmullRomCurve.hpp"
#include "Math/Math.hpp"
#include "Type/Type.hpp"


namespace Grain {


    CatmullRomCurve::CatmullRomCurve() noexcept {

        m_points.reserve(64);
    }


    CatmullRomCurve::CatmullRomCurve(int32_t point_capacity) noexcept {

        m_points.reserve(point_capacity);
    }


    CatmullRomCurve::~CatmullRomCurve()  noexcept {
    }


    /**
     *  @brief Adds a point to a Catmull-Rom spline.
     *
     *  This function adds a new point to the spline. If the point is very close to the
     *  last added point (closer than a predefined threshold), it will be ignored to
     *  avoid redundant points in the spline.
     *
     *  @param point A 2D point represented by a `Vec2d` object to be added to the spline.
     *  @return ErrorCode Returns an appropriate error code based on the operation result.
     *  - `ErrorCode::None` if the point was added successfully.
     *  - Other error codes for exceptional cases (if any).
     *
     *  @note The threshold for determining "same point" is defined by the class and is
     *        not adjustable through this function.
     */
    ErrorCode CatmullRomCurve::addPoint(const Vec2d &point) noexcept {

        if (pointCount() > 0) {
            auto p = pointAtIndex(lastPointIndex());
            if (p.distance(point) < m_point_distance_threshold) {
                // Ignore points closer than `m_point_distance_threshold`
                // This is intentional behavior, not an error, so return success
                return ErrorCode::None;
            }
        }

        if (!m_points.push(point)) {
            return ErrorCode::MemCantAllocate;
        }

        return ErrorCode::None;
    }


    /**
     *  @brief Extends the spline at its start and end points.
     *
     *  This method modifies the Catmull-Rom spline so that the curve passes through
     *  the first and last points. By default, Catmull-Rom splines do not interpolate
     *  the endpoints, but calling this method ensures the curve is extended to
     *  include them.
     *
     *  @return ErrorCode `ErrorCode::None` if curve could be extended, or an appropriate error code otherwise.
     *
     *  @note This function is particularly useful when the exact interpolation
     *        of the first and last points is required.
     */
    ErrorCode CatmullRomCurve::extend() noexcept {

        if (pointCount() > 3) {

            // Reorganize points memory
            for (int32_t i = 0; i < 2; i++) {
                if (!m_points.push(Vec2d())) {
                    return ErrorCode::MemCantAllocate;
                }
            }

            auto points = m_points.mutDataPtr();
            for (int32_t i = pointCount() - 2; i > 0; i--) {
                points[i] = points[i - 1];
            }

            // Compute the extended start point
            {
                auto points = m_points.mutDataPtr();
                auto a = &points[0];
                auto b = &points[1];
                auto c = &points[2];
                *a = *b + (*b - *c);
            }

            // Compute the extended end point
            {
                auto points = m_points.mutElementPtrAtIndex(lastPointIndex());
                auto a = &points[0];
                auto b = &points[-1];
                auto c = &points[-2];
                *a = *b + (*b - *c);
            }
        }

        return ErrorCode::None;
    }


    Vec2d CatmullRomCurve::pointAtIndex(int32_t index) noexcept {

        auto point = m_points.elementPtrAtIndex(index);
        if (point) {
            return Vec2d(point->m_x, point->m_y);
        }
        else {
            return Vec2d();
        }
    }


    double CatmullRomCurve::rawLength() const noexcept {

        double length = 0.0;

        auto points = m_points.dataPtr();
        for (int32_t i = 0; i < pointCount() - 1; i++) {
            length += points[i].distance(points[i + 1]);
        }

        return length;
    }


    void CatmullRomCurve::pointOnCurve(double t, Vec2d &out_point) const noexcept {

        if (pointCount() < 4) {
            if (pointCount() > 0) {
                out_point.zero();
            }
            else {
                out_point = m_points[0];
            }
            return;
        }

        t = std::clamp<double>(t, 0.0, 1.0);

        int32_t segment_n = segmentCount();
        int32_t segment_index = static_cast<int32_t>(t * segment_n);
        int32_t last_segment_index = lastSegmentIndex();
        if (segment_index > last_segment_index) {
            segment_index = last_segment_index;
        }

        float t2 = Math::remapnorm(static_cast<double>(segment_index) / segment_n, static_cast<double>(segment_index + 1) / segment_n, t);

        _getPoint(segment_index, t2, m_alpha, out_point);
    }


    double CatmullRomCurve::_getT(double t, float alpha, const Vec2d &p0, const Vec2d &p1) const noexcept {

        auto d  = p1 - p0;

        double a = d.dot(d);
        double b = std::pow(a, alpha * 0.5);

        return (b + t);
    }


    void CatmullRomCurve::_getPoint(const Vec2d &p0, const Vec2d &p1, const Vec2d &p2, const Vec2d &p3, double t, float alpha, Vec2d &out_point) const noexcept {

        double t0 = 0.0;
        double t1 = _getT(t0, alpha, p0, p1);
        double t2 = _getT(t1, alpha, p1, p2);
        double t3 = _getT(t2, alpha, p2, p3);

        t = Math::lerp(t1, t2, t);

        Vec2d pa1 = p0  * ((t1 - t) / (t1 - t0)) + p1  * ((t - t0) / (t1 - t0));
        Vec2d pa2 = p1  * ((t2 - t) / (t2 - t1)) + p1  * ((t - t1) / (t2 - t1));
        Vec2d pa3 = p2  * ((t3 - t) / (t3 - t2)) + p3  * ((t - t2) / (t3 - t2));
        Vec2d pb1 = pa1 * ((t2 - t) / (t2 - t0)) + pa2 * ((t - t0) / (t2 - t0));
        Vec2d pb2 = pa2 * ((t3 - t) / (t3 - t1)) + pa3 * ((t - t1) / (t3 - t1));

        out_point = pb1 * ((t2 - t) / (t2 - t1)) + pb2 * ((t - t1) / (t2 - t1));
    }


    void CatmullRomCurve::_getPoint(int32_t segment_index, double t, float alpha, Vec2d &out_point) const noexcept {

        auto p = m_points.elementPtrAtIndex(segment_index);

        if (p) {
            Vec2d p0{ p[0].m_x, p[0].m_y };
            Vec2d p1{ p[1].m_x, p[1].m_y };
            Vec2d p2{ p[2].m_x, p[2].m_y };
            Vec2d p3{ p[3].m_x, p[3].m_y };

            double t0 = 0.0;
            double t1 = _getT(t0, alpha, p0, p1);
            double t2 = _getT(t1, alpha, p1, p2);
            double t3 = _getT(t2, alpha, p2, p3);
            t = Math::lerp(t1, t2, t);

            double td_1_0 = t1 - t0;
            double td_2_1 = t2 - t1;
            double td_3_2 = t3 - t2;
            double td_2_0 = t2 - t0;
            double td_3_1 = t3 - t1;

            Vec2d pa1 = p0 * ((t1 - t) / td_1_0) + p1 * ((t - t0) / td_1_0);
            Vec2d pa2 = p1 * ((t2 - t) / td_2_1) + p2 * ((t - t1) / td_2_1);
            Vec2d pa3 = p2 * ((t3 - t) / td_3_2) + p3 * ((t - t2) / td_3_2);
            Vec2d pb1 = pa1 * ((t2 - t) / td_2_0) + pa2 * ((t - t0) / td_2_0);
            Vec2d pb2 = pa2 * ((t3 - t) / td_3_1) + pa3 * ((t - t1) / td_3_1);

            out_point = pb1 * ((t2 - t) / td_2_1) + pb2 * ((t - t1) / td_2_1);
        }
        else {
            out_point.zero();
        }
    }


} // End of namespace Grain.
