//
//  Superellipse.hpp
//
//  Created by Roald Christesen on from 03.05.25.
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "2d/Superellipse.hpp"
#include "Bezier/Bezier.hpp"


namespace Grain {


    /**
     *  @brief Parametric form of the superellipse
     */
    Vec2d Superellipse::posAtT(double t, double a, double b, double n) noexcept {

        t *= 2.0 * std::numbers::pi;

        double cos_t = cos(t);
        double sin_t = sin(t);
        double x = a * _sign(cos_t) * pow(fabs(cos_t), 2.0 / n);
        double y = b * _sign(sin_t) * pow(fabs(sin_t), 2.0 / n);

        return (Vec2d){ x, y };
    }


    /**
     *  @brief Derivative w.r.t t (tangent vector)
     */
    Vec2d Superellipse::tangentAtT(double t, double a, double b, double n) noexcept {

        t *= 2.0 * std::numbers::pi;

        double cos_t = std::cos(t), sin_t = std::sin(t);
        double cos_a = std::pow(std::abs(cos_t), 2.0 / n - 1.0);
        double sin_a = std::pow(std::abs(sin_t), 2.0 / n - 1.0);
        double dx = -a * (2.0 / n) * cos_a * sin_t;
        double dy =  b * (2.0 / n) * sin_a * cos_t;
        dx *= (cos_t == 0 ? 0 : std::copysign(1.0, cos_t));
        dy *= (sin_t == 0 ? 0 : std::copysign(1.0, sin_t));

        return Vec2d(dx, dy);
    }

} // End of namespace Grain
