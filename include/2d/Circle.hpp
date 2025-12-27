//
//  Circle.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainCircle_hpp
#define GrainCircle_hpp

#include "Grain.hpp"
#include "Math/Vec2.hpp"


namespace Grain {

/**
 *  @brief Circle.
 */
template <class T>
class Circle {

public:
    Vec2<T> center_{};
    T radius_{};

public:
    Circle() noexcept : center_(0, 0), radius_(10) {}
    Circle(const Vec2<T>& center, T radius) noexcept { set(center, radius); }
    Circle(T x, T y, T radius) noexcept : center_(x, y), radius_(radius) {}

    virtual ~Circle() = default;

    [[nodiscard]] virtual const char* className() const noexcept {
        return "Circle";
    }

    friend std::ostream& operator << (std::ostream& os, const Circle* o) {
        o == nullptr ? os << "Circle nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const Circle& o) {
        os << "center: " << o.center_ << ", radius: " << o.radius_;
        return os;
    }


    [[nodiscard]] Vec2<T> center() const noexcept { return center_; }
    [[nodiscard]] T radius() const noexcept { return radius_; }

    [[nodiscard]] T minX() const noexcept { return center_.x_ - radius_; }
    [[nodiscard]] T minY() const noexcept { return center_.y_ - radius_; }
    [[nodiscard]] T maxX() const noexcept { return center_.x_ + radius_; }
    [[nodiscard]] T maxY() const noexcept { return center_.y_ + radius_; }

    [[nodiscard]] double area() const noexcept { return std::numbers::pi * radius_ * radius_; }
    [[nodiscard]] double circumference() { return 2.0 * std::numbers::pi * radius_; }

    void set(const Vec2<T>& center, T radius) noexcept { center_ = center; radius_ = radius; }
    void set(T x, T y, T radius) noexcept { center_.x_ = x; center_.y_ = y; radius_ = radius; }
    void setCenter(const Vec2<T>& center) noexcept { center_ = center; }
    void setRadius(T radius) noexcept { radius_ = radius; }
    void setCircumference(T circumference) noexcept { radius_ = circumference / (2.0 * std::numbers::pi); }

    void translate(T tx, T ty) noexcept { center_.x_ += tx; center_.y_ += ty; }
    void translate(const Vec2<T>& t) noexcept { center_.x_ += t; }

    // TODO:
    // bool intersects(const Line& line, Vec2<T>& out_vec) const noexcept {}
    // bool hit(const Vec2<T>& pos, T tolerance, T& out_distance) const noexcept {}
    // bool intersects(const Rect<T>& rect) const noexcept {}


    /**
     *  @brief Compute tangent points for connecting two circles.
     */
    bool outerTangentPoints(const Circle<T>& circle, Vec2<T>* out_tangent_points) const noexcept {
        if (!out_tangent_points) {
            return false;
        }

        double x1 = center_.x_;
        double y1 = center_.y_;
        double r1 = radius_;
        double x2 = circle.center_.x_;
        double y2 = circle.center_.y_;
        double r2 = circle.radius_;

        double dx = x2 - x1;
        double dy = y2 - y1;
        double dist = std::sqrt(dx * dx + dy * dy);

        if (dist <= std::fabs(r2 - r1)) {
            return false; // No valid tangents
        }

        // Rotation from x-axis
        double angle1 = std::atan2(dy, dx);
        double angle2 = std::acos((r1 - r2) / dist);

        int32_t index = 0;
        out_tangent_points[index].x_ = x1 + r1 * std::cos(angle1 + angle2);
        out_tangent_points[index].y_ = y1 + r1 * std::sin(angle1 + angle2);
        index++;

        out_tangent_points[index].x_ = x2 + r2 * std::cos(angle1 + angle2);
        out_tangent_points[index].y_ = y2 + r2 * std::sin(angle1 + angle2);
        index++;

        out_tangent_points[index].x_ = x1 + r1 * std::cos(angle1 - angle2);
        out_tangent_points[index].y_ = y1 + r1 * std::sin(angle1 - angle2);
        index++;

        out_tangent_points[index].x_ = x2 + r2 * std::cos(angle1 - angle2);
        out_tangent_points[index].y_ = y2 + r2 * std::sin(angle1 - angle2);
        index++;

        return true;
    }

    bool innerTangentPoints(const Circle<T>& circle, Vec2<T>* out_tangent_points) const noexcept {
        if (!out_tangent_points) {
            return false;
        }

        double x1 = center_.x_;
        double y1 = center_.y_;
        double r1 = radius_;
        double x2 = circle.center_.x_;
        double y2 = circle.center_.y_;
        double r2 = circle.radius_;

        double dx = x2 - x1;
        double dy = y2 - y1;
        double dist = std::sqrt(dx * dx + dy * dy);

        // Check if valid inner tangents exist
        if (dist <= (r1 + r2)) {
            return false; // No valid inner tangents
        }

        // Compute the angles for the tangents
        double angle1 = std::atan2(dy, dx); // Angle from the first circle's center to the second
        double angle2 = std::acos((r1 + r2) / dist); // Adjustment for inner tangent angles

        int32_t index = 0;

        // First pair of tangent points
        out_tangent_points[index].x_ = x1 + r1 * std::cos(angle1 + angle2);
        out_tangent_points[index].y_ = y1 + r1 * std::sin(angle1 + angle2);
        index++;

        out_tangent_points[index].x_ = x2 - r2 * std::cos(angle1 + angle2);
        out_tangent_points[index].y_ = y2 - r2 * std::sin(angle1 + angle2);
        index++;

        // Second pair of tangent points
        out_tangent_points[index].x_ = x1 + r1 * std::cos(angle1 - angle2);
        out_tangent_points[index].y_ = y1 + r1 * std::sin(angle1 - angle2);
        index++;

        out_tangent_points[index].x_ = x2 - r2 * std::cos(angle1 - angle2);
        out_tangent_points[index].y_ = y2 - r2 * std::sin(angle1 - angle2);
        index++;

        return true;
    }
};


// Standard types
using Circlei = Circle<int32_t>;    ///< 32 bit integer
using Circlel = Circle<int64_t>;    ///< 64 bit integer
using Circlef = Circle<float>;      ///< 32 bit floating point
using Circled = Circle<double>;     ///< 64 bit floating point


} // End of namespace Grain

#endif // GrainCircle_hpp
