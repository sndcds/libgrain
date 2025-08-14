//
//  GraphicPathPoint.hpp
//
//  Created by Roald Christesen on from 22.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//
#ifndef GrainGraphicPathPoint_hpp
#define GrainGraphicPathPoint_hpp

#include "Grain.hpp"
#include "Math/Vec2.hpp"
#include "Math/Mat3.hpp"


namespace Grain {

    class Quadrilateral;

    class GraphicPathPoint {
        friend class GraphicPath;
        friend class GraphicCompoundPath;
        friend class GraphicContext;

    protected:
        Vec2d m_anchor;
        Vec2d m_left;
        Vec2d m_right;
        bool m_left_flag = false;
        bool m_right_flag = false;
        double m_bezier_segment_length = -1.0;

    public:
        GraphicPathPoint() noexcept {}
        GraphicPathPoint(const GraphicPathPoint& point) noexcept;
        GraphicPathPoint(GraphicPathPoint&& other) noexcept;

        GraphicPathPoint(double x, double y) noexcept;
        GraphicPathPoint(double x, double y, double lx, double ly, double rx, double ry) noexcept;
        GraphicPathPoint(double x, double y, bool left_flag, double lx, double ly, bool right_flag, double rx, double ry) noexcept;
        GraphicPathPoint(const Vec2d& anchor, bool left_flag, const Vec2d& left, bool rightFlag, const Vec2d& right) noexcept;

        // Destructor
        ~GraphicPathPoint();


        friend std::ostream& operator << (std::ostream& os, const GraphicPathPoint* o) {
            o == nullptr ? os << "nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const GraphicPathPoint& o) {
            os << "anhor: " << o.m_anchor;
            if (o.m_left_flag) {
                os << ", left: " << o.m_left;
            }
            if (o.m_right_flag) {
                os << ", right: " << o.m_right;
            }
            if (o.m_bezier_segment_length > 0.0) {
                os << ", length: " << o.m_bezier_segment_length;
            }
            return os;
        }


        // Copy assignment operator.
        GraphicPathPoint& operator = (const GraphicPathPoint& point) noexcept;

        // Move assignment operator (C++11 and later).
        GraphicPathPoint& operator = (GraphicPathPoint&& point) noexcept;


        Vec2d anchor() const noexcept { return m_anchor; }
        Vec2d left() const noexcept { return m_left; }
        Vec2d right() const noexcept { return m_right; }

        bool usesLeft() noexcept { return m_left_flag; }
        bool usesRight() noexcept { return m_right_flag; }
        bool isCorner() noexcept { return !m_left_flag && !m_right_flag; }

        void setAnchor(const Vec2d& anchor) noexcept { m_anchor = anchor; m_bezier_segment_length = -1.0; }
        void setAnchor(double x, double y) noexcept { m_anchor.m_x = x; m_anchor.m_y = y; m_bezier_segment_length = -1.0; }
        void setLeft(const Vec2d& left) noexcept { m_left = left; m_left_flag = true; m_bezier_segment_length = -1.0; }
        void setLeft(double x, double y) noexcept { m_left.m_x = x; m_left.m_y = y; m_left_flag = true; m_bezier_segment_length = -1.0; }
        void setRight(const Vec2d& right) noexcept { m_right = right; m_right_flag = true; m_bezier_segment_length = -1.0; }
        void setRight(double x, double y) noexcept { m_right.m_x = x; m_right.m_y = y; m_right_flag = true; m_bezier_segment_length = -1.0; }

        void translate(const Vec2d& t) noexcept;
        void translate(double tx, double ty) noexcept;
        void rotate(double angle) noexcept;

        void projectToQuadrilateral(const Quadrilateral& quadrilateral, const Mat3d* matrix = nullptr) noexcept;
    };


} // End of namespace Grain

#endif // GrainGraphicPathPoint_hpp
