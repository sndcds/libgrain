//
//  GraphicPath.hpp
//
//  Created by Roald Christesen on from 22.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrGraphicPath_hpp
#define GrGraphicPath_hpp

#include "Grain.hpp"
#include "Math/Vec2.hpp"
#include "Math/Mat3.hpp"
#include "2d/Rect.hpp"
#include "Type/Object.hpp"
#include "Type/Type.hpp"
#include "2d/GraphicPathPoint.hpp"
#include "Type/List.hpp"


namespace Grain {


    class GraphicContext;
    class GraphicPathPoint;
    class Bezier;
    class Quadrilateral;


    class GraphicPathSplitParam {

    public:
        bool m_valid = false;
        double m_start = 0.0;
        double m_end = 1.0;
        int32_t m_start_index = -1;
        int32_t m_end_index = -1;
        double m_t0 = -1.0;
        double m_t1 = -1.0;
    };


    class GraphicPath : protected Object {

        friend class GraphicPathPoint;
        friend class GraphicCompoundPath;
        friend class GraphicContext;

    protected:
        List<GraphicPathPoint> m_points;
        bool m_closed = false;
        bool m_must_update = false;
        int32_t m_bezier_segment_resolution = 100;
        double m_length = -1.0;


    public:
        GraphicPath(int32_t point_capacity = 32) noexcept;
        ~GraphicPath() noexcept;

        virtual const char *className() const noexcept { return "GraphicPath"; }

        friend std::ostream& operator << (std::ostream &os, const GraphicPath *o) {
            o == nullptr ? os << "GraphicPath nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream &os, const GraphicPath &o) {
            os << "GraphicPath:";
            os << "\npoint count: " << o.pointCount();
            os << "\nclosed: " << Grain::Type::boolToYesNoStr(o.m_closed);
            os << "\nmust update: " << Grain::Type::boolToYesNoStr(o.m_must_update);
            return os;
        }


        void _init(int32_t point_capacity) noexcept;
        void _update() noexcept;
        void _updateLength(int32_t bezier_resolution = 100) noexcept;

        bool hasPoints() const noexcept;
        int32_t pointCount() const noexcept;
        int32_t segmentCount() const noexcept;
        double length() noexcept;
        double polygonCentroid(Vec2d &out_centroid) const noexcept;
        Vec2d simplePolygonCentroid() const noexcept;



        int32_t lastPointIndex() const noexcept { return pointCount() - 1; }
        GraphicPathPoint *pointPtrAtIndex(int32_t index) noexcept;
        GraphicPathPoint *lastPointPtr() noexcept;
        Rectd bounds() noexcept;
        bool bounds(Rectd &out_bounds) noexcept;
        bool bezierAtIndex(int32_t segment_index, Bezier &out_bezier) noexcept;
        static bool bezierFromTwoPathPoints(const GraphicPathPoint *p1, const GraphicPathPoint *p2, Bezier &out_bezier) noexcept;

        void clear() noexcept;

        void addPoint(double x, double y, bool use_left, double lx, double ly, bool use_right, double rx, double ry) noexcept;
        void addPoint(const GraphicPathPoint *point) noexcept;
        void addPoint(const Vec2d &pos) noexcept;
        void addPoint(double x, double y) noexcept;
        void addPoint(const Vec2d &pos, const Vec2d &left, const Vec2d &right) noexcept;
        void addPoint(double x, double y, double lx, double ly, double rx, double ry) noexcept;
        void addPointLeft(const Vec2d &pos, const Vec2d &left) noexcept;
        void addPointLeft(double x, double y, double lx, double ly) noexcept;
        void addPointRight(const Vec2d &pos, const Vec2d &right) noexcept;
        void addPointRight(double x, double y, double rx, double ry) noexcept;
        void addPointByAngle(const Vec2d &pos, double angle, double left_length, double right_length) noexcept;
        void addPointByAngle(double x, double y, double angle, double left_length, double right_length) noexcept;
        void addPointByAngle(const Vec2d &pos, double left_angle, double left_length, double right_angle, double right_length) noexcept;
        void addPointByAngle(double x, double y, double left_angle, double left_length, double right_angle, double right_length) noexcept;
        void addArcAsBezier(const Vec2d &radii, double x_axis_rotation, bool large_arc_flag, bool sweep_flag, const Vec2d &end_pos, int32_t max_segment_n) noexcept;

        void addBezier(const Vec2d &control1_pos, const Vec2d &control2_pos, const Vec2d &end_pos) noexcept;
        void addQuadraticBezier(const Vec2d &control_pos, const Vec2d &end_pos) noexcept;
        void addSmoothBezier(const Vec2d &control2_pos, const Vec2d &end_pos) noexcept;
        void addSmoothQuadraticBezier(const Vec2d &end_pos) noexcept;

        void setLastLeft(const Vec2d &left) noexcept;
        void setLastRight(const Vec2d &right) noexcept;

        bool isClosed() const noexcept { return m_closed; }
        void close() noexcept { m_closed = true; }

        void translatePoint(int32_t index, double tx, double ty) noexcept;
        void rotatePoint(int32_t index, double angle) noexcept;

        void projectToQuadrilateral(const Quadrilateral &quadrilateral, const Mat3d *matrix = nullptr) noexcept;


        void fill(GraphicContext &gc) const noexcept;
        void stroke(GraphicContext &gc) const noexcept;
        void addClip(GraphicContext &gc) const noexcept;

        void split(double start, double end, GraphicPathSplitParam &out_split_param);
    };


} // End of namespace Grain.

#endif /* GrGraphicPath_hpp */
