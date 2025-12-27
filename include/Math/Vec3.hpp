//
//  Vec3.hpp
//
//  Created by Roald Christesen on 30.09.2019
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 19.08.2025
//

#ifndef GrainVec3_hpp
#define GrainVec3_hpp

#include "Grain.hpp"
#include "String/String.hpp"
#include "String/CSVString.hpp"
#include "Random.hpp"


namespace Grain {

class File;

template <ScalarType T>
class Vec3 {
public:
    T x_{};
    T y_{};
    T z_{};

public:
    Vec3() noexcept : x_(static_cast<T>(0)), y_(static_cast<T>(0)), z_(static_cast<T>(0)) {}
    Vec3(T x, T y, T z) noexcept : x_(x), y_(y), z_(z) {}
    Vec3(const Vec3& other) noexcept : x_(other.x_), y_(other.y_), z_(other.z_) {}

    template <typename U>
    requires (!std::same_as<U, T>)
    explicit Vec3(const Vec3<U>& v) noexcept : x_(static_cast<T>(v.x_)), y_(static_cast<T>(v.y_), z_(static_cast<T>(v.z_))) {}

    explicit Vec3(const char* csv, char delimiter = ',') noexcept { setByCSV(csv, delimiter); }
    explicit Vec3(const String& csv, char delimiter = ',') noexcept { setByCSV(csv, delimiter); }

    virtual ~Vec3() noexcept = default;

    [[nodiscard]] virtual const char* className() const noexcept { return "Vec3"; }

    friend std::ostream& operator << (std::ostream& os, const Vec3* o) {
        o == nullptr ? os << "Vec3 nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const Vec3& o) {
        if (std::is_same<T, int8_t>::value || std::is_same<T, uint8_t>::value) {
            // Force to print integer values instead of chars
            return os << static_cast<int32_t>(o.x_) << ", " << static_cast<int32_t>(o.y_) << ", " << static_cast<int32_t>(o.z_);
        }
        else {
            return os << o.x_ << ", " << o.y_ << ", " << o.z_;
        }
    }

    // Move constructor (default provided by the compiler)
    Vec3(Vec3&&) = default;

    // Copy assignment operator
    Vec3& operator = (const Vec3& other) {
        if (this == &other) return *this;
        x_ = other.x_;
        y_ = other.y_;
        z_ = other.z_;
        return *this;
    }

    // Move assignment operator (default provided by the compiler)
    Vec3& operator = (Vec3&&) = default;

    // Template specialization for assigning from Vec3 of type U to Vec2 of type T
    template <typename U>
    requires std::is_convertible_v<U, T>
    Vec3<T>& operator = (const Vec3<U>& other) {
        x_ = static_cast<T>(other.x_);
        y_ = static_cast<T>(other.y_);
        z_ = static_cast<T>(other.z_);
        return *this;
    }

    bool operator == (const Vec3& other) const { return x_ == other.x_ && y_ == other.y_ && z_ == other.z_; }
    bool operator != (const Vec3& other) const { return x_ != other.x_ || y_ != other.y_ || z_ != other.z_; }

    Vec3 operator - () const { return Vec3(-x_, -y_, -y_); }

    Vec3 operator + (const Vec3& other) const { return Vec3(x_ + other.x_, y_ + other.y_, z_ + other.z_); }
    Vec3 operator - (const Vec3& other) const { return Vec3(x_ - other.x_, y_ - other.y_, z_ - other.z_); }
    Vec3 operator * (const Vec3& other) const { return Vec3(x_ * other.x_, y_ * other.y_, z_ * other.z_); }
    Vec3 operator * (const T v) const { return Vec3(x_ * v, y_ * v, z_ * v); }

    // Cross product
    Vec3 operator ^ (const Vec3& other) const {
        return Vec3(y_ * other.z_ - z_ * other.y_, z_ * other.x_ - x_ * other.z_, x_ * other.y_ - y_ * other.x_);
    }

    Vec3& operator += (const Vec3& other) { x_ += other.x_; y_ += other.y_; z_ += other.z_; return *this; }
    Vec3& operator -= (const Vec3& other) { x_ -= other.x_; y_ -= other.y_; z_ -= other.z_; return *this; }
    Vec3& operator *= (const Vec3& other) { x_ *= other.x_; y_ *= other.y_; z_ *= other.z_; return *this; }
    Vec3& operator *= (const T v) { x_ *= v; y_ *= v; z_ *= v; return *this; }

    // Non-const array operator version for assignment
    T& operator[](size_t index) {
        switch (index % 3) {
            case 0: return x_;
            case 1: return y_;
            case 2: return z_;
        }
        return x_;
    }

    // Const version for read-only access
    const T& operator[](size_t index) const {
        switch (index % 3) {
            case 0: return x_;
            case 1: return y_;
            case 2: return z_;
        }
        return x_;
    }


    [[nodiscard]] T x() const noexcept { return x_; }
    [[nodiscard]] T y() const noexcept { return y_; }
    [[nodiscard]] T z() const noexcept { return z_; }
    [[nodiscard]] double length() const { return std::sqrt(x_ * x_ + y_ * y_ + z_ * z_); }
    [[nodiscard]] double squaredLength() const { return x_ * x_ + y_ * y_ + z_ * z_; }

    [[nodiscard]] double distance(const Vec3& v) const noexcept {
        T dx = v.x_ - x_;
        T dy = v.y_ - y_;
        T dz = v.z_ - z_;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    [[nodiscard]] double squaredDistance(const Vec3& v) const noexcept {
        T dx = v.x_ - x_;
        T dy = v.y_ - y_;
        T dz = v.z_ - z_;
        return dx * dx + dy * dy + dz * dz;
    }

    [[nodiscard]] Vec3 flipped() const noexcept { return Vec3(-x_, -y_, -z_); }
    [[nodiscard]] Vec3 reflectedPoint(const Vec3& pivot) const noexcept { return pivot + (pivot - *this); }
    [[nodiscard]] Vec3 normalized() const noexcept {
        Vec3 result = *this;
        result.normalize();
        return result;
    }

    /// Returns true if the distance to `v` is less than or equal to `threshold`.
    [[nodiscard]] bool checkEqual(const Vec3& v, T threshold) const noexcept {
        return distance(v) <= threshold;
    }

    [[nodiscard]] double angle(const Vec3& v) const noexcept {
        T dot = x_ * v.x_ + y_ * v.y_ + z_ * v.z_;
        T l1 = x_ * x_ + y_ * y_ + z_ * z_;
        T l2 = v.x_ * v.x_ + v.y_ * v.y_ + v.z_ * v.z_;
        return std::acos(dot / std::sqrt(l1 * l2)) * 180.0 / std::numbers::pi;
    }

    void set(T x, T y, T z) noexcept { x_ = x; y_ = y; z_ = z; }

    bool setByCSV(const char* csv, char delimiter) noexcept {
        int32_t result = 0;
        if (csv) {
            CSVLineParser csv_line_parser(csv);
            csv_line_parser.setDelimiter(delimiter);
            T values[3]{};
            result = csv_line_parser.values(3, values);
            x_ = values[0];
            y_ = values[1];
            z_ = values[2];
        }
        return result == 3;
    }

    bool setByCSV(const String& csv, char delimiter) noexcept {
        return setByCSV(csv.utf8(), delimiter);
    }

    void setLerp(const Vec3& a, const Vec3& b, double t) noexcept {
        x_ = a.x_ + t * (b.x_ - a.x_);
        y_ = a.y_ + t * (b.y_ - a.y_);
        z_ = a.z_ + t * (b.z_ - a.z_);
    }

    void zero() noexcept { x_ = y_ = z_ = 0; }

    void normalize() noexcept {
        double l = length();
        if (l != 0.0) {
            double s = 1.0 / l;
            x_ *= s;
            y_ *= s;
            z_ *= s;
        }
    }

    void setLength(T length) noexcept {
        double l = this->length();
        if (l != 0.0) {
            double s = length / l;
            x_ = s * x_;
            y_ = s * y_;
            z_ = s * z_;
        }
    }


    void flip() noexcept { x_ = -x_; y_ = -y_; z_ = -z_; }

    void translateX(T tx) noexcept { x_ += tx; }
    void translateY(T ty) noexcept { y_ += ty; }
    void translateZ(T tz) noexcept { z_ += tz; }
    void translate(T tx, T ty, T tz) noexcept { x_ += tx; y_ += ty; z_ += tz; }

    void scaleX(T sx) noexcept { x_ *= sx; }
    void scaleY(T sy) noexcept { y_ *= sy; }
    void scaleZ(T sz) noexcept { z_ *= sz; }
    void scale(T s) noexcept { x_ *= s; y_ *= s; z_ *= s; }
    void scale(T sx, T sy, T sz) noexcept { x_ *= sx; y_ *= sy; z_ *= sz; }

    void rotateX(double angle) noexcept { rotateXRad(angle * std::numbers::pi / 180.0); }
    void rotateY(double angle) noexcept { rotateYRad(angle * std::numbers::pi / 180.0); }
    void rotateZ(double angle) noexcept { rotateZRad(angle * std::numbers::pi / 180.0); }


    void rotateXRad(double rad) noexcept {
        double c = std::cos(rad);
        double s = std::sin(rad);
        double y = c * y_ - s * z_;
        double z = s * y_ + c * z_;
        y_ = y;
        z_ = z;
    }

    void rotateYRad(double rad) noexcept {
        double c = std::cos(rad);
        double s = std::sin(rad);
        double x = c * x_ + s * z_;
        double z = s * -x_ + c * z_;
        x_ = x;
        z_ = z;
    }

    void rotateZRad(double rad) noexcept {
        double c = std::cos(rad);
        double s = std::sin(rad);
        double x = c * x_ - s * y_;
        double y = s * x_ + c * y_;
        x_ = x;
        y_ = y;
    }


    [[nodiscard]] double dot(const Vec3& v) const noexcept {
        return x_ * v.x_ + y_ * v.y_ + z_ * v.z_;
    }

    [[nodiscard]] Vec3 cross(const Vec3& v) const noexcept {
        return Vec3(y_ * v.z_ - z_ * v.y_, z_ * v.x_ - x_ * v.z_, x_ * v.y_ - y_ * v.x_);
    }


    void setTriangleNormal(const Vec3& a, const Vec3& b, const Vec3& c) noexcept {
        *this = (a - b) % (b - c);    // Perform cross product of two lines on plane.
        normalize();
    }

    void setReflect(const Vec3& direction, const Vec3& normal) noexcept {

        Vec3 temp_normal = normal;
        double dot = direction.dot(temp_normal);

        if (dot > 0.0) {
            // Ensure that the surface normal points outward by negating it if necessary.
            temp_normal.flip();
            dot = -dot;
        }

        T f = dot + dot;
        x_ = direction.x_ - temp_normal.x_ * f;
        y_ = direction.y_ - temp_normal.y_ * f;
        z_ = direction.y_ - temp_normal.z_ * f;
    }

    void setFromSphericalLonlat(T lon, T lat) { // TODO: TEST!
        T lat_rad = lat * std::numbers::pi / 180.0;
        T lon_rad = lon * std::numbers::pi / 180.0;
        x_ = std::sin(lat_rad) * std::cos(lon_rad);
        y_ = std::sin(lat_rad) * std::sin(lon_rad);
        z_ = std::cos(lat_rad);
    }

    /**
     *  @brief Converts a location to a position.
     *
     *  Location has angle in degree, distance and elevation.
     *  Position has x, y and z in cartesian coordinates.
     */
    [[nodiscard]] Vec3 locToPos() const noexcept {
        Vec3 result(0, y_, z_);
        result.rotateZ(-x_);
        return result;
    }

    /**
     *  @brief Converts position to a location.
     */
    [[nodiscard]] Vec3 posToLoc() const noexcept {
        T distance = std::sqrt(x_ * x_ + y_ * y_);
        T angle = distance > 0 ? (atan2(-x_, -y_) / std::numbers::pi) * 180 + 180 : 0;
        return Vec3(angle, distance, z_);
    }

    void writeToFile(File& file);
    void readFromFile(File& file);
};


// Standard types
using Vec3i = Vec3<int32_t>;    ///< 32 bit integer
using Vec3l = Vec3<int64_t>;    ///< 64 bit integer
using Vec3f = Vec3<float>;      ///< 32 bit floating point
using Vec3d = Vec3<double>;     ///< 64 bit floating point


} // End of namespace Grain

#endif // GrainVec3_hpp
