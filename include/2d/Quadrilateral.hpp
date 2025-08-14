//
//  Quadrilateral.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
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

     *  -  Perspective Grid Line Creation:
     *     Generating grid lines for art and design applications, simulating perspective.

     *  -  Perspective Projection:
     *     Calculating the projection of objects in a 3D space onto a 2D plane,
     *     considering a viewer's perspective.

     *  -  Point Inclusion Test:
     *     Determining whether a given point lies inside the quadrilateral, useful
     *     for hit-testing in graphics applications and spatial analysis.
     */
    class Quadrilateral {

        friend class GraphicContext;
        friend class GeoProj;

    protected:
        Vec2d m_points[4] { Vec2d(-1.0, -1.0), Vec2d(1.0, -1.0), Vec2d(1.0, 1.0), Vec2d(-1.0, 1.0) };

        bool m_valid_points = false;
        bool m_can_project_perspective = false;

        double _m_coef_a = 0.0;
        double _m_coef_b = 0.0;
        double _m_coef_d = 0.0;
        double _m_coef_e = 0.0;
        double _m_coef_g = 0.0;
        double _m_coef_h = 0.0;


    public:
        Quadrilateral() noexcept {}
        Quadrilateral(const Vec2d& p1, const Vec2d& p2, const Vec2d& p3, const Vec2d& p4) noexcept {
            set(p1, p2, p3, p4);
        }
        Quadrilateral(const Vec2d& p_min, const Vec2d& p_max) noexcept {
            setByMinMax(p_min, p_max);
        }
        Quadrilateral(const Rectd& rect) noexcept { setByRect(rect); }
        Quadrilateral(const RangeRectd& range_rect) noexcept { setByRangeRect(range_rect); }

        ~Quadrilateral() noexcept {}

        const char* className() const noexcept { return "Quadrilateral"; }


        friend std::ostream& operator << (std::ostream& os, const Quadrilateral* o) {
            o == nullptr ? os << "nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const Quadrilateral& o) {
            os << o.m_points[0] << '|';
            os << o.m_points[1] << '|';
            os << o.m_points[2] << '|';
            os << o.m_points[3];
            return os;
        }


        Vec2d p1() noexcept { return m_points[0]; }
        Vec2d p2() noexcept { return m_points[1]; }
        Vec2d p3() noexcept { return m_points[2]; }
        Vec2d p4() noexcept { return m_points[3]; }
        void pointsToArray(Vec2d* out_points) noexcept {
            if (out_points != nullptr) {
                for (int32_t i = 0; i < 4; i++) {
                    out_points[i] = m_points[i];
                }
            }
        }

        const Vec2d* pointsPtr() const noexcept { return m_points; }
        Vec2d* mutPointsPtr() noexcept { return m_points; }

        RangeRectd axisAlignedBbox() const noexcept;

        bool horizontalLine(double v, Lined& out_line) noexcept;
        bool verticalLine(double u, Lined& out_line) noexcept;
        bool bezierCirclePoints(Vec2d* out_points) noexcept;
        double area() const noexcept;

        bool isSimple() const noexcept;
        bool isConvex() const noexcept;
        double flattestAngle() const noexcept;


        bool canProjectPerspective() const noexcept { return m_can_project_perspective; }

        void set(const Vec2d& p1, const Vec2d& p2, const Vec2d& p3, const Vec2d& p4) noexcept;
        void setPointAtIndex(int32_t index, const Vec2d& p) noexcept;
        void setByMinMax(const Vec2d& p_min, const Vec2d& p_max) noexcept;
        void setByRect(const Rectd& rect) noexcept;
        void setByRangeRect(const RangeRectd& range_rect) noexcept;

        bool project(Vec2d& vec) const noexcept { return project(vec, vec); }
        bool project(const Vec2d& uv, Vec2d& out_vec) const noexcept;
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

        bool project(double u, double v, Vec2d& out_vec) const noexcept;

        bool map(double x, double y, Vec2d& out_uv) const noexcept;

        bool contains(const Vec2d& pos) const noexcept;

        Vec2d centroid() const noexcept;

        void remap(const RemapRectd& remap_rect) noexcept;

        bool solvePerspective() noexcept;
    };


} // End of namespace Grain

#endif // GrainQuadrilateral_hpp
