//
//  Vec3Fix.hpp
//
//  Created by Roald Christesen on 17.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 12.07.2025
//

#ifndef GrainVec3Fix_hpp
#define GrainVec3Fix_hpp

#include "Type/Fix.hpp"
#include "Vec3.hpp"


namespace Grain {

    class Vec3Fix {
    public:
        Vec3Fix() noexcept { zero(); }
        Vec3Fix(const Fix& x, const Fix& y, const Fix& z) noexcept {
            m_x = x; m_y = y;  m_z = z;
        }

        [[nodiscard]] virtual const char* className() const noexcept { return "Vec3Fix"; }

        friend std::ostream& operator << (std::ostream& os, const Vec3Fix& o) {
            os << o.m_x << ", " << o.m_y << ", " << o.m_z;
            return os;
        }


        bool operator == (const Vec3Fix& other) const {
            return other.m_x == m_x && other.m_y == m_y && other.m_z == m_z;
        }

        bool operator != (const Vec3Fix& other) const {
            return other.m_x != m_x || other.m_y != m_y || other.m_z != m_z;
        }

        Vec3Fix operator + (const Vec3Fix& other) const {
            return { m_x + other.m_x, m_y + other.m_y, m_z + other.m_z };
        }

        Vec3Fix operator - (const Vec3Fix& other) const {
            return { m_x - other.m_x, m_y - other.m_y, m_z - other.m_z };
        }

        Vec3Fix operator * (const Vec3Fix& other) const {
            return { m_x * other.m_x, m_y * other.m_y, m_z * other.m_z };
        }

        [[nodiscard]] Fix x() const noexcept { return m_x; }
        [[nodiscard]] Fix y() const noexcept { return m_y; }
        [[nodiscard]] Fix z() const noexcept { return m_z; }
        [[nodiscard]] float xFloat() const noexcept { return m_x.asFloat(); }
        [[nodiscard]] float yFloat() const noexcept { return m_y.asFloat(); }
        [[nodiscard]] float zFloat() const noexcept { return m_z.asFloat(); }
        [[nodiscard]] double xDouble() const noexcept { return m_x.asDouble(); }
        [[nodiscard]] double yDouble() const noexcept { return m_y.asDouble(); }
        [[nodiscard]] double zDouble() const noexcept { return m_z.asDouble(); }


        void zero() noexcept { m_x = 0; m_y = 0; m_z = 0; }

        bool set(int32_t x, int32_t y, int32_t z) noexcept {
            if (m_x != x || m_y != y || m_z != z) {
                m_x = x;
                m_y = y;
                m_z = z;
                return true;
            }
            else {
                return false;
            }
        }

        bool set(const Fix& x, const Fix& y, const Fix& z) noexcept {
            if (x != m_x || y != m_y || z != m_z) {
                m_x = x;
                m_y = y;
                m_z = z;
                return true;
            }
            else {
                return false;
            }
        }

        bool set(const char* x_str, const char* y_str, const char* z_str) noexcept {
            int64_t x_old = m_x.m_raw_value;
            int64_t y_old = m_y.m_raw_value;
            int64_t z_old = m_z.m_raw_value;
            m_x.setStr(x_str);
            m_y.setStr(y_str);
            m_z.setStr(z_str);
            return m_x.m_raw_value != x_old || m_y.m_raw_value != y_old || m_z.m_raw_value != z_old;
        }


        bool setByCSV(const String& string, char delimiter) noexcept;

        void vec3f(Vec3f& out_vec) const noexcept {
            out_vec.m_x = m_x.asFloat();
            out_vec.m_y = m_y.asFloat();
            out_vec.m_z = m_z.asFloat();
        }

        void vec3d(Vec3d& out_vec) const noexcept {
            out_vec.m_x = m_x.asDouble();
            out_vec.m_y = m_y.asDouble();
            out_vec.m_z = m_z.asDouble();
        }

        void setVec3f(const Vec3f& vec) noexcept {
            m_x.setFloat(vec.m_x);
            m_y.setFloat(vec.m_y);
            m_z.setFloat(vec.m_z);
        }

        void setVec3d(const Vec3d& vec) noexcept {
            m_x.setDouble(vec.m_x);
            m_y.setDouble(vec.m_y);
            m_z.setDouble(vec.m_z);
        }

        void setPrecision(int32_t precision) noexcept {
            m_x.setPrecision(precision);
            m_y.setPrecision(precision);
            m_z.setPrecision(precision);
        }

    public:
        Fix m_x, m_y, m_z;
    };


} // End of namespace Grain

#endif // GrainVec3Fix_hpp
