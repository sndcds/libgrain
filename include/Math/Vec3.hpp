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
        Vec3() noexcept : m_x(static_cast<T>(0)), m_y(static_cast<T>(0)), m_z(static_cast<T>(0)) {}
        Vec3(T x, T y, T z) noexcept : m_x(x), m_y(y), m_z(z) {}
        Vec3(const Vec3& other) noexcept : m_x(other.m_x), m_y(other.m_y), m_z(other.m_z) {}

        template <typename U>
        requires (!std::same_as<U, T>)
        explicit Vec3(const Vec3<U>& v) noexcept : m_x(static_cast<T>(v.m_x)), m_y(static_cast<T>(v.m_y), m_z(static_cast<T>(v.m_z))) {}

        explicit Vec3(const char* csv, char delimiter = ',') noexcept { setByCSV(csv, delimiter); }
        explicit Vec3(const String& csv, char delimiter = ',') noexcept { setByCSV(csv, delimiter); }


        [[nodiscard]] virtual const char* className() const noexcept { return "Vec3"; }

        friend std::ostream& operator << (std::ostream& os, const Vec3* o) {
            o == nullptr ? os << "Vec3 nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const Vec3& o) {
            if (std::is_same<T, int8_t>::value || std::is_same<T, uint8_t>::value) {
                // Force to print integer values instead of chars
                return os << static_cast<int32_t>(o.m_x) << ", " << static_cast<int32_t>(o.m_y) << ", " << static_cast<int32_t>(o.m_z);
            }
            else {
                return os << o.m_x << ", " << o.m_y << ", " << o.m_z;
            }
        }

        // Move constructor (default provided by the compiler)
        Vec3(Vec3&&) = default;

        // Copy assignment operator
        Vec3& operator = (const Vec3& other) {
            if (this == &other) return *this;
            m_x = other.m_x;
            m_y = other.m_y;
            m_z = other.m_z;
            return *this;
        }

        // Move assignment operator (default provided by the compiler)
        Vec3& operator = (Vec3&&) = default;

        // Template specialization for assigning from Vec3 of type U to Vec2 of type T
        template <typename U>
        requires std::is_convertible_v<U, T>
        Vec3<T>& operator = (const Vec3<U>& other) {
            m_x = static_cast<T>(other.m_x);
            m_y = static_cast<T>(other.m_y);
            m_z = static_cast<T>(other.m_z);
            return *this;
        }

        bool operator == (const Vec3& other) const { return m_x == other.m_x && m_y == other.m_y && m_z == other.m_z; }
        bool operator != (const Vec3& other) const { return m_x != other.m_x || m_y != other.m_y || m_z != other.m_z; }

        Vec3 operator - () const { return Vec3(-m_x, -m_y, -m_y); }

        Vec3 operator + (const Vec3& other) const { return Vec3(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z); }
        Vec3 operator - (const Vec3& other) const { return Vec3(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z); }
        Vec3 operator * (const Vec3& other) const { return Vec3(m_x * other.m_x, m_y * other.m_y, m_z * other.m_z); }
        Vec3 operator * (const T v) const { return Vec3(m_x * v, m_y * v, m_z * v); }

        // Cross product
        Vec3 operator ^ (const Vec3& other) const {
            return Vec3(m_y * other.m_z - m_z * other.m_y, m_z * other.m_x - m_x * other.m_z, m_x * other.m_y - m_y * other.m_x);
        }

        Vec3& operator += (const Vec3& other) { m_x += other.m_x; m_y += other.m_y; m_z += other.m_z; return *this; }
        Vec3& operator -= (const Vec3& other) { m_x -= other.m_x; m_y -= other.m_y; m_z -= other.m_z; return *this; }
        Vec3& operator *= (const Vec3& other) { m_x *= other.m_x; m_y *= other.m_y; m_z *= other.m_z; return *this; }
        Vec3& operator *= (const T v) { m_x *= v; m_y *= v; m_z *= v; return *this; }

        // Non-const array operator version for assignment
        T& operator[](size_t index) {
            switch (index % 3) {
                case 0: return m_x;
                case 1: return m_y;
                case 2: return m_z;
            }
            return m_x;
        }

        // Const version for read-only access
        const T& operator[](size_t index) const {
            switch (index % 3) {
                case 0: return m_x;
                case 1: return m_y;
                case 2: return m_z;
            }
            return m_x;
        }


        [[nodiscard]] T x() const noexcept { return m_x; }
        [[nodiscard]] T y() const noexcept { return m_y; }
        [[nodiscard]] T z() const noexcept { return m_z; }
        [[nodiscard]] double length() const { return std::sqrt(m_x * m_x + m_y * m_y + m_z * m_z); }
        [[nodiscard]] double squaredLength() const { return m_x * m_x + m_y * m_y + m_z * m_z; }

        [[nodiscard]] double distance(const Vec3& v) const noexcept {
            T dx = v.m_x - m_x;
            T dy = v.m_y - m_y;
            T dz = v.m_z - m_z;
            return std::sqrt(dx * dx + dy * dy + dz * dz);
        }

        [[nodiscard]] double squaredDistance(const Vec3& v) const noexcept {
            T dx = v.m_x - m_x;
            T dy = v.m_y - m_y;
            T dz = v.m_z - m_z;
            return dx * dx + dy * dy + dz * dz;
        }

        [[nodiscard]] Vec3 flipped() const noexcept { return Vec3(-m_x, -m_y, -m_z); }
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
            T dot = m_x * v.m_x + m_y * v.m_y + m_z * v.m_z;
            T l1 = m_x * m_x + m_y * m_y + m_z * m_z;
            T l2 = v.m_x * v.m_x + v.m_y * v.m_y + v.m_z * v.m_z;
            return std::acos(dot / std::sqrt(l1 * l2)) * 180.0 / std::numbers::pi;
        }

        void set(T x, T y, T z) noexcept { m_x = x; m_y = y; m_z = z; }

        bool setByCSV(const char* csv, char delimiter) noexcept {
            int32_t result = 0;
            if (csv) {
                CSVLineParser csv_line_parser(csv);
                csv_line_parser.setDelimiter(delimiter);
                T values[3]{};
                result = csv_line_parser.values(3, values);
                m_x = values[0];
                m_y = values[1];
                m_z = values[2];
            }
            return result == 3;
        }

        bool setByCSV(const String& csv, char delimiter) noexcept {
            return setByCSV(csv.utf8(), delimiter);
        }

        void setLerp(const Vec3& a, const Vec3& b, double t) noexcept {
            m_x = a.m_x + t * (b.m_x - a.m_x);
            m_y = a.m_y + t * (b.m_y - a.m_y);
            m_z = a.m_z + t * (b.m_z - a.m_z);
        }

        void zero() noexcept { m_x = m_y = m_z = 0; }

        void normalize() noexcept {
            double l = length();
            if (l != 0.0) {
                double s = 1.0 / l;
                m_x *= s;
                m_y *= s;
                m_z *= s;
            }
        }

        void setLength(T length) noexcept {
            double l = this->length();
            if (l != 0.0) {
                double s = length / l;
                m_x = s * m_x;
                m_y = s * m_y;
                m_z = s * m_z;
            }
        }


        void flip() noexcept { m_x = -m_x; m_y = -m_y; m_z = -m_z; }

        void translateX(T tx) noexcept { m_x += tx; }
        void translateY(T ty) noexcept { m_y += ty; }
        void translateZ(T tz) noexcept { m_z += tz; }
        void translate(T tx, T ty, T tz) noexcept { m_x += tx; m_y += ty; m_z += tz; }

        void scaleX(T sx) noexcept { m_x *= sx; }
        void scaleY(T sy) noexcept { m_y *= sy; }
        void scaleZ(T sz) noexcept { m_z *= sz; }
        void scale(T s) noexcept { m_x *= s; m_y *= s; m_z *= s; }
        void scale(T sx, T sy, T sz) noexcept { m_x *= sx; m_y *= sy; m_z *= sz; }

        void rotateX(double angle) noexcept { rotateXRad(angle * std::numbers::pi / 180.0); }
        void rotateY(double angle) noexcept { rotateYRad(angle * std::numbers::pi / 180.0); }
        void rotateZ(double angle) noexcept { rotateZRad(angle * std::numbers::pi / 180.0); }


        void rotateXRad(double rad) noexcept {
            double c = std::cos(rad);
            double s = std::sin(rad);
            double y = c * m_y - s * m_z;
            double z = s * m_y + c * m_z;
            m_y = y;
            m_z = z;
        }

        void rotateYRad(double rad) noexcept {
            double c = std::cos(rad);
            double s = std::sin(rad);
            double x = c * m_x + s * m_z;
            double z = s * -m_x + c * m_z;
            m_x = x;
            m_z = z;
        }

        void rotateZRad(double rad) noexcept {
            double c = std::cos(rad);
            double s = std::sin(rad);
            double x = c * m_x - s * m_y;
            double y = s * m_x + c * m_y;
            m_x = x;
            m_y = y;
        }


        [[nodiscard]] double dot(const Vec3& v) const noexcept {
            return m_x * v.m_x + m_y * v.m_y + m_z * v.m_z;
        }

        [[nodiscard]] Vec3 cross(const Vec3& v) const noexcept {
            return Vec3(m_y * v.m_z - m_z * v.m_y, m_z * v.m_x - m_x * v.m_z, m_x * v.m_y - m_y * v.m_x);
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
            m_x = direction.m_x - temp_normal.m_x * f;
            m_y = direction.m_y - temp_normal.m_y * f;
            m_z = direction.m_y - temp_normal.m_z * f;
        }

        void setFromSphericalLonlat(T lon, T lat) { // TODO: TEST!
            T lat_rad = lat * std::numbers::pi / 180.0;
            T lon_rad = lon * std::numbers::pi / 180.0;
            m_x = std::sin(lat_rad) * std::cos(lon_rad);
            m_y = std::sin(lat_rad) * std::sin(lon_rad);
            m_z = std::cos(lat_rad);
        }

        /**
         *  @brief Converts a location to a position.
         *
         *  Location has angle in degree, distance and elevation.
         *  Position has x, y and z in cartesian coordinates.
         */
        [[nodiscard]] Vec3 locToPos() const noexcept {
            Vec3 result(0, m_y, m_z);
            result.rotateZ(-m_x);
            return result;
        }

        /**
         *  @brief Converts position to a location.
         */
        [[nodiscard]] Vec3 posToLoc() const noexcept {
            T distance = std::sqrt(m_x * m_x + m_y * m_y);
            T angle = distance > 0 ? (atan2(-m_x, -m_y) / std::numbers::pi) * 180 + 180 : 0;
            return Vec3(angle, distance, m_z);
        }

        void writeToFile(File& file);
        void readFromFile(File& file);

    public:
        T m_x;
        T m_y;
        T m_z;
    };


    // Standard types
    using Vec3i = Vec3<int32_t>;    ///< 32 bit integer
    using Vec3l = Vec3<int64_t>;    ///< 64 bit integer
    using Vec3f = Vec3<float>;      ///< 32 bit floating point
    using Vec3d = Vec3<double>;     ///< 64 bit floating point


} // End of namespace Grain

#endif // GrainVec3_hpp
