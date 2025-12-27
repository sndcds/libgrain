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
    Fix x_{};
    Fix y_{};

public:
    Vec2Fix() noexcept { zero(); }
    Vec2Fix(const Fix& x, const Fix& y) noexcept {
        x_ = x;
        y_ = y;
    }

    template<std::integral U>
    explicit Vec2Fix(const Vec2<U>& v) noexcept {
        std::cout << "Vec2Fix integral\n";
        x_.setInt64(static_cast<int64_t>(v.x_));
        y_.setInt64(static_cast<int64_t>(v.m_y));
    }

    template<std::floating_point U>
    explicit Vec2Fix(const Vec2<U>& v) noexcept {
        std::cout << "Vec2Fix floating_point\n";
        x_.setDouble(static_cast<double>(v.x_));
        y_.setDouble(static_cast<double>(v.m_y));
    }

    explicit Vec2Fix(const char* csv, char delimiter = ',') noexcept { setByCSV(csv, delimiter); }
    explicit Vec2Fix(const String& csv, char delimiter = ',') noexcept { setByCSV(csv, delimiter); }


    [[nodiscard]] virtual const char *className() const noexcept { return "Vec2Fix"; }

    friend std::ostream& operator << (std::ostream& os, const Vec2Fix* o) {
        return o == nullptr ? os << "Vec2Fix nullptr" : os << *o;
    }

    friend std::ostream& operator << (std::ostream& os, const Vec2Fix& o) {
        os << o.x_ << ", " << o.y_;
        return os;
    }

    bool operator == (const Vec2Fix& other) const { return other.x_ == x_ && other.y_ == y_; }
    bool operator != (const Vec2Fix& other) const { return other.x_ != x_ || other.y_ != y_; }

    Vec2Fix operator + (const Vec2Fix& other) const { return { x_ + other.x_, y_ + other.y_ }; }
    Vec2Fix operator - (const Vec2Fix& other) const { return { x_ - other.x_, y_ - other.y_ }; }

    Vec2Fix operator * (const Vec2Fix& other) const { return { x_ * other.x_, y_ * other.y_ }; }

    [[nodiscard]] Fix x() const noexcept { return x_; }
    [[nodiscard]] Fix y() const noexcept { return y_; }
    [[nodiscard]] float xFloat() const noexcept { return x_.asFloat(); }
    [[nodiscard]] float yFloat() const noexcept { return y_.asFloat(); }
    [[nodiscard]] double xDouble() const noexcept { return x_.asDouble(); }
    [[nodiscard]] double yDouble() const noexcept { return y_.asDouble(); }

    [[nodiscard]] inline Fix* xData() noexcept { return &x_; }
    [[nodiscard]] inline Fix* yData() noexcept { return &y_; }

    bool set(const Vec2Fix& vec) noexcept {
        if (vec != *this) {
            *this = vec;
            return true;
        }
        return false;
    }

    bool set(int32_t x, int32_t y) noexcept {
        if (x_ != x && y_ != y) {
            x_ = x;
            y_ = y;
            return true;
        }
        else {
            return false;
        }
    }

    bool set(const Fix& x, const Fix& y) noexcept {
        if (x != x_ || y != y_) {
            x_ = x;
            y_ = y;
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
            if (!csv_line_parser.nextFix(x_)) { return false; }
            return csv_line_parser.nextFix(y_);
        }
        return false;
    }

    bool setByCSV(const String& csv, char delimiter) noexcept {
        return setByCSV(csv.utf8(), delimiter);
    }

    void zero() noexcept { x_ = 0; y_ = 0; }

    void clampX(const Fix& min, const Fix& max) noexcept { x_.clamp(min, max); }
    void clampY(const Fix& min, const Fix& max) noexcept { y_.clamp(min, max); }

    [[nodiscard]] Vec2f vec2f() const noexcept { return { x_.asFloat(), y_.asFloat() }; }
    [[nodiscard]] Vec2d vec2d() const noexcept { return { x_.asDouble(), y_.asDouble() }; }

    void vec2f(Vec2f& out_vec) const noexcept {
        out_vec.x_ = x_.asFloat();
        out_vec.y_ = y_.asFloat();
    }

    void setVec2(const Vec2f& vec) noexcept {
        x_.setDouble(vec.x_);
        y_.setDouble(vec.y_);
    }

    void setPrecision(int32_t precision) noexcept {
        x_.setPrecision(precision);
        y_.setPrecision(precision);
    }

};


} // End of namespace Grain

#endif // GrainVec2Fix_hpp
