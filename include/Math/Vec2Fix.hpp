//
//  Vec2Fix.hpp
//
//  Created by Roald Christesen on 20.03.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 19.08.2025
//

#ifndef GrainVec2Fix_hpp
#define GrainVec2Fix_hpp

#include "Type/Fix.hpp"
#include "Vec2.hpp"


namespace Grain {

    class Vec2Fix {
    public:
        Vec2Fix() noexcept { zero(); }
        Vec2Fix(const Fix& x, const Fix& y) noexcept {
            m_x = x;
            m_y = y;
        }

        template<std::integral U>
        explicit Vec2Fix(const Vec2<U>& v) noexcept {
            std::cout << "Vec2Fix integral\n";
            m_x.setInt64(static_cast<int64_t>(v.m_x));
            m_y.setInt64(static_cast<int64_t>(v.m_y));
        }

        template<std::floating_point U>
        explicit Vec2Fix(const Vec2<U>& v) noexcept {
            std::cout << "Vec2Fix floating_point\n";
            m_x.setDouble(static_cast<double>(v.m_x));
            m_y.setDouble(static_cast<double>(v.m_y));
        }

        explicit Vec2Fix(const char* csv, char delimiter = ',') noexcept { setByCSV(csv, delimiter); }
        explicit Vec2Fix(const String& csv, char delimiter = ',') noexcept { setByCSV(csv, delimiter); }


        [[nodiscard]] virtual const char *className() const noexcept { return "Vec2Fix"; }

        friend std::ostream& operator << (std::ostream& os, const Vec2Fix* o) {
            return o == nullptr ? os << "Vec2Fix nullptr" : os << *o;
        }

        friend std::ostream& operator << (std::ostream& os, const Vec2Fix& o) {
            os << o.m_x << ", " << o.m_y;
            return os;
        }

        bool operator == (const Vec2Fix& other) const { return other.m_x == m_x && other.m_y == m_y; }
        bool operator != (const Vec2Fix& other) const { return other.m_x != m_x || other.m_y != m_y; }

        Vec2Fix operator + (const Vec2Fix& other) const { return { m_x + other.m_x, m_y + other.m_y }; }
        Vec2Fix operator - (const Vec2Fix& other) const { return { m_x - other.m_x, m_y - other.m_y }; }

        Vec2Fix operator * (const Vec2Fix& other) const { return { m_x * other.m_x, m_y * other.m_y }; }

        [[nodiscard]] Fix x() const noexcept { return m_x; }
        [[nodiscard]] Fix y() const noexcept { return m_y; }
        [[nodiscard]] float xFloat() const noexcept { return m_x.asFloat(); }
        [[nodiscard]] float yFloat() const noexcept { return m_y.asFloat(); }
        [[nodiscard]] double xDouble() const noexcept { return m_x.asDouble(); }
        [[nodiscard]] double yDouble() const noexcept { return m_y.asDouble(); }

        [[nodiscard]] inline Fix* xData() noexcept { return &m_x; }
        [[nodiscard]] inline Fix* yData() noexcept { return &m_y; }

        bool set(const Vec2Fix& vec) noexcept {
            if (vec != *this) {
                *this = vec;
                return true;
            }
            return false;
        }

        bool set(int32_t x, int32_t y) noexcept {
            if (m_x != x && m_y != y) {
                m_x = x;
                m_y = y;
                return true;
            }
            else {
                return false;
            }
        }

        bool set(const Fix& x, const Fix& y) noexcept {
            if (x != m_x || y != m_y) {
                m_x = x;
                m_y = y;
                return true;
            }
            else {
                return false;
            }
        }

        bool setByCSV(const char* csv, char delimiter) noexcept {
            if (csv) {
                CSVLineParser csv_line_parser(csv);
                csv_line_parser.setDelimiter(delimiter);
                if (!csv_line_parser.nextFix(m_x)) { return false; }
                return csv_line_parser.nextFix(m_y);
            }
            return false;
        }

        bool setByCSV(const String& csv, char delimiter) noexcept {
            return setByCSV(csv.utf8(), delimiter);
        }

        void zero() noexcept { m_x = 0; m_y = 0; }

        void clampX(const Fix& min, const Fix& max) noexcept { m_x.clamp(min, max); }
        void clampY(const Fix& min, const Fix& max) noexcept { m_y.clamp(min, max); }

        [[nodiscard]] Vec2f vec2f() const noexcept { return { m_x.asFloat(), m_y.asFloat() }; }
        [[nodiscard]] Vec2d vec2d() const noexcept { return { m_x.asDouble(), m_y.asDouble() }; }

        void vec2f(Vec2f& out_vec) const noexcept {
            out_vec.m_x = m_x.asFloat();
            out_vec.m_y = m_y.asFloat();
        }

        void setVec2(const Vec2f& vec) noexcept {
            m_x.setDouble(vec.m_x);
            m_y.setDouble(vec.m_y);
        }

        void setPrecision(int32_t precision) noexcept {
            m_x.setPrecision(precision);
            m_y.setPrecision(precision);
        }

    public:
        Fix m_x, m_y;
    };


} // End of namespace Grain

#endif // GrainVec2Fix_hpp
