//
//  Line.hpp
//
//  Created by Roald Christesen on from 24.01.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainLine_hpp
#define GrainLine_hpp

#include "Grain.hpp"
#include "Math/Vec2.hpp"
#include "2d/Rect.hpp"


namespace Grain {

/**
 *  @brief Line.
 *
 *  `Line` represents a straight line segment in two-dimensional space.
 *  It is defined by two endpoints. The class provides methods for calculating
 *  the length of the line, determining its slope, and checking for
 *  intersections with other lines or shapes.
 *
 *  This templated class supports various data types and can be used for
 *  different numerical representations. Predefined specialized versions include
 *  datatypes `int32_t`, `int64_t`, `float`and`double`, named as `Linei`,
 *  `Linel`, `Linef`, and `Lined` respectively.
 *
 *  @note This class is commonly used in geometry, computer graphics, and other
 *        fields where straight lines are relevant.
 */
template <class T>
class Line {

public:
    Vec2<T> p1_{};
    Vec2<T> p2_{};

public:
    Line() noexcept = default;
    Line(T x1, T y1, T x2, T y2) noexcept : p1_(x1, y1), p2_(x2, y2) {}
    explicit Line(const Vec2<T>& p1, const Vec2<T>& p2) noexcept : p1_(p1), p2_(p2) {}
    explicit Line(const Vec2<T>& center, T length, double deg) noexcept {
        p1_.x_ = -length / static_cast<T>(2);
        p1_.y_ = 0;
        p2_.x_ = length / static_cast<T>(2);
        p2_.y_ = 0;
        rotate(deg);
        translate(center);
    }

    virtual ~Line() = default;

    [[nodiscard]] virtual const char* className() const noexcept {
        return "Line";
    }

    friend std::ostream& operator << (std::ostream& os, const Line* o) {
        o == nullptr ? os << "Line nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const Line& o) {
        return os << o.p1_ << " .. " << o.p2_ << std::endl;
    }

    bool operator == (const Line& v) const {
        return p1_ == v.p1_ && p2_ == v.p2_;
    }

    bool operator != (const Line& v) const {
        return p1_ != v.p1_ || p2_ != v.p2_;
    }


    [[nodiscard]] T length() const noexcept {
        return vec().length();
    }

    [[nodiscard]] T squaredLength() const noexcept {
        return vec().squaredLength();
    }

    [[nodiscard]] T minX() const noexcept {
        return std::min<T>(p1_.x_, p2_.x_);
    }

    [[nodiscard]] T minY() const noexcept {
        return std::min<T>(p1_.y_, p2_.y_);
    }

    [[nodiscard]] T maxX() const noexcept {
        return std::max<T>(p1_.x_, p2_.x_);
    }

    [[nodiscard]] T maxY() const noexcept {
        return std::max<T>(p1_.y_, p2_.y_);
    }

    [[nodiscard]] Vec2<T> vec() const noexcept {
        return p2_ - p1_;
    }

    [[nodiscard]] Vec2<T> normalizedVec() const noexcept {
        return (p2_ - p1_).normalized();
    }

    [[nodiscard]] Vec2<T> center() const noexcept {
        return p1_ + (p2_ - p1_) / static_cast<T>(2);
    }

    [[nodiscard]] Vec2<T> normal() const noexcept {
        return Vec2<T>(-(p2_.y_ - p1_.y_), p2_.x_ - p1_.x_).normalized();
    }

    /**
     *  @brief Computes the perpendicular distance from a point to an infinite line.
     *
     *  @param v Point whose distance to the line is computed.
     *  @return The shortest (perpendicular) distance from the point @p v to the infinite line.
     *          Returns DBL_MAX if the line is degenerate (i.e., @p p1_ == @p p2_).
     */
     [[nodiscard]] double distance(const Vec2<T>& v) const noexcept {
        Vec2<T> line_dir = p2_ - p1_;
        double numerator = std::fabs(line_dir.y_ * v.x_ - line_dir.x_ * v.y_ + p2_.x_ * p1_.y_ - p2_.y_ * p1_.x_);
        double denominator = std::sqrt(line_dir.y_ * line_dir.y_ + line_dir.x_ * line_dir.x_);
        return denominator <= 0 ? DBL_MAX : numerator / denominator;
    }

    /**
     *  @brief Computes the shortest distance from a point to a finite line segment.
     *
     *  @param v Point whose distance to the segment is computed.
     *  @return The shortest distance from the point @p v to the line segment
     *          connecting @p p1_ and @p p2_. Returns DBL_MAX if the segment is degenerate.
     */
    [[nodiscard]] double distanceToSegment(const Vec2<T>& v) const noexcept {
        Vec2<T> seg_dir = p2_ - p1_;
        double seg_len2 = seg_dir.x_ * seg_dir.x_ + seg_dir.y_ * seg_dir.y_;
        if (seg_len2 <= 0.0) {
            return DBL_MAX; // Degenerate segment
        }

        double t = ((v - p1_).dot(seg_dir)) / seg_len2;
        t = std::clamp(t, 0.0, 1.0);

        Vec2<T> closest = p1_ + seg_dir * static_cast<T>(t);

        Vec2<T> diff = v - closest;
        return std::sqrt(diff.x_ * diff.x_ + diff.y_ * diff.y_);
    }

    /**
     *  @brief Computes the signed area (cross product) of the parallelogram
     *         formed by the directed line segment and the vector to the point.
     *
     *  This is useful for geometric algorithms such as determining the
     *  orientation of points or checking if a point is inside a polygon.
     *
     *  @param v The point to check relative to the directed line segment.
     *  @return A signed value representing the relative position of the point:
     *          - Positive if the point is to the left of the line segment.
     *          - Negative if the point is to the right of the line segment.
     *          - Zero if the point is collinear with the line segment.
     */
    [[nodiscard]] T side(const Vec2<T>& v) const noexcept {
        return (p2_.x_ - p1_.x_) * (v.y_ - p1_.y_) - (p2_.y_ - p1_.y_) * (v.x_ - p1_.x_);
    }

    /**
     *  @brief Computes a point along the line at a specified normalized
     *         parameter `t`.
     *
     *  @param t The normalized parameter, typically in the range [0, 1].
     *  @return The point on the line at the given `t`.
     */
    [[nodiscard]] inline constexpr Vec2<T> pointAtT(const T t) const {
        return p1_ + vec() * t;
    }

    /**
     *  @brief Computes the coefficients of the line equation in the form
     *         "mx + a".
     *
     *  If the line is vertical (i.e., the x-coordinates of the two points
     *  are equal), the function returns {inf, inf}` as the slope and intercept
     *  are undefined for a vertical line.
     *
     *  @note The function checks if the line is vertical by comparing the
     *        difference in the x-coordinates of the two points to a small
     *        epsilon value (`std::numeric_limits<float>::epsilon()`).
     *
     *  @return A `Vec2d` representing the slope `m` and the intercept `a`
     *          of the line. If the line is vertical, it returns `{inf, inf}`
     */
    [[nodiscard]] Vec2d coefficients() const {
        double x1 = p1_.x_;
        double y1 = p1_.y_;
        double x2 = p2_.x_;
        double y2 = p2_.y_;

        // Check if line is vertical or close to vertical
        if (std::abs(x2 - x1) < std::numeric_limits<float>::epsilon()) {
            return Vec2d { std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity() };
        }
        else {
            double m = (y2 - y1) / (x2 - x1);
            return Vec2d { m, -m * x1 + y1 };
        }
    }


    void set(const Vec2<T>& p1, const Vec2<T>& p2) noexcept {
        p1_ = p1;
        p2_ = p2;
    }

    void set(T x1, T y1, T x2, T y2) noexcept {
        p1_.x_ = x1;
        p1_.y_ = y1;
        p2_.x_ = x2;
        p2_.y_ = y2;
    }

    void inset(T inset1, T inset2) noexcept {
        Vec2<T> v = normalizedVec();
        p1_ = p1_ + v * inset1;
        p2_ = p2_ - v * inset2;
    }

    void translate(T tx, T ty) noexcept {
        p1_.x_ += tx;
        p1_.y_ += ty;
        p2_.x_ += tx;
        p2_.y_ += ty;
    }

    void translate(const Vec2<T>& t) noexcept {
        p1_ += t;
        p2_ += t;
    }

    void rotate(T deg) noexcept {
        p1_.rotate(deg);
        p2_.rotate(deg);
    }

    void rotate(const Vec2<T>& pivot, T deg) noexcept {
        p1_.rotate(pivot, deg);
        p2_.rotate(pivot, deg);
    }

    void rotateP1(T deg) noexcept {
        p1_ -= p2_;
        p1_.rotate(deg);
        p1_ += p2_;
    }

    void rotateP2(T deg) noexcept {
        p2_ -= p1_;
        p2_.rotate(deg);
        p2_ += p1_;
    }

    void rotateCentered(T deg) noexcept {
        Vec2<T>pivot = center();
        p1_.rotate(pivot, deg);
        p2_.rotate(pivot, deg);
    }

    void offset(T offset) noexcept {
        Vec2<T> v(-(p2_.y_ - p1_.y_), p2_.x_ - p1_.x_);
        v.setLength(offset);
        p1_ += v;
        p2_ += v;
    }

    void scaleCentered(T scale) noexcept {
        Vec2<T> center = this->center();
        p1_.scaleFrom(center, scale);
        p2_.scaleFrom(center, scale);
    }

    /**
     *  @brief Checks if this line intersects with another line.
     *
     *  This function determines whether the line represented by this object
     *  intersects with the specified line.
     *
     *  @param line The line to check for intersection with.
     *  @param[out] out_vec The position where the two lines intersect, if they
     *                      do. If the lines do not intersect, this value is
     *                      not modified.
     *
     *  @return true if this line intersects with the specified line, false
     *          otherwise.
     */
    [[nodiscard]] bool intersects(const Line& line, Vec2<T>& out_vec) const noexcept {
        T x1 = p1_.x_, x2 = p2_.x_, x3 = line.p1_.x_, x4 = line.p2_.x_;
        T y1 = p1_.y_, y2 = p2_.y_, y3 = line.p1_.y_, y4 = line.p2_.y_;
        T d = (x2 - x1) * (y4 - y3) - (y2 - y1) * (x4 - x3);
        if (d == 0) {
            return false;
        }

        T r = (((y1 - y3) * (x4 - x3)) - (x1 - x3) * (y4 - y3)) / d;
        // T s = (((y1 - y3) * (x2 - x1)) - (x1 - x3) * (y2 - y1)) / d;
        out_vec.x_ = x1 + r * (x2 - x1);
        out_vec.y_ = y1 + r * (y2 - y1);
        return true;
    }

    /**
     *  @brief Computes the shortest distance between the endpoints of two
     *         line segments.
     *
     *  @param line The other line segment to compare against.
     *  @return The shortest distance between any pair of endpoints from the
     *          two line segments.
     */
     [[nodiscard]] double nearestPoint(const Line& line) const noexcept {
        double result = DBL_MAX;
        double d = p1_.distance(line.p1_);
        if (d < result) {
            result = d;
        }
        d = p1_.distance(line.p2_);
        if (d < result) {
            result = d;
        }
        d = p2_.distance(line.p1_);
        if (d < result) {
            result = d;
        }
        d = p2_.distance(line.p2_);
        if (d < result) {
            result = d;
        }
        return result;
    }

    /**
     *  @brief Checks if a given point is within a certain distance (tolerance)
     *         from this line segment.
     *
     *  @param pos The point to check.
     *  @param tolerance The distance within which the point is considered to
     *                   be "hitting" the line segment.
     *  @param[out] out_distance The actual distance from the point to the
     *                           closest point on the line segment.
     *  @return true if the point is within the specified tolerance from the
     *          line segment, false otherwise.
     */
    [[nodiscard]] bool hit(const Vec2<T>& pos, T tolerance, T& out_distance) const noexcept {
        T line_distance = p2_.distance(p1_);
        if (line_distance <= 0) {
            return false;
        }

        T u = (((pos.x_ - p1_.x_) * (p2_.x_ - p1_.x_)) + ((pos.y_ - p1_.y_) * (p2_.y_ - p1_.y_))) / (line_distance * line_distance);
        if (u < 0 || u > 1) {
            // Closest point does not fall within the line segment
            return false;
        }

        Vec2<T> intersection(p1_.x_ + u * (p2_.x_ - p1_.x_), p1_.y_ + u * (p2_.y_ - p1_.y_));
        T distance = pos.distance(intersection);
        out_distance = distance;

        return distance < tolerance;
    }

    /**
     *  @brief Checks if this line segment intersects with a given rectangle.
     *
     *  @param rect The rectangle to check for intersection with.
     *  @return true if the line segment intersects with the specified
     *          rectangle, false otherwise.
     */
    [[nodiscard]] bool intersects(const Rect<T>& rect) const noexcept {
        T min_x = this->minX();
        T max_x = this->maxX();
        T rect_min_x = rect.x();
        T rect_max_x = rect.x2();
        T rect_min_y = rect.y();
        T rect_max_y = rect.y2();

        // Find the intersection of the segment's and rectangle's x-projections
        if (max_x > rect_max_x) {
            max_x = rect_max_x;
        }

        if (min_x < rect_min_x) {
            min_x = rect_min_x;
        }

        if (min_x > max_x) {
            // If their projections do not intersect return false
            return false;
        }

        // Find corresponding min and max Y for min and max X we found before
        T min_y = p1_.y_;
        T max_y = p2_.y_;
        T dx = p2_.x_ - p1_.x_;

        if (std::fabs(dx) > 0) {
            T a = (p2_.y_ - p1_.y_) / dx;
            T b = p1_.y_ - a * p1_.x_x_;
            min_y = a * min_x + b;
            max_y = a * max_x + b;
        }

        if (min_y > max_y) {
            T tmp = max_y;
            max_y = min_y;
            min_y = tmp;
        }

        // Find the intersection of the segment's and rectangle's y-projections
        if (max_y > rect_max_y) {
            max_y = rect_max_y;
        }

        if (min_y < rect_min_y) {
            min_y = rect_min_y;
        }

        if (min_y > max_y) {
            // If Y-projections do not intersect return false
            return false;
        }

        return true;
    }
};


// Standard types
using Linei = Line<int32_t>;    ///< 32 bit integer
using Linel = Line<int64_t>;    ///< 64 bit integer
using Linef = Line<float>;      ///< 32 bit floating point
using Lined = Line<double>;     ///< 64 bit floating point


} // End of namespace Grain

#endif // GrainLine_hpp
