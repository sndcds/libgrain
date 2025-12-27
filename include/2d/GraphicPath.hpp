//
//  GraphicPath.hpp
//
//  Created by Roald Christesen on from 22.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrGraphicPath_hpp
#define GrGraphicPath_hpp

#include "Math/Vec2.hpp"
#include "Math/Mat3.hpp"
#include "2d/Rect.hpp"
#include "2d/GraphicPathPoint.hpp"
#include "2d/Quadrilateral.hpp"
#include "Type/Object.hpp"
#include "Type/Type.hpp"
#include "Type/List.hpp"


namespace Grain {

class GraphicContext;
class GraphicPathPoint;
class Bezier;


class GraphicPathSplitParam {

public:
    bool valid_ = false;
    double start_ = 0.0;
    double end_ = 1.0;
    int32_t start_index_ = -1;
    int32_t end_index_ = -1;
    double t0_ = -1.0;
    double t1_ = -1.0;
};


class GraphicPath : protected Object {

    friend class GraphicPathPoint;
    friend class GraphicCompoundPath;
    friend class GraphicContext;

protected:
    List<GraphicPathPoint> points_;
    bool closed_ = false;
    bool must_update_ = false;
    int32_t bezier_segment_resolution_ = 100;
    double length_ = -1.0;

public:
    explicit GraphicPath(int32_t point_capacity = 32) noexcept;
    ~GraphicPath() noexcept override;

    [[nodiscard]] const char* className() const noexcept override {
        return "GraphicPath";
    }

    friend std::ostream& operator << (std::ostream& os, const GraphicPath* o) {
        o == nullptr ? os << "GraphicPath nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const GraphicPath& o) {
        os << "GraphicPath:";
        os << "\npoint count: " << o.pointCount();
        os << "\nclosed: " << Grain::Type::boolToYesNoStr(o.closed_);
        os << "\nmust update: " << Grain::Type::boolToYesNoStr(o.must_update_);
        return os;
    }


    void _init(int32_t point_capacity) noexcept;
    void _update() noexcept;
    void _updateLength(int32_t bezier_resolution = 100) noexcept;

    [[nodiscard]] bool hasPoints() const noexcept;
    [[nodiscard]] int32_t pointCount() const noexcept;
    [[nodiscard]] int32_t segmentCount() const noexcept;
    double length() noexcept;
    double polygonCentroid(Vec2d& out_centroid) const noexcept;
    [[nodiscard]] Vec2d simplePolygonCentroid() const noexcept;



    [[nodiscard]] int32_t lastPointIndex() const noexcept { return pointCount() - 1; }
    GraphicPathPoint* pointPtrAtIndex(int32_t index) noexcept;
    GraphicPathPoint* lastPointPtr() noexcept;
    Rectd bounds() noexcept;
    bool bounds(Rectd& out_bounds) noexcept;
    bool bezierAtIndex(int32_t segment_index, Bezier& out_bezier) noexcept;
    static bool bezierFromTwoPathPoints(const GraphicPathPoint* p1, const GraphicPathPoint* p2, Bezier& out_bezier) noexcept;

    void clear() noexcept;

    void addPoint(double x, double y, bool use_left, double lx, double ly, bool use_right, double rx, double ry) noexcept;
    void addPoint(const GraphicPathPoint* point) noexcept;
    void addPoint(const Vec2d& pos) noexcept;
    void addPoint(double x, double y) noexcept;
    void addPoint(const Vec2d& pos, const Vec2d& left, const Vec2d& right) noexcept;
    void addPoint(double x, double y, double lx, double ly, double rx, double ry) noexcept;
    void addPointLeft(const Vec2d& pos, const Vec2d& left) noexcept;
    void addPointLeft(double x, double y, double lx, double ly) noexcept;
    void addPointRight(const Vec2d& pos, const Vec2d& right) noexcept;
    void addPointRight(double x, double y, double rx, double ry) noexcept;
    void addPointByAngle(const Vec2d& pos, double angle, double left_length, double right_length) noexcept;
    void addPointByAngle(double x, double y, double angle, double left_length, double right_length) noexcept;
    void addPointByAngle(const Vec2d& pos, double left_angle, double left_length, double right_angle, double right_length) noexcept;
    void addPointByAngle(double x, double y, double left_angle, double left_length, double right_angle, double right_length) noexcept;
    void addArcAsBezier(const Vec2d& radii, double x_axis_rotation, bool large_arc_flag, bool sweep_flag, const Vec2d& end_pos, int32_t max_segment_n) noexcept;

    void addBezier(const Vec2d& control1_pos, const Vec2d& control2_pos, const Vec2d& end_pos) noexcept;
    void addQuadraticBezier(const Vec2d& control_pos, const Vec2d& end_pos) noexcept;
    void addSmoothBezier(const Vec2d& control2_pos, const Vec2d& end_pos) noexcept;
    void addSmoothQuadraticBezier(const Vec2d& end_pos) noexcept;

    void setLastLeft(const Vec2d& left) noexcept;
    void setLastRight(const Vec2d& right) noexcept;

    [[nodiscard]] bool isClosed() const noexcept { return closed_; }
    void close() noexcept { closed_ = true; }

    void translatePoint(int32_t index, double tx, double ty) noexcept;
    void rotatePoint(int32_t index, double angle) noexcept;

    void projectToQuadrilateral(const Quadrilateral& quadrilateral, const Mat3d* matrix = nullptr) noexcept;


    void fill(GraphicContext& gc) const noexcept;
    void stroke(GraphicContext& gc) const noexcept;
    void addClip(GraphicContext& gc) const noexcept;

    void split(double start, double end, GraphicPathSplitParam& out_split_param);
};


} // End of namespace Grain

#endif // GrGraphicPath_hpp
