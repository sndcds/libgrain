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
    T min_x_{};
    T max_x_{};
    T min_y_{};
    T max_y_{};
    T min_z_{};
    T max_z_{};

public:
    RangeCube() noexcept = default;
    RangeCube(T min_x, T max_x, T min_y, T max_y, T min_z, T max_z) noexcept :
        min_x_(min_x), max_x_(max_x),
        min_y_(min_y), max_y_(max_y),
        min_z_(min_z), max_z_(max_z) {
    }

    virtual ~RangeCube() noexcept = default;

    [[nodiscard]] virtual const char* className() const noexcept {
        return "RangeCube";
    }


    friend std::ostream& operator << (std::ostream& os, const RangeCube& o) {
        if (std::is_same<T, int8_t>::value || std::is_same<T, uint8_t>::value) {
            // Force to print integer values instead of chars.
            os << static_cast<int32_t>(o.min_x_) << ", " << static_cast<int32_t>(o.max_x_) << " | ";
            os << static_cast<int32_t>(o.min_y_) << ", " << static_cast<int32_t>(o.max_y_) << " | ";
            os << static_cast<int32_t>(o.min_z_) << ", " << static_cast<int32_t>(o.max_z_);
        }
        else {
            os << std::setprecision(10);
            os << o.min_x_ << ", " << o.max_x_ << " | ";
            os << o.min_y_ << ", " << o.max_y_ << " | ";
            os << o.min_z_ << ", " << o.max_z_;
            os << std::defaultfloat;  // Back to standard
        }
        return os;
    }

    RangeCube& operator = (const Vec3<T>& v) {
        min_x_ = max_x_ = v.m_x;
        min_y_ = max_y_ = v.m_y;
        min_z_ = max_z_ = v.m_z;
        return *this;
    }

    RangeCube& operator = (const Cube<T>& r) {
        min_x_ = r.width_ > 0 ? r.m_x : r.m_x + r.width_;
        max_x_ = r.width_ > 0 ? r.m_x + r.width_ : r.m_x;
        min_y_ = r.height_ > 0 ? r.m_y : r.m_y + r.height_;
        max_y_ = r.height_ > 0 ? r.m_y + r.height_ : r.m_y;
        min_z_ = r.m_depth > 0 ? r.m_z : r.m_z + r.m_depth;
        max_z_ = r.m_depth > 0 ? r.m_z + r.m_depth : r.m_z;
        return *this;
    }

    bool operator == (const RangeCube& v) const {
        return min_x_ == v.min_x_ && max_x_ == v.max_x_ && min_y_ == v.min_y_ && max_y_ == v.max_y_ && min_z_ == v.min_z_ && max_z_ == v.max_z_;
    }

    bool operator != (const RangeCube& v) const {
        return min_x_ != v.min_x_ || max_x_ != v.max_x_ || min_y_ != v.min_y_ || max_y_ != v.max_y_ || min_z_ != v.min_z_ || max_z_ != v.max_z_;
    }

    RangeCube operator + (const RangeCube<T>& r) const {
        RangeCube result;
        result.min_x_ = min_x_ < r.min_x_ ? min_x_ : r.min_x_;
        result.min_y_ = min_y_ < r.min_y_ ? min_y_ : r.min_y_;
        result.min_z_ = min_z_ < r.min_z_ ? min_z_ : r.min_z_;
        result.max_x_ = max_x_ > r.max_x_ ? max_x_ : r.max_x_;
        result.max_y_ = max_y_ > r.max_y_ ? max_y_ : r.max_y_;
        result.max_z_ = max_z_ > r.max_z_ ? max_z_ : r.max_z_;
        return result;
    }

    RangeCube operator + (const Vec3<T>& v) const {
        RangeCube result = *this;
        if (v.m_x < min_x_) { result.min_x_ = v.m_x; }
        if (v.m_x > max_x_) { result.max_x_ = v.m_x; }
        if (v.m_y < min_y_) { result.min_y_ = v.m_y; }
        if (v.m_y > max_y_) { result.max_y_ = v.m_y; }
        if (v.m_z < min_z_) { result.min_z_ = v.m_z; }
        if (v.m_z > max_z_) { result.max_z_ = v.m_z; }
        return result;
    }

    RangeCube operator + (const Cube<T>& r) const {
        RangeCube result;
        T r_min_x = r.width_ > 0 ? r.m_x : r.m_x + r.width_;
        T r_max_x = r.width_ > 0 ? r.m_x + r.width_ : r.m_x;
        T r_min_y = r.height_ > 0 ? r.m_y : r.m_y + r.height_;
        T r_max_y = r.height_ > 0 ? r.m_y + r.height_ : r.m_y;
        T r_min_z = r.m_depth > 0 ? r.m_z : r.m_z + r.m_depth;
        T r_max_z = r.m_depth > 0 ? r.m_z + r.m_depth : r.m_z;
        result.min_x_ = min_x_ < r_min_x ? min_x_ : r_min_x;
        result.min_y_ = min_y_ < r_min_y ? min_y_ : r_min_y;
        result.min_z_ = min_z_ < r_min_z ? min_z_ : r_min_z;
        result.max_x_ = max_x_ > r_max_x ? max_x_ : r_max_x;
        result.max_y_ = max_y_ > r_max_y ? max_y_ : r_max_y;
        result.max_z_ = max_z_ > r_max_z ? max_z_ : r_max_z;
        return result;
    }

    RangeCube& operator += (const RangeCube<T>& r) {
        min_x_ = min_x_ < r.min_x_ ? min_x_ : r.min_x_;
        min_y_ = min_y_ < r.min_y_ ? min_y_ : r.min_y_;
        min_z_ = min_z_ < r.min_z_ ? min_z_ : r.min_z_;
        max_x_ = max_x_ > r.max_x_ ? max_x_ : r.max_x_;
        max_y_ = max_y_ > r.max_y_ ? max_y_ : r.max_y_;
        max_z_ = max_z_ > r.max_z_ ? max_z_ : r.max_z_;
        return *this;
    }

    RangeCube& operator += (const Vec3<T>& v) {
        if (v.m_x < min_x_) { min_x_ = v.m_x; }
        if (v.m_x > max_x_) { max_x_ = v.m_x; }
        if (v.m_y < min_y_) { min_y_ = v.m_y; }
        if (v.m_y > max_y_) { max_y_ = v.m_y; }
        if (v.m_z < min_z_) { min_z_ = v.m_z; }
        if (v.m_z > max_z_) { max_z_ = v.m_z; }
        return *this;
    }

    RangeCube& operator += (const Cube<T>& r) {
        T r_min_x = r.width_ > 0 ? r.m_x : r.m_x + r.width_;
        T r_max_x = r.width_ > 0 ? r.m_x + r.width_ : r.m_x;
        T r_min_y = r.height_ > 0 ? r.m_y : r.m_y + r.height_;
        T r_max_y = r.height_ > 0 ? r.m_y + r.height_ : r.m_y;
        T r_min_z = r.m_depth > 0 ? r.m_z : r.m_z + r.m_depth;
        T r_max_z = r.m_depth > 0 ? r.m_z + r.m_depth : r.m_z;
        min_x_ = min_x_ < r_min_x ? min_x_ : r_min_x;
        min_y_ = min_y_ < r_min_y ? min_y_ : r_min_y;
        min_z_ = min_z_ < r_min_z ? min_z_ : r_min_z;
        max_x_ = max_x_ > r_max_x ? max_x_ : r_max_x;
        max_y_ = max_y_ > r_max_y ? max_y_ : r_max_y;
        max_z_ = max_z_ > r_max_z ? max_z_ : r_max_z;
        return *this;
    }

    T minX() const noexcept { return min_x_; }
    T maxX() const noexcept { return max_x_; }
    T minY() const noexcept { return min_y_; }
    T maxY() const noexcept { return max_y_; }
    T minZ() const noexcept { return min_z_; }
    T maxZ() const noexcept { return max_z_; }
    T centerX() const noexcept { return min_x_ + (max_x_ - min_x_) / 2; }
    T centerY() const noexcept { return min_y_ + (max_y_ - min_y_) / 2; }
    T centerZ() const noexcept { return min_z_ + (max_z_ - min_z_) / 2; }
    T width() const noexcept { return std::fabs(max_x_ - min_x_); }
    T height() const noexcept { return std::fabs(max_y_ - min_y_); }
    T depth() const noexcept { return std::fabs(max_z_ - min_z_); }

    Cube<T> toCube() const noexcept {
        return Cube<T>(min_x_, min_y_, min_z_, max_x_ - min_x_, max_y_ - min_y_, max_z_ - min_z_);
    }

    Rect<T> toRectXY() const noexcept {
        return Rect<T>(min_x_, min_y_, max_x_ - min_x_, max_y_ - min_y_);
    }

    Rect<T> toRectXZ() const noexcept {
        return Rect<T>(min_x_, min_z_, max_x_ - min_x_, max_z_ - min_z_);
    }

    Rect<T> toRectYZ() const noexcept {
        return Rect<T>(min_y_, min_z_, max_y_ - min_y_, max_y_ - min_y_);
    }


    void set(T x, T y, T z) noexcept { min_x_ = max_x_ = x; min_y_ = max_y_ = y; min_z_ = max_z_ = z; }
    void set(Vec3<T>& v) noexcept { min_x_ = max_x_ = v.m_x; min_y_ = max_y_ = v.m_y; min_z_ = max_z_ = v.m_z; }
    void set(Vec3<T>* v) noexcept { if (v) { min_x_ = max_x_ = v->m_x; min_y_ = max_y_ = v->m_y; min_z_ = max_z_ = v->m_z; } }

    void set(T min_x, T max_x, T min_y, T max_y, T min_z, T max_z) noexcept {
        min_x_ = min_x; max_x_ = max_x;
        min_y_ = min_y; max_y_ = max_y;
        min_z_ = min_z; max_z_ = max_z;
    }

    bool add(const Vec3<T>& v) noexcept {
        bool result = false;
        if (v.m_x < min_x_) { min_x_ = v.m_x; result = true; }
        if (v.m_x > max_x_) { max_x_ = v.m_x; result = true; }
        if (v.m_y < min_y_) { min_y_ = v.m_y; result = true; }
        if (v.m_y > max_y_) { max_y_ = v.m_y; result = true; }
        if (v.m_z < min_z_) { min_z_ = v.m_z; result = true; }
        if (v.m_z > max_z_) { max_z_ = v.m_z; result = true; }
        return result;
    }

    bool add(const Vec3<T>* v) noexcept { if (v) { return add(*v); } return false; }

    bool addX(T x) noexcept {
        bool result = false;
        if (x < min_x_) { min_x_ = x; result = true; }
        if (x > max_x_) { max_x_ = x; result = true; }
        return result;
    }

    bool addY(T y) noexcept {
        bool result = false;
        if (y < min_y_) { min_y_ = y; result = true; }
        if (y > max_y_) { max_y_ = y; result = true; }
        return result;
    }

    bool addZ(T z) noexcept {
        bool result = false;
        if (z < min_z_) { min_z_ = z; result = true; }
        if (z > max_z_) { max_z_ = z; result = true; }
        return result;
    }

    bool add(T x, T y, T z) noexcept {
        bool result = false;
        if (x < min_x_) { min_x_ = x; result = true; }
        if (x > max_x_) { max_x_ = x; result = true; }
        if (y < min_y_) { min_y_ = y; result = true; }
        if (y > max_y_) { max_y_ = y; result = true; }
        if (z < min_z_) { min_z_ = z; result = true; }
        if (z > max_z_) { max_z_ = z; result = true; }
        return result;
    }

    void add(const Cube<T>& cube) noexcept {
        if (cube.m_x < min_x_) min_x_ = cube.m_x;
        if (cube.x2() > max_x_) max_x_ = cube.x2();
        if (cube.m_y < min_y_) min_y_ = cube.m_y;
        if (cube.y2() > max_y_) max_y_ = cube.y2();
        if (cube.m_z < min_z_) min_z_ = cube.m_z;
        if (cube.z2() > max_z_) max_z_ = cube.z2();
    }

    void add(const RangeCube<T>& r) noexcept {
        *this += r;
    }

    void limit(const RangeCube& max_cube) noexcept {
        if (min_x_ < max_cube.min_x_) min_x_ = max_cube.min_x_;
        if (max_x_ > max_cube.max_x_) max_x_ = max_cube.max_x_;
        if (min_y_ < max_cube.min_y_) min_y_ = max_cube.min_y_;
        if (max_y_ > max_cube.max_y_) max_y_ = max_cube.max_y_;
    }

    RangeCube<T> lerp(const RangeCube<T> r, double t) noexcept {
        RangeCube<T> result;
        result.min_x_ = min_x_ + t * (r.min_x_ - min_x_);
        result.min_y_ = min_y_ + t * (r.min_y_ - min_y_);
        result.min_z_ = min_z_ + t * (r.min_z_ - min_z_);
        result.max_x_ = max_x_ + t * (r.max_x_ - max_x_);
        result.max_y_ = max_y_ + t * (r.max_y_ - max_y_);
        result.max_z_ = max_z_ + t * (r.max_z_ - max_z_);
        return result;
    }

    static RangeCube<T> lerp(const RangeCube<T> a, const RangeCube<T> b, double t) noexcept {
        RangeCube<T> result;
        result.min_x_ = a.min_x_ + t * (b.min_x_ - a.min_x_);
        result.min_y_ = a.min_y_ + t * (b.min_y_ - a.min_y_);
        result.min_z_ = a.min_z_ + t * (b.min_z_ - a.min_z_);
        result.max_x_ = a.max_x_ + t * (b.max_x_ - a.max_x_);
        result.max_y_ = a.max_y_ + t * (b.max_y_ - a.max_y_);
        result.max_z_ = a.max_z_ + t * (b.max_z_ - a.max_z_);
        return result;
    }
};


// Standard types
using RangeCubei = RangeCube<int32_t>;  ///< 32 bit integer
using RangeCubel = RangeCube<int64_t>;  ///< 64 bit integer
using RangeCubef = RangeCube<float>;    ///< 32 bit floating point
using RangeCubed = RangeCube<double>;   ///< 64 bit floating point


class RangeCubeFix {

public:
    Fix min_x_{};
    Fix max_x_{};
    Fix min_y_{};
    Fix max_y_{};
    Fix min_z_{};
    Fix max_z_{};

public:
    RangeCubeFix() noexcept = default;
    RangeCubeFix(
        const Fix& min_x, const Fix& max_x,
        const Fix& min_y, const Fix& max_y,
        const Fix& min_z, const Fix& max_z) noexcept :
            min_x_(min_x), max_x_(max_x),
            min_y_(min_y), max_y_(max_y),
            min_z_(min_z), max_z_(max_z) {
    }

    virtual ~RangeCubeFix() noexcept = default;

    [[nodiscard]] virtual const char* className() const noexcept {
        return "RangeCubeFix";
    }

    friend std::ostream& operator << (std::ostream& os, const RangeCubeFix& o) {
        os << o.min_x_ << ", " << o.max_x_ << " | ";
        os << o.min_y_ << ", " << o.max_y_ << " | ";
        os << o.min_z_ << ", " << o.max_z_;
        return os;
    }

    RangeCubeFix& operator = (const Vec3Fix& v);
    RangeCubeFix operator + (const Vec3Fix& v) const;
    RangeCubeFix& operator += (const Vec3Fix& v);

    [[nodiscard]] Fix minX() const noexcept { return min_x_; }
    [[nodiscard]] Fix maxX() const noexcept { return max_x_; }
    [[nodiscard]] Fix minY() const noexcept { return min_y_; }
    [[nodiscard]] Fix maxY() const noexcept { return max_y_; }
    [[nodiscard]] Fix minZ() const noexcept { return min_z_; }
    [[nodiscard]] Fix maxZ() const noexcept { return max_z_; }

    [[nodiscard]] Fix width() const noexcept { return max_x_ > min_x_ ? max_x_ - min_x_ : min_x_ - max_x_; }
    [[nodiscard]] Fix height() const noexcept { return max_y_ > min_y_ ? max_y_ - min_y_ : min_y_ - max_y_; }
    [[nodiscard]] Fix depth() const noexcept { return max_z_ > min_z_ ? max_z_ - min_z_ : min_z_ - max_z_; }

    void initForMinMaxSearch() noexcept {
        min_x_.setToMax(); min_y_.setToMax(); min_z_.setToMax();
        max_x_.setToMin(); max_y_.setToMin(); max_z_.setToMin();
    }

    void add(const Vec3Fix& v) noexcept {
        addX(v.x_); addY(v.y_); addZ(v.z_);
    }

    void add(const Vec3Fix* v) noexcept {
        if (v != nullptr) {
            addX(v->x_); addY(v->y_); addZ(v->z_);
        }
    }

    void  addX(const Fix& x) noexcept {
        if (x < min_x_) { min_x_ = x; }
        if (x > max_x_) { max_x_ = x; }
    }

    void addY(const Fix& y) noexcept {
        if (y < min_y_) { min_y_ = y; }
        if (y > max_y_) { max_y_ = y; }
    }

    void addZ(const Fix& z) noexcept {
        if (z < min_z_) { min_z_ = z; }
        if (z > max_z_) { max_z_ = z; }
    }

    void add(const Fix& x, const Fix& y, const Fix& z) noexcept {
        addX(x); addY(y); addZ(z);
    }
};


} // End of namespace Grain

#endif // GrainRangeCube_hpp
