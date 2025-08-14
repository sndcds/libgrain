//
//  Circle.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainCircle_hpp
#define GrainCircle_hpp

#include "Grain.hpp"
#include "Math/Vec2.hpp"
#include "2d/Rect.hpp"


namespace Grain {

    /**
     *  @class Circle.
     *  @brief Circle.
     */
    template <class T>
    class Circle {

    public:
        Vec2<T> m_center;
        T m_radius;

    public:
        Circle() noexcept : m_center(0, 0), m_radius(10) {}
        Circle(const Vec2<T>& center, T radius) noexcept { set(center, radius); }
        Circle(T x, T y, T radius) noexcept : m_center(x, y), m_radius(radius) {}

        virtual const char* className() const noexcept { return "Circle"; }



        T minX() const noexcept { return m_center.m_x - m_radius; }
        T minY() const noexcept { return m_center.m_y - m_radius; }
        T maxX() const noexcept { return m_center.m_x + m_radius; }
        T maxY() const noexcept { return m_center.m_y + m_radius; }

        Vec2<T> center() const noexcept { return m_center; }

        double area() const noexcept { return std::numbers::pi * m_radius * m_radius; }
        double circumference() { return 2.0 * std::numbers::pi * m_radius; }

        void set(const Vec2<T>& center, T radius) noexcept { m_center = center; m_radius = radius; }
        void set(T x, T y, T radius) noexcept { m_center.m_x = x; m_center.m_y = y; m_radius = radius; }
        void setCenter(const Vec2<T>& center) noexcept { m_center = center; }
        void setRadius(T radius) noexcept { m_radius = radius; }
        void setCircumference(T circumference) noexcept { m_radius = circumference / (2.0 * std::numbers::pi); }

        void translate(T tx, T ty) noexcept { m_center.m_x += tx; m_center.m_y += ty; }
        void translate(const Vec2<T>& t) noexcept { m_center.m_x += t; }

        // TODO:
        // bool intersects(const Line& line, Vec2<T>& out_vec) const noexcept {}
        // bool hit(const Vec2<T>& pos, T tolerance, T& out_distance) const noexcept {}
        // bool intersects(const Rect<T>& rect) const noexcept {}


        /**
         *  @brief Compute tangent points for connecting two circles.
         */
        bool outerTangentPoints(const Circle<T>& circle, Vec2<T>* out_tangent_points) const noexcept {

            if (out_tangent_points == nullptr) {
                return false;
            }

            double x1 = m_center.m_x;
            double y1 = m_center.m_y;
            double r1 = m_radius;
            double x2 = circle.m_center.m_x;
            double y2 = circle.m_center.m_y;
            double r2 = circle.m_radius;


            double dx = x2 - x1;
            double dy = y2 - y1;
            double dist = std::sqrt(dx * dx + dy * dy);

            if (dist <= std::fabs(r2 - r1)) {
                return false; // No valid tangents.
            }

            // Rotation from x-axis.
            double angle1 = std::atan2(dy, dx);
            double angle2 = std::acos((r1 - r2) / dist);

            int32_t index = 0;
            out_tangent_points[index].m_x = x1 + r1 * std::cos(angle1 + angle2);
            out_tangent_points[index].m_y = y1 + r1 * std::sin(angle1 + angle2);
            index++;

            out_tangent_points[index].m_x = x2 + r2 * std::cos(angle1 + angle2);
            out_tangent_points[index].m_y = y2 + r2 * std::sin(angle1 + angle2);
            index++;

            out_tangent_points[index].m_x = x1 + r1 * std::cos(angle1 - angle2);
            out_tangent_points[index].m_y = y1 + r1 * std::sin(angle1 - angle2);
            index++;

            out_tangent_points[index].m_x = x2 + r2 * std::cos(angle1 - angle2);
            out_tangent_points[index].m_y = y2 + r2 * std::sin(angle1 - angle2);
            index++;

            return true;
        }

        bool innerTangentPoints(const Circle<T>& circle, Vec2<T>* out_tangent_points) const noexcept {

            if (out_tangent_points == nullptr) {
                return false;
            }

            double x1 = m_center.m_x;
            double y1 = m_center.m_y;
            double r1 = m_radius;
            double x2 = circle.m_center.m_x;
            double y2 = circle.m_center.m_y;
            double r2 = circle.m_radius;

            double dx = x2 - x1;
            double dy = y2 - y1;
            double dist = std::sqrt(dx * dx + dy * dy);

            // Check if valid inner tangents exist.
            if (dist <= (r1 + r2)) {
                return false; // No valid inner tangents
            }

            // Compute the angles for the tangents.
            double angle1 = std::atan2(dy, dx); // Angle from the first circle's center to the second.
            double angle2 = std::acos((r1 + r2) / dist); // Adjustment for inner tangent angles.

            int32_t index = 0;

            // First pair of tangent points.
            out_tangent_points[index].m_x = x1 + r1 * std::cos(angle1 + angle2);
            out_tangent_points[index].m_y = y1 + r1 * std::sin(angle1 + angle2);
            index++;

            out_tangent_points[index].m_x = x2 - r2 * std::cos(angle1 + angle2);
            out_tangent_points[index].m_y = y2 - r2 * std::sin(angle1 + angle2);
            index++;

            // Second pair of tangent points.
            out_tangent_points[index].m_x = x1 + r1 * std::cos(angle1 - angle2);
            out_tangent_points[index].m_y = y1 + r1 * std::sin(angle1 - angle2);
            index++;

            out_tangent_points[index].m_x = x2 - r2 * std::cos(angle1 - angle2);
            out_tangent_points[index].m_y = y2 - r2 * std::sin(angle1 - angle2);
            index++;

            return true;
        }
    };

    // Standard types.
    using Circlei = Circle<int32_t>;    ///< 32 bit integer.
    using Circlel = Circle<int64_t>;    ///< 64 bit integer.
    using Circlef = Circle<float>;      ///< 32 bit floating point.
    using Circled = Circle<double>;     ///< 64 bit floating point.


} // End of namespace Grain

#endif // GrainCircle_hpp
