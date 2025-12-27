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
    Fix x_{};
    Fix y_{};
    Fix z_{};

public:
    Vec3Fix() noexcept = default;
    Vec3Fix(const Fix& x, const Fix& y, const Fix& z) noexcept {
        x_ = x; y_ = y;  z_ = z;
    }

    virtual ~Vec3Fix() = default;

    [[nodiscard]] virtual const char* className() const noexcept { return "Vec3Fix"; }

    friend std::ostream& operator << (std::ostream& os, const Vec3Fix& o) {
        os << o.x_ << ", " << o.y_ << ", " << o.z_;
        return os;
    }


    bool operator == (const Vec3Fix& other) const {
        return other.x_ == x_ && other.y_ == y_ && other.z_ == z_;
    }

    bool operator != (const Vec3Fix& other) const {
        return other.x_ != x_ || other.y_ != y_ || other.z_ != z_;
    }

    Vec3Fix operator + (const Vec3Fix& other) const {
        return { x_ + other.x_, y_ + other.y_, z_ + other.z_ };
    }

    Vec3Fix operator - (const Vec3Fix& other) const {
        return { x_ - other.x_, y_ - other.y_, z_ - other.z_ };
    }

    Vec3Fix operator * (const Vec3Fix& other) const {
        return { x_ * other.x_, y_ * other.y_, z_ * other.z_ };
    }

    [[nodiscard]] Fix x() const noexcept { return x_; }
    [[nodiscard]] Fix y() const noexcept { return y_; }
    [[nodiscard]] Fix z() const noexcept { return z_; }
    [[nodiscard]] float xFloat() const noexcept { return x_.asFloat(); }
    [[nodiscard]] float yFloat() const noexcept { return y_.asFloat(); }
    [[nodiscard]] float zFloat() const noexcept { return z_.asFloat(); }
    [[nodiscard]] double xDouble() const noexcept { return x_.asDouble(); }
    [[nodiscard]] double yDouble() const noexcept { return y_.asDouble(); }
    [[nodiscard]] double zDouble() const noexcept { return z_.asDouble(); }


    void zero() noexcept { x_ = 0; y_ = 0; z_ = 0; }

    bool set(int32_t x, int32_t y, int32_t z) noexcept {
        if (x_ != x || y_ != y || z_ != z) {
            x_ = x;
            y_ = y;
            z_ = z;
            return true;
        }
        else {
            return false;
        }
    }

    bool set(const Fix& x, const Fix& y, const Fix& z) noexcept {
        if (x != x_ || y != y_ || z != z_) {
            x_ = x;
            y_ = y;
            z_ = z;
            return true;
        }
        else {
            return false;
        }
    }

    bool set(const char* x_str, const char* y_str, const char* z_str) noexcept {
        int64_t x_old = x_.m_raw_value;
        int64_t y_old = y_.m_raw_value;
        int64_t z_old = z_.m_raw_value;
        x_.setStr(x_str);
        y_.setStr(y_str);
        z_.setStr(z_str);
        return x_.m_raw_value != x_old || y_.m_raw_value != y_old || z_.m_raw_value != z_old;
    }


    bool setByCSV(const String& string, char delimiter) noexcept;

    void vec3f(Vec3f& out_vec) const noexcept {
        out_vec.x_ = x_.asFloat();
        out_vec.y_ = y_.asFloat();
        out_vec.z_ = z_.asFloat();
    }

    void vec3d(Vec3d& out_vec) const noexcept {
        out_vec.x_ = x_.asDouble();
        out_vec.y_ = y_.asDouble();
        out_vec.z_ = z_.asDouble();
    }

    void setVec3f(const Vec3f& vec) noexcept {
        x_.setFloat(vec.x_);
        y_.setFloat(vec.y_);
        z_.setFloat(vec.z_);
    }

    void setVec3d(const Vec3d& vec) noexcept {
        x_.setDouble(vec.x_);
        y_.setDouble(vec.y_);
        z_.setDouble(vec.z_);
    }

    void setPrecision(int32_t precision) noexcept {
        x_.setPrecision(precision);
        y_.setPrecision(precision);
        z_.setPrecision(precision);
    }
};


} // End of namespace Grain

#endif // GrainVec3Fix_hpp
