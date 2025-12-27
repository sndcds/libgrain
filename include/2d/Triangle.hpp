//
//  Triangle.hpp
//
//  Created by Roald Christesen on from 16.01.2018
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainTriangle_hpp
#define GrainTriangle_hpp

#include "Math/Vec2.hpp"
#include "2d/Line.hpp"


namespace Grain {

template <class T>
class Triangle {

public:
    Vec2<T> points_[3]{};

public:
    Triangle() noexcept = default;
    Triangle(const Vec2<T>& p1, const Vec2<T>& p2, const Vec2<T>& p3) noexcept {
        points_[0] = p1;
        points_[1] = p2;
        points_[2] = p3;
    }
    virtual ~Triangle() noexcept = default;

    [[nodiscard]] virtual const char* className() const noexcept {
        return "Triangle";
    }

    friend std::ostream& operator << (std::ostream& os, const Triangle* o) {
        o == nullptr ? os << "Line nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const Triangle& o) {
        return os << o.points_[0] << " .. " << o.points_[1] << " .. " << o.points_[2] << std::endl;
    }

    [[nodiscard]] double sideLength(int16_t side_index) const noexcept {
        if (side_index < 0 || side_index > 2) {
            return 0.0;
        }
        else if (side_index < 2) {
            return points_[side_index].distance(points_[side_index + 1]);
        }
        else {
            return points_[side_index].distance(points_[0]);
        }
    }

    [[nodiscard]] double perimeter() const noexcept {
        return
                points_[0].distance(points_[1]) +
                points_[1].distance(points_[2]) +
                points_[2].distance(points_[0]);
    }

    [[nodiscard]] double area() const noexcept {
        double a = points_[0].distance(points_[1]);
        double b = points_[1].distance(points_[2]);
        double c = points_[2].distance(points_[0]);

        if (std::isnan(a) || std::isnan(b) || std::isnan(c)) {
            return 0.0;
        }

        double s = (a + b + c) / 2.0; // Semiperimeter (half perimeter)
        return std::sqrt(s * (s - a) * (s - b) * (s - c));
    }

    [[nodiscard]] double altitude(int16_t side_index) const noexcept {
        if (side_index < 0 || side_index > 2) {
            return 0.0;
        }

        double l[3];
        l[0] = points_[0].distance(points_[1]);
        l[1] = points_[1].distance(points_[2]);
        l[2] = points_[2].distance(points_[0]);

        // Semiperimeter (half perimeter)
        double s = (l[0] + l[1] + l[2]) / 2.0;
        return (2.0 * std::sqrt(s * (s - l[0]) * (s - l[1]) * (s - l[2]))) / l[side_index];
    }

    Vec2<T> centroid() const noexcept {
        return Vec2<T>(
            (points_[0].x_ + points_[1].x_ + points_[2].x_) / 3.0,
            (points_[0].y_ + points_[1].y_ + points_[2].y_) / 3.0);
    }

    Triangle offsettedTriangle(double distance) const noexcept {
        Vec2d v1 = points_[1] - points_[0];
        Vec2d v2 = points_[2] - points_[1];
        Vec2d v3 = points_[0] - points_[2];

        v1.normalize();
        v2.normalize();
        v3.normalize();

        v1.ortho();
        v2.ortho();
        v3.ortho();

        v1.setLength(distance);
        v2.setLength(distance);
        v3.setLength(distance);

        Line line1(points_[0] + v1, points_[1] + v1);
        Line line2(points_[1] + v2, points_[2] + v2);
        Line line3(points_[2] + v3, points_[0] + v3);

        Triangle new_triangle;
        line3.intersects(line1, new_triangle.points_[0]);
        line1.intersects(line2, new_triangle.points_[1]);
        line2.intersects(line3, new_triangle.points_[2]);

        return new_triangle;
    }
};


// Standard types
using Trianglei = Triangle<int32_t>;    ///< 32 bit integer
using Trianglel = Triangle<int64_t>;    ///< 64 bit integer
using Trianglef = Triangle<float>;      ///< 32 bit floating point
using Triangled = Triangle<double>;     ///< 64 bit floating point


} // End of namespace Grain

#endif // GrainTriangle_hpp
