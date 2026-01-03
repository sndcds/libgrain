//
//  Vec2.hpp
//
//  Created by Roald Christesen on 30.09.2019
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 19.08.2025
//

#ifndef GrainVec2_hpp
#define GrainVec2_hpp

#include "Grain.hpp"
#include "String/CSVString.hpp"
#include "Random.hpp"
#include "File/File.hpp"

#if defined(__APPLE__) && defined(__MACH__)
    #import <CoreGraphics/CoreGraphics.h>
#endif


namespace Grain {

class File;
class String;
class CSVLineParser;


template <ScalarType T>
class Vec2 {
public:
    T x_{};
    T y_{};

public:
    constexpr Vec2() noexcept : x_(T(0)), y_(T(0)) {}
    constexpr Vec2(T x, T y) noexcept : x_(x), y_(y) {}
    constexpr Vec2(const Vec2& other) noexcept = default;

    // Templated converting constructor (only from Vec2<U>)
    template <typename U>
    explicit constexpr Vec2(const Vec2<U>& v) noexcept
        requires (!std::same_as<U,T> && std::is_convertible_v<U,T>)
        : x_(static_cast<T>(v.x_)), y_(static_cast<T>(v.y_)) {}

    explicit Vec2(const char* csv, char delimiter = ',') noexcept {
        setByCSV(csv, delimiter);
    }

    explicit Vec2(const String& csv, char delimiter = ',') noexcept {
        setByCSV(csv, delimiter);
    }

    virtual ~Vec2() noexcept = default;

    [[nodiscard]] virtual const char* className() const noexcept { return "Vec2"; }

    friend std::ostream& operator << (std::ostream& os, const Vec2* o) {
        o == nullptr ? os << "Vec2 nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const Vec2& o) {
        if (std::is_same<T, int8_t>::value || std::is_same<T, uint8_t>::value) {
            // Force to print integer values instead of chars
            return os << static_cast<int32_t>(o.x_) << ", " << static_cast<int32_t>(o.y_);
        }
        return os << o.x_ << ", " << o.y_;
    }

    Vec2& operator = (const Vec2&) = default;
    Vec2& operator = (Vec2&&) noexcept = default;
    Vec2(Vec2&&) = default;

    // Template specialization for assigning from Vec2 of type U to Vec2 of type T
    template <typename U>
    requires std::is_convertible_v<U, T>
    Vec2<T>& operator = (const Vec2<U>& other) {
        x_ = static_cast<T>(other.x_);
        y_ = static_cast<T>(other.y_);
        return *this;
    }


#if defined(__APPLE__) && defined(__MACH__)
    Vec2& operator = (const CGPoint& p) {
        x_ = static_cast<T>(p.x);
        y_ = static_cast<T>(p.y);
        return *this;
    }
#endif

    constexpr bool operator == (const Vec2& other) const noexcept {
        return x_ == other.x_ && y_ == other.y_;
    }

    constexpr bool operator != (const Vec2& other) const noexcept {
        return !(*this == other);
    }

    constexpr Vec2 operator-() const noexcept {
        return Vec2(-x_, -y_);
    }

    constexpr Vec2 operator + (const Vec2& other) const noexcept {
        return Vec2(x_ + other.x_, y_ + other.y_);
    }

    constexpr Vec2 operator - (const Vec2& other) const noexcept {
        return Vec2(x_ - other.x_, y_ - other.y_);
    }

    constexpr Vec2 operator * (const Vec2& other) const noexcept {
        return Vec2(x_ * other.x_, y_ * other.y_);
    }

    template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    constexpr Vec2<std::common_type_t<T, U>> operator * (U s) const noexcept {
        using R = std::common_type_t<T, U>;
        return Vec2<R>(static_cast<R>(x_) * s, static_cast<R>(y_) * s);
    }

    template <typename U>
    constexpr Vec2<std::common_type_t<T, U>> operator / (U s) const noexcept {
        using R = std::common_type_t<T, U>;
        return Vec2<R>(static_cast<R>(x_) / s, static_cast<R>(y_) / s);
    }


    Vec2 operator * (T s) const { return Vec2(x_ * s, y_ * s); }

    constexpr Vec2& operator += (const Vec2& other) noexcept {
        x_ += other.x_;
        y_ += other.y_;
        return *this;
    }

    constexpr Vec2& operator -= (const Vec2& other) noexcept {
        x_ -= other.x_;
        y_ -= other.y_;
        return *this;
    }

    constexpr Vec2& operator *= (const Vec2& other) noexcept {
        x_ *= other.x_;
        y_ *= other.y_;
        return *this;
    }

    template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    constexpr Vec2& operator *= (U s) noexcept {
        x_ = static_cast<T>(x_ * s);
        y_ = static_cast<T>(y_ * s);
        return *this;
    }

    template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    constexpr Vec2& operator /= (U s) noexcept {
        x_ = static_cast<T>(x_ / s);
        y_ = static_cast<T>(y_ / s);
        return *this;
    }

    // Non-const array operator version for assignment
    T& operator[](size_t index) {
        return (index % 2) == 0 ? x_ : y_;
    }

    // Const version for read-only access
    const T& operator[](size_t index) const {
        return (index % 2) == 0 ? x_ : y_;
    }

    [[nodiscard]] T x() const noexcept { return x_; }
    [[nodiscard]] T y() const noexcept { return y_; }
    [[nodiscard]] double length() const noexcept { return std::sqrt(x_ * x_ + y_ * y_); }

    /**
     *  @brief Computes the squared length of the 2D vector.
     *
     *  This is faster than computing the actual length and is
     *  useful for comparisons without requiring a square root.
     *
     *  @return The squared length of the vector.
     */
    [[nodiscard]] double squaredLength() const noexcept {
        return x_ * x_ + y_ * y_;
    }

    /**
     *  @brief Computes the Euclidean distance between this vector and another
     *         vector `v`.
     *
     *  @param v The vector to which the distance is calculated.
     *  @return The Euclidean distance between this vector and the vector `v`.
     */
    [[nodiscard]] double distance(const Vec2& v) const noexcept {
        T dx = v.x_ - x_;
        T dy = v.y_ - y_;
        return std::sqrt(dx * dx + dy * dy);
    }

    /**
     *  @brief Computes the Euclidean squared distance between this vector and
     *         another vector `v`.
     *
     *  @param v The vector to which the squared distance is calculated.
     *  @return The Euclidean squared distance between this vector and the vector `v`.
     */
    [[nodiscard]] double squaredDistance(const Vec2& v) const noexcept {
        T dx = v.x_ - x_;
        T dy = v.y_ - y_;
        return dx * dx + dy * dy;
    }

    /**
     *  @brief Computes the sign of the cross product between this vector and
     *         two other vectors.
     *
     *  This method calculates the sign of the 2D cross product formed by this
     *  vector and two other vectors, `a` and `b`. The sign of the cross product
     *  indicates whether the vectors form a clockwise or counterclockwise
     *  rotation when viewed from `b`. If the sign is positive, the rotation is
     *  counterclockwise; if negative, it's clockwise.
     *
     *  @param a The first vector.
     *  @param b The second vector.
     *  @return The sign of the cross product.
     */
    [[nodiscard]] double sign(const Vec2& a, const Vec2& b) const noexcept {
        return (x_ - b.x_) * (a.y_ - b.y_) - (a.x_ - b.x_) * (y_ - b.y_);
    }

    /**
     *  @brief Returns a flipped copy of this vector.
     *
     *  The resulting vector has the same magnitude as the original vector but
     *  points in the opposite direction.
     *
     *  @return A flipped copy of this vector.
     */
    [[nodiscard]] Vec2 flipped() const noexcept {
        return Vec2(-x_, -y_);
    }

    /**
     *  @brief Computes the reflection of the current point about a given pivot point.
     *
     *  The reflection of a point is the point located symmetrically opposite
     *  to it relative to the pivot point. This function calculates the reflected
     *  point using the formula:
     *
     *  @param pivot The pivot point about which the reflection is computed.
     *  @return Vec2 The reflected point.
     */
    [[nodiscard]] Vec2 reflectedPoint(const Vec2& pivot) const noexcept {
        return pivot + (pivot - *this);
    }

    /**
     *  @brief Returns a normalized copy of this vector.
     *
     *  The resulting vector has the same direction as the original vector,
     *  but its magnitude is equal to 1.
     *
     *  @return A normalized copy of this vector.
     */
    [[nodiscard]] Vec2 normalized() const noexcept {
        Vec2 result = *this;
        result.normalize();
        return result;
    }

    /**
      *  @brief Computes a vector perpendicular to this vector.
      *
      *  Rotates the vector counterclockwise by 90 degrees.
      *
      *  @return A perpendicular vector.
      */
    [[nodiscard]] Vec2 perpendicular() const {
        return Vec2(-y_, x_);
    }

    /**
     *  @brief Rounds both components of the vector down to the nearest integer.
     *
     *  @return A new vector with both components rounded down.
     */
    [[nodiscard]] inline constexpr Vec2 floor() const {
        return Vec2(std::floor(x_), std::floor(y_));
    }

    /**
     *  @brief Rounds both components of the vector up to the nearest integer.
     *
     *  @return A new vector with both components rounded up.
     */
    [[nodiscard]] inline constexpr Vec2 ceil() const {
        return Vec2(std::ceil(x_), std::ceil(y_));
    }

    /**
     *  @brief Computes the element-wise minimum with another vector.
     *
     *  @param v The vector to compare against.
     *  @return A vector where each component is the minimum of the corresponding components.
     */
    [[nodiscard]] inline constexpr Vec2 min(const Vec2& v) const {
        return Vec2(std::min<T>(x_, v.x_), std::min<T>(y_, v.y_));
    }

    /**
     *  @brief Computes the element-wise maximum with another vector.
     *
     *  @param v The vector to compare against.
     *  @return A vector where each component is the maximum of the corresponding components.
     */
    [[nodiscard]] inline constexpr Vec2 max(const Vec2& v) const {
        return Vec2(std::max<T>(x_, v.x_), std::max<T>(y_, v.y_));
    }

    /**
     *  @brief Clamps the components of this vector between the element-wise minimum and maximum of two other vectors.
     *
     *  For each component, clamps the value of this vector such that:
     *  - If it is less than the corresponding component in `a`, it takes the value of `a`.
     *  - If it is greater than the corresponding component in `b`, it takes the value of `b`.
     *
     *  @param a The vector providing the lower bounds for clamping.
     *  @param b The vector providing the upper bounds for clamping.
     *  @return A new vector with its components clamped between `a` and `b` element-wise.
     */
    [[nodiscard]] inline constexpr Vec2 clamped(const Vec2& a, const Vec2& b) const {
        return this->max(a).min(b);
    }

    /**
     *  @brief Converts polar coordinates to Cartesian coordinates.
     *
     *  Treats the vector as polar coordinates (R, θ), where:
     *  - R is the radial distance (magnitude).
     *  - θ is the angle in radians (measured counterclockwise from the positive X-axis).
     *
     *  @return A vector representing the equivalent Cartesian coordinates (X, Y).
     */
    [[nodiscard]] inline constexpr Vec2 cartesian() const {
        return Vec2(std::cos(y_) * x_, std::sin(y_) * x_);
    }

    /**
     *  @brief Converts Cartesian coordinates to polar coordinates.
     *
     *  Treats the vector as Cartesian coordinates (X, Y) and computes:
     *  - R: The radial distance (magnitude).
     *  - θ: The angle in radians (measured counterclockwise from the positive X-axis).
     *
     *  @return A vector representing the equivalent polar coordinates (R, θ).
     */
    [[nodiscard]] inline constexpr Vec2 polar() const {
        return Vec2(length(), std::atan2(y_, x_));
    }

    /**
     *  @brief Linearly interpolates between this vector and another vector.
     *
     *  Computes the linear interpolation (LERP) between this vector and `v` using a normalized parameter `t`, where:
     *  - `t = 0` returns this vector.
     *  - `t = 1` returns `v`.
     *  - Intermediate values of `t` return a weighted blend of the two vectors.
     *
     *  @param v The target vector to interpolate toward.
     *  @param t The interpolation parameter, typically in the range [0, 1].
     *  @return A new vector representing the interpolated result.
     */
    [[nodiscard]] inline constexpr Vec2 lerp(const Vec2& v, double t) const {
        return (*this) * (T(1.0 - t)) + (v * T(t));
    }



    /// Returns true if the distance to `v` is less than or equal to `threshold`.
    [[nodiscard]] bool checkEqual(const Vec2& v, T threshold) const noexcept {
        return distance(v) <= threshold;
    }

    /**
     *  @brief Monotonically increases with real angle, but doesn't need
     *         expensive trigonometry.
     *
     *  @return A value in [0, 1], increasing with angle, not the real angle.
     */
    [[nodiscard]] double pseudoAngle() const {
        const double p = static_cast<double>(x_) / (std::abs(x_) + std::abs(y_));
        return (y_ > 0.0 ? 3.0 - p : 1.0 + p) / 4.0; // [0..1)
    }

    /**
     *  @brief Computes the angle in radians from this vector's position to
     *         another vector's position.
     *
     *  This function calculates the angle between the line from this vector to
     *  the vector `v` and the positive x-axis, using the atan2 function.
     *
     *  @param v The target vector to which the angle is measured.
     *  @return The angle in radians in the range [-π, π], representing the
     *          direction from this vector to `v`.
     */
    [[nodiscard]] double angleTo(const Vec2& v) const {
        return std::atan2(v.y_ - y_, v.x_ - x_);
    }

    /**
     *  @brief Computes the angle between this vector and another vector.
     *
     *  Calculates the angle in degrees between the currentMillis vector and the given
     *  vector `v`, using the `atan2` of the cross and dot products. The result
     *  is in the range [0, 360].
     *
     *  This is useful for determining the relative direction or orientation
     *  between two vectors.
     *
     *  @param v The vector to compute the angle to.
     *  @return Angle in degrees between this vector and `v`, in the range
     *          [0, 360], measured counterclockwise from the x-axis.
     */
    [[nodiscard]] double angle(const Vec2& v) const noexcept {
        return std::atan2(cross(v), dot(v)) / std::numbers::pi * 180.0 + 180.0;
    }

    /**
     *  @brief Computes the angle between this vector and another vector.
     *
     *  This function calculates the angle in radians between the currentMillis vector
     *  and the given vector `v`.
     *
     *  @param v The vector to which the angle is calculated.
     *  @return The angle in radians between this vector and the vector `v`.
     */
    [[nodiscard]] double angleRad(const Vec2& v) const noexcept {
        return std::atan2(cross(v), dot(v));
    }

    /**
     *  @brief Computes the signed angle in radians between two vectors.
     *
     *  This function calculates the angle between the vector and another vector
     *  `v`, with the result being:
     *  - Positive if the rotation to `v` is counterclockwise.
     *  - Negative if the rotation to `v` is clockwise.
     *
     *  The angle is measured in radians.
     *
     *  @param v The vector to which the rotation should be measured.
     *
     *  @return double The signed angle in radians between the two vectors.
     *
     *  @note Both vectors should not both be zero-length, as this will result
     *        in undefined behavior due to division by zero.
     *
     *  ### Edge Cases:
     *  - If either vector is a zero vector, the result is undefined (division
     *    by zero).
     *  - If the vectors are collinear, the result will be 0, PI or -PI.
     */
    [[nodiscard]] double signedAngleRad(const Vec2& v) noexcept {
        double dot = x_ * v.x_ + y_ * v.y_;
        double length = std::sqrt((x_ * x_ + y_ * y_) * (v.x_ * v.x_ + v.y_ * v.y_));
        if (length == 0.0) {
            return 0.0;
        }
        double angle = std::acos(std::max<double>(-1.0, std::min<double>(1.0, dot / length)));
        if (x_ * v.y_ - y_ * v.x_ < 0) {
            angle = -angle;
        }
        return angle;
    }

    [[nodiscard]] static double signedAngleRad(T ux, T uy, T vx, T vy) noexcept {
        double dot = ux * vx + uy * vy;
        double length = std::sqrt((ux * ux + uy * uy) * (vx * vx + vy * vy));
        if (length == 0.0) {
            return 0.0;
        }
        double angle = std::acos(std::max(-1.0, std::min(1.0, dot / length)));
        if (ux * vy - uy * vx < 0) {
            angle = -angle;
        }
        return angle;
    }

    /**
     *  @brief Computes the angle between vectors `a` and `b` with respect to
     *         this vector.
     *
     *  @param a The first vector.
     *  @param b The second vector.
     *  @return The angle in degrees between vector `a` and vector `b` relative
     *          to this vector.
     */
    [[nodiscard]] double angle(const Vec2& a, const Vec2& b) const noexcept {
        Vec2 va = a - *this;
        Vec2 vb = b - *this;
        return std::atan2(va.cross(vb), va.dot(vb)) / std::numbers::pi * 180.0;
    }

    /**
     *  @brief Computes the angle (in degrees) from the origin (0,0) to this
     *         point, measured clockwise from the positive Y-axis.
     *
     *  @return Angle in degrees in the range [0, 360].
     *          0 degrees points upward (+Y), 90 degrees points right (+X),
     *          180 degrees downward, 270 degrees left.
     */
    [[nodiscard]] double angleToPos() const noexcept {
        double angle = std::atan2(x_, -y_) * 180.0 / std::numbers::pi;
        if (angle < 0.0) angle += 360.0;
        return angle;
    }

    /**
     *  @brief Computes the angle (in degrees) from a given pivot point to this
     *         point, measured clockwise from the pivot's +Y-axis.
     *
     *  @param pivot The pivot position from which to measure the angle.
     *  @return Angle in degrees in the range [0, 360]. 0 degrees means the
     *          point lies directly above the pivot.
     */
    [[nodiscard]] double angleToPos(const Vec2& pivot) const noexcept {
        double angle = std::atan2(x_ - pivot.x_, pivot.y_ - y_) * 180.0 / std::numbers::pi;
        if (angle < 0.0) angle += 360.0;
        return angle;
    }



    /**
     *  @brief Checks if both components of the vector are valid numbers.
     *
     *  This method verifies whether both components of the vector are valid
     *  numbers. It returns true if both components are numeric values and false
     *  otherwise. This can be useful for checking if the vector has been
     *  properly initialized with valid numerical values.
     *
     *  @return `true` if both components are valid numbers, `false` otherwise.
     */
    [[nodiscard]] bool isNumber() const noexcept {
        return x_ == x_ && y_ == y_;
    }

    /**
     *  @brief Checks if the vector is predominantly horizontal.
     *
     *  This method determines whether the vector is predominantly horizontal
     *  by comparing the absolute values of its x and y components. If the
     *  absolute value of the x component is greater than the absolute value
     *  of the y component, the vector is considered predominantly horizontal.
     *
     *  @return True if the vector is predominantly horizontal, false otherwise.
     */
    [[nodiscard]] bool isHorizontal() const noexcept {
        return std::fabs(x_) > std::fabs(y_);
    }

    /**
     *  @brief Checks if the vector is predominantly vertical.
     *
     *  This method determines whether the vector is predominantly vertical by
     *  comparing the absolute values of its x and y components. If the absolute
     *  value of the y component is greater than the absolute value of the x
     *  component, the vector is considered predominantly vertical.
     *
     *  @return `true` if the vector is predominantly vertical, `false`
     *          otherwise.
     */
    [[nodiscard]] bool isVertical() const noexcept {
        return std::fabs(y_) > std::fabs(x_);
    }

    [[nodiscard]] bool isOpposite(const Vec2& v) noexcept {
        return (x_ * v.x_ + y_ * v.y_) < 0.0;
    }

    /**
     *  @brief Sets the components of the vector to the specified values.
     *
     *  @param x The value to set as the x component.
     *  @param y The value to set as the y component.
     */
    void set(T x, T y) noexcept {
        x_ = x; y_ = y;
    }

    void zero() noexcept { x_ = y_ = 0; }
    void initRight() noexcept { x_ = 1; y_ = 0; }
    void initLeft() noexcept { x_ = -1; y_ = 0; }
    void initUp() noexcept { x_ = 0; y_ = 1; }
    void initDown() noexcept { x_ = 0; y_ = -1; }

    /**
     *  @brief Centroid of a line.
     *
     *  The centroid of a line is the average of its vertices.
     */
    template <typename U = T>
    requires std::floating_point<U>
    void setToLineCentroid(const Vec2& a, const Vec2& b) noexcept {
        x_ = (a.x_ + b.x_) / 2;
        y_ = (a.y_ + b.y_) / 2;
    }

    /**
     *  @brief Centroid of a triangle.
     *
     *  The centroid of a triangle is the average of its vertices.
     */
    template <typename U = T>
    requires std::floating_point<U>
    void setToTriangleCentroid(const Vec2& a, const Vec2& b, const Vec2& c) noexcept {
        x_ = (a.x_ + b.x_ + c.x_) / 3.0;
        y_ = (a.y_ + b.y_ + c.y_) / 3.0;
    }


    /**
     *  @brief Sets the vector's components based on a comma-separated string.
     *
     *  This method parses a comma-separated string `csv` and sets the vector's
     *  components accordingly.The string should contain two numerical values
     *  separated by a comma, representing the x and y components respectively.
     *  If the string format is valid, the method sets the components and returns
     *  true; otherwise, it leaves the vector unchanged and returns false.
     *
     *  @param csv A pointer to the comma-separated string.
     *  @return `true` if the components were successfully set, `false`
     *          otherwise.
     */
    bool setByCSV(const char* csv, char delimiter) noexcept {
        if (csv) {
            CSVLineParser csv_line_parser(csv);
            csv_line_parser.setDelimiter(delimiter);
            T values[2]{};
            if (csv_line_parser.values(2, values) == 2) {
                x_ = values[0];
                y_ = values[1];
                return true;
            }
        }
        return false;
    }

    /**
     *  @brief Sets the vector's components based on a comma-separated string.
     *
     *  This method parses a comma-separated string `csv` and sets the vector's
     *  components accordingly.The string should contain two numerical values
     *  separated by a comma, representing the x and y components respectively.
     *  If the string format is valid, the method sets the components and returns
     *  true; otherwise, it leaves the vector unchanged and returns false.
     *
     *  @param csv The comma-separated string.
     *  @return `true` if the components were successfully set, `false`
     *           otherwise.
     */
    bool setByCSV(const String& csv, char delimiter) noexcept {
        return setByCSV(csv.utf8(), delimiter);
    }

    /**
     *  @brief Clamps the x component of the vector within a specified range.
     *
     *  This method clamps the x component of the vector to ensure it falls
     *  within the specified range defined by the minimum and maximum values
     *  `min` and `max` respectively.
     *
     *  @param min The minimum value allowed for the x component.
     *  @param max The maximum value allowed for the x component.
     */
    inline void clampX(T min, T max) noexcept {
        x_ = std::clamp(x_, min, max);
    }

    /**
     *  @brief Clamps the y component of the vector within a specified range.
     *
     *  This method clamps the y component of the vector to ensure it falls
     *  within the specified range defined by the minimum and maximum values
     *  `min` and `max` respectively.
     *
     *  @param min The minimum value allowed for the y component.
     *  @param max The maximum value allowed for the y component.
     */
    inline void clampY(T min, T max) noexcept {
        y_ = std::clamp(y_, min, max);
    }

    /**
     *  @brief Normalizes the vector to unit length.
     *
     *  This method normalizes the vector, ensuring that its length becomes 1
     *  while preserving its direction.
     *
     *  @note If the vector is a zero vector (length == 0), no action is taken.
     */
    void normalize() noexcept {
        double l = length();
        if (l != 0.0) {
            double s = 1.0 / l;
            x_ *= s;
            y_ *= s;
        }
    }

    /**
     *  @brief Sets the length of the vector to a specified value.
     *
     *  This method sets the length of the vector to the specified value
     *  `length`, while preserving its direction.
     *
     *  @param length The desired length for the vector.
     *  @note If the specified length is zero, the vector will become a zero
     *        vector (0, 0).
     */
    void setLength(double length) noexcept {
        normalize();
        x_ *= length;
        y_ *= length;
    }

    /**
     *  @brief Truncates the vector's length to a maximum value.
     *
     *  This method truncates the length of the vector to a maximum value `max`,
     *  if its currentMillis lengt exceeds the specified maximum length.
     *
     *  @param max The maximum length allowed for the vector.
     */
    void truncate(double max) noexcept {
        if (max > 0.0) {
            double l = length();
            if (l > max) {
                x_ = x_ / l * max;
                y_ = y_ / l * max;
            }
        }
    }

    /**
     *  @brief Swaps the x and y components of the vector.
     */
    inline void swap() noexcept {
        T temp = x_;
        x_ = y_;
        y_ = temp;
    }

    /**
     *  @brief Flips the signs of the x and y components of the vector.
     *
     *  This method flips the signs of the x and y components of the vector,
     *  effectively changing the direction of the vector while maintaining its
     *  magnitude.
     */
    inline void flip() noexcept {
        x_ = -x_;
        y_ = -y_;
    }

    /**
     *  @brief Flips the sign of the x component of the vector.
     *
     *  This method flips the sign of the x component of the vector, effectively
     *  changing its direction along the x-axis while maintaining its magnitude.
     */
    inline void flipX() noexcept {
        x_ = -x_;
    }

    /**
     *  @brief Flips the sign of the y component of the vector.
     *
     *  This method flips the sign of the y component of the vector, effectively
     *  changing its direction along the y-axis while maintaining its magnitude.
     */
    inline void flipY() noexcept {
        y_ = -y_;
    }

    /**
     *  @brief Rotates the vector by a specified angle in degrees.
     *
     *  The rotation is performed counterclockwise around the origin of the
     *  coordinate system.
     *
     *  @param deg The angle of rotation in degrees.
     */
    void rotate(double deg) noexcept {
        rotateRad(deg / 180.0 * std::numbers::pi);
    }

    /**
     *  @brief Rotates the vector by a specified angle in radians.
     *
     *  The rotation is performed counterclockwise around the origin of the
     *  coordinate system.
     *
     *  @param rad The angle of rotation in radians.
     */
    void rotateRad(double rad) noexcept {
        double c = std::cos(rad);
        double s = std::sin(rad);
        double tx = x_ * c - y_ * s;
        double ty = x_ * s + y_ * c;
        x_ = tx;
        y_ = ty;
    }

    /**
     *  @brief Rotates the vector around a specified pivot point by a specified
     *         angle in degrees.
     *
     *  The rotation is performed counterclockwise around the pivot point.
     *
     *  @param pivot The pivot point around which the vector will be rotated.
     *  @param deg The angle of rotation in degrees.
     */
    void rotate(const Vec2& pivot, double deg) noexcept {
        x_ -= pivot.x_;
        y_ -= pivot.y_;
        rotate(deg);
        x_ += pivot.x_;
        y_ += pivot.y_;
    }

    /**
     *  @brief Applies an orthogonal transformation to the vector.
     *
     *  This method applies an orthogonal transformation to the vector, swapping
     *  its x and y components and negating the new y component.
     */
    void ortho() noexcept {
        T temp = y_;
        y_ = -x_;
        x_ = temp;
    }

    /**
     *  @brief Translates the vector by the specified translation vector.
     *
     *  @param v The translation vector.
     */
    void translate(const Vec2& v) noexcept {
        x_ += v.x_;
        y_ += v.y_;
    }

    /**
     * @brief Translates the vector by the specified amounts along the x and y axes.
     *
     * @param x The amount to translate along the x-axis.
     * @param y The amount to translate along the y-axis.
     */
    void translate(T x, T y) noexcept {
        x_ += x;
        y_ += y;
    }

    /**
     *  @brief Translates the vector by the specified amount along the x-axis.
     *
     *  @param x The amount to translate along the x-axis.
     */
    void translateX(T x) noexcept {
        x_ += x;
    }

    /**
     *  @brief Translates the vector by the specified amount along the y-axis.
     *
     *  @param y The amount to translate along the y-axis.
     */
    void translateY(T y) noexcept {
        y_ += y;
    }

    /**
     *  @brief Scales the vector uniformly by a specified factor.
     *
     *  @param s The scaling factor.
     */
    void scale(T s) noexcept {
        x_ *= s;
        y_ *= s;
    }

    /**
     *  @brief Scales the vector independently along the x and y axes.
     *
     *  @param sx The scaling factor for the x-axis.
     *  @param sy The scaling factor for the y-axis.
     */
    void scale(T sx, T sy) noexcept {
        x_ *= sx;
        y_ *= sy;
    }

    /**
     *  @brief Scales the vector around a specified pivot point by a specified
     *         factor.
     *
     *  @param pivot The pivot point around which the vector will be scaled.
     *  @param s The scaling factor.
     */
    void scaleFrom(const Vec2& pivot, T s) noexcept {
        x_ = pivot.x_ + (x_ - pivot.x_) * s;
        y_ = pivot.y_ + (y_ - pivot.y_) * s;
    }

    /**
     *  @brief Snaps the vector to the nearest grid point defined by the grid
     *         step.
     *
     *  @param grid_step The size of the grid step.
     */
    void snap(T grid_step) {
        x_ = static_cast<T>(std::round(static_cast<double>(x_) / grid_step) * grid_step);
        y_ = static_cast<T>(std::round(static_cast<double>(y_) / grid_step) * grid_step);
    }

    /**
     *  @brief Snaps the vector to the nearest grid point defined by individual
     *  grid steps along the x and y axes.
     *
     *  @param grid_step_x The size of the grid step along the x-axis.
     *  @param grid_step_y The size of the grid step along the y-axis.
     */
    void snap(T grid_step_x, T grid_step_y) {
        x_ = static_cast<T>(std::round(static_cast<double>(x_) / grid_step_x) * grid_step_x);
        y_ = static_cast<T>(std::round(static_cast<double>(y_) / grid_step_y) * grid_step_y);
    }

    /**
     *  @brief Calculates the dot product of the vector with another vector.
     *
     *  Dot products of vectors are useful in various mathematical and
     *  computational contexts, including: Projection, Angle between vectors,
     *  Orthogonality, Geometry and trigonometry.
     *
     *  @param v The vector to compute the dot product with.
     *  @return The dot product of the two vectors.
     */
    [[nodiscard]] double dot(const Vec2& v) const noexcept {
        return x_ * v.x_ + y_ * v.y_;
    }

    /**
     *  @brief Calculates the cross product of the vector with another vector.
     *
     *  Cross products of vectors are useful in several applications, including:
     *  Determining orientation, Calculating area, Determining collinearity.
     *
     *  @param v The vector to compute the cross product with.
     *  @return The cross product of the two vectors.
     */
    [[nodiscard]] double cross(const Vec2& v) const noexcept {
        return (x_ * v.y_) - (y_ * v.x_);
    }

    /**
     *  @brief Performs linear interpolation towards another vector.
     *
     *  This method performs linear interpolation (lerp) towards another vector
     *  `v` based on the interpolation parameter `t`. Linear interpolation
     *  computes intermediate values between the currentMillis vector and `v` based on
     *  the blending factor `t`, where 0 represents the currentMillis vector and 1
     *  represents `v`. After calling this method, the currentMillis vector will be
     *  updated to be an interpolated value between its original state and `v`.
     *
     *  @param v The target vector to interpolate towards.
     *  @param t The interpolation parameter. Should be in the range [0, 1].
     */
    void lerpTowards(const Vec2& v, double t) noexcept {
        x_ += t * (v.x_ - x_);
        y_ += t * (v.y_ - y_);
    }

    /**
     *  @brief Performs linear interpolation between two vectors.
     *
     *  @param a The start vector for interpolation.
     *  @param b The end vector for interpolation.
     *  @param t The interpolation parameter. Should be in the range [0, 1].
     *  @return The interpolated vector between `a` and `b`.
     */
    [[nodiscard]] static Vec2 lerp(const Vec2& a, const Vec2& b, double t) noexcept {
        return Vec2(a.x_ + t * (b.x_ - a.x_), a.y_ + t * (b.y_ - a.y_));
    }

    /**
     *  @brief Sets the vector to the result of linear interpolation between two
     *         vectors.
     *
     *  @param a The start vector for interpolation.
     *  @param b The end vector for interpolation.
     *  @param t The interpolation parameter. Should be in the range [0, 1].
     */
    void setLerp(const Vec2& a, const Vec2& b, double t) noexcept {
        x_ = a.x_ + t * (b.x_ - a.x_);
        y_ = a.y_ + t * (b.y_ - a.y_);
    }

    /**
     *  @brief Sets the vector components to random values within a specified
     *         range.
     *
     *  This method sets the x and y components of the vector to random values
     *  generated by the `Random::next` function within the specified range
     *  [0, v]. After calling this method, the vector will have random values
     *  for both components within the specified range.
     *
     *  @param v The upper bound (exclusive) of the range for generating random
     *           values.
     */
    void random(T v) noexcept {
        x_ = Random::next(v);
        y_ = Random::next(v);
    }

    /**
     *  @brief Sets the vector components to random values within specified
     *         ranges.
     *
     *  @param x The upper bound (exclusive) of the range for generating random
     *           values for the x component.
     *  @param y The upper bound (exclusive) of the range for generating random
     *           values for the y component.
     */
    void random(T x, T y) noexcept {
        x_ = Random::next(x);
        y_ = Random::next(y);
    }

    /**
     *  @brief Sets the vector components to random values within bidirectional
     *         ranges.
     *
     *  @param v The upper bound (inclusive) of the bidirectional range for
     *           generating random values.
     */
    void randomBidirectional(T v) noexcept {
        x_ = Random::next(-v, v);
        y_ = Random::next(-v, v);
    }

    /**
     *  @brief Sets the vector components to random values within bidirectional
     *         ranges.
     *
     *  @param x The upper bound (inclusive) of the bidirectional range for
     *           generating random values for the x component.
     *  @param y The upper bound (inclusive) of the bidirectional range for
     *           generating random values for the y component.
     */
    void randomBidirectional(T x, T y) noexcept {
        x_ = Random::next(-x, x);
        y_ = Random::next(-y, y);
    }

    /**
     *  @brief Sets the vector components to random values within specified
     *         ranges.
     *
     *  @param min_x The lower bound (inclusive) of the range for generating
     *               random values for the x component.
     *  @param min_y The lower bound (inclusive) of the range for generating
     *               random values for the y component.
     *  @param max_x The upper bound (inclusive) of the range for generating
     *               random values for the x component.
     *  @param max_y The upper bound (inclusive) of the range for generating
     *               random values for the y component.
     */
    void randomRange(double min_x, double min_y, double max_x, double max_y) noexcept {
        x_ = static_cast<T>(Random::next(static_cast<float>(min_x), static_cast<float>(max_x)));
        y_ = static_cast<T>(Random::next(static_cast<float>(min_y), static_cast<float>(max_y)));
    }

    /**
     *  @brief Sets the vector to a random direction.
     *
     *  After calling this method, the vector will represent a random direction
     *  in 2D space.
     */
    void randomDirection() noexcept {
        x_ = 0;
        y_ = 1;
        rotate(Random::next(360));
    }

    /**
     *  @brief Adds random values to the vector components.
     *
     *  @param v The upper bound (exclusive) of the range for generating random
     *           values.
     */
    void randomize(T v) noexcept {
        x_ += static_cast<T>(Random::next(v));
        y_ += static_cast<T>(Random::next(v));
    }

    /**
     *  @brief Adds random values to the vector components within specified
     *         ranges.
     *
     *  @param x The upper bound (exclusive) of the range for generating random
     *           values for the x component.
     *  @param y The upper bound (exclusive) of the range for generating random
     *           values for the y component.
     */
    void randomize(T x, T y) noexcept {
        x_ += static_cast<T>(Random::next(x));
        y_ += static_cast<T>(Random::next(y));
    }

    /**
     *  @brief Adds random values to the vector components within bidirectional
     *         ranges.
     *
     *  @param v The upper bound (inclusive) of the bidirectional range for
     *           generating random values.
     */
    void randomizeBidirectional(T v) noexcept {
        x_ += static_cast<T>(Random::next(-v, v));
        y_ += static_cast<T>(Random::next(-v, v));
    }

    /**
     *  @brief Adds random values to the vector components within bidirectional
     *         ranges.
     *
     *  @param x The upper bound (exclusive) of the bidirectional range for
     *           generating random values for the x component.
     *  @param y The upper bound (exclusive) of the bidirectional range for
     *           generating random values for the y component.
     */
    void randomizeBidirectional(T x, T y) noexcept {
        x_ += static_cast<T>(Random::next(-x, x));
        y_ += static_cast<T>(Random::next(-y, y));
    }

    /**
     *  @brief Converts a location to a position.
     *
     *  Location has angle in degree an distance.
     *  Position has x and y.
     */
    [[nodiscard]] Vec2 locToPos() const noexcept {
        Vec2 result(0, y_);
        result.rotate(-x_);
        return result;
    }

    /**
     *  @brief Converts position to a location.
     */
    [[nodiscard]] Vec2 posToLoc() const noexcept {
        T distance = std::sqrt(x_ * x_ + y_ * y_);
        T angle = distance > 0 ? (std::atan2(-x_, -y_) / std::numbers::pi) * 180 + 180 : 0;
        return Vec2(angle, distance);
    }


    /**
     *  @brief Determines if three 2D points form a clockwise orientation.
     *
     *  @param a First point
     *  @param b Second point
     *  @param c Third point
     *  @return true if the orientation of triangle abc is clockwise
     */
    [[nodiscard]] static inline bool isClockwise(const Vec2& a, const Vec2& b, const Vec2& c) noexcept {
        return (b.x_ - a.x_) * (c.y_ - a.y_) - (b.y_ - a.y_) * (c.x_ - a.x_) < 0.0;
    }

    /**
     *  @brief Checks whether the vector lies inside the circumcircle
     *         of the triangle defined by `a`, `b`, and `c`.
     *
     * @param a First vertex of the triangle.
     * @param b Second vertex of the triangle.
     * @param c Third vertex of the triangle.
     * @return true if vector lies inside the circumcircle of triangle abc,
     *         false otherwise.
     */
    [[nodiscard]] bool inCircle(const Vec2& a, const Vec2& b, const Vec2& c) const noexcept {
        const double dx = a.x_ - x_;
        const double dy = a.y_ - y_;
        const double ex = b.x_ - x_;
        const double ey = b.y_ - y_;
        const double fx = c.x_ - x_;
        const double fy = c.y_ - y_;

        const double ap = dx * dx + dy * dy;
        const double bp = ex * ex + ey * ey;
        const double cp = fx * fx + fy * fy;

        return (dx * (ey * cp - bp * fy) -
                dy * (ex * cp - bp * fx) +
                ap * (ex * fy - ey * fx)) < 0.0;
    }

    /**
     *  @brief Set the vector to be a the center of the circle that passes
     *         through three given points.
     *
     *  @param a A Vec2 representing the first point on the circle.
     *  @param b A Vec2 representing the second point on the circle.
     *  @param c A Vec2 representing the third point on the circle.
     */
    void circumcenter(const Vec2& a, const Vec2& b, const Vec2& c) {
        const double dx = b.x_ - a.x_;
        const double dy = b.y_ - a.y_;
        const double ex = c.x_ - a.x_;
        const double ey = c.y_ - a.y_;

        const double bl = dx * dx + dy * dy;
        const double cl = ex * ex + ey * ey;
        const double d = dx * ey - dy * ex;

        x_ = a.x_ + (ey * bl - dy * cl) * 0.5 / d;
        y_ = a.y_ + (dx * cl - ex * bl) * 0.5 / d;
    }

    /**
     *  @brief Computes the squared radius of the circumcircle of the triangle
     *         defined by three 2D points.
     *
     *  This function calculates the square of the radius of the unique circle
     *  (circumcircle) that passes through the three given points `a`, `b`, and
     *  `c`. It avoids computing the square root for performance and returns a
     *  large value (`std::numeric_limits<double>::max()`) if the triangle is
     *  degenerate (i.e., points are colinear or overlapping).
     *
     *  @param a The first vertex of the triangle.
     *  @param b The second vertex of the triangle.
     *  @param c The third vertex of the triangle.
     *  @return The squared radius of the circumcircle, or a large fallback
     *          value if invalid.
     */
    [[nodiscard]] static double squaredCircumradius(const Vec2& a, const Vec2& b, const Vec2& c) {
        const Vec2 d = b - a;
        const Vec2 e = c - a;

        const double bl = d.squaredLength();
        const double cl = e.squaredLength();
        const double dc = d.cross(e);

        if (bl != 0.0 && cl != 0.0 && dc != 0.0) {
            const double x = (e.y_ * bl - d.y_ * cl) * 0.5 / dc;
            const double y = (d.x_ * cl - e.x_ * bl) * 0.5 / dc;
            return x * x + y * y;
        }
        else {
            return std::numeric_limits<double>::max();
        }
    }

    /**
     *  @brief Set the vector to be a point on an arc defined by three points.
     *
     *  This function computes a point on a circular arc defined by three points:
     *  `a`, `b`, and `c`. The parameter `t` is a value between 0 and 1,
     *  representing the position along the arc:
     *  - `t = 0` gives the point `a`
     *  - `t = 1` gives the point `c`
     *  - `t = 0.5` gives the midpoint of the arc between `a` and `c`, passing through `b`
     *
     *  The arc is assumed to be part of a circle passing through the three given points.
     *  The function will update the currentMillis vector (the object the method is called on) to
     *  represent a point along this arc based on the parameter `t`.
     *
     *  @param a The first point on the arc (start point of the arc).
     *  @param b The second point on the arc, used to define the curvature (typically the apex).
     *  @param c The third point on the arc (end point of the arc).
     *  @param t A parameter between 0 and 1 that specifies the position on the arc:
     *           - 0 means the point is at `a`
     *           - 1 means the point is at `c`
     *           - 0.5 gives the midpoint of the arc.
     */
    void pointOnArc(const Vec2& a, const Vec2& b, const Vec2& c, double t) {
        // Calculate the center of the circle
        Vec2 center;
        center.circumcenter(a, b, c);

        // Calculate the radius of the circle
        double radius = std::hypot(center.x_ - a.x_, center.y_ - a.y_);

        // Calculate angles from center to points `a`, `b`, and `c`
        double angle_ca = std::atan2(a.y_ - center.y_, a.x_ - center.x_);
        double angle_cc = std::atan2(c.y_ - center.y_, c.x_ - center.x_);

        // Calculate the total angle (taking care of direction).
        double total_angle = angle_cc - angle_ca;
        if (total_angle < 0.0) {
            total_angle += 2.0 * std::numbers::pi;
        }

        // Calculate the angle at parameter `t`.
        double angle_at_t = angle_ca + total_angle * t;

        // Calculate the point on the arc using polar coordinates.
        x_ = center.x_ + radius * std::cos(angle_at_t);
        y_ = center.y_ + radius * std::sin(angle_at_t);
    }

    void pointOnArc(const Vec2* p, double t) {
        if (p) {
            pointOnArc(p[0], p[1], p[2], t);
        }
    }

    /**
     *  @brief Computes the area of a simple 2D polygon using the shoelace
     *         formula.
     *
     *  This function calculates the absolute area of a polygon defined by an
     *  array of points. The polygon is assumed to be simple
     *  (non-self-intersecting) and the vertices should be ordered either
     *  clockwise or counterclockwise.
     *
     *  @param point_n The number of points (vertices) in the polygon.
     *  @param points A pointer to an array of Vec2<T> representing
     *                the polygon's vertices.
     *  @return The absolute area of the polygon.
     */
    [[nodiscard]] static double polygonArea(int32_t point_n, const Vec2<T>* points) noexcept {
        if (point_n < 3) {
            return 0.0;  // Not a polygon
        }

        double area = 0.0;
        for (int32_t i = 0; i < point_n; ++i) {
            const Vec2& v0 = points[i];
            const Vec2& v1 = points[(i + 1) % point_n];
            area += v0.x_ * v1.y_ - v1.x_ * v0.y_;
        }
        return std::abs(area) * 0.5;
    }

    /**
     *  @brief Computes the centroid of a polygon.
     *
     *  This function calculates the centroid (center of mass) of a simple,
     *  non-self-intersecting polygon.
     *
     *  @param point_n The number of vertices in the polygon.
     *  @param points A pointer to an array of Vec2<T> representing the polygon's vertices.
     *  @return The centroid as a Vec2<double>.
     */
    [[nodiscard]] static Vec2<double> polygonCentroid(int32_t point_n, const Vec2<T>* points) noexcept {
        double cx = 0.0, cy = 0.0;
        double area = 0.0;

        for (int32_t i = 0; i < point_n; ++i) {
            const Vec2& v0 = points[i];
            const Vec2& v1 = points[(i + 1) % point_n];
            double cross = v0.x_ * v1.y_ - v1.x_ * v0.y_;
            cx += (v0.x_ + v1.x_) * cross;
            cy += (v0.y_ + v1.y_) * cross;
            area += cross;
        }

        area *= 0.5;
        if (std::abs(area) < 1e-10) {
            // Degenerate case (area ≈ 0), return average of points
            double avg_x = 0.0, avg_y = 0.0;
            for (int32_t i = 0; i < point_n; ++i) {
                avg_x += points[i].x_;
                avg_y += points[i].y_;
            }
            return { avg_x / point_n, avg_y / point_n };
        }

        cx /= (6.0 * area);
        cy /= (6.0 * area);

        return { cx, cy };
    }

    #if defined(__APPLE__) && defined(__MACH__)
        [[nodiscard]] CGPoint cgPoint() const noexcept { return CGPointMake(x_, y_); }
    #endif

    void writeToFile(File& file);
    void readFromFile(File& file);
};


// Standard types
using Vec2u8 = Vec2<uint8_t>;   ///< 8 bit unsigned integer
using Vec2i = Vec2<int32_t>;    ///< 32 bit integer
using Vec2l = Vec2<int64_t>;    ///< 64 bit integer
using Vec2f = Vec2<float>;      ///< 32 bit floating point
using Vec2d = Vec2<double>;     ///< 64 bit floating point

// Function pointer types for sampling 2D positions on a curve at parameter t ∈ [0, 1]
typedef Vec2f (*vec2fAtTFunc)(float t);
typedef Vec2d (*vec2dAtTFunc)(double t);


} // End of namespace Grain

#endif // GrainVec2_hpp
