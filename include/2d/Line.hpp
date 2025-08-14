//
//  Line.hpp
//
//  Created by Roald Christesen on from 24.01.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
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
     *  the length of the line, determining its slope, and checking for intersections
     *  with other lines or shapes.
     *
     *  This templated class supports various data types and can be used for different
     *  numerical representations. Predefined specialized versions include datatypes
     *  `int32_t`, `int64_t`, `float`and`double`, named as `Linei`, `Linel`, `Linef`,
     *  and `Lined` respectively.
     *
     *  @note This class is commonly used in geometry, computer graphics, and other
     *        fields where straight lines are relevant.
     */
    template <class T>
    class Line {

    public:
        Line() noexcept : m_p1(0, 0), m_p2(0, 0) {}
        explicit Line(const Vec2<T>& p1, const Vec2<T>& p2) noexcept : m_p1(p1), m_p2(p2) {}
        explicit Line(T x1, T y1, T x2, T y2) noexcept : m_p1(x1, y1), m_p2(x2, y2) {}

        explicit Line(const Vec2<T>& center, T length, double deg) noexcept {
            m_p1.m_x = -length / 2;
            m_p1.m_y = 0;
            m_p2.m_x = length / 2;
            m_p2.m_y = 0;
            rotate(deg);
            translate(center);
        }

        virtual ~Line() = default;

        [[nodiscard]] virtual const char* className() const noexcept { return "Line"; }

        friend std::ostream& operator << (std::ostream& os, const Line* o) {
            o == nullptr ? os << "Line nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const Line& o) {
            return os << o.m_p1 << " .. " << o.m_p2 << std::endl;
        }


        // Operator overloading

        bool operator == (const Line& v) const { return m_p1 == v.m_p1 && m_p2 == v.m_p2; }
        bool operator != (const Line& v) const { return m_p1 != v.m_p1 || m_p2 != v.m_p2; }


        [[nodiscard]] T length() const noexcept { return vec().length(); }
        [[nodiscard]] T lengthSquared() const noexcept { return vec().lengthSquared(); }
        [[nodiscard]] T minX() const noexcept { return std::min<T>(m_p1.m_x, m_p2.m_x); }
        [[nodiscard]] T minY() const noexcept { return std::min<T>(m_p1.m_y, m_p2.m_y); }
        [[nodiscard]] T maxX() const noexcept { return std::max<T>(m_p1.m_x, m_p2.m_x); }
        [[nodiscard]] T maxY() const noexcept { return std::max<T>(m_p1.m_y, m_p2.m_y); }

        [[nodiscard]] Vec2<T> vec() const noexcept { return m_p2 - m_p1; }
        [[nodiscard]] Vec2<T> normalizedVec() const noexcept { return (m_p2 - m_p1).normalized(); }

        [[nodiscard]] Vec2<T> center() const noexcept { return m_p1 + (m_p2 - m_p1) / 2; }
        [[nodiscard]] Vec2<T> normal() const noexcept { return (-(m_p2.m_y - m_p1.m_y), m_p2.m_x - m_p1.m_x).normalized(); }

        [[nodiscard]] double distance(const Vec2<T>& v) const noexcept {
            Vec2<T> line_dir = m_p2 - m_p1;
            double numerator = std::fabs(line_dir.m_y * v.m_x - line_dir.m_x * v.m_y + m_p2.m_x * m_p1.m_y - m_p2.m_y * m_p1.m_x);
            double denominator = std::sqrt(line_dir.m_y * line_dir.m_y + line_dir.m_x * line_dir.m_x);
            return denominator <= 0 ? DBL_MAX : numerator / denominator;
        }

        /**
         *  @brief Computes the signed area (cross product) of the parallelogram formed by the directed line segment and the vector to the point.
         *
         *  This is useful for geometric algorithms such as determining the orientation of points or checking if a point
         *  is inside a polygon.
         *
         *  @param v The point to check relative to the directed line segment.
         *  @return A signed value representing the relative position of the point:
         *          - Positive if the point is to the left of the line segment.
         *          - Negative if the point is to the right of the line segment.
         *          - Zero if the point is collinear with the line segment.
         */
        [[nodiscard]] T side(const Vec2<T>& v) const noexcept {
            return (m_p2.m_x - m_p1.m_x) * (v.m_y - m_p1.m_y) - (m_p2.m_y - m_p1.m_y) * (v.m_x - m_p1.m_x);
        }

        /**
         *  @brief Computes a point along the line at a specified normalized parameter `t`.
         *
         *  Given a normalized parameter `t` (ranging from 0 to 1), this function returns the point along the line
         *  defined by the line segment (from `m_p1` to `m_p2`).
         *  - `t = 0` returns `m_p1`.
         *  - `t = 1` returns `m_p2`.
         *
         *  @param t The normalized parameter, typically in the range [0, 1].
         *  @return The point on the line at the given `t`.
         */
        [[nodiscard]] inline constexpr Vec2<T> pointAtT(const T t) const {
            return m_p1 + vec() * t;
        }

        /**
         *  @brief Computes the coefficients of the line equation in the form "mx + a".
         *
         *  Given two points defining a line, this function computes the slope \( m \) and y-intercept \( a \)
         *  of the line equation \( y = mx + a \). It returns the values as a `Vec2d` where:
         *  - `m` is the slope of the line.
         *  - `a` is the y-intercept of the line.
         *
         *  If the line is vertical (i.e., the x-coordinates of the two points are equal), the function returns
         * `    {inf, inf}` as the slope and intercept are undefined for a vertical line.
         *
         *   @note The function checks if the line is vertical by comparing the difference in the x-coordinates
         *  of the two points to a small epsilon value (`std::numeric_limits<float>::epsilon()`).
         *
         *  @return A `Vec2d` representing the slope `m` and the intercept `a` of the line.
         *             If the line is vertical, it returns `{std::numeric_limits<double>::infinity(),
         *             std::numeric_limits<double>::infinity()}`.
         */
        [[nodiscard]] Vec2d coefficients() const {

            double x1 = m_p1.m_x;
            double y1 = m_p1.m_y;
            double x2 = m_p2.m_x;
            double y2 = m_p2.m_y;

            // Check if line is vertical or close to vertical
            if (std::abs(x2 - x1) < std::numeric_limits<float>::epsilon()) {
                return Vec2d { std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity() };
            }
            else {
                double m = (y2 - y1) / (x2 - x1);
                return Vec2d { m, -m * x1 + y1 };
            }
        }


        void set(const Vec2<T>& p1, const Vec2<T>& p2) noexcept { m_p1 = p1; m_p2 = p2; }
        void set(T x1, T y1, T x2, T y2) noexcept { m_p1.m_x = x1; m_p1.m_y = y1; m_p2.m_x = x2; m_p2.m_y = y2; }

        void inset(T inset1, T inset2) noexcept {
            Vec2<T> v = normalizedVec();
            m_p1 += m_p1 + v * inset1;
            m_p2 -= m_p2 - v * inset2;
        }

        void translate(T tx, T ty) noexcept {
            m_p1.m_x += tx;
            m_p1.m_y += ty;
            m_p2.m_x += tx;
            m_p2.m_y += ty;
        }

        void translate(const Vec2<T>& t) noexcept {
            m_p1 += t;
            m_p2 += t;
        }

        void rotate(T deg) noexcept {
            m_p1.rotate(deg);
            m_p2.rotate(deg);
        }

        void rotate(const Vec2<T>& pivot, T deg) noexcept {
            m_p1.rotate(pivot, deg);
            m_p2.rotate(pivot, deg);
        }

        void rotateP2(T deg) noexcept {
            m_p2 -= m_p1;
            m_p2.rotate(deg);
            m_p2 += m_p1;
        }

        void rotateCentered(T deg) noexcept {
            Vec2<T>pivot = center();
            m_p1.rotate(pivot, deg);
            m_p2.rotate(pivot, deg);
        }

        void offset(T offset) noexcept {
            Vec2<T> v(-(m_p2.m_y - m_p1.m_y), m_p2.m_x - m_p1.m_x);
            v.setLength(offset);
            m_p1 += v;
            m_p2 += v;
        }

        void scaleCentered(T scale) noexcept {
            Vec2<T> center = this->center();
            m_p1.scaleCentered(center, scale);
            m_p2.scaleCentered(center, scale);
        }

        /**
         *  @brief Checks if this line intersects with another line.
         *
         *  This function determines whether the line represented by this object
         *  intersects with the specified line.
         *
         *  @param line The line to check for intersection with.
         *  @param[out] out_vec The position where the two lines intersect, if they do.
         *  If the lines do not intersect, this value is not modified.
         *
         *  @return true if this line intersects with the specified line, false otherwise.
         */
        [[nodiscard]] bool intersects(const Line& line, Vec2<T>& out_vec) const noexcept {

            T x1 = m_p1.m_x, x2 = m_p2.m_x, x3 = line.m_p1.m_x, x4 = line.m_p2.m_x;
            T y1 = m_p1.m_y, y2 = m_p2.m_y, y3 = line.m_p1.m_y, y4 = line.m_p2.m_y;
            T d = (x2 - x1) * (y4 - y3) - (y2 - y1) * (x4 - x3);
            if (d == 0) {
                return false;
            }

            T r = (((y1 - y3) * (x4 - x3)) - (x1 - x3) * (y4 - y3)) / d;
            // T s = (((y1 - y3) * (x2 - x1)) - (x1 - x3) * (y2 - y1)) / d;
            out_vec.m_x = x1 + r * (x2 - x1);
            out_vec.m_y = y1 + r * (y2 - y1);

            return true;
        }

        [[nodiscard]] double nearestPoint(const Line& line) const noexcept {
            double result = DBL_MAX;
            double d = m_p1.distance(line.m_p1);
            if (d < result) {
                result = d;
            }
            d = m_p1.distance(line.m_p2);
            if (d < result) {
                result = d;
            }
            d = m_p2.distance(line.m_p1);
            if (d < result) {
                result = d;
            }
            d = m_p2.distance(line.m_p2);
            if (d < result) {
                result = d;
            }
            return result;
        }


        /**
         *  @brief Checks if a given point is within a certain distance (tolerance) from this line segment.
         *
         *  This function determines whether the specified point is within a given distance (tolerance)
         *  from the line segment represented by this object. It calculates the perpendicular distance
         *  from the point to the line segment and checks if this distance is less than the specified tolerance.
         *
         *  @param pos The point to check.
         *  @param tolerance The distance within which the point is considered to be "hitting" the line segment.
         *  @param[out] out_distance The actual distance from the point to the closest point on the line segment.
         *  @return true if the point is within the specified tolerance from the line segment, false otherwise.
         */
        [[nodiscard]] bool hit(const Vec2<T>& pos, T tolerance, T& out_distance) const noexcept {

            T line_distance = m_p2.distance(m_p1);
            if (line_distance <= 0) {
                return false;
            }

            T u = (((pos.m_x - m_p1.m_x) * (m_p2.m_x - m_p1.m_x)) + ((pos.m_y - m_p1.m_y) * (m_p2.m_y - m_p1.m_y))) / (line_distance * line_distance);

            if (u < 0 || u > 1) {
                // closest point does not fall within the line segment
                return false;
            }

            Vec2<T> intersection(m_p1.m_x + u * (m_p2.m_x - m_p1.m_x), m_p1.m_y + u * (m_p2.m_y - m_p1.m_y));
            T distance = pos.distance(intersection);
            out_distance = distance;

            return distance < tolerance;
        }


        /**
         *  @brief Checks if this line segment intersects with a given rectangle.
         *
         *  This function determines whether the line segment represented by this object
         *  intersects with the specified rectangle. The function projects the line segment
         *  onto the x and y axes and checks for intersection within the rectangle's bounds.
         *
         *  @param rect The rectangle to check for intersection with.
         *  @return true if the line segment intersects with the specified rectangle, false otherwise.
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

            T min_y = m_p1.m_y;
            T max_y = m_p2.m_y;
            T dx = m_p2.m_x - m_p1.m_x;

            if (std::fabs(dx) > 0) {
                T a = (m_p2.m_y - m_p1.m_y) / dx;
                T b = m_p1.m_y - a * m_p1.m_x;
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

    public:
        Vec2<T> m_p1, m_p2;
    };


    // Standard types
    using Linei = Line<int32_t>;    ///< 32 bit integer
    using Linel = Line<int64_t>;    ///< 64 bit integer
    using Linef = Line<float>;      ///< 32 bit floating point
    using Lined = Line<double>;     ///< 64 bit floating point


} // End of namespace Grain

#endif // GrainLine_hpp
