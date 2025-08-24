//
//  Quadrilateral.cpp
//
//  Created by Roald Christesen on from 26.01.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "2d/Quadrilateral.hpp"
#include "2d/Line.hpp"


namespace Grain {

    RangeRectd Quadrilateral::axisAlignedBbox() const noexcept {

        RangeRectd bbox;

        bbox.initForMinMaxSearch();
        bbox.add(m_points[0]);
        bbox.add(m_points[1]);
        bbox.add(m_points[2]);
        bbox.add(m_points[3]);

        return bbox;
    }


    bool Quadrilateral::horizontalLine(double v, Lined& out_line) noexcept {

        project(0.0, v, out_line.m_p1);
        project(1.0, v, out_line.m_p2);

        return true;
    }


    bool Quadrilateral::verticalLine(double u, Lined& out_line) noexcept {

        project(u, 0.0, out_line.m_p1);
        project(u, 1.0, out_line.m_p2);

        return true;
    }


    bool Quadrilateral::bezierCirclePoints(Vec2d *out_points) noexcept {
        if (out_points) {
            double a = 0.0;
            double b = 0.5 - 0.551915024494 / 2;
            double c = 0.5;
            double d = 0.5 + 0.551915024494 / 2;
            double e = 1.0;

            project(c, a, out_points[0]);
            project(d, a, out_points[1]);
            project(e, b, out_points[2]);
            project(e, c, out_points[3]);
            project(e, d, out_points[4]);
            project(d, e, out_points[5]);
            project(c, e, out_points[6]);
            project(b, e, out_points[7]);
            project(a, d, out_points[8]);
            project(a, c, out_points[9]);
            project(a, b, out_points[10]);
            project(b, a, out_points[11]);

            return true;
        }

        return false;
    }


    double Quadrilateral::area() const noexcept {

        double p = m_points[0].distance(m_points[2]);
        double q = m_points[1].distance(m_points[3]);

        double a = m_points[0].distance(m_points[1]);
        double b = m_points[1].distance(m_points[2]);
        double c = m_points[2].distance(m_points[3]);
        double d = m_points[3].distance(m_points[0]);

        double m = b * b + d * d - a * a - c * c;

        return 0.25 * std::sqrt(4 * p * p * q * q - m * m);
    }


    bool Quadrilateral::isSimple() const noexcept {

        if (!m_valid_points) {
            return false;
        }

        bool s1 = m_points[0].sign(m_points[1], m_points[3]) > 0;
        bool s3 = m_points[2].sign(m_points[1], m_points[3]) > 0;
        if (s1 == s3) {
            return false;
        }

        bool s2 = m_points[1].sign(m_points[0], m_points[2]) > 0;
        bool s4 = m_points[3].sign(m_points[0], m_points[2]) > 0;
        if (s2 == s4) {
            return false;
        }

        return true;
    }


    bool Quadrilateral::isConvex() const noexcept {

        bool has_positive = false;
        bool has_negative = false;

        for (int i = 0; i < 4; i++) {

            Vec2d p0 = m_points[i];
            Vec2d p1 = m_points[(i + 1) % 4];
            Vec2d p2 = m_points[(i + 2) % 4];

            // Compute the cross product between vectors (p1 - p0) and (p2 - p1).
            double cross_product_z = (p1.m_x - p0.m_x) * (p2.m_y - p1.m_y) - (p1.m_y - p0.m_y) * (p2.m_x - p1.m_x);

            if (cross_product_z > 0) {
                has_positive = true;
            }
            else if (cross_product_z < 0) {
                has_negative = true;
            }

            if (has_positive && has_negative) {
                // Found both clockwise and counterclockwise turns
                return false; // The quadrilateral is concave.
            }
        }

        return true; // The quadrilateral is convex.
    }


    double Quadrilateral::flattestAngle() const noexcept {

        double max_angle = 0.0;

        for (int32_t i = 0; i < 4; i++) {
            Vec2 v1(m_points[i].m_x - m_points[(i + 1) % 4].m_x, m_points[i].m_y - m_points[(i + 1) % 4].m_y);
            Vec2 v2(m_points[(i + 2) % 4].m_x - m_points[(i + 1) % 4].m_x, m_points[(i + 2) % 4].m_y - m_points[(i + 1) % 4].m_y);
            double angle = 180.0 - v1.angle(v2);
            if (angle > max_angle) {
                max_angle = angle;
            }
        }

        return max_angle;
    }


    void Quadrilateral::setPointAtIndex(int32_t index, const Vec2d& p) noexcept {

        if (index >= 0 && index <= 3) {
            m_points[index] = p;
        }
    }


    void Quadrilateral::set(const Vec2d& p1, const Vec2d& p2, const Vec2d& p3, const Vec2d& p4) noexcept {

        m_points[0] = p1;
        m_points[1] = p2;
        m_points[2] = p3;
        m_points[3] = p4;

        m_valid_points = true;
        solvePerspective();
    }


    void Quadrilateral::setByMinMax(const Vec2d& p_min, const Vec2d& p_max) noexcept {

        set(Vec2d(p_min.m_x, p_min.m_y), Vec2d(p_min.m_x, p_max.m_y), Vec2d(p_max.m_x, p_max.m_y), Vec2d(p_max.m_x, p_min.m_y));
    }


    void Quadrilateral::setByRect(const Rectd& rect) noexcept {

        m_points[0].m_x = rect.m_x;
        m_points[0].m_y = rect.m_y;
        m_points[1].m_x = rect.m_x + rect.m_width;
        m_points[1].m_y = rect.m_y;
        m_points[2].m_x = rect.m_x + rect.m_width;
        m_points[2].m_y = rect.m_y + rect.m_height;
        m_points[3].m_x = rect.m_x;
        m_points[3].m_y = rect.m_y + rect.m_height;

        m_valid_points = true;
        solvePerspective();
    }


    void Quadrilateral::setByRangeRect(const RangeRectd& range_rect) noexcept {

        m_points[0].m_x = range_rect.m_min_x; m_points[0].m_y = range_rect.m_min_y;
        m_points[1].m_x = range_rect.m_max_x; m_points[1].m_y = range_rect.m_min_y;
        m_points[2].m_x = range_rect.m_max_x; m_points[2].m_y = range_rect.m_max_y;
        m_points[3].m_x = range_rect.m_min_x; m_points[3].m_y = range_rect.m_max_y;

        m_valid_points = true;
        solvePerspective();
    }


    /**
     *  @brief Compute the transform coefficients.
     *
     *  Perspective projection of a rectangle (homography) by YvesDaoust.
     */
    bool Quadrilateral::solvePerspective() noexcept {

        m_can_project_perspective = false;

        double x1 = m_points[0].m_x, x2 = m_points[1].m_x, x3 = m_points[2].m_x, x4 = m_points[3].m_x;
        double y1 = m_points[0].m_y, y2 = m_points[1].m_y, y3 = m_points[2].m_y, y4 = m_points[3].m_y;

        double t = (x3 - x2) * (y3 - y4) - (x3 - x4) * (y3 - y2);
        if (std::fabs(t) < std::numeric_limits<double>::epsilon()) {
            return false;
        }

        _m_coef_g = ((x3 - x1) * (y3 - y4) - (x3 - x4) * (y3 - y1)) / t;
        _m_coef_h = ((x3 - x2) * (y3 - y1) - (x3 - x1) * (y3 - y2)) / t;

        _m_coef_a = _m_coef_g * (x2 - x1);
        _m_coef_d = _m_coef_g * (y2 - y1);
        _m_coef_b = _m_coef_h * (x4 - x1);
        _m_coef_e = _m_coef_h * (y4 - y1);

        _m_coef_g -= 1.0;
        _m_coef_h -= 1.0;

        m_can_project_perspective = true;

        return true;
    }


    /**
     *  @brief Evaluate the homographic transform.
     */
    bool Quadrilateral::project(const Vec2d& uv, Vec2d& out_vec) const noexcept {

        // Evaluate the homographic transform.

        double u = uv.m_x;
        double v = uv.m_y;
        double t = _m_coef_g * u + _m_coef_h * v + 1.0;
        out_vec.set((_m_coef_a * u + _m_coef_b * v) / t + m_points[0].m_x, (_m_coef_d * u + _m_coef_e * v) / t + m_points[0].m_y);

        return true;
    }


    bool Quadrilateral::project(double u, double v, Vec2d& out_vec) const noexcept {

        Vec2d uv(u, v);
        return Quadrilateral::project(uv, out_vec);
    }


    /**
     *  @brief Converts physical (x, y) to logical (u, v).
     */
    bool Quadrilateral::map(double x, double y, Vec2d& out_uv) const noexcept {

        // Inverse 4 x 4 matrix of 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0.
        double m[16] = { 1, 0, 0, 0, -1, 1, 0, 0, -1, 0, 0, 1, 1, -1, 1, -1 };

        double x1 = m_points[0].m_x;
        double x2 = m_points[1].m_x;
        double x3 = m_points[2].m_x;
        double x4 = m_points[3].m_x;

        double y1 = m_points[0].m_y;
        double y2 = m_points[1].m_y;
        double y3 = m_points[2].m_y;
        double y4 = m_points[3].m_y;


        // Coefficients
        double a1 = m[0] * x1 + m[1] * x2 + m[2] * x3 + m[3] * x4;
        double a2 = m[4] * x1 + m[5] * x2 + m[6] * x3 + m[7] * x4;
        double a3 = m[8] * x1 + m[9] * x2 + m[10] * x3 + m[11] * x4;
        double a4 = m[12] * x1 + m[13] * x2 + m[14] * x3 + m[15] * x4;

        double b1 = m[0] * y1 + m[1] * y2 + m[2] * y3 + m[3] * y4;
        double b2 = m[4] * y1 + m[5] * y2 + m[6] * y3 + m[7] * y4;
        double b3 = m[8] * y1 + m[9] * y2 + m[10] * y3 + m[11] * y4;
        double b4 = m[12] * y1 + m[13] * y2 + m[14] * y3 + m[15] * y4;

        double aa = a4 * b3 - a3 * b4;
        double bb = a4 * b1 - a1 * b4 + a2 * b3 - a3 * b2 + x * b4 - y * a4;
        double cc = a2 * b1 - a1 * b2 + x * b2 - y * a2;

        double det = std::sqrt(bb * bb - 4 * aa * cc);
        double v = (-bb + det) / (2 * aa);
        double u = (x -a1 - a3 * v) / (a2 + a4 * v);

        out_uv.m_x = u;
        out_uv.m_y = v;

        return true;
    }


    /**
     *  @brief Checks if Quadrilateral contains a given position.
     *
     *  This function determines whether the specified position is inside the quadrilateral
     *  represented by this object. It uses the sign of the area method to check if the point
     *  is on the same side of all the edges of the quadrilateral.
     *
     *  @param pos The position to check.
     *  @return true if the position is inside the quadrilateral, false otherwise.
     */
    bool Quadrilateral::contains(const Vec2d& pos) const noexcept {

        if (!m_valid_points) {
            return false;
        }

        bool s1 = pos.sign(m_points[0], m_points[1]) > 0;
        bool s2 = pos.sign(m_points[1], m_points[2]) > 0;
        bool s3 = pos.sign(m_points[2], m_points[3]) > 0;
        bool s4 = pos.sign(m_points[3], m_points[0]) > 0;

        return s1 == s2 && s1 == s3 && s1 == s4;
    }


    /**
     *  @brief Calculates the centroid of this quadrilateral.
     *
     *  This function computes the centroid of the quadrilateral represented by this object.
     *  It divides the quadrilateral into two triangles and calculates the centroids of
     *  these triangles. The centroid of the quadrilateral is then determined as the midpoint
     *  between the centroids of the two triangles.
     *
     *  @return The centroid of the quadrilateral.
     */
    Vec2d Quadrilateral::centroid() const noexcept {

        // Calculate centroids of two triangles dividing the quadrilateral.
        // The centroid of the quadrilateral is the midpoint between the two triangle centroids.

        Vec2d tri1_centroid;
        Vec2d tri2_centroid;

        tri1_centroid.setToTriangleCentroid(m_points[0], m_points[1], m_points[2]);
        tri2_centroid.setToTriangleCentroid(m_points[2], m_points[3], m_points[0]);

        return (tri1_centroid + tri2_centroid) * 0.5;
    }


    void Quadrilateral::remap(const RemapRectd& remap_rect) noexcept {

        remap_rect.mapVec2(m_points[0]);
        remap_rect.mapVec2(m_points[1]);
        remap_rect.mapVec2(m_points[2]);
        remap_rect.mapVec2(m_points[3]);
    }


} // End of namespace Grain
