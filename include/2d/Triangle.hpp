//
//  Triangle.hpp
//
//  Created by Roald Christesen on from 16.01.2018
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainTriangle_hpp
#define GrainTriangle_hpp

#include "Math/Vec2.hpp"


namespace Grain {

    template <class T>
    class Triangle {
    public:
        Vec2<T> m_points[3]{};

    public:
        Triangle() noexcept {}
        Triangle(const Vec2<T>& p1, const Vec2<T>& p2, const Vec2<T>& p3) noexcept;

        double sideLength(int16_t side_index) const noexcept;
        double perimeter() const noexcept;
        double area() const noexcept;
        double altitude(int16_t side_index) const noexcept;

        Vec2<T> centroid() const noexcept;
        Triangle offsettedTriangle(double distance) const noexcept;
    };


    // Standard types
    using Trianglei = Triangle<int32_t>;    ///< 32 bit integer.
    using Trianglel = Triangle<int64_t>;    ///< 64 bit integer.
    using Trianglef = Triangle<float>;      ///< 32 bit floating point.
    using Triangled = Triangle<double>;     ///< 64 bit floating point.


} // End of namespace Grain

#endif // GrainTriangle_hpp
