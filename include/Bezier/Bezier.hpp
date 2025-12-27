//
//  Bezier.hpp
//
//  Created by Roald Christesen on 07.11.2013
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

//  https://pomax.github.io/bezierinfo/

#ifndef GrainBezier_hpp
#define GrainBezier_hpp

#include "Grain.hpp"
#include "Math/Vec2.hpp"
#include "2d/Rect.hpp"
#include "2d/RangeRect.hpp"


namespace Grain {

class BezierValueCurvePoint;

class Bezier {
public:
    enum class PointType {
        Undefined = -1,
        Linear = 0,
        Corner,
        Smooth1,
        Smooth2,
        Right,
        Left
    };


public:
    Vec2d pos_[4]{};

public:
    Bezier() noexcept = default;
    explicit Bezier(double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3) noexcept;
    explicit Bezier(const Vec2d& p0, const Vec2d& p1, const Vec2d& p2, const Vec2d& p3) noexcept;
    explicit Bezier(const Vec2d& p0, const Vec2d& p1, const Vec2d& p2) noexcept;
    explicit Bezier(const Vec2d *pos_array) noexcept;
    explicit Bezier(const BezierValueCurvePoint& p0, const BezierValueCurvePoint& p1) noexcept;

    virtual ~Bezier() noexcept = default;

    [[nodiscard]] virtual const char *className() const noexcept { return "Bezier"; }

    friend std::ostream& operator << (std::ostream& os, const Bezier *o) {
        o == nullptr ? os << "Bezier nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const Bezier& o) {
        os << o.pos_[0].x_ << ", " << o.pos_[0].y_ << " .. ";
        os << o.pos_[1].x_ << ", " << o.pos_[1].y_ << " .. ";
        os << o.pos_[2].x_ << ", " << o.pos_[2].y_ << " .. ";
        os << o.pos_[3].x_ << ", " << o.pos_[3].y_;
        return os;
    }

    [[nodiscard]] Rectd bounds() const noexcept;
    [[nodiscard]] Vec2d startPos() const noexcept { return pos_[0]; }
    [[nodiscard]] Vec2d controlPos1() const noexcept { return pos_[1]; }
    [[nodiscard]] Vec2d controlPos2() const noexcept { return pos_[2]; }
    [[nodiscard]] Vec2d endPos() const noexcept { return pos_[3]; }
    [[nodiscard]] Vec2d posAtPointIndex(int32_t index) const noexcept;
    [[nodiscard]] Vec2d posOnCurve(double t) const noexcept;

    [[nodiscard]] double approximatedCurveLength(int32_t resolution = 100) const noexcept;
    [[nodiscard]] Vec2d tangent1() const noexcept { return (pos_[1] - pos_[0]); }
    [[nodiscard]] Vec2d tangent2() const noexcept { return (pos_[2] - pos_[3]); }

    void set(const Vec2d& p0, const Vec2d& p1, const Vec2d& p2, const Vec2d& p3) noexcept;
    void setQuadratic(const Vec2d& p0, const Vec2d& p1, const Vec2d& p2) noexcept;
    void set(double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3) noexcept;
    void setPointAtIndex(int32_t index, const Vec2d& p) noexcept;
    void setHorizontalSegment(const Vec2d& p_left, const Vec2d& p_right, const Vec2d& left_f, const Vec2d& right_f) noexcept;
    void setWithTangents(const Vec2d& p0, const Vec2d& p3, const Vec2d& t1, const Vec2d& t2) noexcept;

    [[nodiscard]] double hit(const Vec2d& pos, double radius = 6.0) const noexcept;
    [[nodiscard]] bool hitBounds(const Vec2d& pos, double radius = 6.0) const noexcept;
    [[nodiscard]] int32_t hitPoint(const Vec2d& pos, double radius = 6.0) const noexcept;

    // double hit(const Viewport& viewport, const Vec2d& pos, double radius) const noexcept;

    bool split(double t, Bezier& out_bezier1, Bezier& out_bezier2) const noexcept;
    [[nodiscard]] bool truncate(double t_start, double t_end, Bezier& out_bezier) const noexcept;

    void buildVec2LUT(Vec2d *lut, int32_t resolution) const noexcept;

    void translate(double tx, double ty) noexcept;
    void translate(const Vec2d& tv) noexcept;
    void translateX(double tx) noexcept;
    void translateY(double ty) noexcept;

    void scale(double sx, double sy) noexcept;
    void scale(const Vec2d& sv) noexcept;
    void scaleX(double sx) noexcept;
    void scaleY(double sy) noexcept;

    void transformByRect(const Rectd& rect) noexcept;

    // void toViewport(const Viewport& viewport) noexcept;
    // void fromViewport(const Viewport& viewport) noexcept;

    // void stroke(GraphicContext& gc) const noexcept;

    void approximateQuadraticBezierControlPos(Vec2d& out_control_pos) const noexcept;

    [[nodiscard]] static int32_t arcToBezierPosArray(const Vec2d& start_pos, const Vec2d& radii, double rotation, bool large_arc_flag, bool sweep_flag, const Vec2d& end_pos, int32_t max_segment_n, Vec2d *out_pos_array) noexcept;

    [[nodiscard]] static double _bounds_f(double t, double p0, double p1, double p2, double p3);

    // Evaluate Bernstein basis
    [[nodiscard]] inline double evalBernsteinBasis0(double t) { return (1 - t) * (1 - t) * (1 - t); }
    [[nodiscard]] inline double evalBernsteinBasis1(double t) { return 3 * (1 - t) * (1 - t) * t; }
    [[nodiscard]] inline double evalBernsteinBasis2(double t) { return 3 * (1 - t) * t * t; }
    [[nodiscard]] inline double evalBernsteinBasis3(double t) { return t * t * t; }

    [[nodiscard]] ErrorCode fitCubicBezierToPoints(int32_t point_count, const Vec2d *points) noexcept;
};


}  // End of namespace Grain.

#endif // GrainBezier_hpp
