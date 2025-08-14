//
//  Cube.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 01.08.2025
//

#ifndef GrainCube_hpp
#define GrainCube_hpp

#include "Grain.hpp"
#include "Math/Vec3.hpp"
#include "Type/Fix.hpp"
#include "Math/Random.hpp"


namespace Grain {

    /**
     *  @brief Cube.
     *
     *  The `Cube` class represents a three-dimensional cube.
     */
    template <class T>
    class Cube {
    public:
        T m_x = 0;
        T m_y = 0;
        T m_z = 0;
        T m_width = 1;
        T m_height = 1;
        T m_depth = 1;

    public:
        Cube() noexcept {}
        Cube(T size) noexcept : m_width(size), m_height(size) {}
        Cube(T width, T height, T depth) noexcept : m_width(width), m_height(height), m_depth(depth) {}
        Cube(T x, T y, T z, T width, T height, T depth) noexcept : m_x(x), m_y(y), m_z(z), m_width(width), m_height(height), m_depth(depth) {}
        Cube(const Cube<int32_t>& r) noexcept { m_x = r.m_x; m_y = r.m_y; m_z = r.m_z; m_width = r.m_width; m_height = r.m_height; m_depth = r.m_depth; }
        Cube(const Cube<int64_t>& r) noexcept { m_x = r.m_x; m_y = r.m_y; m_z = r.m_z; m_width = r.m_width; m_height = r.m_height; m_depth = r.m_depth; }
        Cube(const Cube<float>& r) noexcept { m_x = r.m_x; m_y = r.m_y; m_z = r.m_z; m_width = r.m_width; m_height = r.m_height; m_depth = r.m_depth; }
        Cube(const Cube<double>& r) noexcept { m_x = r.m_x; m_y = r.m_y; m_z = r.m_z; m_width = r.m_width; m_height = r.m_height; m_depth = r.m_depth; }
        Cube(const Cube& r, T inset_size) noexcept { *this = r; inset(inset_size); }

        [[nodiscard]] const char* className() const noexcept { return "Cube"; }

        Cube& operator = (const Cube<int32_t>& r) {
            m_x = r.m_x; m_y = r.m_y; m_z = r.m_z; m_width = r.m_width; m_height = r.m_height; m_depth = r.m_depth; return *this;
        }

        Cube& operator = (const Cube<int64_t>& r) {
            m_x = r.m_x; m_y = r.m_y; m_z = r.m_z; m_width = r.m_width; m_height = r.m_height; m_depth = r.m_depth; return *this;
        }

        Cube& operator = (const Cube<float>& r) {
            m_x = r.m_x; m_y = r.m_y; m_z = r.m_z; m_width = r.m_width; m_height = r.m_height; m_depth = r.m_depth; return *this;
        }

        Cube& operator = (const Cube<double>& r) {
            m_x = r.m_x; m_y = r.m_y; m_z = r.m_z; m_width = r.m_width; m_height = r.m_height; m_depth = r.m_depth; return *this;
        }

        bool operator == (const Cube& r) const {
            return m_x == r.m_x && m_y == r.m_y && m_z == r.m_z && m_width == r.m_width && m_height == r.m_height && m_depth == r.m_depth;
        }

        bool operator != (const Cube& r) const {
            return m_x != r.m_x || m_y != r.m_y || m_z != r.m_z || m_width != r.m_width || m_height != r.m_height || m_depth != r.m_depth;
        }

        Cube operator + (const Vec3<T>& v) const {
            return Cube(m_x + v.m_x, m_y + v.m_y, m_z + v.m_z, m_width, m_height, m_depth);
        }

        Cube operator - (const Vec3<T>& v) const {
            return Cube(m_x - v.m_x, m_y - v.m_y, m_z - v.m_z, m_width, m_height, m_depth);
        }

        Cube operator * (const Vec3<T>& v) const {
            return Cube(m_x * v.m_x, m_y * v.m_y, m_z * v.m_z, m_width, m_height, m_depth);
        }

        Cube& operator += (const Vec3<T>& v) { m_x += v.m_x; m_y += v.m_y; m_z += v.m_z; return *this; }
        Cube& operator -= (const Vec3<T>& v) { m_x -= v.m_x; m_y -= v.m_y; m_z -= v.m_z; return *this; }
        Cube& operator *= (const Vec3<T>& v) { m_x *= v.m_x; m_y *= v.m_y; m_z *= v.m_z; return *this; }

        Cube& operator += (const Cube<T>& r) {
            Cube<T> result;
            T minX = m_x < r.m_x ? m_x : r.m_x;
            T minY = m_y < r.m_y ? m_y : r.m_y;
            T minZ = m_z < r.m_z ? m_z : r.m_z;
            T maxX = x2() > r.x2() ? x2() : r.x2();
            T maxY = y2() > r.y2() ? y2() : r.y2();
            T maxZ = z2() > r.z2() ? z2() : r.z2();
            m_x = minX;
            m_y = minY;
            m_z = minZ;
            m_width = maxX - minX;
            m_height = maxY - minY;
            m_depth = maxZ - minZ;
            return *this;
        }


        [[nodiscard]] T x() const noexcept { return m_x; }
        [[nodiscard]] T y() const noexcept { return m_y; }
        [[nodiscard]] T z() const noexcept { return m_z; }
        [[nodiscard]] T x2() const noexcept { return m_x + m_width; }
        [[nodiscard]] T y2() const noexcept { return m_y + m_height; }
        [[nodiscard]] T z2() const noexcept { return m_z + m_depth; }
        [[nodiscard]] T mapX(double t) const noexcept { return m_x + m_width * t; }
        [[nodiscard]] T mapY(double t) const noexcept { return m_y + m_height * t; }
        [[nodiscard]] T mapZ(double t) const noexcept { return m_z + m_depth * t; }
        [[nodiscard]] T width() const noexcept { return m_width; }
        [[nodiscard]] T height() const noexcept { return m_height; }
        [[nodiscard]] T depth() const noexcept { return m_depth; }
        [[nodiscard]] T shortSide() const noexcept {
            T min = m_width;
            if (m_height < min) min = m_height;
            if (m_depth < min) min = m_depth;
            return min;
        }
        [[nodiscard]] T longSide() const noexcept {
            T max = m_width;
            if (m_height > max) max = m_height;
            if (m_depth < max) max = m_depth;
            return max;
        }

        [[nodiscard]] bool usable() const noexcept { return m_width > 0 && m_height > 0 && m_depth > 0; }

        [[nodiscard]] T centerX() const noexcept { return m_x + m_width / 2; }
        [[nodiscard]] T centerY() const noexcept { return m_y + m_height / 2; }
        [[nodiscard]] T centerZ() const noexcept { return m_z + m_depth / 2; }
        [[nodiscard]] Vec3<T> center() const noexcept { return Vec3<T>(m_x + m_width / 2, m_y + m_height / 2, m_z + m_depth / 2); }
        [[nodiscard]] Vec3<T> sizeCenter() const noexcept { return Vec3<T>(m_width / 2, m_height / 2, m_depth / 2); }
        [[nodiscard]] T radius() const noexcept { return shortSide() / 2; }
        [[nodiscard]] T circumcircleRadius() {
            return std::sqrt(m_width * m_width + m_height * m_height + m_depth * m_depth) / 2;
        }

        [[nodiscard]] Vec3<T> randomPos() const noexcept {
            return Vec3<T>(m_x + Random::next(m_width), m_y + Random::next(m_height), m_z + Random::next(m_depth));
        }


        void zero() noexcept { m_x = m_y = m_z = m_width = m_height = m_depth = 0; }

        void set(T x, T y, T z, T width, T height, T depth) noexcept {
            m_x = x; m_y = y; m_z = z; m_width = width; m_height = height; m_depth = depth;
        }

        void set(T x, T y, T z, T size) noexcept {
            m_x = x; m_y = y; m_z = z; m_width = m_height = m_depth = size;
        }

        void set(const Cube& r) noexcept {
            m_x = r.m_x; m_y = r.m_y; m_z = r.m_z; m_width = r.m_width; m_height = r.m_height; m_depth = r.m_depth;
        }

        void set(T width, T height, T depth) noexcept {
            m_x = m_y = 0; m_width = width; m_height = height; m_depth = depth;
        }

        void set(const Vec3<T>& center, T radius) noexcept {
            m_x = center.m_x - radius;
            m_y = center.m_y - radius;
            m_z = center.m_z - radius;
            m_width = m_height = m_depth = radius * 2;
        }

        void setWidth(T width) noexcept { m_width = width; }
        void setWidthFromCenter(T width) noexcept { m_x += (m_width - width) * 0.5; m_width = width; }
        void setWidthFromMax(T width) noexcept { m_x += m_width - width; m_width = width; }

        void setHeight(T height) noexcept { m_height = height; }
        void setHeightFromCenter(T height) noexcept { m_y += (m_height - height) * 0.5; m_height = height; }
        void setHeightFromMax(T height) noexcept { m_y += m_height - height; m_height = height; }

        void setDepth(T depth) noexcept { m_depth = depth; }
        void setDepthFromCenter(T depth) noexcept { m_z += (m_depth - depth) * 0.5; m_depth = depth; }
        void setDepthFromMax(T depth) noexcept { m_z += m_depth - depth; m_depth = depth; }

        void setPos(const Vec3<T>& pos) noexcept { m_x = pos.m_x; m_y = pos.m_y; m_z = pos.m_z; }
        void setPos(T x, T y, T z) noexcept { m_x = x; m_y = y; m_z = z; }
        void setPos2(const Vec3<T>& pos) { m_width = pos.m_x - m_x; m_height = pos.m_y - m_y; m_width = pos.m_z - m_z; }
        void setPos2(T x, T y) noexcept { m_width = x - m_x; m_height = y - m_y; }

        void setSize(T size) noexcept { m_width = m_height = m_width = size; }
        void setSize(T width, T height, T depth) noexcept { m_width = width; m_height = height; m_depth = depth; }
        void setSizeFromCenter(T width, T height, T depth) noexcept {
            setWidthFromCenter(width);
            setHeightFromCenter(height);
            setDepthFromCenter(depth);
        }


        void moveLeft() noexcept { m_x -= m_width; }
        void moveRight() noexcept { m_x += m_width; }
        void moveUp() noexcept { m_y -= m_height; }
        void moveDown() noexcept { m_y += m_height; }
        void moveNear() noexcept { m_z -= m_depth; }
        void moveFar() noexcept { m_z += m_depth; }

        void inset(T size) noexcept {
            m_x += size;
            m_y += size;
            m_z += size;
            m_width -= size * 2;
            m_height -= size * 2;
            m_depth -= size * 2;
        }

        void inset(T top, T right, T bottom, T left, T near, T far) noexcept {
            m_x += left;
            m_y += top;
            m_y += near;
            m_width -= left + right;
            m_height -= top + bottom;
            m_depth -= near + far;
        }

        void insetLeft(T size) noexcept { m_x += size; m_width -= size; }
        void insetRight(T size) noexcept { m_width -= size; }
        void insetTop(T size) noexcept { m_y += size; m_height -= size; }
        void insetBottom(T size) noexcept { m_height -= size; }
        void insetNear(T size) noexcept { m_z += size; m_depth -= size; }
        void insetFar(T size) noexcept { m_depth -= size; }

        void insetFromCenter(T x_size, T y_size, T z_size) noexcept {
            m_x += x_size; m_width -= x_size * 2;
            m_y += y_size; m_height -= y_size * 2;
            m_z += z_size; m_depth -= z_size * 2;
        }

        void insetHorizontalFromCenter(T size) noexcept { m_x += size; m_width -= size * 2; }
        void insetVerticalFromCenter(T size) noexcept { m_y += size; m_height -= size * 2; }
        void insetDepthFromCenter(T size) noexcept { m_z += size; m_depth -= size * 2; }

        void expand(T size) noexcept {
            m_x -= size;
            m_y -= size;
            m_z -= size;
            m_width += size * 2;
            m_height += size * 2;
            m_depth += size * 2;
        }

        void roundValues() noexcept {
            m_x = std::round(m_x);
            m_y = std::round(m_y);
            m_z = std::round(m_y);
            m_width = std::round(m_width);
            m_height = std::round(m_height);
            m_depth = std::round(m_depth);
        }

        void translateX(T tx) noexcept { m_x += tx; }
        void translateY(T ty) noexcept { m_y += ty; }
        void translateZ(T tz) noexcept { m_z += tz; }
        void translate(T tx, T ty, T tz) noexcept { m_x += tx; m_y += ty; m_z += tz; }
        void translate(Vec3<T> t) noexcept { m_x += t.m_x; m_y += t.m_y; m_z += t.m_z; }

        void scale(T scale) noexcept { m_width *= scale; m_height *= scale; m_depth *= scale; }
        void scaleWidth(T scale) noexcept { m_width *= scale; }
        void scaleHeight(T scale) noexcept { m_height *= scale; }
        void scaleDepth(T scale) noexcept { m_depth *= scale; }

        void scaleCentered(T scale) noexcept {
            double new_width = m_width * scale;
            double new_height = m_height * scale;
            double new_depth = m_depth * scale;
            m_x -= (new_width - m_width) / 2;
            m_y -= (new_height - m_height) / 2;
            m_z -= (new_depth - m_depth) / 2;
            m_width = new_width;
            m_height = new_height;
            m_depth = new_depth;
        }

        void makePositiveSize() noexcept {
            if (m_width < 0) {
                m_x += m_width;
                m_width = -m_width;
            }
            if (m_height < 0) {
                m_y += m_height;
                m_height = -m_height;
            }
            if (m_depth < 0) {
                m_z += m_depth;
                m_depth = -m_depth;
            }
        }

        void avoidNegativeSize() noexcept {
            if (m_width < 0) m_width = 0;
            if (m_height < 0) m_height = 0;
            if (m_depth < 0) m_depth = 0;
        }

        /**
         *  @brief Checks if Cube contains a given position.
         *
         *  This function determines whether the specified position is within the boundaries
         *  of the cube represented by this object.
         *
         *  @param pos The position to check for containment within the rectangle.
         *  @return true if the specified position is contained within the cube, false otherwise.
         */
        [[nodiscard]] bool contains(const Vec3<T>& pos) const noexcept {
            return (pos.m_x >= m_x && pos.m_x < m_x + m_width &&
                    pos.m_y >= m_y && pos.m_y < m_y + m_height &&
                    pos.m_z >= m_z && pos.m_z < m_z + m_height );
        }

        void clipVec3(Vec3<T>& v) const noexcept {
            T x2 = this->x2();
            T y2 = this->y2();
            T z2 = this->z2();
            if (v.m_x < m_x) { v.m_x = m_x; } else if (v.m_x > x2) { v.m_x = x2; }
            if (v.m_y < m_y) { v.m_y = m_y; } else if (v.m_y > y2) { v.m_y = y2; }
            if (v.m_z < m_z) { v.m_z = m_z; } else if (v.m_z > z2) { v.m_z = z2; }
        }

        /**
         *  @brief Checks if Cube overlaps with another cube.
         *
         *  This function determines whether the cube represented by this object
         *  overlaps with the specified cube.
         *
         *  @param cube The cube to check for overlap with.
         *  @return true if this cube overlaps with the specified cube, false otherwise.
         */
        [[nodiscard]] bool overlaps(const Cube& cube) noexcept {
            T x2 = this->x2();
            T y2 = this->y2();
            T z2 = this->z2();
            T cx2 = cube.x2();
            T cy2 = cube.y2();
            T cz2 = cube.z2();

            if (cube.m_x > m_x) { m_x = cube.m_x; }
            if (cx2 < x2) { x2 = cx2; }
            if (cube.m_y > m_y) { m_y = cube.m_y; }
            if (cy2 < y2) { y2 = cy2; }
            if (cube.m_z > m_z) { m_z = cube.m_z; }
            if (cz2 < z2) { z2 = cz2; }

            m_width = x2 - m_x;
            m_height = y2 - m_y;
            m_depth = z2 - m_z;

            return m_width > 0 && m_height > 0 && m_depth > 0;
        }
    };


    // Standard types
    using Cubei = Cube<int32_t>;    ///< 32 bit integer
    using Cubel = Cube<int64_t>;    ///< 64 bit integer
    using Cubef = Cube<float>;      ///< 32 bit floating point
    using Cubed = Cube<double>;     ///< 64 bit floating point


} // End of namespace Grain

#endif // GrainCube_hpp
