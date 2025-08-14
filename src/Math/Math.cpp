//
//  Math.cpp
//
//  Created by Roald Christesen on 02.10.2017
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Math/Math.hpp"


namespace Grain {


    /**
     *  @brief Rounds a value to the nearest power of two.
     *
     *  This function computes the base-2 logarithm of the input value, rounds it to
     *  the nearest integer exponent, and returns 2 raised to that exponent.
     *  If the input value is less than or equal to zero, the function returns 1
     *  to avoid undefined behavior from log2(0) or negative values.
     *
     *  @param value A positive floating-point number to be rounded.
     *  @return The nearest power of two as a 64-bit signed integer.
     *
     *  @note For values >= 2^63, the result may overflow and wrap around.
     */
    int64_t Math::roundToNearestPowerOfTwo(double value) noexcept {

        if (value <= 0) {
            // Avoid log(0) or negative input
            return 1;
        }
        else {
            int32_t exp = static_cast<int32_t>(std::round(std::log2(value)));
            return int64_t(1) << exp;
        }
    }


    /**
     *  @brief Greatest common divisor (GCD) for values a and b.
     *
     *  The greatest common divisor (GCD) of two nonzero integers a and b is the
     *  largest positive integer that divides both a and b without leaving a remainder.
     *  In other words, it is the highest number that can divide both a and b evenly.
     */
    int64_t Math::greatestCommonDivisor(int64_t a, int64_t b) noexcept {

        return b == 0 ? a : greatestCommonDivisor(b, a % b);
    }


    /**
     *  @brief Computes how many steps are needed from `v` to reach or exceed `start`.
     *
     *  Given an initial value `v`, a positive step size `step`, and a target `start`,
     *  this function calculates how many full `step` increments from `v` are needed
     *  to reach a value that is >= `start`.
     *
     *  @param v     The starting value of the sequence.
     *  @param step  The step increment (must be non-zero and positive).
     *  @param start The minimum target value.
     *  @return The number of steps needed to reach or exceed `start` from `v`.
     *
     *  @note If `step == 0`, the function returns 0.
     */
    int64_t Math::stepsToReachAtOrAfterInt(int64_t v, int64_t step, int64_t start) noexcept {

        if (step == 0) {
            return 0;
        }
        else {
            return (start - v + step - 1) / step;  // integer round up
        }
    }


    /**
     *  @brief Computes how many steps are needed from `v` to reach or exceed `start`
     *         (double version).
     *
     *  Given a starting value `v`, a step size `step`, and a target `start`, this
     *  function returns how many steps of size `step` are required such that
     *  `v + n * step >= start`.
     *
     *  @param v     The starting value of the sequence.
     *  @param step  The step increment (must be non-zero).
     *  @param start The minimum target value.
     *  @return The number of steps needed to reach or exceed `start` from `v`.
     *
     *  @note Works for both positive and negative step directions.
     *        If `step == 0.0`, the function returns 0.
     */
    int64_t Math::stepsToReachAtOrAfter(double v, double step, double start) noexcept {

        if (step == 0.0) {
            return 0;
        }
        else {
            return static_cast<int64_t>(std::ceil((start - v) / step));
        }
    }


    /**
     *  @brief Factorial if n or -1, if the result would exceed the capacity of a 64-bit signed integer.
     *
     *  The factorial of a non-negative integer n, denoted as n!, is the product of all positive integers from 1 to n. In other words, it is calculated by multiplying the number with all the positive integers smaller than it.
     */
    int64_t Math::factorial(int32_t n) noexcept {

        static const int64_t results[] = {
            1,
            1,
            2,
            6,
            24,
            120,
            720,
            5040,
            40320,
            362880,
            3628800,
            39916800,
            479001600,
            6227020800,
            87178291200,
            1307674368000,
            20922789888000,
            355687428096000,
            6402373705728000,
            121645100408832000,
            2432902008176640000
        };

        return n >= 0 && n <= 20 ? results[n] : -1;
    }


    /**
     *  @brief Sum of all integers from 1 up to n.
     */
    int64_t Math::sumN(int32_t n) noexcept {

        if (n == 0) {
            return 0;
        }

        if (n < 0) {
            n = -n;
        }

        return ((n + 1) * n) / 2;
    }


    inline void _gr_pushRoot(double root, double* out_values, int32_t& index) {

        if (root >= 0.0 && root <= 1.0) {
            out_values[index++] = root;
        }
    }


    int32_t Math::solveQuadratic(double a, double b, double c, double (&out_values)[2]) noexcept {

        double discr = b * b - 4.0 * a * c;
        if (discr < 0) {
            return 0;
        }

        if (discr == 0.0) {
            out_values[0] = -0.5f * b / a;
            return 1;
        }
        else {
            double q = (b > 0) ? -0.5f * (b + std::sqrt(discr)) : -0.5f * (b - std::sqrt(discr));
            out_values[0] = q / a;
            out_values[1] = c / q;
            if (out_values[0] > out_values[1]) {
                double temp = out_values[0];
                out_values[0] = out_values[1];
                out_values[1] = temp;
            }
            return 2;
        }
    }


    int32_t Math::solveCubic(double a, double b, double c, double d, double (&out_values)[3]) noexcept {

        if (a == 0.0) {
            double quadratic[2];
            int32_t n = Math::solveQuadratic(b, c, d, quadratic);
            out_values[0] = quadratic[0];
            out_values[1] = quadratic[1];
            return n;
        }

        b /= a;
        c /= a;
        d /= a;

        double p = (3.0 * c - b * b) / 3.0;
        double q = (2.0 * b * b * b - 9.0 * b * c + 27.0 * d) / 27.0;

        int32_t index = 0;

        if (p == 0.0) {
            _gr_pushRoot(std::pow(-q, 1.0 / 3.0), out_values, index);
        }
        else if (q == 0.0) {
            _gr_pushRoot(std::sqrt(-p), out_values, index);
            _gr_pushRoot(-std::sqrt(-p), out_values, index);
            return index;
        }
        else {
            double discriminant = std::pow(q / 2.0, 2.0) + std::pow(p / 3.0, 3.0);

            if (discriminant == 0) {
                _gr_pushRoot(std::pow(q / 2.0, 1.0 / 3.0) - b / 3.0, out_values, index);
                return index;
            }
            else if (discriminant > 0) {
                _gr_pushRoot(std::pow(-(q / 2.0) + std::sqrt(discriminant), 1.0 / 3.0) - std::pow((q / 2.0) + std::sqrt(discriminant), 1.0 / 3.0) - b / 3.0, out_values, index);
                return index;
            }
            else {
                double r = std::sqrt( std::pow(-(p / 3.0), 3.0));
                double phi = std::acos(-(q / (2.0 * std::sqrt(std::pow(-(p / 3.0), 3.0)))));
                double s = 2.0 * std::pow(r, 1.0 / 3.0);

                _gr_pushRoot(s * std::cos(phi / 3.0) - b / 3.0, out_values, index);
                _gr_pushRoot(s * std::cos((phi + 2 * std::numbers::pi) / 3.0) - b / 3.0, out_values, index);
                _gr_pushRoot(s * std::cos((phi + 4 * std::numbers::pi) / 3.0) - b / 3.0, out_values, index);
                return index;
            }
        }

        return 0;
    }


    int32_t Math::solveCubicBezier(double p0, double p1, double p2, double p3, double p, double (&out_values)[3]) noexcept {

        p0 -= p;
        p1 -= p;
        p2 -= p;
        p3 -= p;

        double a = p3 - 3.0 * p2 + 3.0 * p1 - p0;
        double b = 3.0 * p2 - 6.0 * p1 + 3.0 * p0;
        double c = 3.0 * p1 - 3.0 * p0;
        double d = p0;

        double cubic_roots[3];
        int32_t cubic_roots_n = Math::solveCubic(a, b, c, d, cubic_roots);

        int32_t index = 0;

        for (int32_t i = 0; i < cubic_roots_n; i++) {
            if (cubic_roots[i] >= 0 && cubic_roots[i] <= 1)
            _gr_pushRoot(cubic_roots[i] , out_values, index);
        }

        return index;
    }


    /**
     *  cubicInterpolate
     *  bicubicInterpolate
     *  tricubicInterpolate
     *  nCubicInterpolate
     *
     *  Based on code from Paul Breeuwsma: https://www.paulinternet.nl/?page=bicubic
     */

    double Math::cubicInterpolate(double p[4], double x) noexcept {

        return p[1] + 0.5 * x * (p[2] - p[0] + x * (2.0 * p[0] - 5.0 * p[1] + 4.0 * p[2] - p[3] + x * (3.0 * (p[1] - p[2]) + p[3] - p[0])));
    }


    double Math::bicubicInterpolate(double p[4][4], double x, double y) noexcept {

        double arr[4];

        arr[0] = cubicInterpolate(p[0], y);
        arr[1] = cubicInterpolate(p[1], y);
        arr[2] = cubicInterpolate(p[2], y);
        arr[3] = cubicInterpolate(p[3], y);

        return cubicInterpolate(arr, x);
    }


    double Math::tricubicInterpolate(double p[4][4][4], double x, double y, double z) noexcept {

        double arr[4];

        arr[0] = bicubicInterpolate(p[0], y, z);
        arr[1] = bicubicInterpolate(p[1], y, z);
        arr[2] = bicubicInterpolate(p[2], y, z);
        arr[3] = bicubicInterpolate(p[3], y, z);

        return cubicInterpolate(arr, x);
    }


    double Math::nCubicInterpolate(int32_t n, double* p, double coordinates[]) noexcept {

        if (n == 1) {
            return cubicInterpolate(p, *coordinates);
        }
        else if (n > 1) {
            double arr[4];
            int32_t skip = 1 << (n - 1) * 2;

            arr[0] = nCubicInterpolate(n - 1, p, coordinates + 1);
            arr[1] = nCubicInterpolate(n - 1, p + skip, coordinates + 1);
            arr[2] = nCubicInterpolate(n - 1, p + 2*skip, coordinates + 1);
            arr[3] = nCubicInterpolate(n - 1, p + 3*skip, coordinates + 1);

            return cubicInterpolate(arr, *coordinates);
        }
        else {
            return std::numeric_limits<double>::quiet_NaN();
        }
    }


    /**
     *  @brief Distance between two given points on a sphere.
     *
     *  Haversine formula.
     *
     *  @param radius The Radius of the given sphere.
     *  @param lat1 Latitude of point 1.
     *  @param lon1 Longitude of point 1.
     *  @param lat2 Latitude of point 2.
     *  @param lon2 Longitude of point 2.
     *
     *  @return The distance between two points on a sphere.
     */
    double Math::distanceOnSphere(double radius, double lat1, double lon1, double lat2, double lon2) noexcept {

        lon1 -= lon2;
        lon1 *= std::numbers::pi / 180.0;
        lat1 *= std::numbers::pi / 180.0;
        lat2 *= std::numbers::pi / 180.0;

        double dz = std::sin(lat1) - std::sin(lat2);
        double dx = std::cos(lon1) * std::cos(lat1) - std::cos(lat2);
        double dy = std::sin(lon1) * std::cos(lat1);

        return std::asin(std::sqrt(dx * dx + dy * dy + dz * dz) / 2.0) * 2.0 * radius;
    }


    /**
     *  @brief Distance between two given points on earth.
     *
     *  @note Assumes the earth to be a perfect sphere, which is not the case but give reasonable results for som uses. The radius of earth is approximately 6371000 m.
     *
     *  @param lat1 Latitude of point 1.
     *  @param lon1 Longitude of point 1.
     *  @param lat2 Latitude of point 2.
     *  @param lon2 Longitude of point 2.
     *
     *  @return The distance in meter between two points on earth.
     */
    double Math::distanceOnEarth(double lat1, double lon1, double lat2, double lon2) noexcept {

        return Math::distanceOnSphere(6371000.0, lat1, lon1, lat2, lon2);
    }


    int64_t Math::next_pow2(int64_t x) noexcept {

        int64_t power = 2;
        while (x >>= 1) {
            power <<= 1;
        }
        return power;
    }


    int32_t Math::pad_two(int32_t length) noexcept {

        int32_t n = 1;
        while (n < length) {
            n <<= 1;
        }
        return n;
    }

} // End of namespace Grain
