//
//  RangeCube.hpp
//
//  Created by Roald Christesen on from 17.11.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 01.08.2025
//

#ifndef GrainRangeCube_hpp
#define GrainRangeCube_hpp


#include "Grain.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec3Fix.hpp"
#include "3d/Cube.hpp"
#include "2d/Rect.hpp"


namespace Grain {

    class Vec3Fix;


    template <class T>
    class RangeCube {
    public:
        T m_min_x = 0;
        T m_max_x = 0;
        T m_min_y = 0;
        T m_max_y = 0;
        T m_min_z = 0;
        T m_max_z = 0;

    public:
        RangeCube() noexcept {}
        RangeCube(T min_x, T max_x, T min_y, T max_y, T min_z, T max_z) noexcept : m_min_x(min_x), m_min_y(min_y), m_min_z(min_z), m_max_x(max_x), m_max_y(max_y), m_max_z(max_z) {}

        virtual const char* className() const noexcept { return "RangeCube"; }


        friend std::ostream& operator << (std::ostream& os, const RangeCube& o) {
            if (std::is_same<T, int8_t>::value || std::is_same<T, uint8_t>::value) {
                // Force to print integer values instead of chars.
                os << static_cast<int32_t>(o.m_min_x) << ", " << static_cast<int32_t>(o.m_max_x) << " | ";
                os << static_cast<int32_t>(o.m_min_y) << ", " << static_cast<int32_t>(o.m_max_y) << " | ";
                os << static_cast<int32_t>(o.m_min_z) << ", " << static_cast<int32_t>(o.m_max_z);
            }
            else {
                os << std::setprecision(10);
                os << o.m_min_x << ", " << o.m_max_x << " | ";
                os << o.m_min_y << ", " << o.m_max_y << " | ";
                os << o.m_min_z << ", " << o.m_max_z;
                os << std::defaultfloat;  // Back to standard.
            }
            return os;
        }


        RangeCube& operator = (const Vec3<T>& v) {
            m_min_x = m_max_x = v.m_x;
            m_min_y = m_max_y = v.m_y;
            m_min_z = m_max_z = v.m_z;
            return *this;
        }


        RangeCube& operator = (const Cube<T>& r) {
            m_min_x = r.m_width > 0 ? r.m_x : r.m_x + r.m_width;
            m_max_x = r.m_width > 0 ? r.m_x + r.m_width : r.m_x;
            m_min_y = r.m_height > 0 ? r.m_y : r.m_y + r.m_height;
            m_max_y = r.m_height > 0 ? r.m_y + r.m_height : r.m_y;
            m_min_z = r.m_depth > 0 ? r.m_z : r.m_z + r.m_depth;
            m_max_z = r.m_depth > 0 ? r.m_z + r.m_depth : r.m_z;
            return *this;
        }


        bool operator == (const RangeCube& v) const {
            return m_min_x == v.m_min_x && m_max_x == v.m_max_x && m_min_y == v.m_min_y && m_max_y == v.m_max_y && m_min_z == v.m_min_z && m_max_z == v.m_max_z;
        }

        bool operator != (const RangeCube& v) const {
            return m_min_x != v.m_min_x || m_max_x != v.m_max_x || m_min_y != v.m_min_y || m_max_y != v.m_max_y || m_min_z != v.m_min_z || m_max_z != v.m_max_z;
        }

        RangeCube operator + (const RangeCube<T>& r) const {
            RangeCube result;
            result.m_min_x = m_min_x < r.m_min_x ? m_min_x : r.m_min_x;
            result.m_min_y = m_min_y < r.m_min_y ? m_min_y : r.m_min_y;
            result.m_min_z = m_min_z < r.m_min_z ? m_min_z : r.m_min_z;
            result.m_max_x = m_max_x > r.m_max_x ? m_max_x : r.m_max_x;
            result.m_max_y = m_max_y > r.m_max_y ? m_max_y : r.m_max_y;
            result.m_max_z = m_max_z > r.m_max_z ? m_max_z : r.m_max_z;
            return result;
        }

        RangeCube operator + (const Vec3<T>& v) const {
            RangeCube result = *this;
            if (v.m_x < m_min_x) { result.m_min_x = v.m_x; }
            if (v.m_x > m_max_x) { result.m_max_x = v.m_x; }
            if (v.m_y < m_min_y) { result.m_min_y = v.m_y; }
            if (v.m_y > m_max_y) { result.m_max_y = v.m_y; }
            if (v.m_z < m_min_z) { result.m_min_z = v.m_z; }
            if (v.m_z > m_max_z) { result.m_max_z = v.m_z; }
            return result;
        }

        RangeCube operator + (const Cube<T>& r) const {
            RangeCube result;
            T r_min_x = r.m_width > 0 ? r.m_x : r.m_x + r.m_width;
            T r_max_x = r.m_width > 0 ? r.m_x + r.m_width : r.m_x;
            T r_min_y = r.m_height > 0 ? r.m_y : r.m_y + r.m_height;
            T r_max_y = r.m_height > 0 ? r.m_y + r.m_height : r.m_y;
            T r_min_z = r.m_depth > 0 ? r.m_z : r.m_z + r.m_depth;
            T r_max_z = r.m_depth > 0 ? r.m_z + r.m_depth : r.m_z;
            result.m_min_x = m_min_x < r_min_x ? m_min_x : r_min_x;
            result.m_min_y = m_min_y < r_min_y ? m_min_y : r_min_y;
            result.m_min_z = m_min_z < r_min_z ? m_min_z : r_min_z;
            result.m_max_x = m_max_x > r_max_x ? m_max_x : r_max_x;
            result.m_max_y = m_max_y > r_max_y ? m_max_y : r_max_y;
            result.m_max_z = m_max_z > r_max_z ? m_max_z : r_max_z;
            return result;
        }

        RangeCube& operator += (const RangeCube<T>& r) {
            m_min_x = m_min_x < r.m_min_x ? m_min_x : r.m_min_x;
            m_min_y = m_min_y < r.m_min_y ? m_min_y : r.m_min_y;
            m_min_z = m_min_z < r.m_min_z ? m_min_z : r.m_min_z;
            m_max_x = m_max_x > r.m_max_x ? m_max_x : r.m_max_x;
            m_max_y = m_max_y > r.m_max_y ? m_max_y : r.m_max_y;
            m_max_z = m_max_z > r.m_max_z ? m_max_z : r.m_max_z;
            return *this;
        }

        RangeCube& operator += (const Vec3<T>& v) {
            if (v.m_x < m_min_x) { m_min_x = v.m_x; }
            if (v.m_x > m_max_x) { m_max_x = v.m_x; }
            if (v.m_y < m_min_y) { m_min_y = v.m_y; }
            if (v.m_y > m_max_y) { m_max_y = v.m_y; }
            if (v.m_z < m_min_z) { m_min_z = v.m_z; }
            if (v.m_z > m_max_z) { m_max_z = v.m_z; }
            return *this;
        }

        RangeCube& operator += (const Cube<T>& r) {
            T r_min_x = r.m_width > 0 ? r.m_x : r.m_x + r.m_width;
            T r_max_x = r.m_width > 0 ? r.m_x + r.m_width : r.m_x;
            T r_min_y = r.m_height > 0 ? r.m_y : r.m_y + r.m_height;
            T r_max_y = r.m_height > 0 ? r.m_y + r.m_height : r.m_y;
            T r_min_z = r.m_depth > 0 ? r.m_z : r.m_z + r.m_depth;
            T r_max_z = r.m_depth > 0 ? r.m_z + r.m_depth : r.m_z;
            m_min_x = m_min_x < r_min_x ? m_min_x : r_min_x;
            m_min_y = m_min_y < r_min_y ? m_min_y : r_min_y;
            m_min_z = m_min_z < r_min_z ? m_min_z : r_min_z;
            m_max_x = m_max_x > r_max_x ? m_max_x : r_max_x;
            m_max_y = m_max_y > r_max_y ? m_max_y : r_max_y;
            m_max_z = m_max_z > r_max_z ? m_max_z : r_max_z;
            return *this;
        }

        T minX() const noexcept { return m_min_x; }
        T maxX() const noexcept { return m_max_x; }
        T minY() const noexcept { return m_min_y; }
        T maxY() const noexcept { return m_max_y; }
        T minZ() const noexcept { return m_min_z; }
        T maxZ() const noexcept { return m_max_z; }
        T centerX() const noexcept { return m_min_x + (m_max_x - m_min_x) / 2; }
        T centerY() const noexcept { return m_min_y + (m_max_y - m_min_y) / 2; }
        T centerZ() const noexcept { return m_min_z + (m_max_z - m_min_z) / 2; }
        T width() const noexcept { return std::fabs(m_max_x - m_min_x); }
        T height() const noexcept { return std::fabs(m_max_y - m_min_y); }
        T depth() const noexcept { return std::fabs(m_max_z - m_min_z); }

        Cube<T> toCube() const noexcept {
            return Cube<T>(m_min_x, m_min_y, m_min_z, m_max_x - m_min_x, m_max_y - m_min_y, m_max_z - m_min_z);
        }

        Rect<T> toRectXY() const noexcept {
            return Rect<T>(m_min_x, m_min_y, m_max_x - m_min_x, m_max_y - m_min_y);
        }

        Rect<T> toRectXZ() const noexcept {
            return Rect<T>(m_min_x, m_min_z, m_max_x - m_min_x, m_max_z - m_min_z);
        }

        Rect<T> toRectYZ() const noexcept {
            return Rect<T>(m_min_y, m_min_z, m_max_y - m_min_y, m_max_y - m_min_y);
        }


        void set(T x, T y, T z) noexcept { m_min_x = m_max_x = x; m_min_y = m_max_y = y; m_min_z = m_max_z = z; }
        void set(Vec3<T>& v) noexcept { m_min_x = m_max_x = v.m_x; m_min_y = m_max_y = v.m_y; m_min_z = m_max_z = v.m_z; }
        void set(Vec3<T>* v) noexcept { if (v) { m_min_x = m_max_x = v->m_x; m_min_y = m_max_y = v->m_y; m_min_z = m_max_z = v->m_z; } }

        void set(T min_x, T max_x, T min_y, T max_y, T min_z, T max_z) noexcept {
            m_min_x = min_x; m_max_x = max_x;
            m_min_y = min_y; m_max_y = max_y;
            m_min_z = min_z; m_max_z = max_z;
        }

        bool add(const Vec3<T>& v) noexcept {
            bool result = false;
            if (v.m_x < m_min_x) { m_min_x = v.m_x; result = true; }
            if (v.m_x > m_max_x) { m_max_x = v.m_x; result = true; }
            if (v.m_y < m_min_y) { m_min_y = v.m_y; result = true; }
            if (v.m_y > m_max_y) { m_max_y = v.m_y; result = true; }
            if (v.m_z < m_min_z) { m_min_z = v.m_z; result = true; }
            if (v.m_z > m_max_z) { m_max_z = v.m_z; result = true; }
            return result;
        }

        bool add(const Vec3<T>* v) noexcept { if (v) { return add(*v); } return false; }

        bool addX(T x) noexcept {
            bool result = false;
            if (x < m_min_x) { m_min_x = x; result = true; }
            if (x > m_max_x) { m_max_x = x; result = true; }
            return result;
        }

        bool addY(T y) noexcept {
            bool result = false;
            if (y < m_min_y) { m_min_y = y; result = true; }
            if (y > m_max_y) { m_max_y = y; result = true; }
            return result;
        }

        bool addZ(T z) noexcept {
            bool result = false;
            if (z < m_min_z) { m_min_z = z; result = true; }
            if (z > m_max_z) { m_max_z = z; result = true; }
            return result;
        }

        bool add(T x, T y, T z) noexcept {
            bool result = false;
            if (x < m_min_x) { m_min_x = x; result = true; }
            if (x > m_max_x) { m_max_x = x; result = true; }
            if (y < m_min_y) { m_min_y = y; result = true; }
            if (y > m_max_y) { m_max_y = y; result = true; }
            if (z < m_min_z) { m_min_z = z; result = true; }
            if (z > m_max_z) { m_max_z = z; result = true; }
            return result;
        }

        void add(const Cube<T>& cube) noexcept {
            if (cube.m_x < m_min_x) m_min_x = cube.m_x;
            if (cube.x2() > m_max_x) m_max_x = cube.x2();
            if (cube.m_y < m_min_y) m_min_y = cube.m_y;
            if (cube.y2() > m_max_y) m_max_y = cube.y2();
            if (cube.m_z < m_min_z) m_min_z = cube.m_z;
            if (cube.z2() > m_max_z) m_max_z = cube.z2();
        }

        void add(const RangeCube<T>& r) noexcept {
            *this += r;
        }

        void limit(const RangeCube& max_cube) noexcept {
            if (m_min_x < max_cube.m_min_x) m_min_x = max_cube.m_min_x;
            if (m_max_x > max_cube.m_max_x) m_max_x = max_cube.m_max_x;
            if (m_min_y < max_cube.m_min_y) m_min_y = max_cube.m_min_y;
            if (m_max_y > max_cube.m_max_y) m_max_y = max_cube.m_max_y;
        }


        RangeCube<T> lerp(const RangeCube<T> r, double t) noexcept {
            RangeCube<T> result;
            result.m_min_x = m_min_x + t * (r.m_min_x - m_min_x);
            result.m_min_y = m_min_y + t * (r.m_min_y - m_min_y);
            result.m_min_z = m_min_z + t * (r.m_min_z - m_min_z);
            result.m_max_x = m_max_x + t * (r.m_max_x - m_max_x);
            result.m_max_y = m_max_y + t * (r.m_max_y - m_max_y);
            result.m_max_z = m_max_z + t * (r.m_max_z - m_max_z);
            return result;
        }

        static RangeCube<T> lerp(const RangeCube<T> a, const RangeCube<T> b, double t) noexcept {
            RangeCube<T> result;
            result.m_min_x = a.m_min_x + t * (b.m_min_x - a.m_min_x);
            result.m_min_y = a.m_min_y + t * (b.m_min_y - a.m_min_y);
            result.m_min_z = a.m_min_z + t * (b.m_min_z - a.m_min_z);
            result.m_max_x = a.m_max_x + t * (b.m_max_x - a.m_max_x);
            result.m_max_y = a.m_max_y + t * (b.m_max_y - a.m_max_y);
            result.m_max_z = a.m_max_z + t * (b.m_max_z - a.m_max_z);
            return result;
        }
    };



// Standard types.
    using RangeCubei = RangeCube<int32_t>;  ///< 32 bit integer.
    using RangeCubel = RangeCube<int64_t>;  ///< 64 bit integer.
    using RangeCubef = RangeCube<float>;    ///< 32 bit floating point.
    using RangeCubed = RangeCube<double>;   ///< 64 bit floating point.



    class RangeCubeFix {

    public:
        Fix m_min_x = 0;
        Fix m_max_x = 0;
        Fix m_min_y = 0;
        Fix m_max_y = 0;
        Fix m_min_z = 0;
        Fix m_max_z = 0;

    public:
        RangeCubeFix() noexcept {}
        RangeCubeFix(Fix min_x, Fix max_x, Fix min_y, Fix max_y, Fix min_z, Fix max_z) noexcept :
                m_min_x(min_x),
                m_max_x(max_x),
                m_min_y(min_y),
                m_max_y(max_y),
                m_min_z(min_z),
                m_max_z(max_z) {
        }

        virtual const char* className() const noexcept { return "RangeCubeFix"; }


        friend std::ostream& operator << (std::ostream& os, const RangeCubeFix& o) {
            os << o.m_min_x << ", " << o.m_max_x << " | ";
            os << o.m_min_y << ", " << o.m_max_y << " | ";
            os << o.m_min_z << ", " << o.m_max_z;
            return os;
        }


        RangeCubeFix& operator = (const Vec3Fix& v);
        RangeCubeFix operator + (const Vec3Fix& v) const;
        RangeCubeFix& operator += (const Vec3Fix& v);

        Fix minX() const noexcept { return m_min_x; }
        Fix maxX() const noexcept { return m_max_x; }
        Fix minY() const noexcept { return m_min_y; }
        Fix maxY() const noexcept { return m_max_y; }
        Fix minZ() const noexcept { return m_min_z; }
        Fix maxZ() const noexcept { return m_max_z; }

        Fix width() const noexcept { return m_max_x > m_min_x ? m_max_x - m_min_x : m_min_x - m_max_x; }
        Fix height() const noexcept { return m_max_y > m_min_y ? m_max_y - m_min_y : m_min_y - m_max_y; }
        Fix depth() const noexcept { return m_max_z > m_min_z ? m_max_z - m_min_z : m_min_z - m_max_z; }

        void initForMinMaxSearch() noexcept {
            m_min_x.setToMax(); m_min_y.setToMax(); m_min_z.setToMax();
            m_max_x.setToMin(); m_max_y.setToMin(); m_max_z.setToMin();
        }

        void add(const Vec3Fix& v) noexcept {
            addX(v.m_x); addY(v.m_y); addZ(v.m_z);
        }

        void add(const Vec3Fix* v) noexcept {
            if (v != nullptr) {
                addX(v->m_x); addY(v->m_y); addZ(v->m_z);
            }
        }

        void  addX(const Fix& x) noexcept {
            if (x < m_min_x) { m_min_x = x; }
            if (x > m_max_x) { m_max_x = x; }
        }

        void addY(const Fix& y) noexcept {
            if (y < m_min_y) { m_min_y = y; }
            if (y > m_max_y) { m_max_y = y; }
        }

        void addZ(const Fix& z) noexcept {
            if (z < m_min_z) { m_min_z = z; }
            if (z > m_max_z) { m_max_z = z; }
        }

        void add(const Fix& x, const Fix& y, const Fix& z) noexcept {
            addX(x); addY(y); addZ(z);
        }
    };


} // End of namespace Grain

#endif // GrainRangeCube_hpp
