//
//  RangeRect.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 12.07.2025
//

#ifndef GrainRangeRect_hpp
#define GrainRangeRect_hpp

#include "Grain.hpp"
#include "Type/Fix.hpp"
#include "2d/Rect.hpp"
#include "Math/Vec2.hpp"
#include "Math/Vec2Fix.hpp"
#include "Math/Math.hpp"

#include <cstdint>
#include <iostream>


namespace Grain {

    class RangeRectFix {
    public:
        RangeRectFix() noexcept = default;
        explicit RangeRectFix(Fix min_x, Fix min_y, Fix max_x, Fix max_y) noexcept : m_min_x(min_x), m_min_y(min_y), m_max_x(max_x), m_max_y(max_y) {}

        [[nodiscard]] virtual const char* className() const noexcept { return "RangeRectFix"; }

        friend std::ostream& operator << (std::ostream& os, const RangeRectFix& o) {
            return os << o.m_min_x << ", " << o.m_min_y << " | " << o.m_max_x << ", " << o.m_max_y;
        }

        RangeRectFix& operator = (const Vec2Fix& v) {
            m_min_x = m_max_x = v.m_x;
            m_min_y = m_max_y = v.m_y;
            return *this;
        }

        RangeRectFix& operator = (const RectFix& r) {
            m_min_x = r.m_width > 0 ? r.m_x : r.m_x + r.m_width;
            m_min_y = r.m_height > 0 ? r.m_y : r.m_y + r.m_height;
            m_max_x = r.m_width > 0 ? r.m_x + r.m_width : r.m_x;
            m_max_y = r.m_height > 0 ? r.m_y + r.m_height : r.m_y;
            return *this;
        }

        bool operator == (const RangeRectFix& other) const {
            return
                m_min_x == other.m_min_x && m_min_y == other.m_min_y &&
                m_max_x == other.m_max_x && m_max_y == other.m_max_y;
        }

        bool operator != (const RangeRectFix& other) const {
            return
                m_min_x != other.m_min_x || m_min_y != other.m_min_y ||
                m_max_x != other.m_max_x || m_max_y != other.m_max_y;
        }

        RangeRectFix operator + (const RangeRectFix& other) const {
            RangeRectFix result;
            result.m_min_x = m_min_x < other.m_min_x ? m_min_x : other.m_min_x;
            result.m_min_y = m_min_y < other.m_min_y ? m_min_y : other.m_min_y;
            result.m_max_x = m_max_x > other.m_max_x ? m_max_x : other.m_max_x;
            result.m_max_y = m_max_y > other.m_max_y ? m_max_y : other.m_max_y;
            return result;
        }

        RangeRectFix operator + (const Vec2Fix& v) const {
            RangeRectFix result = *this;
            if (v.m_x < m_min_x) { result.m_min_x = v.m_x; }
            if (v.m_y < m_min_y) { result.m_min_y = v.m_y; }
            if (v.m_x > m_max_x) { result.m_max_x = v.m_x; }
            if (v.m_y > m_max_y) { result.m_max_y = v.m_y; }
            return result;
        }

        RangeRectFix operator + (const RectFix& r) const {
            RangeRectFix result;
            Fix r_min_x = r.m_width > 0 ? r.m_x : r.m_x + r.m_width;
            Fix r_min_y = r.m_height > 0 ? r.m_y : r.m_y + r.m_height;
            Fix r_max_x = r.m_width > 0 ? r.m_x + r.m_width : r.m_x;
            Fix r_max_y = r.m_height > 0 ? r.m_y + r.m_height : r.m_y;
            result.m_min_x = m_min_x < r_min_x ? m_min_x : r_min_x;
            result.m_min_y = m_min_y < r_min_y ? m_min_y : r_min_y;
            result.m_max_x = m_max_x > r_max_x ? m_max_x : r_max_x;
            result.m_max_y = m_max_y > r_max_y ? m_max_y : r_max_y;
            return result;
        }

        RangeRectFix& operator += (const RangeRectFix& other) {
            m_min_x = m_min_x < other.m_min_x ? m_min_x : other.m_min_x;
            m_min_y = m_min_y < other.m_min_y ? m_min_y : other.m_min_y;
            m_max_x = m_max_x > other.m_max_x ? m_max_x : other.m_max_x;
            m_max_y = m_max_y > other.m_max_y ? m_max_y : other.m_max_y;
            return *this;
        }

        RangeRectFix& operator += (const Vec2Fix& r) {
            if (r.m_x < m_min_x) { m_min_x = r.m_x; }
            if (r.m_y < m_min_y) { m_min_y = r.m_y; }
            if (r.m_x > m_max_x) { m_max_x = r.m_x; }
            if (r.m_y > m_max_y) { m_max_y = r.m_y; }
            return *this;
        }

        RangeRectFix& operator += (const RectFix& r) {
            Fix r_min_x = r.m_width > 0 ? r.m_x : r.m_x + r.m_width;
            Fix r_min_y = r.m_height > 0 ? r.m_y : r.m_y + r.m_height;
            Fix r_max_x = r.m_width > 0 ? r.m_x + r.m_width : r.m_x;
            Fix r_max_y = r.m_height > 0 ? r.m_y + r.m_height : r.m_y;
            m_min_x = m_min_x < r_min_x ? m_min_x : r_min_x;
            m_min_y = m_min_y < r_min_y ? m_min_y : r_min_y;
            m_max_x = m_max_x > r_max_x ? m_max_x : r_max_x;
            m_max_y = m_max_y > r_max_y ? m_max_y : r_max_y;
            return *this;
        }


        [[nodiscard]] Fix minX() const noexcept { return m_min_x; }
        [[nodiscard]] Fix maxX() const noexcept { return m_max_x; }
        [[nodiscard]] Fix minY() const noexcept { return m_min_y; }
        [[nodiscard]] Fix maxY() const noexcept { return m_max_y; }
        [[nodiscard]] Fix centerX() const noexcept { return m_min_x + (m_max_x - m_min_x) / 2; }
        [[nodiscard]] Fix centerY() const noexcept { return m_min_y + (m_max_y - m_min_y) / 2; }
        [[nodiscard]] Fix width() const noexcept { Fix w = m_max_x - m_min_x; return w < 0 ? -w : w; }
        [[nodiscard]] Fix height() const noexcept { Fix h = m_max_y - m_min_y; return h < 0 ? -h : h; }

        [[nodiscard]] Vec2d centerAsVec2d() const noexcept { return Vec2d(centerX().asDouble(), centerY().asDouble()); }

        [[nodiscard]] RectFix rect() const noexcept {
            return RectFix(m_min_x, m_min_y, m_max_x - m_min_x, m_max_y - m_min_y);
        }

        void initForMinMaxSearch() noexcept {
            m_min_x.setToMax();
            m_min_y.setToMax();
            m_max_x.setToMin();
            m_max_y.setToMin();
        }

        void set(const Fix& x, const Fix& y) noexcept { m_min_x = m_max_x = x; m_min_y = m_max_y = y; }
        void set(const Vec2Fix& v) noexcept { m_min_x = m_max_x = v.m_x; m_min_y = m_max_y = v.m_y; }
        void set(Vec2Fix* v) noexcept { if (v) { m_min_x = m_max_x = v->m_x; m_min_y = m_max_y = v->m_y; } }

        void setMinX(const Fix& v) noexcept { m_min_x = v; }
        void setMinY(const Fix& v) noexcept { m_min_y = v; }
        void setMaxX(const Fix& v) noexcept { m_max_x = v; }
        void setMaxY(const Fix& v) noexcept { m_max_y = v; }

        void set(double min_x, double min_y, double max_x, double max_y) noexcept {
            m_min_x = min_x; m_min_y = min_y;
            m_max_x = max_x; m_max_y = max_y;
        }

        void set(const Fix& min_x, const Fix& min_y, const Fix& max_x, const Fix& max_y) noexcept {
            m_min_x = min_x; m_min_y = min_y;
            m_max_x = max_x; m_max_y = max_y;
        }

        void set(const Vec2Fix& min, const Vec2Fix& max) noexcept {
            m_min_x = min.m_x; m_min_y = min.m_y;
            m_max_x = max.m_x; m_max_y = max.m_y;
        }

        void add(const Vec2Fix& v) noexcept {
            addX(v.m_x); addY(v.m_y);
        }

        void add(const Vec2Fix* v) noexcept {
            if (v) {
                addX(v->m_x); addY(v->m_y);
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

        void add(const Fix& x, const Fix& y) noexcept {
            addX(x); addY(y);
        }

        void add(const RectFix& rect) noexcept {
            addX(rect.m_x); addX(rect.x2());
            addY(rect.m_y); addY(rect.y2());
        }

        void add(const RangeRectFix& r) noexcept {
            addX(r.m_min_x); addX(r.m_max_x);
            addY(r.m_min_y); addY(r.m_max_y);
        }

        void limit(const RangeRectFix& max_rect) noexcept {
            if (m_min_x < max_rect.m_min_x) m_min_x = max_rect.m_min_x;
            if (m_min_y < max_rect.m_min_y) m_min_y = max_rect.m_min_y;
            if (m_max_x > max_rect.m_max_x) m_max_x = max_rect.m_max_x;
            if (m_max_y > max_rect.m_max_y) m_max_y = max_rect.m_max_y;
        }

        void scrollX(const Fix& amount) noexcept { m_min_x += amount; m_max_x += amount; }
        void scrollY(const Fix& amount) noexcept { m_min_y += amount; m_max_y += amount; }
        void scroll(const Fix& x_amount, const Fix& y_amount) noexcept { scrollX(x_amount); scrollY(y_amount); }
        void scrollRight() noexcept { scrollX(width() / 2); }
        void scrollLeft() noexcept { scrollX(-width() / 2); }
        void scrollUp() noexcept { scrollY(height() / 2); }
        void scrollDown() noexcept { scrollY(-height() / 2); }

        void lerp(const RangeRectFix& r, const Fix& t) noexcept {
            RangeRectFix result;
            result.m_min_x = m_min_x + t * (r.m_min_x - m_min_x);
            result.m_min_y = m_min_y + t * (r.m_min_y - m_min_y);
            result.m_max_x = m_max_x + t * (r.m_max_x - m_max_x);
            result.m_max_y = m_max_y + t * (r.m_max_y - m_max_y);
            *this = result;
        }

        static RangeRectFix lerp(const RangeRectFix& a, const RangeRectFix& b, const Fix& t) noexcept {
            RangeRectFix result;
            result.m_min_x = a.m_min_x + t * (b.m_min_x - a.m_min_x);
            result.m_min_y = a.m_min_y + t * (b.m_min_y - a.m_min_y);
            result.m_max_x = a.m_max_x + t * (b.m_max_x - a.m_max_x);
            result.m_max_y = a.m_max_y + t * (b.m_max_y - a.m_max_y);
            return result;
        }

        [[nodiscard]] bool isInside(const Fix& x, const Fix& y) const noexcept {
            return (x >= m_min_x && x <= m_max_x && y >= m_min_y && y <= m_max_y);
        }

        [[nodiscard]] bool isInside(const Vec2Fix& v) const noexcept {
            return (v.x() >= m_min_x && v.x() <= m_max_x && v.y() >= m_min_y && v.y() <= m_max_y);
        }

    public:
        Fix m_min_x = 0;
        Fix m_min_y = 0;
        Fix m_max_x = 0;
        Fix m_max_y = 0;
    };


    template <ScalarType T>
    class RangeRect {
    public:
        RangeRect() noexcept = default;
        RangeRect(T min_x, T min_y, T max_x, T max_y) noexcept : m_min_x(min_x), m_min_y(min_y), m_max_x(max_x), m_max_y(max_y) {}

        [[nodiscard]] virtual const char* className() const noexcept { return "RangeRect"; }

        friend std::ostream& operator << (std::ostream& os, const RangeRect* o) {
            o == nullptr ? os << "RangeRect nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const RangeRect& o) {
            os << std::fixed << o.m_min_x << ", " << o.m_min_y << ", " << o.m_max_x << ", " << o.m_max_y << std::defaultfloat;
            return os;
        }

        RangeRect& operator = (const Vec2<T>& v) {
            m_min_x = m_max_x = v.m_x;
            m_min_y = m_max_y = v.m_y;
            return *this;
        }

        RangeRect& operator = (const Rect<T>& r) {
            m_min_x = r.m_width >= 0 ? r.m_x : r.m_x + r.m_width;
            m_min_y = r.m_height >= 0 ? r.m_y : r.m_y + r.m_height;
            m_max_x = r.m_width >= 0 ? r.m_x + r.m_width : r.m_x;
            m_max_y = r.m_height >= 0 ? r.m_y + r.m_height : r.m_y;
            return *this;
        }

        RangeRect& operator = (const RangeRectFix& r) {
            if constexpr (std::is_same_v<T, double>) {
                m_min_x = r.m_min_x.asDouble();
                m_max_x = r.m_max_x.asDouble();
                m_min_y = r.m_min_y.asDouble();
                m_max_y = r.m_max_y.asDouble();
            }
            else if constexpr (std::is_same_v<T, float>) {
                m_min_x = r.m_min_x.asFloat();
                m_max_x = r.m_max_x.asFloat();
                m_min_y = r.m_min_y.asFloat();
                m_max_y = r.m_max_y.asFloat();
            }
            else if constexpr (std::is_same_v<T, int64_t>) {
                m_min_x = r.m_min_x.asInt64();
                m_max_x = r.m_max_x.asInt64();
                m_min_y = r.m_min_y.asInt64();
                m_max_y = r.m_max_y.asInt64();
            }
            else {
                m_min_x = r.m_min_x.asInt32();
                m_max_x = r.m_max_x.asInt32();
                m_min_y = r.m_min_y.asInt32();
                m_max_y = r.m_max_y.asInt32();
            }
            return *this;
        }

        bool operator == (const RangeRect& other) const {
            return
                m_min_x == other.m_min_x && m_min_y == other.m_min_y &&
                m_max_x == other.m_max_x && m_max_y == other.m_max_y;
        }

        bool operator != (const RangeRect& other) const {
            return
                m_min_x != other.m_min_x || m_min_y != other.m_min_y ||
                m_max_x != other.m_max_x || m_max_y != other.m_max_y;
        }

        RangeRect operator + (const RangeRect& other) const {
            RangeRect result;
            result.m_min_x = m_min_x < other.m_min_x ? m_min_x : other.m_min_x;
            result.m_min_y = m_min_y < other.m_min_y ? m_min_y : other.m_min_y;
            result.m_max_x = m_max_x > other.m_max_x ? m_max_x : other.m_max_x;
            result.m_max_y = m_max_y > other.m_max_y ? m_max_y : other.m_max_y;
            return result;
        }

        RangeRect operator + (const Vec2<T>& v) const {
            RangeRect result = *this;
            if (v.m_x < m_min_x) { result.m_min_x = v.m_x; }
            if (v.m_y < m_min_y) { result.m_min_y = v.m_y; }
            if (v.m_x > m_max_x) { result.m_max_x = v.m_x; }
            if (v.m_y > m_max_y) { result.m_max_y = v.m_y; }
            return result;
        }

        RangeRect operator + (const Rect<T>& r) const {
            RangeRect result;
            T r_min_x = r.m_width > 0 ? r.m_x : r.m_x + r.m_width;
            T r_min_y = r.m_height > 0 ? r.m_y : r.m_y + r.m_height;
            T r_max_x = r.m_width > 0 ? r.m_x + r.m_width : r.m_x;
            T r_max_y = r.m_height > 0 ? r.m_y + r.m_height : r.m_y;
            result.m_min_x = m_min_x < r_min_x ? m_min_x : r_min_x;
            result.m_min_y = m_min_y < r_min_y ? m_min_y : r_min_y;
            result.m_max_x = m_max_x > r_max_x ? m_max_x : r_max_x;
            result.m_max_y = m_max_y > r_max_y ? m_max_y : r_max_y;
            return result;
        }

        RangeRect& operator += (const RangeRect& other) {
            m_min_x = m_min_x < other.m_min_x ? m_min_x : other.m_min_x;
            m_min_y = m_min_y < other.m_min_y ? m_min_y : other.m_min_y;
            m_max_x = m_max_x > other.m_max_x ? m_max_x : other.m_max_x;
            m_max_y = m_max_y > other.m_max_y ? m_max_y : other.m_max_y;
            return *this;
        }

        RangeRect& operator += (const Vec2<T>& v) {
            if (v.m_x < m_min_x) { m_min_x = v.m_x; }
            if (v.m_y < m_min_y) { m_min_y = v.m_y; }
            if (v.m_x > m_max_x) { m_max_x = v.m_x; }
            if (v.m_y > m_max_y) { m_max_y = v.m_y; }
            return *this;
        }

        RangeRect& operator += (const Rect<T>& r) {
            T r_min_x = r.m_width > 0 ? r.m_x : r.m_x + r.m_width;
            T r_min_y = r.m_height > 0 ? r.m_y : r.m_y + r.m_height;
            T r_max_x = r.m_width > 0 ? r.m_x + r.m_width : r.m_x;
            T r_max_y = r.m_height > 0 ? r.m_y + r.m_height : r.m_y;
            m_min_x = m_min_x < r_min_x ? m_min_x : r_min_x;
            m_min_y = m_min_y < r_min_y ? m_min_y : r_min_y;
            m_max_x = m_max_x > r_max_x ? m_max_x : r_max_x;
            m_max_y = m_max_y > r_max_y ? m_max_y : r_max_y;
            return *this;
        }


        [[nodiscard]] T minX() const noexcept { return m_min_x; }
        [[nodiscard]] T maxX() const noexcept { return m_max_x; }
        [[nodiscard]] T minY() const noexcept { return m_min_y; }
        [[nodiscard]] T maxY() const noexcept { return m_max_y; }
        [[nodiscard]] Vec2<T> pos1() const noexcept { return Vec2<T>(m_min_x, m_min_y); }
        [[nodiscard]] Vec2<T> pos2() const noexcept { return Vec2<T>(m_max_x, m_min_y); }
        [[nodiscard]] Vec2<T> pos3() const noexcept { return Vec2<T>(m_max_x, m_max_y); }
        [[nodiscard]] Vec2<T> pos4() const noexcept { return Vec2<T>(m_min_x, m_max_y); }
        [[nodiscard]] Vec2<T> center() const noexcept { return Vec2<T>(m_min_x + (m_max_x - m_min_x) / 2, m_min_y + (m_max_y - m_min_y) / 2); }
        [[nodiscard]] T centerX() const noexcept { return m_min_x + (m_max_x - m_min_x) / 2; }
        [[nodiscard]] T centerY() const noexcept { return m_min_y + (m_max_y - m_min_y) / 2; }
        [[nodiscard]] T width() const noexcept { return m_max_x - m_min_x; }
        [[nodiscard]] T height() const noexcept { return m_max_y - m_min_y; }
        [[nodiscard]] T shortSide() const noexcept { T w = width(); T h = height(); return w < h ? w : h; }
        [[nodiscard]] T longSide() const noexcept { T w = width(); T h = height(); return w > h ? w : h; }

        [[nodiscard]] Vec2<T> randomPos() const noexcept {
            return Vec2<T>(m_min_x + Random::next(m_max_x - m_min_x), m_min_y + Random::next(m_max_y - m_min_y));
        }

        [[nodiscard]] Vec2d innerPos(double x, double y) const noexcept {
            return Vec2d(Math::lerp(m_min_x, m_max_x, x), Math::lerp(m_min_y, m_max_y, y));
        }


        [[nodiscard]] double aspectRatio() const noexcept {
            double w = width();
            double h = height();
            if (w > std::numeric_limits<double>::epsilon() && h > std::numeric_limits<double>::epsilon()) {
                return h / w;
            }
            return 1;
        }

        [[nodiscard]] bool isLandscape() const noexcept { return aspectRatio() < 1.0; }
        [[nodiscard]] bool isPortrait() const noexcept { return aspectRatio() > 1.0; }
        [[nodiscard]] bool isSquare() const noexcept { return std::fabs(1.0 - aspectRatio() <= std::numeric_limits<float>::epsilon()); }


        [[nodiscard]] Rect<T> rect() const noexcept {
            return Rect<T>(m_min_x, m_min_y, m_max_x - m_min_x, m_max_y - m_min_y);
        }

        [[nodiscard]] Rect<T> rectWidthMaxSideLength(T max_side_length) const noexcept {
            double scale = 0.0;
            T w = width();
            T h = height();
            std::cout << ">>>>> w: " << w << ", h: " << h << ", isLandscape(): " << isLandscape();
            if (isLandscape() && w > std::numeric_limits<float>::epsilon()) {
                scale = max_side_length / w;
            }
            else if (h > std::numeric_limits<float>::epsilon()) {
                scale = max_side_length / h;
            }
            std::cout << ", scale: " << scale << std::endl;

            return Rect<T>(0, 0, scale * w, scale * h);
        }


        void initForMinMaxSearch() noexcept;

        void set(T x, T y) noexcept { m_min_x = m_max_x = x; m_min_y = m_max_y = y; }
        void set(const Vec2<T>& v) noexcept { m_min_x = m_max_x = v.m_x; m_min_y = m_max_y = v.m_y; }
        void set(Vec2<T>* v) noexcept { if (v) { m_min_x = m_max_x = v->m_x; m_min_y = m_max_y = v->m_y; } }

        void set(T min_x, T min_y, T max_x, T max_y) noexcept {
            m_min_x = min_x;
            m_min_y = min_y;
            m_max_x = max_x;
            m_max_y = max_y;
        }

        void set(const Vec2<T>& min, const Vec2<T>& max) noexcept {
            m_min_x = min.m_x;
            m_max_x = max.m_x;
            m_min_y = min.m_y;
            m_max_y = max.m_y;
        }

        void set(T*  values, int32_t n) noexcept {
            if (values && n == 4) {
                m_min_x = values[0];
                m_min_y = values[1];
                m_max_x = values[2];
                m_max_y = values[3];
            }
        }

        void setLonlat(T lon1, T lat1, T lon2, T lat2) noexcept {
            m_min_x = lon1;
            m_min_y = lat1;
            m_max_x = lon2;
            m_max_y = lat2;
        }


        void sanitizeMinMax() noexcept {
            if (m_min_x > m_max_x) { swapX(); }
            if (m_min_y > m_max_y) { swapY(); }
        }

        void swapX() noexcept {
            T temp = m_min_x; m_min_x = m_max_x; m_max_x = temp;
        }

        void swapY() noexcept {
            T temp = m_min_y; m_min_y = m_max_y; m_max_y = temp;
        }

        bool add(const Vec2<T>& v) noexcept {
            bool result = false;
            if (v.m_x < m_min_x) { m_min_x = v.m_x; result = true; }
            if (v.m_x > m_max_x) { m_max_x = v.m_x; result = true; }
            if (v.m_y < m_min_y) { m_min_y = v.m_y; result = true; }
            if (v.m_y > m_max_y) { m_max_y = v.m_y; result = true; }
            return result;
        }

        bool add(const Vec2<T>* v) noexcept { if (v) { return add(*v); } return false; }

        bool add(const Vec2<T>* v, int32_t n) noexcept {
            bool result = false;
            if (v) {
                for (int32_t i = 0; i < n; i++) {
                    result |= add(v[i]);
                }
            }
            return result;
        }

        bool setByVec2Array(const Vec2<T>* v, int32_t n) noexcept {
            initForMinMaxSearch();
            return add(v, n);
        }

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

        bool add(T x, T y) noexcept {
            bool result = false;
            if (x < m_min_x) { m_min_x = x; result = true; }
            if (x > m_max_x) { m_max_x = x; result = true; }
            if (y < m_min_y) { m_min_y = y; result = true; }
            if (y > m_max_y) { m_max_y = y; result = true; }
            return result;
        }

        void add(const Rect<T>& rect) noexcept {
            if (rect.m_x < m_min_x) m_min_x = rect.m_x;
            if (rect.x2() > m_max_x) m_max_x = rect.x2();
            if (rect.m_y < m_min_y) m_min_y = rect.m_y;
            if (rect.y2() > m_max_y) m_max_y = rect.y2();
        }

        void add(const RangeRect<T>& rect) noexcept {
            *this += rect;
        }


        void limit(const RangeRect& max_rect) noexcept {
            if (m_min_x < max_rect.m_min_x) m_min_x = max_rect.m_min_x;
            if (m_min_y < max_rect.m_min_y) m_min_y = max_rect.m_min_y;
            if (m_max_x > max_rect.m_max_x) m_max_x = max_rect.m_max_x;
            if (m_max_y > max_rect.m_max_y) m_max_y = max_rect.m_max_y;
        }

        void adjustUniform(T view_width, T view_height) noexcept {

            T xScale = width() / view_width;    // TODO: check 0!
            T yScale = height() / view_height;

            if (yScale < xScale) {
                T c = centerY();
                T r = height() * (xScale / yScale) / 2;
                m_min_y = c - r;
                m_max_y = c + r;
            }
            else {
                T c = centerX();
                T r = width() * (yScale / xScale) / 2;
                m_min_x = c - r;
                m_max_x = c + r;
            }
        }

        [[nodiscard]] bool contains(const Vec2<T> pos) const noexcept {
            return pos.m_x >= m_min_x &&
                   pos.m_x <= m_max_x &&
                   pos.m_y >= m_min_y &&
                   pos.m_y <= m_max_y;
        }

        [[nodiscard]] bool contains(const Vec2<T> pos, T tolerance) const noexcept {
            return pos.m_x >= m_min_x - tolerance &&
                   pos.m_x <= m_max_x + tolerance &&
                   pos.m_y >= m_min_y - tolerance &&
                   pos.m_y <= m_max_y + tolerance;
        }

        /**
         *  @brief Checks if this rectangle overlaps with another rectangle.
         *
         *  This method determines if there is any overlapping area between this
         *  rectangle and another rectangle specified by `range_rect`.
         *  Overlap is determined based on the x and y coordinates of the two rectangles.
         *
         *  @param range_rect The rectangle to check for overlap with.
         *                    It is represented by another instance of RangeRect.
         *  @return `true` if the rectangles overlap; `false` otherwise.
         */
        [[nodiscard]] bool overlaps(const RangeRect& range_rect) const noexcept {
            if (m_max_x < range_rect.m_min_x || range_rect.m_max_x < m_min_x) {
                return false;
            }
            if (m_max_y < range_rect.m_min_y || range_rect.m_max_y < m_min_y) {
                return false;
            }
            return true;
        }

        void scrollX(T amount) noexcept { m_min_x += amount; m_max_x += amount; }
        void scrollY(T amount) noexcept { m_min_y += amount; m_max_y += amount; }
        void scroll(T x_amount, T y_amount) noexcept { scrollX(x_amount); scrollY(y_amount); }
        void scrollRight() noexcept { scrollX(width() / 2); }
        void scrollLeft() noexcept { scrollX(-width() / 2); }
        void scrollUp() noexcept { scrollY(height() / 2); }
        void scrollDown() noexcept { scrollY(-height() / 2); }

        void zoom(T factor) noexcept {
            zoomX(factor);
            zoomY(factor);
        }

        void zoom(const Vec2<T>& pivot, T factor) noexcept {
            if (factor != 0) {
                m_min_x = pivot.m_x + (m_min_x - pivot.m_x) / factor;
                m_max_x = pivot.m_x + (m_max_x - pivot.m_x) / factor;
                m_min_y = pivot.m_y + (m_min_y - pivot.m_y) / factor;
                m_max_y = pivot.m_y + (m_max_y - pivot.m_y) / factor;
            }
        }

        void zoomX(T factor) noexcept {
            if (factor != 0) {
                T c = centerX();
                T s = width() / factor / 2;
                m_min_x = c - s;
                m_max_x = c + s;
            }
        }

        void zoomX(T pivot, T factor) noexcept {
            if (factor != 0) {
                m_min_x =  pivot + (m_min_x - pivot) / factor;
                m_max_x =  pivot + (m_max_x - pivot) / factor;
            }
        }

        void zoomY(T factor) noexcept {
            if (factor != 0) {
                T c = centerY();
                T s = height() / factor / 2.0;
                m_min_y = c - s;
                m_max_y = c + s;
            }
        }

        void zoomY(T pivot, T factor) noexcept {
            if (factor != 0) {
                m_min_y =  pivot + (m_min_y - pivot) / factor;
                m_max_y =  pivot + (m_max_y - pivot) / factor;
            }
        }

        void zoomIn() noexcept { zoomInX(); zoomInY(); }

        void zoomInX() noexcept {
            T amount = width() / 4;
            T center = centerX();
            m_min_x = center - amount;
            m_max_x = center + amount;
        }

        void zoomInY() noexcept {
            T amount = height() / 4;
            T center = centerY();
            m_min_y = center - amount;
            m_max_y = center + amount;
        }

        void zoomOut() noexcept { zoomOutX(); zoomOutY(); }

        void zoomOutX() noexcept {
            T amount = width();
            T center = centerX();
            m_min_x = center - amount;
            m_max_x = center + amount;
        }

        void zoomOutY() noexcept {
            T amount = height();
            T center = centerY();
            m_min_y = center - amount;
            m_max_y = center + amount;
        }

        void extend(double f) noexcept {
            double w = width() * f * 0.5;
            double h = height() * f * 0.5;
            if (m_min_x < m_max_x) {
                m_min_x -= w;
                m_max_x += w;
            }
            else {
                m_min_x += w;
                m_max_x -= w;
            }
            if (m_min_y < m_max_y) {
                m_min_y -= h;
                m_max_y += h;
            }
            else {
                m_min_y += h;
                m_max_y -= h;
            }
        }

        void lerp(const RangeRect<T>& r, double t) noexcept {
            RangeRect<T> result;
            result.m_min_x = m_min_x + t * (r.m_min_x - m_min_x);
            result.m_min_y = m_min_y + t * (r.m_min_y - m_min_y);
            result.m_max_x = m_max_x + t * (r.m_max_x - m_max_x);
            result.m_max_y = m_max_y + t * (r.m_max_y - m_max_y);
            *this = result;
        }

        static RangeRect<T> lerp(const RangeRect<T>& a, const RangeRect<T>& b, double t) noexcept {
            RangeRect<T> result;
            result.m_min_x = a.m_min_x + t * (b.m_min_x - a.m_min_x);
            result.m_min_y = a.m_min_y + t * (b.m_min_y - a.m_min_y);
            result.m_max_x = a.m_max_x + t * (b.m_max_x - a.m_max_x);
            result.m_max_y = a.m_max_y + t * (b.m_max_y - a.m_max_y);
            return result;
        }

        void writeToFile(File& file);
        void readFromFile(File& file);

    public:
        T m_min_x = 0;
        T m_min_y = 0;
        T m_max_x = 0;
        T m_max_y = 0;
    };


    // Standard types
    using RangeRecti = RangeRect<int32_t>;  ///< 32 bit integer.
    using RangeRectl = RangeRect<int64_t>;  ///< 64 bit integer.
    using RangeRectf = RangeRect<float>;    ///< 32 bit floating point.
    using RangeRectd = RangeRect<double>;   ///< 64 bit floating point.

    // Specialized methods
    template <> void RangeRect<int32_t>::initForMinMaxSearch() noexcept;
    template <> void RangeRect<int64_t>::initForMinMaxSearch() noexcept;
    template <> void RangeRect<float>::initForMinMaxSearch() noexcept;
    template <> void RangeRect<double>::initForMinMaxSearch() noexcept;


} // End of namespace Grain

#endif // GrainRangeRect_hpp
