//
//  Quadrilateral.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainQuadrilateral_hpp
#define GrainQuadrilateral_hpp

#include "Grain.hpp"
#include "Math/Vec2.hpp"
#include "2d/Line.hpp"
#include "2d/Rect.hpp"
#include "RangeRect.hpp"


namespace Grain {

/**
 *  @brief The Quadrilateral class represents a four-sided polygon defined by
 *  four points in a 2D space.
 *
 *  It is designed to facilitate operations related to
 *  perspective transformations and geometric calculations, making it
 *  particularly useful in graphics programming, game development, and
 *  geometric computing.
 *
 *  Use Cases:
 *  - Perspective Grid Line Creation:
 *    Generating grid lines for art and design applications, simulating
 *    perspective.
 *  - Perspective Projection:
 *    Calculating the projection of objects in a 3D space onto a 2D plane,
 *    considering a viewer's perspective.
 *  - Point Inclusion Test:
 *    Determining whether a given point lies inside the quadrilateral, useful
 *    for hit-testing in graphics applications and spatial analysis.
 */
class Quadrilateral {

    friend class GraphicContext;
    friend class GeoProj;

protected:
    Vec2d points_[4] {
            Vec2d(-1.0, -1.0),
            Vec2d(1.0, -1.0),
            Vec2d(1.0, 1.0),
            Vec2d(-1.0, 1.0)
    };

    bool valid_points_ = false;
    bool can_project_perspective_ = false;

    double coef_a_{};
    double coef_b_{};
    double coef_d_{};
    double coef_e_{};
    double coef_g_{};
    double coef_h_{};


public:
    Quadrilateral() noexcept = default;
    explicit Quadrilateral(const Vec2d& p1, const Vec2d& p2, const Vec2d& p3, const Vec2d& p4) noexcept {
        set(p1, p2, p3, p4);
    }
    explicit Quadrilateral(const Vec2d& p_min, const Vec2d& p_max) noexcept {
        setByMinMax(p_min, p_max);
    }
    explicit Quadrilateral(const Rectd& rect) noexcept { setByRect(rect); }
    explicit Quadrilateral(const RangeRectd& range_rect) noexcept { setByRangeRect(range_rect); }

    virtual ~Quadrilateral() noexcept = default;

    [[nodiscard]] virtual const char* className() const noexcept {
        return "Quadrilateral";
    }


    friend std::ostream& operator << (std::ostream& os, const Quadrilateral* o) {
        o == nullptr ? os << "nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const Quadrilateral& o) {
        os << o.points_[0] << " .. ";
        os << o.points_[1] << " .. ";
        os << o.points_[2] << " .. ";
        os << o.points_[3];
        return os;
    }


    Vec2d p1() noexcept { return points_[0]; }
    Vec2d p2() noexcept { return points_[1]; }
    Vec2d p3() noexcept { return points_[2]; }
    Vec2d p4() noexcept { return points_[3]; }

    void pointsToArray(Vec2d* out_points) noexcept {
        if (out_points != nullptr) {
            for (int32_t i = 0; i < 4; i++) {
                out_points[i] = points_[i];
            }
        }
    }

    [[nodiscard]] const Vec2d* pointsPtr() const noexcept { return points_; }
    [[nodiscard]] Vec2d* mutPointsPtr() noexcept { return points_; }

    [[nodiscard]] RangeRectd axisAlignedBbox() const noexcept {
        RangeRectd bbox;
        bbox.initForMinMaxSearch();
        bbox.add(points_[0]);
        bbox.add(points_[1]);
        bbox.add(points_[2]);
        bbox.add(points_[3]);
        return bbox;
    }

    bool horizontalLine(double v, Lined& out_line) const noexcept {
        project(0.0, v, out_line.p1_);
        project(1.0, v, out_line.p2_);
        return true;
    }

    bool verticalLine(double u, Lined& out_line) const noexcept {
        project(u, 0.0, out_line.p1_);
        project(u, 1.0, out_line.p2_);
        return true;
    }

    bool bezierCirclePoints(Vec2d* out_points) const noexcept {
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

    [[nodiscard]] double area() const noexcept {
        double p = points_[0].distance(points_[2]);
        double q = points_[1].distance(points_[3]);
        double a = points_[0].distance(points_[1]);
        double b = points_[1].distance(points_[2]);
        double c = points_[2].distance(points_[3]);
        double d = points_[3].distance(points_[0]);
        double m = b * b + d * d - a * a - c * c;
        return 0.25 * std::sqrt(4 * p * p * q * q - m * m);
    }

    [[nodiscard]] bool isSimple() const noexcept {
        if (!valid_points_) {
            return false;
        }
        bool s1 = points_[0].sign(points_[1], points_[3]) > 0;
        bool s3 = points_[2].sign(points_[1], points_[3]) > 0;
        if (s1 == s3) {
            return false;
        }
        bool s2 = points_[1].sign(points_[0], points_[2]) > 0;
        bool s4 = points_[3].sign(points_[0], points_[2]) > 0;
        if (s2 == s4) {
            return false;
        }
        return true;
    }

    [[nodiscard]] bool isConvex() const noexcept {
        bool has_positive = false;
        bool has_negative = false;
        for (int i = 0; i < 4; i++) {
            auto p0 = &points_[i];
            auto p1 = &points_[(i + 1) % 4];
            auto p2 = &points_[(i + 2) % 4];
            // Compute the cross product between vectors (p1 - p0) and (p2 - p1)
            double cross_product_z = (p1->x_ - p0->x_) * (p2->y_ - p1->y_) - (p1->y_ - p0->y_) * (p2->x_ - p1->x_);
            if (cross_product_z > 0) {
                has_positive = true;
            }
            else if (cross_product_z < 0) {
                has_negative = true;
            }
            if (has_positive && has_negative) {
                // Found both clockwise and counterclockwise turns
                return false; // The quadrilateral is concave
            }
        }
        return true; // The quadrilateral is convex
    }

    [[nodiscard]] double flattestAngle() const noexcept {
        double max_angle = 0.0;
        for (int32_t i = 0; i < 4; i++) {
            Vec2d v1(points_[i].x_ - points_[(i + 1) % 4].x_, points_[i].y_ - points_[(i + 1) % 4].y_);
            Vec2d v2(points_[(i + 2) % 4].x_ - points_[(i + 1) % 4].x_, points_[(i + 2) % 4].y_ - points_[(i + 1) % 4].y_);
            double angle = 180.0 - v1.angle(v2);
            if (angle > max_angle) {
                max_angle = angle;
            }
        }
        return max_angle;
    }

    [[nodiscard]] bool canProjectPerspective() const noexcept {
        return can_project_perspective_;
    }

    void set(const Vec2d& p1, const Vec2d& p2, const Vec2d& p3, const Vec2d& p4) noexcept {
        points_[0] = p1;
        points_[1] = p2;
        points_[2] = p3;
        points_[3] = p4;
        valid_points_ = true;
        solvePerspective();
    }

    void setPointAtIndex(int32_t index, const Vec2d& p) noexcept {
        if (index >= 0 && index <= 3) {
            points_[index] = p;
        }
    }

    void setByMinMax(const Vec2d& p_min, const Vec2d& p_max) noexcept {
        set(Vec2d(p_min.x_, p_min.y_), Vec2d(p_min.x_, p_max.y_), Vec2d(p_max.x_, p_max.y_), Vec2d(p_max.x_, p_min.y_));
    }

    void setByRect(const Rectd& rect) noexcept {
        points_[0].x_ = rect.x_;
        points_[0].y_ = rect.y_;
        points_[1].x_ = rect.x_ + rect.width_;
        points_[1].y_ = rect.y_;
        points_[2].x_ = rect.x_ + rect.width_;
        points_[2].y_ = rect.y_ + rect.height_;
        points_[3].x_ = rect.x_;
        points_[3].y_ = rect.y_ + rect.height_;
        valid_points_ = true;
        solvePerspective();
    }

    void setByRangeRect(const RangeRectd& range_rect) noexcept {
        points_[0].x_ = range_rect.min_x_; points_[0].y_ = range_rect.min_y_;
        points_[1].x_ = range_rect.max_x_; points_[1].y_ = range_rect.min_y_;
        points_[2].x_ = range_rect.max_x_; points_[2].y_ = range_rect.max_y_;
        points_[3].x_ = range_rect.min_x_; points_[3].y_ = range_rect.max_y_;
        valid_points_ = true;
        solvePerspective();
    }

    bool project(Vec2d& vec) const noexcept {
        return project(vec, vec);
    }

    /**
     *  @brief Evaluate the homographic transform.
     */
    bool project(const Vec2d& uv, Vec2d& out_vec) const noexcept {
        // Evaluate the homographic transform
        double u = uv.x_;
        double v = uv.y_;
        double t = coef_g_ * u + coef_h_ * v + 1.0;
        out_vec.set((coef_a_ * u + coef_b_ * v) / t + points_[0].x_, (coef_d_ * u + coef_e_ * v) / t + points_[0].y_);
        return true;
    }

    bool projectPoints(int32_t count, Vec2d* points) const noexcept {
        if (points != nullptr) {
            for (int32_t i = 0; i < count; i++) {
                project(points[i]);
            }
            return true;
        }
        else {
            return false;
        }
    }

    bool project(double u, double v, Vec2d& out_vec) const noexcept {
        return Quadrilateral::project({ u, v }, out_vec);
    }

    /**
     *  @brief Converts physical (x, y) to logical (u, v).
     */
    bool map(double x, double y, Vec2d& out_uv) const noexcept {
        // Inverse 4 x 4 matrix of 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0.
        double m[16] = { 1, 0, 0, 0, -1, 1, 0, 0, -1, 0, 0, 1, 1, -1, 1, -1 };

        double x1 = points_[0].x_;
        double x2 = points_[1].x_;
        double x3 = points_[2].x_;
        double x4 = points_[3].x_;

        double y1 = points_[0].y_;
        double y2 = points_[1].y_;
        double y3 = points_[2].y_;
        double y4 = points_[3].y_;


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

        out_uv.x_ = u;
        out_uv.y_ = v;

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
    [[nodiscard]] bool contains(const Vec2d& pos) const noexcept {
        if (!valid_points_) {
            return false;
        }
        bool s1 = pos.sign(points_[0], points_[1]) > 0;
        bool s2 = pos.sign(points_[1], points_[2]) > 0;
        bool s3 = pos.sign(points_[2], points_[3]) > 0;
        bool s4 = pos.sign(points_[3], points_[0]) > 0;
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
    [[nodiscard]] Vec2d centroid() const noexcept {
        Vec2d tri1_centroid;
        Vec2d tri2_centroid;
        tri1_centroid.setToTriangleCentroid(points_[0], points_[1], points_[2]);
        tri2_centroid.setToTriangleCentroid(points_[2], points_[3], points_[0]);
        return (tri1_centroid + tri2_centroid) * 0.5;
    }

    void remap(const RemapRectd& remap_rect) noexcept {
        remap_rect.mapVec2(points_[0]);
        remap_rect.mapVec2(points_[1]);
        remap_rect.mapVec2(points_[2]);
        remap_rect.mapVec2(points_[3]);
    }

    /**
     *  @brief Compute the transform coefficients.
     *
     *  Perspective projection of a rectangle (homography) by YvesDaoust.
     */
    bool solvePerspective() noexcept {
        can_project_perspective_ = false;

        double x1 = points_[0].x_, x2 = points_[1].x_, x3 = points_[2].x_, x4 = points_[3].x_;
        double y1 = points_[0].y_, y2 = points_[1].y_, y3 = points_[2].y_, y4 = points_[3].y_;

        double t = (x3 - x2) * (y3 - y4) - (x3 - x4) * (y3 - y2);
        if (std::fabs(t) < std::numeric_limits<double>::epsilon()) {
            return false;
        }

        coef_g_ = ((x3 - x1) * (y3 - y4) - (x3 - x4) * (y3 - y1)) / t;
        coef_h_ = ((x3 - x2) * (y3 - y1) - (x3 - x1) * (y3 - y2)) / t;

        coef_a_ = coef_g_ * (x2 - x1);
        coef_d_ = coef_g_ * (y2 - y1);
        coef_b_ = coef_h_ * (x4 - x1);
        coef_e_ = coef_h_ * (y4 - y1);

        coef_g_ -= 1.0;
        coef_h_ -= 1.0;

        can_project_perspective_ = true;

        return true;
    }
};


} // End of namespace Grain

#endif // GrainQuadrilateral_hpp
