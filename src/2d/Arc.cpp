//
//  Arc.cpp
//
//  Created by Roald Christesen on from 06.11.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Grain.hpp"
#include "2d/Arc.hpp"
#include "Math/Vec2.hpp"
#include "String/String.hpp"
#include "Bezier/Bezier.hpp"
#include "Graphic/GraphicContext.hpp"
#include "Core/Log.hpp"


namespace Grain {

void Arc::log(std::ostream& os, int32_t indent, const char* label) const {
    Log l(os);
    l.header(label);
    l << "m_set_mode: " << (int32_t)set_mode_ << l.endl;
    l << "m_start_pos: " << start_pos_ << l.endl;
    l << "m_end_pos: " << end_pos_ << l.endl;
    l << "m_radii: " << radii_ << l.endl;
    l << "m_rotation: " << rotation_ << l.endl;
    l << "m_large_arc_flag: " << large_arc_flag_ << l.endl;
    l << "m_clockwise_flag: " << clockwise_flag_ << l.endl;
    l << "isValid(): " << isValid() << l.endl;
    l << "isCircle(): " << isCircle() << l.endl;
}


/**
 *  @brief Sets a specific position on the arc at a given index.
 *
 *  This function sets one of the three defining positions of the arc (start pos, middle control pos, or end pos)
 *  based on the provided index. The arc is defined by three positions, where:
 *  - Index 0 corresponds to the start position (`m_start_pos`).
 *  - Index 1 corresponds to the middle control position (`m_mid_pos`).
 *  - Index 2 corresponds to the end position (`m_end_pos`).
 *
 *  @param index An integer specifying which position to set (0, 1, or 2).
 *  @param pos A `Vec2d` object representing the coordinates of the point to be set.
 *  @return `true` if point could be set, `false` otherwise.
 */
bool Arc::setPosAtIndex(int32_t index, const Vec2d& pos) noexcept {
    switch (index) {
        case 0: return setStartPos(pos);
        case 1: return setMidPos(pos);
        case 2: return setEndPos(pos);
        default: return false;
     }
}


/**
 *  @brief Get a position on the arc.
 *
 *  @param t A parameter between 0 and 1 that specifies the position on the arc:
 *           - 0 means the start point of the arc.
 *           - 1 means the end point of the arc.
 *           - 0.5 gives the midpoint of the arc.
 */
bool Arc::posAtT(double t, Vec2d& out_pos) const noexcept {
    if (radii_.x_ == 0.0 ||
        radii_.y_ == 0.0 ||
        (start_pos_.distance(end_pos_) < std::numeric_limits<double>::epsilon())) {
        return false;
    }

    t = std::clamp<double>(t, 0.0, 1.0);

    // Apply transformations to center the arc
    double rx = std::abs(radii_.x_);
    double ry = std::abs(radii_.y_);

    double dx = (start_pos_.x_ - end_pos_.x_) / 2.0;
    double dy = (start_pos_.y_ - end_pos_.y_) / 2.0;

    double rotation_rad = rotation_ * M_PI / 180.0;
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

    // Calculate the center of the ellipse in the transformed coordinate system
    double sign = (large_arc_flag_ != clockwise_flag_) ? 1 : -1;
    double sq = ((rx_sq * ry_sq) - (rx_sq * y1p_sq) - (ry_sq * x1p_sq)) / ((rx_sq * y1p_sq) + (ry_sq * x1p_sq));
    sq = (sq < 0) ? 0 : sq;

    double cxp = sign * std::sqrt(sq) * (rx * y1p / ry);
    double cyp = sign * std::sqrt(sq) * (-ry * x1p / rx);

    // Transform the center back to the global coordinate system
    double cx = cos_angle * cxp - sin_angle * cyp + (start_pos_.x_ + end_pos_.x_) / 2.0;
    double cy = sin_angle * cxp + cos_angle * cyp + (start_pos_.y_ + end_pos_.y_) / 2.0;

    double theta = Vec2d::signedAngleRad(1, 0, (x1p - cxp) / rx, (y1p - cyp) / ry);
    double delta_theta = Vec2d::signedAngleRad((x1p - cxp) / rx, (y1p - cyp) / ry, (-x1p - cxp) / rx, (-y1p - cyp) / ry);

    if (!clockwise_flag_ == true && delta_theta > 0.0) {
        delta_theta -= Math::kTau;
    }
    else if (clockwise_flag_ == true && delta_theta < 0.0) {
        delta_theta += Math::kTau;
    }

    // Compute the angle at parameter t
    double angle_t = theta + t * delta_theta;

    // Compute the position on the rotated ellipse
    double cos_t = std::cos(angle_t);
    double sin_t = std::sin(angle_t);

    out_pos.x_ = cx + cos_angle * (rx * cos_t) - sin_angle * (ry * sin_t);
    out_pos.y_ = cy + sin_angle * (rx * cos_t) + cos_angle * (ry * sin_t);

    return true;
}


bool Arc::setStartPos(const Vec2d& pos) noexcept {
    if (set_mode_ == SetMode::ThreePoints) {
        setByThreePoints(pos, mid_pos_, end_pos_);
        return true;
    }
    if (set_mode_ == SetMode::SVG) {
        setSVG(pos, end_pos_, radii_, rotation_, large_arc_flag_, clockwise_flag_);
        return true;
    }
    return false;
}


bool Arc::setMidPos(const Vec2d& pos) noexcept {
    if (set_mode_ == SetMode::ThreePoints) {
        setByThreePoints(start_pos_, pos, end_pos_);
        return true;
    }
    return false;
}


bool Arc::setEndPos(const Vec2d& pos) noexcept {
    if (set_mode_ == SetMode::ThreePoints) {
        setByThreePoints(start_pos_, mid_pos_, pos);
        return true;
    }
    else if (set_mode_ == SetMode::SVG) {
        setSVG(start_pos_, pos, radii_, rotation_, large_arc_flag_, clockwise_flag_);
        return true;
    }
    return false;
}


bool Arc::setRadius(double radius) noexcept {
    if (set_mode_ == SetMode::CoreGraphics) {
        setCoreGraphics(center_, radius, start_angle_, end_angle_, clockwise_flag_);
        return true;
    }
    if (set_mode_ == SetMode::SVG) {
        setSVG(start_pos_, end_pos_, Vec2d(radius, radius), rotation_, large_arc_flag_, clockwise_flag_);
        return true;
    }
    return false;
}


bool Arc::setRadiusX(double rx) noexcept {
    if (set_mode_ == SetMode::SVG) {
        setSVG(start_pos_, end_pos_, Vec2d(rx, radiusY()), rotation_, large_arc_flag_, clockwise_flag_);
        return true;
    }
    return false;
}


bool Arc::setRadiusY(double ry) noexcept {
    if (set_mode_ == SetMode::SVG) {
        setSVG(start_pos_, end_pos_, Vec2d(radiusX(), ry), rotation_, large_arc_flag_, clockwise_flag_);
        return true;
    }
    return false;
}


bool Arc::setRotation(double rotation) noexcept {
    if (set_mode_ == SetMode::SVG) {
        setSVG(start_pos_, end_pos_, radii_, rotation, large_arc_flag_, clockwise_flag_);
        return true;
    }
    return false;
}


bool Arc::setStartAngle(double start_angle) noexcept {
    if (set_mode_ == SetMode::CoreGraphics) {
        setCoreGraphics(center_, radii_.x_, start_angle, end_angle_, clockwise_flag_);
        return true;
    }
    return false;
}


bool Arc::setEndAngle(double end_angle) noexcept {
    if (set_mode_ == SetMode::CoreGraphics) {
        setCoreGraphics(center_, radii_.x_, start_angle_, end_angle, clockwise_flag_);
        return true;
    }
    return false;
}


/**
 *  @brief Set Arc by three points.
 *
 *  @return `true`, if the given three points defines a valid arc, otherwise `false`.
 */
bool Arc::setByThreePoints(const Vec2d& start_pos, const Vec2d& mid_pos, const Vec2d& end_pos) noexcept {
    // Calculate the chord midpoint and radius
    Vec2d chord_mid =
        {(start_pos.x_ + end_pos.x_) / 2.0,
         (start_pos.y_ + end_pos.y_) / 2.0};
    double chord_len = start_pos.distance(end_pos);

    Vec2d vec_mid = {mid_pos.x_ - chord_mid.x_, mid_pos.y_ - chord_mid.y_};
    double orth_dist = std::sqrt(vec_mid.x_ * vec_mid.x_ + vec_mid.y_ * vec_mid.y_);

    // Calculate ellipse radii
    double radius = (orth_dist * orth_dist + chord_len * chord_len) / (2.0 * orth_dist);
    double radius_x = radius; // Assuming a circle for simplicity
    double radius_y = radius; // Assuming a circle for simplicity

    // Compute the rotation (assume 0 for simplicity)
    double rotation = 0.0;

    // Compute the angles
    auto vector_angle = [](double ux, double uy, double vx, double vy) {
        double dot = ux * vx + uy * vy;
        double len = std::sqrt((ux * ux + uy * uy) * (vx * vx + vy * vy));
        double angle = std::acos(std::max(-1.0, std::min(1.0, dot / len)));
        if (ux * vy - uy * vx < 0) angle = -angle;
        return angle;
    };

    Vec2d vec_start = {start_pos.x_ - chord_mid.x_, start_pos.y_ - chord_mid.y_};
    Vec2d vec_end = {end_pos.x_ - chord_mid.x_, end_pos.y_ - chord_mid.y_};
    double delta_theta = vector_angle(vec_start.x_, vec_start.y_, vec_end.x_, vec_end.y_);

    // Step 5: Determine large_arc_flag and sweep_flag
    bool large_arc_flag = std::abs(delta_theta) > M_PI;
    bool sweep_flag = delta_theta > 0;

    return setSVG(start_pos, end_pos, Vec2d(radius_x, radius_y), rotation, large_arc_flag, sweep_flag);
}


/**
 *  @brief Set Arc by CoreGraphics parameters.
 *
 *  @return returns `true`, if the given arguments defines a valid arc, otherwise `false`.
 */
bool Arc::setCoreGraphics(const Vec2d& center, double radius, double start_angle, double end_angle, bool clockwise_flag) noexcept {
    set_mode_ = SetMode::CoreGraphics;

    if (radius < std::numeric_limits<double>::epsilon()) {
        valid_ = false;
        return false;
    }

    start_angle_ = Type::wrappedValue<double>(start_angle, 0.0, 360);
    end_angle_ = Type::wrappedValue<double>(end_angle_, 0.0, 360);
    std::swap(start_angle_, end_angle_);

    center_ = center;
    radii_.x_ = radii_.y_ = radius;
    start_angle_ = start_angle;
    end_angle_ = end_angle;
    clockwise_flag_ = clockwise_flag;

    // Derived properties, CoreGraphics -> SVG

    // Rotation is always 0 for a circular arc
    rotation_ = 0;

    // Calculate start and end points based on angles
    double start_rad = Math::degtorad(start_angle);
    double end_rad = Math::degtorad(end_angle);
    start_pos_.x_ = center.x_ + radius * std::cos(start_rad);
    start_pos_.y_ = center.y_ + radius * std::sin(start_rad);
    end_pos_.x_ = center.x_ + radius * std::cos(end_rad);
    end_pos_.y_ = center.y_ + radius * std::sin(end_rad);

    // Calculate the angular span and normalize it to [0, 2π]
    double span = end_rad - start_rad;
    if (span < 0.0) {
        span += Math::kTau; // Normalize to positive span
    }

    // Determine the large-arc-flag
    large_arc_flag_ = (span > M_PI) ? 1 : 0;

    valid_ = true;

    return true;
}


/**
 *  @brief Set Arc by SVG parameters.
 *
 *  @return returns `true`, if the given arguments defines a valid arc, otherwise `false`.
 */
bool Arc::setSVG(const Vec2d& start_pos, const Vec2d& end_pos, const Vec2d& radii, double rotation, bool large_arc_flag, bool clockwise_flag) noexcept {
    set_mode_ = SetMode::SVG;

    if (radii.x_ < std::numeric_limits<double>::epsilon() ||
        radii.y_ < std::numeric_limits<double>::epsilon()) {
        valid_ = false;
        return false;
    }

    start_pos_ = start_pos;
    end_pos_ = end_pos;
    radii_ = radii;
    rotation_ = rotation;
    large_arc_flag_ = large_arc_flag;
    clockwise_flag_ = clockwise_flag;

    _svgUpdateCenter();

    return true;
}


void Arc::fill(GraphicContext& gc) const noexcept {
    // TODO: Implement
}


void Arc::stroke(GraphicContext& gc) const noexcept {
    const int32_t max_segment_n = 8;
    Vec2d bezier_pos_array[max_segment_n * 3 + 1];

    int32_t segment_n =
        Bezier::arcToBezierPosArray(
            start_pos_,
            radii_,
            rotation_,
            large_arc_flag_,
            clockwise_flag_,
            end_pos_,
            max_segment_n,
            bezier_pos_array);

    auto p = bezier_pos_array;
    for (int32_t i = 0; i <= segment_n; i++) {
        if (i == 0) {
            gc.moveTo(p[0]);
            p++;
        }
        else {
            gc.curveTo(p[0], p[1], p[2]);
            p += 3;
        }
    }

    gc.strokePath();
}


void Arc::addClip(GraphicContext& gc) const noexcept {
    // TODO: Implement
}


void Arc::svgCode(String& out_code, int32_t precision) const noexcept {
    out_code = 'M';
    out_code.appendDouble(start_pos_.x_, precision);
    out_code.appendChar(' ');
    out_code.appendDouble(start_pos_.y_, precision);
    out_code += 'A';
    out_code.appendDouble(radii_.x_, precision);
    out_code.appendChar(' ');
    out_code.appendDouble(radii_.y_, precision);
    out_code += ' ';
    out_code.appendDouble(rotation_, precision);
    out_code += ' ';
    out_code.appendBool(large_arc_flag_);
    out_code += ' ';
    out_code.appendBool(clockwise_flag_);
    out_code += ' ';
    out_code.appendDouble(end_pos_.x_, precision);
    out_code.appendChar(' ');
    out_code.appendDouble(end_pos_.y_, precision);
}


// Helper function for vector angle calculation
double vectorAngle(double ux, double uy, double vx, double vy) {
    double dot = ux * vx + uy * vy;
    double len = std::sqrt((ux * ux + uy * uy) * (vx * vx + vy * vy));
    double angle = std::acos(std::max(-1.0, std::min(1.0, dot / len))); // Clamp value to avoid domain errors
    if (ux * vy - uy * vx < 0) angle = -angle;
    return angle;
}


void Arc::_svgUpdateCenter() noexcept {
    if (radii_.x_ == 0.0 ||
        radii_.y_ == 0.0 ||
        (start_pos_.distance(end_pos_) < std::numeric_limits<double>::epsilon())) {
        return;
    }

    // Apply transformations to center the arc
    double rx = std::abs(radii_.x_);
    double ry = std::abs(radii_.y_);

    double dx = (start_pos_.x_ - end_pos_.x_) / 2.0;
    double dy = (start_pos_.y_ - end_pos_.y_) / 2.0;

    double rotation_rad = rotation_ * M_PI / 180.0;
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
        radius_scale_ = std::sqrt(radius_check);
        rx *= std::sqrt(radius_check);
        ry *= std::sqrt(radius_check);
        rx_sq = rx * rx;
        ry_sq = ry * ry;
    }
    else {
        radius_scale_ = 1.0;
    }

    // Calculate the center of the ellipse in the transformed coordinate system
    double sign = (large_arc_flag_ != clockwise_flag_) ? 1 : -1;
    double sq = ((rx_sq * ry_sq) - (rx_sq * y1p_sq) - (ry_sq * x1p_sq)) / ((rx_sq * y1p_sq) + (ry_sq * x1p_sq));
    sq = (sq < 0) ? 0 : sq;

    double cxp = sign * std::sqrt(sq) * (rx * y1p / ry);
    double cyp = sign * std::sqrt(sq) * (-ry * x1p / rx);

    // Transform the center back to the global coordinate system
    double cx = cos_angle * cxp - sin_angle * cyp + (start_pos_.x_ + end_pos_.x_) / 2.0;
    double cy = sin_angle * cxp + cos_angle * cyp + (start_pos_.y_ + end_pos_.y_) / 2.0;

    center_.x_ = cx;
    center_.y_ = cy;
}


} // End of namespace Grain.
