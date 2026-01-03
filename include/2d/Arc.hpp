//
//  Arc.hpp
//
//  Created by Roald Christesen on from 06.11.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainArc_hpp
#define GrainArc_hpp

#include"Math/Vec2.hpp"


namespace Grain {

class GraphicContext;

class Arc {

public:
    enum class SetMode {
        Undefined = -1,
        ThreePoints = 0,
        CoreGraphics,
        SVG             ///< Elliptical endpoint parameterization
    };

public:
    SetMode set_mode_ = SetMode::Undefined;    ///< Indicates which method was used to set the parameters
    bool valid_ = false;       ///< Indicates, if parameteres describe a valid arc

    Vec2d start_pos_{};        ///< Start point of the arc
    Vec2d end_pos_{};          ///< Start point of the arc
    Vec2d radii_{};            ///< Radius in x and y direction
    double rotation_;          ///< Rotation of the ellipse (always 0 for circular arcs)
    bool large_arc_flag_;      ///< `true` if the arc spans more than 180 degrees, otherwise `false`
    bool clockwise_flag_;      ///< `true` for clockwise, `false` for counterclockwise (aka sweep flag in SVG Arc)

    Vec2d center_{};           ///< The center of the arc
    Vec2d mid_pos_{};          ///< The mid position, if arc was set by three points
    double start_angle_{};
    double end_angle_{};
    double radius_scale_ = 1.0;

public:
    Arc() noexcept = default;
    Arc(const Vec2d& p1, const Vec2d& p2, const Vec2d& p3) noexcept {
        setByThreePoints(p1, p2, p3);
    }

    friend std::ostream& operator << (std::ostream& os, const Arc* o) {
        o == nullptr ? os << "Arc nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const Arc& o) {
        o.log(os, 0, nullptr);
        return os;
    }

    void log(std::ostream& os, int32_t indent = 0, const char* label = nullptr) const;

    bool isValid() const noexcept {
        return std::abs(radii_.x_ - radii_.y_) < std::numeric_limits<double>::epsilon();
    }
    bool isCircle() const noexcept {
        return std::abs(radii_.x_ - radii_.y_) < std::numeric_limits<double>::epsilon();
    }
    Vec2d center() const noexcept { return center_; }
    double radiusX() const noexcept { return radii_.x_; }
    double radiusY() const noexcept { return radii_.y_; }
    double effectiveRadiusX() const noexcept {
        if (set_mode_ == SetMode::SVG) { return radiusX() * radius_scale_; }
        else return radiusX();
    }
    double effectiveRadiusY() const noexcept {
        if (set_mode_ == SetMode::SVG) { return radiusY() * radius_scale_; }
        else return radiusY();
    }
    Vec2d startPos() const noexcept { return start_pos_; }
    double startX() const noexcept { return start_pos_.x_; }
    double startY() const noexcept { return start_pos_.y_; }
    Vec2d midPos() const noexcept { return mid_pos_; }
    double midX() const noexcept { return mid_pos_.x_; }
    double midY() const noexcept { return mid_pos_.y_; }
    Vec2d endPos() const noexcept { return end_pos_; }
    double endX() const noexcept { return end_pos_.x_; }
    double endY() const noexcept { return end_pos_.y_; }
    double rotation() const noexcept { return rotation_; }
    double startAngle() const noexcept { return start_angle_; }
    double endAngle() const noexcept { return end_angle_; }
    bool posAtT(double t, Vec2d& out_pos) const noexcept;
    bool isClockwise() noexcept { return large_arc_flag_; }
    bool isLargeArc() noexcept { return clockwise_flag_; }

    bool setPosAtIndex(int32_t index, const Vec2d& pos) noexcept;

    bool setStartPos(const Vec2d& pos) noexcept;
    bool setStartX(double x) noexcept { return setStartPos(Vec2d(x, start_pos_.y_)); }
    bool setStartY(double y) noexcept { return setStartPos(Vec2d(start_pos_.x_, y)); }
    bool setMidPos(const Vec2d& pos) noexcept;
    bool setMidPosX(double x) noexcept { return setMidPos(Vec2d(x, mid_pos_.y_)); }
    bool setMidPosY(double y) noexcept { return setMidPos(Vec2d(mid_pos_.x_, y)); }
    bool setEndPos(const Vec2d& pos) noexcept;
    bool setEndX(double x) noexcept { return setEndPos(Vec2d(x, end_pos_.y_)); }
    bool setEndY(double y) noexcept { return setEndPos(Vec2d(end_pos_.x_, y)); }
    bool setRadius(double radius) noexcept;
    bool setRadiusX(double rx) noexcept;
    bool setRadiusY(double ry) noexcept;
    bool setRotation(double rotation) noexcept;
    bool setStartAngle(double start_angle) noexcept;
    bool setEndAngle(double end_angle) noexcept;
    void setClockwiseFlag(bool large_arc_flag) noexcept {
        large_arc_flag_ = large_arc_flag;
        _svgUpdateCenter();
    }
    void setLargeArcFlag(bool clockwise_flag) noexcept {
        clockwise_flag_ = clockwise_flag;
        _svgUpdateCenter();
    }

    bool setByThreePoints(const Vec2d& start_pos, const Vec2d& mid_pos, const Vec2d& end_pos) noexcept;
    bool setCoreGraphics(const Vec2d& center, double radius, double start_angle, double end_angle, bool clockwise_flag) noexcept;
    bool setSVG(const Vec2d& start_pos, const Vec2d& end_pos, const Vec2d& radii, double rotation, bool large_arc_flag, bool clockwise_flag) noexcept;

    void fill(GraphicContext& gc) const noexcept;
    void stroke(GraphicContext& gc) const noexcept;
    void addClip(GraphicContext& gc) const noexcept;

    void svgCode(String& out_code, int32_t precision = 2) const noexcept;

    void _svgUpdateCenter() noexcept;
};

}

#endif //GrainArc_hpp