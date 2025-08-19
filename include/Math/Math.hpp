//
//  Math.hpp
//
//  Created by Roald Christesen on 02.10.2017
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 12.07.2025
//

#ifndef GrainMath_hpp
#define GrainMath_hpp

#include <cstdint>
#include <cmath>
#include <numbers>
#include <limits>
#include <algorithm>


namespace Grain {

    class Math {
    public:
        enum EaseMode {
            InSine = 0, OutSine, InOutSine,
            InQuad, OutQuad, InOutQuad,
            InCubic, OutCubic, InOutCubic,
            InQuart, OutQuart, InOutQuart,
            InQuint, OutQuint, InOutQuint,
            InExpo, OutExpo, InOutExpo,
            InCirc, OutCirc, InOutCirc,
            InBack, OutBack, InOutBack,
            InElastic, OutElastic, InOutElastic,
            InBounce, OutBounce, InOutBounce,

            First = 0,
            Last = InOutBounce
        };

    public:
        [[nodiscard]] static int64_t roundToNearestPowerOfTwo(double value) noexcept;
        [[nodiscard]] static int64_t greatestCommonDivisor(int64_t a, int64_t b) noexcept;
        [[nodiscard]] static int64_t stepsToReachAtOrAfterInt(int64_t x, int64_t step, int64_t start) noexcept;
        [[nodiscard]] static int64_t stepsToReachAtOrAfter(double x, double step, double start) noexcept;
        [[nodiscard]] static int64_t factorial(int32_t n) noexcept;
        [[nodiscard]] static int64_t sumN(int32_t n) noexcept;


        [[nodiscard]] static int32_t solveQuadratic(double a, double b, double c, double (&out_values)[2])  noexcept;
        [[nodiscard]] static int32_t solveCubic(double a, double b, double c, double d, double (&out_values)[3]) noexcept;
        [[nodiscard]] static int32_t solveCubicBezier(double p0, double p1, double p2, double p3, double p, double (&out_values)[3]) noexcept;

        [[nodiscard]] static double cubicInterpolate(double p[4], double x) noexcept;
        [[nodiscard]] static double bicubicInterpolate(double p[4][4], double x, double y) noexcept;
        [[nodiscard]] static double tricubicInterpolate(double p[4][4][4], double x, double y, double z) noexcept;
        [[nodiscard]] static double nCubicInterpolate(int32_t n, double* p, double coordinates[]) noexcept;

        // Circle
        [[nodiscard]] static double circleArea(double radius) { return std::numbers::pi * radius * radius; }
        [[nodiscard]] static double circumference(double radius) { return std::numbers::pi * 2.0 * radius; }
        [[nodiscard]] static double circleRadiusFromArea(double area) { return std::sqrt(area / std::numbers::pi); }

        // Sphere
        [[nodiscard]] static double sphereArea(double radius) { return 4.0 * std::numbers::pi * radius * radius; }

        [[nodiscard]] static double partialSphereArea(double angle, double radius) {
            double angle_radians = angle * std::numbers::pi / 180.0;
            return 2.0 * std::numbers::pi * radius * radius * (1.0 - std::cos(angle_radians));
        }

        [[nodiscard]] static double distanceOnSphere(double radius, double lat1, double lon1, double lat2, double lon2) noexcept;
        [[nodiscard]] static double distanceOnEarth(double lat1, double lon1, double lat2, double lon2) noexcept;

        // Optics
        [[nodiscard]] static double angleOfView(double sensor_size, double focal_length) {
            return 2.0 * std::atan(sensor_size / (2.0 * focal_length)) * 180.0 / std::numbers::pi;
        }

        // Bit
        [[nodiscard]] static inline double bitReduction(double value, double bit_depth) {
            double f = std::pow(2, bit_depth - 1);
            return static_cast<float>((int)std::round(value * f)) / f;
        }

        // Easing, see https://easings.net
        [[nodiscard]] static double ease(EaseMode mode, double t) {
            switch (mode) {
                case EaseMode::InSine: return easeInSine(t);
                case EaseMode::OutSine: return easeOutSine(t);
                case EaseMode::InOutSine: return easeInOutSine(t);
                case EaseMode::InQuad: return easeInQuad(t);
                case EaseMode::OutQuad: return easeOutQuad(t);
                case EaseMode::InOutQuad: return easeInOutQuad(t);
                case EaseMode::InCubic: return easeInCubic(t);
                case EaseMode::OutCubic: return easeOutCubic(t);
                case EaseMode::InOutCubic: return easeInOutCubic(t);
                case EaseMode::InQuart: return easeInQuart(t);
                case EaseMode::OutQuart: return easeOutQuart(t);
                case EaseMode::InOutQuart: return easeInOutQuart(t);
                case EaseMode::InQuint: return easeInQuint(t);
                case EaseMode::OutQuint: return easeOutQuint(t);
                case EaseMode::InOutQuint: return easeInOutQuint(t);
                case EaseMode::InExpo: return easeInExpo(t);
                case EaseMode::OutExpo: return easeOutExpo(t);
                case EaseMode::InOutExpo: return easeInOutExpo(t);
                case EaseMode::InCirc: return easeInCirc(t);
                case EaseMode::OutCirc: return easeOutCirc(t);
                case EaseMode::InOutCirc: return easeInOutCirc(t);
                case EaseMode::InBack: return easeInBack(t);
                case EaseMode::OutBack: return easeOutBack(t);
                case EaseMode::InOutBack: return easeInOutBack(t);
                case EaseMode::InElastic: return easeInElastic(t);
                case EaseMode::OutElastic: return easeOutElastic(t);
                case EaseMode::InOutElastic: return easeInOutElastic(t);
                case EaseMode::InBounce: return easeInBounce(t);
                case EaseMode::OutBounce: return easeOutBounce(t);
                case EaseMode::InOutBounce: return easeInOutBounce(t);
                default: return t;
            }
        }

        [[nodiscard]] static const char* easeModeName(EaseMode mode) {
            static const char* names[] = {
                    "InSine", "OutSine", "InOutSine",
                    "InQuad", "OutQuad", "InOutQuad",
                    "InCubic", "OutCubic", "InOutCubic",
                    "InQuart", "OutQuart", "InOutQuart",
                    "InQuint", "OutQuint", "InOutQuint",
                    "InExpo", "OutExpo", "InOutExpo",
                    "InCirc", "OutCirc", "InOutCirc",
                    "InBack", "OutBack", "InOutBack",
                    "InElastic", "OutElastic", "InOutElastic",
                    "InBounce", "OutBounce", "InOutBounce",
                    "Unknown"
            };
            if (mode >= EaseMode::First && mode <= EaseMode::Last) {
                return names[static_cast<int32_t>(mode)];
            }
            else {
                return names[static_cast<int32_t>(EaseMode::Last) + 1];
            }
        }

        [[nodiscard]] static int32_t easeModeCount() { return 1 + EaseMode::Last; }


        [[nodiscard]] static double easeInSine(double t) { return 1.0 - std::cos((t * std::numbers::pi) * 0.5); }
        [[nodiscard]] static double easeOutSine(double t) { return std::sin((t * std::numbers::pi) * 0.5); }
        [[nodiscard]] static double easeInOutSine(double t) { return -(std::cos(std::numbers::pi * t) - 1.0) * 0.5; }

        [[nodiscard]] static double easeInQuad(double t) { return t * t; }
        [[nodiscard]] static double easeOutQuad(double t) { return 1.0 - (1.0 - t) * (1.0 - t); }
        [[nodiscard]] static double easeInOutQuad(double t) { return t < 0.5 ? 2.0 * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 2.0) * 0.5; }

        [[nodiscard]] static double easeInCubic(double t) { return t * t * t; }
        [[nodiscard]] static double easeOutCubic(double t) { return 1.0 - std::pow(1.0 - t, 3); }
        [[nodiscard]] static double easeInOutCubic(double t) { return t < 0.5 ? 4.0 * t * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 3.0) * 0.5; }

        [[nodiscard]] static double easeInQuart(double t) { return t * t * t * t; }
        [[nodiscard]] static double easeOutQuart(double t) { return 1.0 - std::pow(1.0 - t, 4.0); }
        [[nodiscard]] static double easeInOutQuart(double t) { return t < 0.5 ? 8.0 * t * t * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 4.0) * 0.5; }

        [[nodiscard]] static double easeInQuint(double t) { return t * t * t * t * t; }
        [[nodiscard]] static double easeOutQuint(double t) { return 1.0 - std::pow(1.0 - t, 5.0); }
        [[nodiscard]] static double easeInOutQuint(double t) {
            return t < 0.5 ? 16.0 * t * t * t * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 5.0) * 0.5;
        }

        [[nodiscard]] static double easeInExpo(double t) {
            return std::fabs(t) < kEpsilon ? 0.0 : std::pow(2.0, 10.0 * t - 10);
        }
        [[nodiscard]] static double easeOutExpo(double t) {
            return std::fabs(1.0 - t) < kEpsilon ? 1.0 : 1.0 - std::pow(2.0, -10.0 * t);
        }
        [[nodiscard]] static double easeInOutExpo(double t) {
            return std::fabs(t) < kEpsilon
                   ? 0.0
                   : std::abs(1.0 - t) < kEpsilon
                     ? 1.0
                     : t < 0.5 ? std::pow(2.0, 20.0 * t - 10.0) * 0.5
                               : (2.0 - std::pow(2.0, -20.0 * t + 10.0)) * 0.5;
        }

        [[nodiscard]] static double easeInCirc(double t) { return 1.0 - std::sqrt(1.0 - std::pow(t, 2.0)); }
        [[nodiscard]] static double easeOutCirc(double t) { return std::sqrt(1 - std::pow(t - 1.0, 2.0)); }
        [[nodiscard]] static double easeInOutCirc(double t) {
            return t < 0.5
                   ? (1.0 - std::sqrt(1.0 - std::pow(2.0 * t, 2.0))) * 0.5
                   : (std::sqrt(1.0 - std::pow(-2.0 * t + 2.0, 2.0)) + 1.0) * 0.5;
        }

        [[nodiscard]] static double easeInBack(double t) { return 2.70158 * t * t * t - 1.70158 * t * t; }
        [[nodiscard]] static double easeOutBack(double t) { return 1.0 + 2.70158 * std::pow(t - 1.0, 3.0) + 1.70158 * std::pow(t - 1.0, 2.0); }
        [[nodiscard]] static double easeInOutBack(double t) {
            return t < 0.5
                   ? (std::pow(2.0 * t, 2.0) * (3.5949095 * 2.0 * t - 2.5949095)) * 0.5
                   : (std::pow(2.0 * t - 2.0, 2.0) * (3.5949095 * (t * 2.0 - 2.0) + 2.5949095) + 2.0) * 0.5;
        }

        [[nodiscard]] static double easeInElastic(double t) {
            return std::fabs(t) < kEpsilon
                   ? 0.0
                   : std::fabs(1.0 - t) < kEpsilon
                     ? 1.0
                     : -std::pow(2.0, 10.0 * t - 10.0) * std::sin((t * 10.0 - 10.75) * ((2.0 * std::numbers::pi) / 3.0));
        }
        [[nodiscard]] static double easeOutElastic(double t) {
            return std::fabs(t) < kEpsilon
                   ? 0.0
                   : std::fabs(1.0 - t) < kEpsilon
                     ? 1.0
                     : std::pow(2.0, -10.0 * t) * std::sin((t * 10.0 - 0.75) * ((2.0 * std::numbers::pi) / 3.0)) + 1.0;
        }
        [[nodiscard]] static double easeInOutElastic(double t) {
            return std::fabs(t) < kEpsilon
                   ? 0.0
                   : std::fabs(1.0 - t) < kEpsilon
                     ? 1.0
                     : t < 0.5
                       ? -(std::pow(2.0, 20.0 * t - 10.0) * std::sin((20.0 * t - 11.125) * ((2.0 * std::numbers::pi) / 4.5))) * 0.5
                       : (std::pow(2.0, -20.0 * t + 10.0) * std::sin((20.0 * t - 11.125) * ((2.0 * std::numbers::pi) / 4.5))) * 0.5 + 1.0;
        }

        [[nodiscard]] static double easeInBounce(double t) { return 1 - easeOutBounce(1.0 - t); }
        [[nodiscard]] static double easeOutBounce(double t) {
            static const double n1 = 7.5625;
            static const double d1 = 2.75;
            if (t < 1.0 / d1) {
                return n1 * t * t;
            }
            else if (t < 2.0 / d1) {
                t -= 1.5;
                return n1 * (t / d1) * t + 0.75;
            }
            else if (t < 2.5 / d1) {
                t -= 2.25;
                return n1 * (t / d1) * t + 0.9375;
            }
            else {
                t -= 2.625;
                return n1 * (t / d1) * t + 0.984375;
            }
        }
        [[nodiscard]] static double easeInOutBounce(double t) {
            return t < 0.5
                   ? (1.0 - easeOutBounce(1.0 - 2.0 * t)) * 0.5
                   : (1.0 + easeOutBounce(2.0 * t - 1.0)) * 0.5;
        }


        [[nodiscard]] static double quotient(double dividend, double divisor) {
            return (divisor <= kEpsilon) ? std::numeric_limits<double>::quiet_NaN() : dividend / divisor;
        }

        [[nodiscard]] static double percent(double part, double full) {
            return (full <= kEpsilon) ? std::numeric_limits<double>::quiet_NaN() : part / full * 100.0;
        }

        [[nodiscard]] static double xpery(double x, double y) {
            return (y <= kEpsilon) ? std::numeric_limits<double>::quiet_NaN() : x / y;
        }

        [[nodiscard]] static int64_t xpery_int(double x, double y) {
            return static_cast<int64_t>((y <= kEpsilon) ? std::numeric_limits<double>::quiet_NaN() : std::round(x / y));
        }


        [[nodiscard]] static float powf_inverse(float value) noexcept { return static_cast<float>(std::log(value) / std::numbers::ln2); }
        [[nodiscard]] static float powf_inverse(float value, float factor) noexcept { return std::log(value) / std::log(factor); }
        [[nodiscard]] static double pow_inverse(double value) noexcept { return std::log(value) / std::numbers::ln2; }
        [[nodiscard]] static double pow_inverse(double value, double factor) noexcept { return std::log(value) / std::log(factor); }

        [[nodiscard]] static int64_t next_pow2(int64_t x) noexcept;
        [[nodiscard]] static int32_t pad_two(int32_t length) noexcept;

        /**
         *  @brief Calculate secant.
         *
         *  @param angle Angle in degree.
         *  @return Secant or NaN
         */
        [[nodiscard]] inline static double secant(double angle) noexcept {
            double c = std::cos(degtorad(angle));
            return c != 0.0 ? 1.0 / c : std::numeric_limits<double>::quiet_NaN();
        }

        [[nodiscard]] inline static double degtorad(double deg) { return deg * std::numbers::pi / 180.0; }
        [[nodiscard]] inline static double radtodeg(double rad) { return rad * 180.0 / std::numbers::pi; }

        [[nodiscard]] inline static double lerp(double a, double b, double t) noexcept { return a + t * (b - a); }
        [[nodiscard]] inline static float lerpf(float a, float b, float t) noexcept { return a + t * (b - a); }
        [[nodiscard]] inline static double invlerp(double a, double b, double v) noexcept { return a != b ? (v - a) / (b - a) : 0.0; }
        [[nodiscard]] inline static float invlerpf(float a, float b, float v) noexcept { return a != b ? (v - a) / (b - a) : 0.0f; }
        [[nodiscard]] inline static double remap(double i_min, double i_max, double o_min, double o_max, double v) noexcept {
            return i_max != i_min ? ((v - i_min) / (i_max - i_min)) * (o_max - o_min) + o_min : o_min;
        }
        [[nodiscard]] inline static float remapf(float i_min, float i_max, float o_min, float o_max, float v) noexcept {
            return i_max != i_min ? ((v - i_min) / (i_max - i_min)) * (o_max - o_min) + o_min : o_min;
        }
        [[nodiscard]] inline static double remapnorm(double i_min, double i_max, double v) noexcept {
            return i_max != i_min ? ((v - i_min) / (i_max - i_min)) : 0.0;
        }
        [[nodiscard]] inline static float remapnormf(float i_min, float i_max, float v) noexcept {
            return i_max != i_min ? ((v - i_min) / (i_max - i_min)) : 0.0f;
        }
        [[nodiscard]] inline static double remapclamped(double i_min, double i_max, double o_min, double o_max, double v) noexcept {
            double result = i_max != i_min ? ((v - i_min) / (i_max - i_min)) * (o_max - o_min) + o_min : o_min;
            return result < o_min ? o_min : result > o_max ? o_max : result;
        }
        [[nodiscard]] inline static float remapclampedf(float i_min, float i_max, float o_min, float o_max, float v) noexcept {
            double result = i_max != i_min ? ((v - i_min) / (i_max - i_min)) * (o_max - o_min) + o_min : o_min;
            return result < o_min ? o_min : result > o_max ? o_max : result;
        }
        [[nodiscard]] inline static double unitstep(double threshold, double v) noexcept {
            return v < threshold ? 0.0 : 1.0;
        }
        [[nodiscard]] inline static float unitstepf(float threshold, float v) noexcept {
            return v < threshold ? 0.0f : 1.0f;
        }
        [[nodiscard]] inline static double unitstep(double threshold, double range, double v) noexcept {
            if (v < threshold - range) return 0.0;
            if (v > threshold + range) return 1.0;
            return remapnorm(threshold - range, threshold + range, v);
        }
        [[nodiscard]] inline static float unitstepf(float threshold, float range, float v) noexcept {
            if (v < threshold - range) return 0.0f;
            if (v > threshold + range) return 1.0f;
            return remapnormf(threshold - range, threshold + range, v);
        }

        [[nodiscard]] static double smoothstep(double t) noexcept { return  t * t * (3.0 - 2.0 * t); }
        [[nodiscard]] static float smoothstepf(float t) noexcept { return  t * t * (3.0f - 2.0f * t); }
        [[nodiscard]] static double smoothstep(double a, double b, double t) noexcept {
            t = t * t * (3.0 - 2.0 * t);
            return a + t * (b - a);
        }
        [[nodiscard]] static float smoothstepf(float a, float b, float t) noexcept {
            t = t * t * (3.0f - 2.0f * t);
            return a + t * (b - a);
        }

        [[nodiscard]] static double smootherstep(double t) noexcept { return t * t * t * (t * (6.0 * t - 15.0) + 10.0); }
        [[nodiscard]] static float smootherstepf(float t) noexcept { return t * t * t * (t * (6.0f * t - 15.0f) + 10.0f); }
        [[nodiscard]] static double smootherstep(double a, double b, double t) noexcept {
            t = t * t * t * (t * (6.0 * t - 15.0) + 10.0);
            return a + t * (b - a);
        }
        [[nodiscard]] static float smootherstepf(float a, float b, float t) noexcept {
            t = t * t * t * (t * (6.0f * t - 15.0f) + 10.0f);
            return a + t * (b - a);
        }

        static void buildPowLookUpTable(float power, int32_t resolution, float* out_lut) noexcept {
            if (out_lut && resolution >= 2) {
                float f = 1.0f / static_cast<float>(resolution - 1);
                for (int32_t i = 0; i < resolution; i++) {
                    out_lut[i] = std::pow(f * static_cast<float>(i), power);
                }
            }
        }

        [[nodiscard]] static double gaussKernel(double x, double sigma_sqr) noexcept {
            // TODO: Check formula!
            double p = std::pow(M_E, -((x * x) / (sigma_sqr * 2)));
            return (1.0 / (std::sqrt(std::numbers::pi * 2 * sigma_sqr))) * p;
        }

        [[nodiscard]] static double sinc(double x) noexcept {
            return x != 0 ? std::sin(x) / x : 1;
            // TODO: Check formula!
            // return x == 0.0 ? 1.0 : std::sin(std::numbers::pi * x) / (std::numbers::pi * x);
        }

        [[nodiscard]] static double bessel(double x) noexcept {

            double sum = 1.0;

            for (int32_t i = 1; i < 10; i++) {
                double x_to_i_power = std::pow(x / 2.0, static_cast<double>(i));
                double factorial = 1.0;
                for (int32_t j = 1; j <= i; j++) {
                    factorial *= j;
                }
                sum += std::pow(x_to_i_power / factorial, 2.0);
            }

            return sum;
        }


        template <typename T>
        T sortArray(T* array, size_t size) {
            std::sort(array, array + size);
        }

        /**
         *  @brief Computes the median of a subrange in an array.
         *
         *  This function calculates the median of a specified subrange within an array.
         *  The median is the middle value when the number of elements is odd, and the
         *  average of the two middle values when the number of elements is even.
         *
         *  @tparam T The type of elements in the array (must support numeric operations).
         *  @param array Pointer to the array containing the data.
         *  @param start The starting index (inclusive) of the subrange.
         *  @param end The ending index (inclusive) of the subrange.
         *  @return The median value as a double. Returns 0.0 if the input is invalid.
         *
         *  @note The array should be sorted for meaningful median calculations.
         *  @warning No bounds checking is performed on `array`. Ensure `start` and `end`
         *           are within the valid array range.
         */
        template <typename T>
        [[nodiscard]] double medianInArray(T* array, size_t start, size_t end) {
            if (!array || start > end) {
                return 0.0;
            }
            size_t size = end - start + 1;
            if (size % 2 == 0) {
                return (static_cast<double>(array[start + size / 2 - 1]) +
                        static_cast<double>(array[start + size / 2])) / 2.0;
            }
            else {
                return static_cast<double>(array[start + size / 2]);
            }
        }

        /**
         *  @brief Computes the sum of an array of doubles using the
         *         Neumaier variant of the Kahan-Babuska summation algorithm.
         *
         *  This function reduces floating-point rounding errors by keeping
         *  a separate compensation (error) term while summing the input values.
         *  It is more accurate than a naive summation, especially when the
         *  numbers vary significantly in magnitude.
         *
         *  @param values Pointer to the array of double values to sum.
         *  @param value_n Number of elements in the array.
         *  @return The numerically stable sum of the values in the array.
         *
         *  @note If the input pointer is null, the function returns 0.0.
         *
         *  @see https://en.wikipedia.org/wiki/Kahan_summation_algorithm
         */
        [[nodiscard]] inline double sum(const double* values, int64_t value_n) {
            if (values != nullptr) {
                double sum = values[0];
                double err = 0.0;
                for (int64_t i = 1; i < value_n; i++) {
                    const double k = values[i];
                    const double m = sum + k;
                    err += std::fabs(sum) >= std::fabs(k) ? sum - m + k : k - m + sum;
                    sum = m;
                }
                return sum + err;
            }
            else {
                return 0.0;
            }
        }
    public:
        static constexpr double kTau = std::numbers::pi * 2;
        static constexpr double kEpsilon = std::numeric_limits<double>::epsilon();
        static constexpr double kEpsilonFloat = std::numeric_limits<float>::epsilon();
    };


    class ValueMapper {
    public:
        ValueMapper() {}
        ValueMapper(double in_min, double in_max, double out_min, double out_max) {
            m_in_min = in_min;
            m_in_max = in_max;
            m_out_min = out_min;
            m_out_max = out_max;
            _update();
        }
        ~ValueMapper() {}

        void set(double in_min, double in_max, double out_min, double out_max) {
            m_in_min = in_min;
            m_in_max = in_max;
            m_out_min = out_min;
            m_out_max = out_max;
            _update();
        }

        void setIn(double min, double max) noexcept {
            m_in_min = min;
            m_in_max = max;
            _update();
        }

        void setOut(double min, double max) noexcept {
            m_out_min = min;
            m_out_max = max;
            _update();
        }

        [[nodiscard]] double remap(double v) const noexcept {
            if (_m_in_valid) {
                return m_out_min + ((v - m_in_min) / _m_in_range) * _m_out_range;
            }
            else {
                return m_out_min;
            }
        }

        [[nodiscard]] double remapclamped(double v) const noexcept {
            if (_m_in_valid) {
                v = m_out_min + ((v - m_in_min) / _m_in_range) * _m_out_range;
                return v < m_out_min ? m_out_min : v > m_out_max ? m_out_max : v;
            }
            else {
                return m_out_min;
            }
        }

        void _update() noexcept {
            _m_in_range = (m_in_max - m_in_min);
            _m_in_valid = std::fabs(_m_in_range) > std::numeric_limits<double>::epsilon();
            _m_out_range = (m_out_max - m_out_min);
            _m_out_valid = std::fabs(_m_in_range) > std::numeric_limits<double>::epsilon();
        }

    public:
        double m_in_min = 0.0;
        double m_in_max = 1.0;
        double m_out_min = 0.0;
        double m_out_max = 1.0;

        double _m_in_range;
        double _m_out_range;
        bool _m_in_valid;
        bool _m_out_valid;
    };


} // End of namespace Grain

#endif // GrainMath_hpp
