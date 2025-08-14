//
//  Bezier.cpp
//
//  Created by Roald Christesen on 07.11.2013
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Bezier/Bezier.hpp"
#include "2d/RangeRect.hpp"
#include "Bezier/BezierValueCurve.hpp"
#include "Graphic/GraphicContext.hpp"
// #include "GrViewport.hpp" !!!!!


namespace Grain {

    Bezier::Bezier(double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3)  noexcept {

        m_pos[0].set(x0, y0);
        m_pos[1].set(x1, y1);
        m_pos[2].set(x2, y2);
        m_pos[3].set(x3, y3);
    }


    Bezier::Bezier(const Vec2d& p0, const Vec2d& p1, const Vec2d& p2, const Vec2d& p3) noexcept {

        m_pos[0] = p0;
        m_pos[1] = p1;
        m_pos[2] = p2;
        m_pos[3] = p3;
    }


    Bezier::Bezier(const Vec2d& p0, const Vec2d& p1, const Vec2d& p2) noexcept {

        setQuadratic(p0, p1, p2);
    }


    Bezier::Bezier(const Vec2d* pos_array) noexcept {

        if (pos_array) {
            m_pos[0] = pos_array[0];
            m_pos[1] = pos_array[1];
            m_pos[2] = pos_array[2];
            m_pos[3] = pos_array[3];
        }
    }


    Bezier::Bezier(const BezierValueCurvePoint& p0, const BezierValueCurvePoint& p1) noexcept {

        m_pos[0] = p0.m_pos;
        m_pos[1] = p0.usesRightControl() ?  p0.m_used_right_pos : p0.m_pos;
        m_pos[2] = p1.usesLeftControl() ? p1.m_used_left_pos : p1.m_pos;
        m_pos[3] = p1.m_pos;
    }


    /**
     *  @brief Bounding box for bezier curve.
     *
     *  @return Rect of bounding box.
     */
    Rectd Bezier::bounds() const noexcept {

        Vec2d p0 = m_pos[0];
        Vec2d p1 = m_pos[1];
        Vec2d p2 = m_pos[2];
        Vec2d p3 = m_pos[3];

        RangeRectd range;
        range = m_pos[0];
        range += m_pos[3];

        {
            double b = (6.0 * p0.m_x - 12.0 * p1.m_x + 6.0 * p2.m_x);
            double a = (-3.0 * p0.m_x + 9.0 * p1.m_x - 9.0 * p2.m_x + 3.0 * p3.m_x);
            double c = (3.0 * p1.m_x - 3.0 * p0.m_x);
            if (a != 0.0 && b != 0.0) {
                double t = -c / b;
                if (0.0 < t && t < 1.0) {
                    range.addX(_bounds_f(t, p0.m_x, p1.m_x, p2.m_x, p3.m_x));
                }
            }

            double b2ac = std::pow(b, 2.0) - 4.0 * c * a;
            if (b2ac >= 0.0) {
                double t1 = (-b + std::sqrt(b2ac)) / (2.0 * a);
                if (0.0 < t1 && t1 < 1.0) {
                    range.addX(_bounds_f(t1, p0.m_x, p1.m_x, p2.m_x, p3.m_x));
                }
                double t2 = (-b - std::sqrt(b2ac)) / (2 * a);
                if (0.0 < t2 && t2 < 1.0) {
                    range.addX(_bounds_f(t2, p0.m_x, p1.m_x, p2.m_x, p3.m_x));
                }
            }
        }

        {
            double b = (6.0 * p0.m_y - 12.0 * p1.m_y + 6.0 * p2.m_y);
            double a = (-3.0 * p0.m_y + 9.0 * p1.m_y - 9.0 * p2.m_y + 3.0 * p3.m_y);
            double c = (3.0 * p1.m_y - 3.0 * p0.m_y);
            if (a != 0.0 && b != 0.0) {
                double t = -c / b;
                if (0.0 < t && t < 1.0) {
                    range.addY(_bounds_f(t, p0.m_y, p1.m_y, p2.m_y, p3.m_y));
                }
            }

            double b2ac = std::pow(b, 2.0) - 4.0 * c * a;
            if (b2ac >= 0.0) {
                double t1 = (-b + std::sqrt(b2ac)) / (2.0 * a);
                if (0.0 < t1 && t1 < 1.0) {
                    range.addY(_bounds_f(t1, p0.m_y, p1.m_y, p2.m_y, p3.m_y));
                }
                double t2 = (-b - std::sqrt(b2ac)) / (2 * a);
                if (0.0 < t2 && t2 < 1.0) {
                    range.addY(_bounds_f(t2, p0.m_y, p1.m_y, p2.m_y, p3.m_y));
                }
            }
        }

        return range.rect();
    }


    double Bezier::approximatedCurveLength(int32_t resolution) const noexcept {

        if (resolution < 64) {
            resolution = 64;
        }

        double t_step = 1.0 / resolution;
        Vec2d v1 = posOnCurve(0.0);
        double t = t_step;
        double length = 0.0;
        for (int32_t i = 0; i < resolution; i++) {
            Vec2d v2 = posOnCurve(t);
            length += v2.distance(v1);
            t += t_step;
            v1 = v2;
        }

        return length;
    }


    void Bezier::set(const Vec2d& p0, const Vec2d& p1, const Vec2d& p2, const Vec2d& p3) noexcept {

        m_pos[0] = p0;
        m_pos[1] = p1;
        m_pos[2] = p2;
        m_pos[3] = p3;
    }


    void Bezier::setQuadratic(const Vec2d& p0, const Vec2d& p1, const Vec2d& p2) noexcept {

        m_pos[0] = p0;
        m_pos[1] = p0 + (p1 - p0) * (2.0 / 3);
        m_pos[2] = p2 + (p1 - p2) * (2.0 / 3);
        m_pos[3] = p2;
    }


    void Bezier::set(double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3) noexcept {

        m_pos[0].set(x0, y0);
        m_pos[1].set(x1, y1);
        m_pos[2].set(x2, y2);
        m_pos[3].set(x3, y3);
    }


    void Bezier::setPointAtIndex(int32_t index, const Vec2d& p) noexcept {

        if (index >= 0 && index <= 3) {
            m_pos[index] = p;
        }
    }


    void Bezier::setHorizontalSegment(const Vec2d& p_left, const Vec2d& p_right, const Vec2d& left_f, const Vec2d& right_f) noexcept {

        double w = p_right.m_x - p_left.m_x;
        double h = p_right.m_y - p_left.m_y;
        m_pos[0] = p_left;
        m_pos[1].m_x = p_left.m_x + left_f.m_x * w;
        m_pos[1].m_y = p_left.m_y + left_f.m_y * h;
        m_pos[2].m_x = p_right.m_x - right_f.m_x * w;
        m_pos[2].m_y = p_right.m_y - right_f.m_y * h;
        m_pos[3] = p_right;
    }


    void Bezier::setWithTangents(const Vec2d& p0, const Vec2d& p3, const Vec2d& t1, const Vec2d& t2) noexcept {

        Vec2d chord = p3 - p0;
        double d = chord.length() / 3.0;

        m_pos[0] = p0;
        m_pos[1] = p0 + t1.normalized() * d;
        m_pos[2] = p3 - t2.normalized() * d;
        m_pos[3] = p3;
    }


    Vec2d Bezier::posAtPointIndex(int32_t index) const noexcept {

        return m_pos[std::clamp<int32_t>(index, 0, 3)];
    }


    Vec2d Bezier::posOnCurve(double t) const noexcept {

        Vec2d pos;

        double d = 1.0 - t;
        double d_sq = d * d;  // Used to reduce the number of multiplications
        double t_sq = t * t;

        double f1 = d * d_sq;
        double f2 = 3.0 * d_sq * t;
        double f3 = 3.0 * d * t_sq;
        double f4 = t * t_sq;

        pos.m_x = m_pos[0].m_x * f1 + m_pos[1].m_x * f2 + m_pos[2].m_x * f3 + m_pos[3].m_x * f4;
        pos.m_y = m_pos[0].m_y * f1 + m_pos[1].m_y * f2 + m_pos[2].m_y * f3 + m_pos[3].m_y * f4;

        return pos;
    }


    bool Bezier::hitBounds(const Vec2d& pos, double radius) const noexcept {

        return bounds().contains(pos, radius);
    }


    /**
     *  @brief Returns the index of the closest control point within a given radius.
     *
     *  @param pos    The position to test against the control points.
     *  @param radius The maximum distance to consider a control point as "hit".
     *  @return Index of the closest control point if within radius, or -1 if none are close enough.
     */
    int32_t Bezier::hitPoint(const Vec2d& pos, double radius) const noexcept {

        int32_t index = -1;
        double distance = DBL_MAX;
        for (int32_t i = 0; i < 4; i++) {
            double d = pos.distance(m_pos[i]);
            if (d < distance) {
                distance = d;
                index = i;
            }
        }

        if (distance < radius) {
            return index;
        }
        else {
            return -1;
        }
    }


    double Bezier::hit(const Vec2d& pos, double radius) const noexcept {

        double t_result = DBL_MAX;

        if (hitBounds(pos, radius)) {

            int32_t resolution = 8;
            int32_t recursion_depth = 16;

            double t_start = 0.0;
            double t_end = 1.0;

            Bezier b1, b2;
            split(0.5, b1, b2);

            for (int32_t recursion_index = 0; recursion_index <= recursion_depth; recursion_index++) {
                double t_step = (t_end - t_start) / resolution;
                double d_min = DBL_MAX;
                int32_t index = -1;

                for (int32_t i = 0; i <= resolution; i++) {
                    double t = t_start + i * t_step;
                    double d = pos.distance(posOnCurve(t));

                    if (d < d_min) {
                        index = i;
                        d_min = d;
                        t_result = t;
                    }
                }

                if (index < 0) {
                    return DBL_MAX;
                }

                if (recursion_index == recursion_depth && d_min > radius) {
                    return DBL_MAX;
                }

                t_start = std::clamp<float>(t_result - t_step, 0.0f, 1.0f);
                t_end = std::clamp<float>(t_result + t_step, 0.0f, 1.0f);
            }
        }

        return t_result;
    }

/* !!!!!!
    double Bezier::hit(const Viewport& viewport, const Vec2d& pos, double radius) const noexcept {

        Bezier b = *this;
        b.toViewport(viewport);
        return b.hit(pos, radius);
    }
    */


    bool Bezier::split(double t, Bezier& out_bezier1, Bezier& out_bezier2) const noexcept {

        t = std::clamp<double>(t, 0.0, 1.0);

        Vec2d a = (m_pos[1] - m_pos[0]) * t + m_pos[0];
        Vec2d b = (m_pos[2] - m_pos[1]) * t + m_pos[1];
        Vec2d c = (m_pos[3] - m_pos[2]) * t + m_pos[2];
        Vec2d ab = (b - a) * t + a;
        Vec2d bc = (c - b) * t + b;
        Vec2d abc = (bc - ab) * t + ab;

        out_bezier1.m_pos[0] = m_pos[0];
        out_bezier1.m_pos[1] = a;
        out_bezier1.m_pos[2] = ab;
        out_bezier1.m_pos[3] = abc;

        out_bezier2.m_pos[0] = abc;
        out_bezier2.m_pos[1] = bc;
        out_bezier2.m_pos[2] = c;
        out_bezier2.m_pos[3] = m_pos[3];

        return true;
    }


    bool Bezier::truncate(double t_start, double t_end, Bezier& out_bezier) const noexcept {

        double t0 = t_start < 0.0 ? 0.0 : t_start > 1.0 ? 1.0 : t_start;
        double t1 = t_end < 0.0 ? 0.0 : t_end > 1.0 ? 1.0 : t_end;
        if (t0 >= t1) {
            return false;
        }

        double u0 = 1.0 - t0;
        double u1 = 1.0 - t1;

        double qxa = m_pos[0].m_x * u0 * u0 + m_pos[1].m_x * 2 * t0 * u0 + m_pos[2].m_x * t0 * t0;
        double qxb = m_pos[0].m_x * u1 * u1 + m_pos[1].m_x * 2 * t1 * u1 + m_pos[2].m_x * t1 * t1;
        double qxc = m_pos[1].m_x * u0 * u0 + m_pos[2].m_x * 2 * t0 * u0 + m_pos[3].m_x * t0 * t0;
        double qxd = m_pos[1].m_x * u1 * u1 + m_pos[2].m_x * 2 * t1 * u1 + m_pos[3].m_x * t1 * t1;

        double qya = m_pos[0].m_y * u0 * u0 + m_pos[1].m_y * 2 * t0 * u0 + m_pos[2].m_y * t0 * t0;
        double qyb = m_pos[0].m_y * u1 * u1 + m_pos[1].m_y * 2 * t1 * u1 + m_pos[2].m_y * t1 * t1;
        double qyc = m_pos[1].m_y * u0 * u0 + m_pos[2].m_y * 2 * t0 * u0 + m_pos[3].m_y * t0 * t0;
        double qyd = m_pos[1].m_y * u1 * u1 + m_pos[2].m_y * 2 * t1 * u1 + m_pos[3].m_y * t1 * t1;

        out_bezier.m_pos[0].m_x = qxa * u0 + qxc * t0;
        out_bezier.m_pos[1].m_x = qxa * u1 + qxc * t1;
        out_bezier.m_pos[2].m_x = qxb * u0 + qxd * t0;
        out_bezier.m_pos[3].m_x = qxb * u1 + qxd * t1;

        out_bezier.m_pos[0].m_y = qya * u0 + qyc * t0;
        out_bezier.m_pos[1].m_y = qya * u1 + qyc * t1;
        out_bezier.m_pos[2].m_y = qyb * u0 + qyd * t0;
        out_bezier.m_pos[3].m_y = qyb * u1 + qyd * t1;

        return true;
    }


    void Bezier::buildVec2LUT(Vec2d* lut, int32_t resolution) const noexcept {

        if (lut != nullptr && resolution > 1) {
            for (int32_t i = 0; i < resolution; i++) {
                lut[i] = posOnCurve((double)i / (resolution - 1));
            }
        }
    }


    void Bezier::translate(double tx, double ty) noexcept {

        for (int32_t i = 0; i < 4; i++) {
            m_pos[i].m_x += tx;
            m_pos[i].m_y += ty;
        }
    }


    void Bezier::translate(const Vec2d& tv) noexcept {

        for (int32_t i = 0; i < 4; i++) {
            m_pos[i] += tv;
        }
    }


    void Bezier::translateX(double tx) noexcept {

        for (int32_t i = 0; i < 4; i++) {
            m_pos[i].m_x += tx;
        }
    }


    void Bezier::translateY(double ty) noexcept {

        for (int32_t i = 0; i < 4; i++) {
            m_pos[i].m_y += ty;
        }
    }


    void Bezier::scale(double sx, double sy) noexcept {

        for (int32_t i = 0; i < 4; i++) {
            m_pos[i].m_x *= sx;
            m_pos[i].m_y *= sy;
        }
    }


    void Bezier::scale(const Vec2d& sv) noexcept {

        for (int32_t i = 0; i < 4; i++) {
            m_pos[i] *= sv;
        }
    }


    void Bezier::scaleX(double sx) noexcept {

        for (int32_t i = 0; i < 4; i++) {
            m_pos[i].m_x *= sx;
        }
    }


    void Bezier::scaleY(double sy) noexcept {

        for (int32_t i = 0; i < 4; i++) {
            m_pos[i].m_y *= sy;
        }
    }


    /**
     *  @brief Transforms the Bézier curve from normalized space [0, 1] into the coordinate space defined by a rectangle.
     *
     *  This function scales and translates the control points of the Bézier curve so that they fit within the given rectangle.
     *  It assumes the current Bézier coordinates are in normalized space (i.e., x and y in [0, 1]).
     *
     *  @param rect The target rectangle defining the desired position and size of the Bézier curve.
     */
    void Bezier::transformByRect(const Rectd& rect) noexcept {

        scale(rect.width(), rect.height());
        translate(rect.x(), rect.y());
    }


    /**
     *  @brief Transforms the Bézier control points from normalized or world coordinates into viewport (screen) coordinates.
     *
     *  This method applies a transformation to all Bézier control points, converting them to the coordinate space of the given viewport.
     *  It's typically used when preparing to render the curve in screen space.
     *
     *  @param viewport The viewport providing the transformation logic to map positions into view/screen space.
     */
     /* !!!!!
    void Bezier::toViewport(const Viewport& viewport) noexcept {

        viewport.transformPosToView(m_pos, 4);
    }
      */


    /**
     *  @brief Transforms the Bézier control points from viewport (screen) coordinates back to normalized or world space.
     *
     *  This method reverses the effect of `toViewport`, converting screen-space coordinates back to the logical coordinate space.
     *  Useful when interpreting user input or data from screen space into the internal coordinate system.
     *
     *  @param viewport The viewport providing the inverse transformation from screen space.
     */
     /* !!!!!
    void Bezier::fromViewport(const Viewport& viewport) noexcept {

        viewport.transformPosFromView(m_pos, 4);
    }
*/

     /* !!!!!
    void Bezier::stroke(GraphicContext& gc) const noexcept {

        gc.strokeBezier(*this);
    }
      */


    /**
     *  @brief Approximates a single control point for a quadratic Bézier curve that best fits this cubic Bézier curve.
     *
     *  This method provides an estimation of a control point that represents a quadratic Bézier approximation
     *  of the original cubic Bézier curve. The resulting control point is written to `out_control_pos`.
     *
     *  The approximation works by blending the inner control points (p2 and p3) more heavily,
     *  and subtracting a smaller influence from the endpoints (p1 and p4), which reflects
     *  the difference in curve structure between cubic and quadratic Bézier curves.
     *
     *  @param[out] out_control_pos The resulting control point position for the quadratic approximation.
     */
    void Bezier::approximateQuadraticBezierControlPos(Vec2d& out_control_pos) const noexcept {

        out_control_pos = (m_pos[1] + m_pos[2]) * (3.0 / 4.0) - (m_pos[0] + m_pos[3]) * (1.0 / 4.0);
    }


    /**
     *  @brief Helper function to calculate cubic Bézier curves for an elliptical arc.
     */
    int32_t Bezier::arcToBezierPosArray(const Vec2d& start_pos, const Vec2d& radii, double rotation, bool large_arc_flag, bool sweep_flag, const Vec2d& end_pos, int32_t max_segment_n, Vec2d* out_pos_array) noexcept {

        // Step 1: Handle special cases where radii are zero or the start and end positions are identical
        if (radii.m_x == 0.0 || radii.m_y == 0.0 || (start_pos.distance(end_pos) < std::numeric_limits<float>::epsilon())) {
            return 0;
        }

        // Step 2: Apply transformations to center the arc
        double rx = std::abs(radii.m_x);
        double ry = std::abs(radii.m_y);

        double dx = (start_pos.m_x - end_pos.m_x) / 2.0;
        double dy = (start_pos.m_y - end_pos.m_y) / 2.0;

        double rotation_rad = rotation * std::numbers::pi / 180.0;
        double cos_angle = std::cos(rotation_rad);
        double sin_angle = std::sin(rotation_rad);

        // Transform positions to the arc's local coordinate system
        double x1p = cos_angle * dx + sin_angle * dy;
        double y1p = -sin_angle * dx + cos_angle * dy;

        // Correct radii if needed
        double rx_sq = rx * rx;
        double ry_sq = ry * ry;
        double x1p_sq = x1p * x1p;
        double y1p_sq = y1p * y1p;

        double radius_check = x1p_sq / rx_sq + y1p_sq / ry_sq;
        if (radius_check > 1.0) {
            rx *= std::sqrt(radius_check);
            ry *= std::sqrt(radius_check);
            rx_sq = rx * rx;
            ry_sq = ry * ry;
        }

        // Step 3: Calculate the center of the ellipse in the transformed coordinate system
        double sign = (large_arc_flag != sweep_flag) ? 1 : -1;
        double sq = ((rx_sq * ry_sq) - (rx_sq * y1p_sq) - (ry_sq * x1p_sq)) / ((rx_sq * y1p_sq) + (ry_sq * x1p_sq));
        sq = (sq < 0) ? 0 : sq;

        double cxp = sign * std::sqrt(sq) * (rx * y1p / ry);
        double cyp = sign * std::sqrt(sq) * (-ry * x1p / rx);

        // Transform the center back to the global coordinate system
        double cx = cos_angle * cxp - sin_angle * cyp + (start_pos.m_x + end_pos.m_x) / 2.0;
        double cy = sin_angle * cxp + cos_angle * cyp + (start_pos.m_y + end_pos.m_y) / 2.0;

        // Step 4: Calculate start and end angles
        auto vector_angle = [](double ux, double uy, double vx, double vy) {
            double dot = ux * vx + uy * vy;
            double len = std::sqrt((ux * ux + uy * uy) * (vx * vx + vy * vy));
            double angle = std::acos(std::max(-1.0, std::min(1.0, dot / len)));
            if (ux * vy - uy * vx < 0) angle = -angle;
            return angle;
        };

        double theta = vector_angle(1, 0, (x1p - cxp) / rx, (y1p - cyp) / ry);
        double delta_theta = vector_angle((x1p - cxp) / rx, (y1p - cyp) / ry, (-x1p - cxp) / rx, (-y1p - cyp) / ry);

        if (!sweep_flag && delta_theta > 0.0) {
            delta_theta -= std::numbers::pi * 2;
        }
        else if (sweep_flag && delta_theta < 0.0) {
            delta_theta += std::numbers::pi * 2;
        }

        // Step 5: Approximate the arc using Bézier curves
        int32_t segment_n = static_cast<int>(std::ceil(std::abs(delta_theta) / (std::numbers::pi / 2.0)));
        if (segment_n > max_segment_n) {
            segment_n = max_segment_n;
        }

        double sweep_per_segment = delta_theta / segment_n;

        double alpha = std::sin(sweep_per_segment / 2.0) * 4.0 / 3.0 / (1.0 + std::cos(sweep_per_segment / 2.0));

        Vec2d *p = out_pos_array;
        for (int32_t i = 0; i < segment_n; ++i) {
            double t0 = theta + i * sweep_per_segment;
            double t1 = t0 + sweep_per_segment;

            double cos_t0 = std::cos(t0);
            double sin_t0 = std::sin(t0);
            double cos_t1 = std::cos(t1);
            double sin_t1 = std::sin(t1);

            Vec2d p0, p1, p2, p3;

            p0.m_x = cx + cos_angle * (rx * cos_t0) - sin_angle * (ry * sin_t0);
            p0.m_y = cy + sin_angle * (rx * cos_t0) + cos_angle * (ry * sin_t0);
            p3.m_x = cx + cos_angle * (rx * cos_t1) - sin_angle * (ry * sin_t1);
            p3.m_y = cy + sin_angle * (rx * cos_t1) + cos_angle * (ry * sin_t1);
            p1.m_x = p0.m_x - alpha * cos_angle * (rx * sin_t0) - alpha * sin_angle * (ry * cos_t0);
            p1.m_y = p0.m_y - alpha * sin_angle * (rx * sin_t0) + alpha * cos_angle * (ry * cos_t0);
            p2.m_x = p3.m_x + alpha * cos_angle * (rx * sin_t1) + alpha * sin_angle * (ry * cos_t1);
            p2.m_y = p3.m_y + alpha * sin_angle * (rx * sin_t1) - alpha * cos_angle * (ry * cos_t1);

            if (i == 0) {
                *p++ = p0;
            }
            *p++ = p1;
            *p++ = p2;
            *p++ = p3;
        }

        return segment_n;
    }


    double Bezier::_bounds_f(double t, double p0, double p1, double p2, double p3) {

        double t_inv = 1.0 - t;

        return std::pow(t_inv, 3.0) * p0
               + 3.0 * std::pow(t_inv, 2.0) * t * p1
               + 3.0 * t_inv * std::pow(t, 2.0) * p2
               + std::pow(t, 3.0) * p3;
    }


    ErrorCode Bezier::fitCubicBezierToPoints(int32_t point_count, const Vec2d* points) noexcept {
        constexpr int32_t kMaxPointLimit = 2048;
        constexpr int32_t kMaxPointCount = 64;
        const bool allocate_flag = (point_count > kMaxPointCount);

        if (!points) {
            return ErrorCode::NullData;
        }

        if (point_count < 2) {
            return ErrorCode::BadArgs;
        }

        if (point_count > kMaxPointLimit) {
            return ErrorCode::LimitExceeded;
        }

        double t_buffer[kMaxPointCount];
        double *t = nullptr;
        if (point_count <= kMaxPointCount) {
            t = t_buffer;
        }
        else {
            t = new (std::nothrow) double[point_count];
            if (!t) {
                return ErrorCode::MemCantAllocate;
            }
        }

        // Chord-length parameterization
        t[0] = 0;
        double total = 0;
        for (int i = 1; i < point_count; ++i) {
            double dx = points[i].m_x - points[i - 1].m_x;
            double dy = points[i].m_y - points[i - 1].m_y;
            total += std::sqrt(dx * dx + dy * dy);
            t[i] = total;
        }

        for (int i = 1; i < point_count; ++i) {
            t[i] /= total;
        }

        m_pos[0] = points[0];
        m_pos[3] = points[point_count - 1];

        // Build matrix and RHS for least squares
        double _c[2][2] = {};  // Coefficient matrix
        Vec2d _x[2] = {};  // Right-hand side vector

        for (int i = 0; i < point_count; ++i) {
            double u = t[i];
            double b0 = evalBernsteinBasis0(u);
            double b1 = evalBernsteinBasis1(u);
            double b2 = evalBernsteinBasis2(u);
            double b3 = evalBernsteinBasis3(u);

            // Vec2d A1 = { b1, 0 };
            // Vec2d A2 = { b2, 0 };

            Vec2d tmp = points[i] - (m_pos[0] * b0 + m_pos[3] * b3);

            _c[0][0] += b1 * b1;
            _c[0][1] += b1 * b2;
            _c[1][0] += b1 * b2;
            _c[1][1] += b2 * b2;

            _x[0].m_x += b1 * tmp.m_x;
            _x[0].m_y += b1 * tmp.m_y;
            _x[1].m_x += b2 * tmp.m_x;
            _x[1].m_y += b2 * tmp.m_y;
        }

        // Solve 2x2 system for control points
        double det = _c[0][0] * _c[1][1] - _c[0][1] * _c[1][0];

        if (std::abs(det) > 1e-10) {
            double invDet = 1.0 / det;

            for (int d = 0; d < 2; ++d) {
                double x0 = (_x[0].m_x * _c[1][1] - _x[1].m_x * _c[0][1]) * invDet;
                double x1 = (_x[1].m_x * _c[0][0] - _x[0].m_x * _c[1][0]) * invDet;
                double y0 = (_x[0].m_y * _c[1][1] - _x[1].m_y * _c[0][1]) * invDet;
                double y1 = (_x[1].m_y * _c[0][0] - _x[0].m_y * _c[1][0]) * invDet;
                m_pos[1].set(x0, y0);
                m_pos[2].set(x1, y1);
            }
        }
        else {
            // Fallback to straight line control points
            m_pos[1] = m_pos[0] + (m_pos[3] - m_pos[0]) * (1.0 / 3);
            m_pos[2] = m_pos[0] + (m_pos[3] - m_pos[0]) * (2.0 / 3);
        }

        if (allocate_flag) {
            delete[] t;
        }

        return ErrorCode::None;
    }

} // End of namespace Grain
