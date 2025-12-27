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
    T x_ = 0;
    T y_ = 0;
    T z_ = 0;
    T width_ = 1;
    T height_ = 1;
    T depth_ = 1;

public:
    Cube() noexcept {}
    Cube(T size) noexcept : width_(size), height_(size) {}
    Cube(T width, T height, T depth) noexcept : width_(width), height_(height), depth_(depth) {}
    Cube(T x, T y, T z, T width, T height, T depth) noexcept : x_(x), y_(y), z_(z), width_(width), height_(height), depth_(depth) {}
    Cube(const Cube<int32_t>& r) noexcept { x_ = r.x_; y_ = r.y_; z_ = r.z_; width_ = r.width_; height_ = r.height_; depth_ = r.depth_; }
    Cube(const Cube<int64_t>& r) noexcept { x_ = r.x_; y_ = r.y_; z_ = r.z_; width_ = r.width_; height_ = r.height_; depth_ = r.depth_; }
    Cube(const Cube<float>& r) noexcept { x_ = r.x_; y_ = r.y_; z_ = r.z_; width_ = r.width_; height_ = r.height_; depth_ = r.depth_; }
    Cube(const Cube<double>& r) noexcept { x_ = r.x_; y_ = r.y_; z_ = r.z_; width_ = r.width_; height_ = r.height_; depth_ = r.depth_; }
    Cube(const Cube& r, T inset_size) noexcept { *this = r; inset(inset_size); }

    [[nodiscard]] const char* className() const noexcept { return "Cube"; }

    Cube& operator = (const Cube<int32_t>& r) {
        x_ = r.x_; y_ = r.y_; z_ = r.z_; width_ = r.width_; height_ = r.height_; depth_ = r.depth_; return *this;
    }

    Cube& operator = (const Cube<int64_t>& r) {
        x_ = r.x_; y_ = r.y_; z_ = r.z_; width_ = r.width_; height_ = r.height_; depth_ = r.depth_; return *this;
    }

    Cube& operator = (const Cube<float>& r) {
        x_ = r.x_; y_ = r.y_; z_ = r.z_; width_ = r.width_; height_ = r.height_; depth_ = r.depth_; return *this;
    }

    Cube& operator = (const Cube<double>& r) {
        x_ = r.x_; y_ = r.y_; z_ = r.z_; width_ = r.width_; height_ = r.height_; depth_ = r.depth_; return *this;
    }

    bool operator == (const Cube& r) const {
        return x_ == r.x_ && y_ == r.y_ && z_ == r.z_ && width_ == r.width_ && height_ == r.height_ && depth_ == r.depth_;
    }

    bool operator != (const Cube& r) const {
        return x_ != r.x_ || y_ != r.y_ || z_ != r.z_ || width_ != r.width_ || height_ != r.height_ || depth_ != r.depth_;
    }

    Cube operator + (const Vec3<T>& v) const {
        return Cube(x_ + v.x_, y_ + v.y_, z_ + v.z_, width_, height_, depth_);
    }

    Cube operator - (const Vec3<T>& v) const {
        return Cube(x_ - v.x_, y_ - v.y_, z_ - v.z_, width_, height_, depth_);
    }

    Cube operator * (const Vec3<T>& v) const {
        return Cube(x_ * v.x_, y_ * v.y_, z_ * v.z_, width_, height_, depth_);
    }

    Cube& operator += (const Vec3<T>& v) { x_ += v.x_; y_ += v.y_; z_ += v.z_; return *this; }
    Cube& operator -= (const Vec3<T>& v) { x_ -= v.x_; y_ -= v.y_; z_ -= v.z_; return *this; }
    Cube& operator *= (const Vec3<T>& v) { x_ *= v.x_; y_ *= v.y_; z_ *= v.z_; return *this; }

    Cube& operator += (const Cube<T>& r) {
        Cube<T> result;
        T minX = x_ < r.x_ ? x_ : r.x_;
        T minY = y_ < r.y_ ? y_ : r.y_;
        T minZ = z_ < r.z_ ? z_ : r.z_;
        T maxX = x2() > r.x2() ? x2() : r.x2();
        T maxY = y2() > r.y2() ? y2() : r.y2();
        T maxZ = z2() > r.z2() ? z2() : r.z2();
        x_ = minX;
        y_ = minY;
        z_ = minZ;
        width_ = maxX - minX;
        height_ = maxY - minY;
        depth_ = maxZ - minZ;
        return *this;
    }


    [[nodiscard]] T x() const noexcept { return x_; }
    [[nodiscard]] T y() const noexcept { return y_; }
    [[nodiscard]] T z() const noexcept { return z_; }
    [[nodiscard]] T x2() const noexcept { return x_ + width_; }
    [[nodiscard]] T y2() const noexcept { return y_ + height_; }
    [[nodiscard]] T z2() const noexcept { return z_ + depth_; }
    [[nodiscard]] T mapX(double t) const noexcept { return x_ + width_ * t; }
    [[nodiscard]] T mapY(double t) const noexcept { return y_ + height_ * t; }
    [[nodiscard]] T mapZ(double t) const noexcept { return z_ + depth_ * t; }
    [[nodiscard]] T width() const noexcept { return width_; }
    [[nodiscard]] T height() const noexcept { return height_; }
    [[nodiscard]] T depth() const noexcept { return depth_; }
    [[nodiscard]] T shortSide() const noexcept {
        T min = width_;
        if (height_ < min) min = height_;
        if (depth_ < min) min = depth_;
        return min;
    }
    [[nodiscard]] T longSide() const noexcept {
        T max = width_;
        if (height_ > max) max = height_;
        if (depth_ < max) max = depth_;
        return max;
    }

    [[nodiscard]] bool usable() const noexcept { return width_ > 0 && height_ > 0 && depth_ > 0; }

    [[nodiscard]] T centerX() const noexcept { return x_ + width_ / 2; }
    [[nodiscard]] T centerY() const noexcept { return y_ + height_ / 2; }
    [[nodiscard]] T centerZ() const noexcept { return z_ + depth_ / 2; }
    [[nodiscard]] Vec3<T> center() const noexcept { return Vec3<T>(x_ + width_ / 2, y_ + height_ / 2, z_ + depth_ / 2); }
    [[nodiscard]] Vec3<T> sizeCenter() const noexcept { return Vec3<T>(width_ / 2, height_ / 2, depth_ / 2); }
    [[nodiscard]] T radius() const noexcept { return shortSide() / 2; }
    [[nodiscard]] T circumcircleRadius() {
        return std::sqrt(width_ * width_ + height_ * height_ + depth_ * depth_) / 2;
    }

    [[nodiscard]] Vec3<T> randomPos() const noexcept {
        return Vec3<T>(x_ + Random::next(width_), y_ + Random::next(height_), z_ + Random::next(depth_));
    }


    void zero() noexcept { x_ = y_ = z_ = width_ = height_ = depth_ = 0; }

    void set(T x, T y, T z, T width, T height, T depth) noexcept {
        x_ = x; y_ = y; z_ = z; width_ = width; height_ = height; depth_ = depth;
    }

    void set(T x, T y, T z, T size) noexcept {
        x_ = x; y_ = y; z_ = z; width_ = height_ = depth_ = size;
    }

    void set(const Cube& r) noexcept {
        x_ = r.x_; y_ = r.y_; z_ = r.z_; width_ = r.width_; height_ = r.height_; depth_ = r.depth_;
    }

    void set(T width, T height, T depth) noexcept {
        x_ = y_ = 0; width_ = width; height_ = height; depth_ = depth;
    }

    void set(const Vec3<T>& center, T radius) noexcept {
        x_ = center.x_ - radius;
        y_ = center.y_ - radius;
        z_ = center.z_ - radius;
        width_ = height_ = depth_ = radius * 2;
    }

    void setWidth(T width) noexcept { width_ = width; }
    void setWidthFromCenter(T width) noexcept { x_ += (width_ - width) * 0.5; width_ = width; }
    void setWidthFromMax(T width) noexcept { x_ += width_ - width; width_ = width; }

    void setHeight(T height) noexcept { height_ = height; }
    void setHeightFromCenter(T height) noexcept { y_ += (height_ - height) * 0.5; height_ = height; }
    void setHeightFromMax(T height) noexcept { y_ += height_ - height; height_ = height; }

    void setDepth(T depth) noexcept { depth_ = depth; }
    void setDepthFromCenter(T depth) noexcept { z_ += (depth_ - depth) * 0.5; depth_ = depth; }
    void setDepthFromMax(T depth) noexcept { z_ += depth_ - depth; depth_ = depth; }

    void setPos(const Vec3<T>& pos) noexcept { x_ = pos.x_; y_ = pos.y_; z_ = pos.z_; }
    void setPos(T x, T y, T z) noexcept { x_ = x; y_ = y; z_ = z; }
    void setPos2(const Vec3<T>& pos) { width_ = pos.x_ - x_; height_ = pos.y_ - y_; width_ = pos.z_ - z_; }
    void setPos2(T x, T y) noexcept { width_ = x - x_; height_ = y - y_; }

    void setSize(T size) noexcept { width_ = height_ = width_ = size; }
    void setSize(T width, T height, T depth) noexcept { width_ = width; height_ = height; depth_ = depth; }
    void setSizeFromCenter(T width, T height, T depth) noexcept {
        setWidthFromCenter(width);
        setHeightFromCenter(height);
        setDepthFromCenter(depth);
    }


    void moveLeft() noexcept { x_ -= width_; }
    void moveRight() noexcept { x_ += width_; }
    void moveUp() noexcept { y_ -= height_; }
    void moveDown() noexcept { y_ += height_; }
    void moveNear() noexcept { z_ -= depth_; }
    void moveFar() noexcept { z_ += depth_; }

    void inset(T size) noexcept {
        x_ += size;
        y_ += size;
        z_ += size;
        width_ -= size * 2;
        height_ -= size * 2;
        depth_ -= size * 2;
    }

    void inset(T top, T right, T bottom, T left, T near, T far) noexcept {
        x_ += left;
        y_ += top;
        y_ += near;
        width_ -= left + right;
        height_ -= top + bottom;
        depth_ -= near + far;
    }

    void insetLeft(T size) noexcept { x_ += size; width_ -= size; }
    void insetRight(T size) noexcept { width_ -= size; }
    void insetTop(T size) noexcept { y_ += size; height_ -= size; }
    void insetBottom(T size) noexcept { height_ -= size; }
    void insetNear(T size) noexcept { z_ += size; depth_ -= size; }
    void insetFar(T size) noexcept { depth_ -= size; }

    void insetFromCenter(T x_size, T y_size, T z_size) noexcept {
        x_ += x_size; width_ -= x_size * 2;
        y_ += y_size; height_ -= y_size * 2;
        z_ += z_size; depth_ -= z_size * 2;
    }

    void insetHorizontalFromCenter(T size) noexcept { x_ += size; width_ -= size * 2; }
    void insetVerticalFromCenter(T size) noexcept { y_ += size; height_ -= size * 2; }
    void insetDepthFromCenter(T size) noexcept { z_ += size; depth_ -= size * 2; }

    void expand(T size) noexcept {
        x_ -= size;
        y_ -= size;
        z_ -= size;
        width_ += size * 2;
        height_ += size * 2;
        depth_ += size * 2;
    }

    void roundValues() noexcept {
        x_ = std::round(x_);
        y_ = std::round(y_);
        z_ = std::round(y_);
        width_ = std::round(width_);
        height_ = std::round(height_);
        depth_ = std::round(depth_);
    }

    void translateX(T tx) noexcept { x_ += tx; }
    void translateY(T ty) noexcept { y_ += ty; }
    void translateZ(T tz) noexcept { z_ += tz; }
    void translate(T tx, T ty, T tz) noexcept { x_ += tx; y_ += ty; z_ += tz; }
    void translate(Vec3<T> t) noexcept { x_ += t.x_; y_ += t.y_; z_ += t.z_; }

    void scale(T scale) noexcept { width_ *= scale; height_ *= scale; depth_ *= scale; }
    void scaleWidth(T scale) noexcept { width_ *= scale; }
    void scaleHeight(T scale) noexcept { height_ *= scale; }
    void scaleDepth(T scale) noexcept { depth_ *= scale; }

    void scaleCentered(T scale) noexcept {
        double new_width = width_ * scale;
        double new_height = height_ * scale;
        double new_depth = depth_ * scale;
        x_ -= (new_width - width_) / 2;
        y_ -= (new_height - height_) / 2;
        z_ -= (new_depth - depth_) / 2;
        width_ = new_width;
        height_ = new_height;
        depth_ = new_depth;
    }

    void makePositiveSize() noexcept {
        if (width_ < 0) {
            x_ += width_;
            width_ = -width_;
        }
        if (height_ < 0) {
            y_ += height_;
            height_ = -height_;
        }
        if (depth_ < 0) {
            z_ += depth_;
            depth_ = -depth_;
        }
    }

    void avoidNegativeSize() noexcept {
        if (width_ < 0) width_ = 0;
        if (height_ < 0) height_ = 0;
        if (depth_ < 0) depth_ = 0;
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
        return (pos.x_ >= x_ && pos.x_ < x_ + width_ &&
                pos.y_ >= y_ && pos.y_ < y_ + height_ &&
                pos.z_ >= z_ && pos.z_ < z_ + height_ );
    }

    void clipVec3(Vec3<T>& v) const noexcept {
        T x2 = this->x2();
        T y2 = this->y2();
        T z2 = this->z2();
        if (v.x_ < x_) { v.x_ = x_; } else if (v.x_ > x2) { v.x_ = x2; }
        if (v.y_ < y_) { v.y_ = y_; } else if (v.y_ > y2) { v.y_ = y2; }
        if (v.z_ < z_) { v.z_ = z_; } else if (v.z_ > z2) { v.z_ = z2; }
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

        if (cube.x_ > x_) { x_ = cube.x_; }
        if (cx2 < x2) { x2 = cx2; }
        if (cube.y_ > y_) { y_ = cube.y_; }
        if (cy2 < y2) { y2 = cy2; }
        if (cube.z_ > z_) { z_ = cube.z_; }
        if (cz2 < z2) { z2 = cz2; }

        width_ = x2 - x_;
        height_ = y2 - y_;
        depth_ = z2 - z_;

        return width_ > 0 && height_ > 0 && depth_ > 0;
    }
};


// Standard types
using Cubei = Cube<int32_t>;    ///< 32 bit integer
using Cubel = Cube<int64_t>;    ///< 64 bit integer
using Cubef = Cube<float>;      ///< 32 bit floating point
using Cubed = Cube<double>;     ///< 64 bit floating point


} // End of namespace Grain

#endif // GrainCube_hpp
