//
//  Triangle.hpp
//
//  Created by Roald Christesen on from 16.01.2018
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Grain.hpp"
#include "2d/Triangle.hpp"
#include "2d/Line.hpp"


namespace Grain {

    template <typename T>
    Triangle<T>::Triangle(const Vec2<T>& p1, const Vec2<T>& p2, const Vec2<T>& p3) noexcept {

        m_points[0] = p1;
        m_points[1] = p2;
        m_points[2] = p3;
    }


    template <typename T>
    double Triangle<T>::sideLength(int16_t side_index) const noexcept {

        if (side_index < 0 || side_index > 2) {
            return 0.0;
        }
        else if (side_index < 2) {
            return m_points[side_index].distance(m_points[side_index + 1]);
        }
        else {
            return m_points[side_index].distance(m_points[0]);
        }
    }


    template <typename T>
    double Triangle<T>::perimeter() const noexcept {

        return
                m_points[0].distance(m_points[1]) +
                m_points[1].distance(m_points[2]) +
                m_points[2].distance(m_points[0]);
    }


    template <typename T>
    double Triangle<T>::area() const noexcept {

        double a = m_points[0].distance(m_points[1]);
        double b = m_points[1].distance(m_points[2]);
        double c = m_points[2].distance(m_points[0]);

        if (std::isnan(a) || std::isnan(b) || std::isnan(c)) {
            return 0.0;
        }

        double s = (a + b + c) / 2.0;    // Semiperimeter (half perimeter).

        return std::sqrt(s * (s - a) * (s - b) * (s - c));
    }


    template <typename T>
    double Triangle<T>::altitude(int16_t side_index) const noexcept {

        if (side_index < 0 || side_index > 2) {
            return 0.0;
        }

        double l[3];
        l[0] = m_points[0].distance(m_points[1]);
        l[1] = m_points[1].distance(m_points[2]);
        l[2] = m_points[2].distance(m_points[0]);

        // Semiperimeter (half perimeter).
        double s = (l[0] + l[1] + l[2]) / 2.0;

        return (2.0 * std::sqrt(s * (s - l[0]) * (s - l[1]) * (s - l[2]))) / l[side_index];
    }


    template <typename T>
    Vec2<T> Triangle<T>::centroid() const noexcept {

        return Vec2<T>((m_points[0].m_x + m_points[1].m_x + m_points[2].m_x) / 3.0,
                       (m_points[0].m_y + m_points[1].m_y + m_points[2].m_y) / 3.0);
    }


    template <typename T>
    Triangle<T> Triangle<T>::offsettedTriangle(double distance) const noexcept {


        Vec2d v1 = m_points[1] - m_points[0];
        Vec2d v2 = m_points[2] - m_points[1];
        Vec2d v3 = m_points[0] - m_points[2];

        v1.normalize();
        v2.normalize();
        v3.normalize();

        v1.ortho();
        v2.ortho();
        v3.ortho();

        v1.setLength(distance);
        v2.setLength(distance);
        v3.setLength(distance);

        Line line1(m_points[0] + v1, m_points[1] + v1);
        Line line2(m_points[1] + v2, m_points[2] + v2);
        Line line3(m_points[2] + v3, m_points[0] + v3);

        Triangle new_triangle;
        line3.intersects(line1, new_triangle.m_points[0]);
        line1.intersects(line2, new_triangle.m_points[1]);
        line2.intersects(line3, new_triangle.m_points[2]);

        return new_triangle;
    }


} // End of namespace Grain
