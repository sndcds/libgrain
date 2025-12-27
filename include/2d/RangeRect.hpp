//
//  RangeRect.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainRangeRect_hpp
#define GrainRangeRect_hpp

#include "Grain.hpp"
#include "Type/Fix.hpp"
#include "2d/Rect.hpp"
#include "Math/Vec2.hpp"
#include "Math/Vec2Fix.hpp"
#include "Math/Math.hpp"

#include <iostream>


namespace Grain {

class RangeRectFix {

public:
    Fix min_x_{};
    Fix min_y_{};
    Fix max_x_{};
    Fix max_y_{};

public:
    RangeRectFix() noexcept = default;
    explicit RangeRectFix(Fix min_x, Fix min_y, Fix max_x, Fix max_y) noexcept :
        min_x_(min_x), min_y_(min_y), max_x_(max_x), max_y_(max_y) {}

    virtual ~RangeRectFix() noexcept = default;

    [[nodiscard]] virtual const char* className() const noexcept {
        return "RangeRectFix";
    }

    friend std::ostream& operator << (std::ostream& os, const RangeRectFix* o) {
        o == nullptr ? os << "RangeRectFix nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const RangeRectFix& o) {
        return os << o.min_x_ << ", " << o.min_y_ << " | " << o.max_x_ << ", " << o.max_y_;
    }

    RangeRectFix& operator = (const Vec2Fix& v) {
        min_x_ = max_x_ = v.x_;
        min_y_ = max_y_ = v.y_;
        return *this;
    }

    RangeRectFix& operator = (const RectFix& r) {
        min_x_ = r.width_ > 0 ? r.x_ : r.x_ + r.width_;
        min_y_ = r.height_ > 0 ? r.y_ : r.y_ + r.height_;
        max_x_ = r.width_ > 0 ? r.x_ + r.width_ : r.x_;
        max_y_ = r.height_ > 0 ? r.y_ + r.height_ : r.y_;
        return *this;
    }

    bool operator == (const RangeRectFix& other) const {
        return
            min_x_ == other.min_x_ && min_y_ == other.min_y_ &&
            max_x_ == other.max_x_ && max_y_ == other.max_y_;
    }

    bool operator != (const RangeRectFix& other) const {
        return
            min_x_ != other.min_x_ || min_y_ != other.min_y_ ||
            max_x_ != other.max_x_ || max_y_ != other.max_y_;
    }

    RangeRectFix operator + (const RangeRectFix& other) const {
        RangeRectFix result;
        result.min_x_ = min_x_ < other.min_x_ ? min_x_ : other.min_x_;
        result.min_y_ = min_y_ < other.min_y_ ? min_y_ : other.min_y_;
        result.max_x_ = max_x_ > other.max_x_ ? max_x_ : other.max_x_;
        result.max_y_ = max_y_ > other.max_y_ ? max_y_ : other.max_y_;
        return result;
    }

    RangeRectFix operator + (const Vec2Fix& v) const {
        RangeRectFix result = *this;
        if (v.x_ < min_x_) { result.min_x_ = v.x_; }
        if (v.y_ < min_y_) { result.min_y_ = v.y_; }
        if (v.x_ > max_x_) { result.max_x_ = v.x_; }
        if (v.y_ > max_y_) { result.max_y_ = v.y_; }
        return result;
    }

    RangeRectFix operator + (const RectFix& r) const {
        RangeRectFix result;
        Fix r_min_x = r.width_ > 0 ? r.x_ : r.x_ + r.width_;
        Fix r_min_y = r.height_ > 0 ? r.y_ : r.y_ + r.height_;
        Fix r_max_x = r.width_ > 0 ? r.x_ + r.width_ : r.x_;
        Fix r_max_y = r.height_ > 0 ? r.y_ + r.height_ : r.y_;
        result.min_x_ = min_x_ < r_min_x ? min_x_ : r_min_x;
        result.min_y_ = min_y_ < r_min_y ? min_y_ : r_min_y;
        result.max_x_ = max_x_ > r_max_x ? max_x_ : r_max_x;
        result.max_y_ = max_y_ > r_max_y ? max_y_ : r_max_y;
        return result;
    }

    RangeRectFix& operator += (const RangeRectFix& other) {
        min_x_ = min_x_ < other.min_x_ ? min_x_ : other.min_x_;
        min_y_ = min_y_ < other.min_y_ ? min_y_ : other.min_y_;
        max_x_ = max_x_ > other.max_x_ ? max_x_ : other.max_x_;
        max_y_ = max_y_ > other.max_y_ ? max_y_ : other.max_y_;
        return *this;
    }

    RangeRectFix& operator += (const Vec2Fix& r) {
        if (r.x_ < min_x_) { min_x_ = r.x_; }
        if (r.y_ < min_y_) { min_y_ = r.y_; }
        if (r.x_ > max_x_) { max_x_ = r.x_; }
        if (r.y_ > max_y_) { max_y_ = r.y_; }
        return *this;
    }

    RangeRectFix& operator += (const RectFix& r) {
        Fix r_min_x = r.width_ > 0 ? r.x_ : r.x_ + r.width_;
        Fix r_min_y = r.height_ > 0 ? r.y_ : r.y_ + r.height_;
        Fix r_max_x = r.width_ > 0 ? r.x_ + r.width_ : r.x_;
        Fix r_max_y = r.height_ > 0 ? r.y_ + r.height_ : r.y_;
        min_x_ = min_x_ < r_min_x ? min_x_ : r_min_x;
        min_y_ = min_y_ < r_min_y ? min_y_ : r_min_y;
        max_x_ = max_x_ > r_max_x ? max_x_ : r_max_x;
        max_y_ = max_y_ > r_max_y ? max_y_ : r_max_y;
        return *this;
    }


    [[nodiscard]] Fix minX() const noexcept { return min_x_; }
    [[nodiscard]] Fix maxX() const noexcept { return max_x_; }
    [[nodiscard]] Fix minY() const noexcept { return min_y_; }
    [[nodiscard]] Fix maxY() const noexcept { return max_y_; }
    [[nodiscard]] Fix centerX() const noexcept { return min_x_ + (max_x_ - min_x_) / 2; }
    [[nodiscard]] Fix centerY() const noexcept { return min_y_ + (max_y_ - min_y_) / 2; }
    [[nodiscard]] Fix width() const noexcept { Fix w = max_x_ - min_x_; return w < 0 ? -w : w; }
    [[nodiscard]] Fix height() const noexcept { Fix h = max_y_ - min_y_; return h < 0 ? -h : h; }

    [[nodiscard]] Vec2d centerAsVec2d() const noexcept {
        return { centerX().asDouble(), centerY().asDouble() };
    }

    [[nodiscard]] RectFix rect() const noexcept {
        return RectFix(min_x_, min_y_, max_x_ - min_x_, max_y_ - min_y_);
    }

    void initForMinMaxSearch() noexcept {
        min_x_.setToMax();
        min_y_.setToMax();
        max_x_.setToMin();
        max_y_.setToMin();
    }

    void set(const Fix& x, const Fix& y) noexcept { min_x_ = max_x_ = x; min_y_ = max_y_ = y; }
    void set(const Vec2Fix& v) noexcept { min_x_ = max_x_ = v.x_; min_y_ = max_y_ = v.y_; }
    void set(Vec2Fix* v) noexcept { if (v) { min_x_ = max_x_ = v->x_; min_y_ = max_y_ = v->y_; } }

    void setMinX(const Fix& v) noexcept { min_x_ = v; }
    void setMinY(const Fix& v) noexcept { min_y_ = v; }
    void setMaxX(const Fix& v) noexcept { max_x_ = v; }
    void setMaxY(const Fix& v) noexcept { max_y_ = v; }

    void set(double min_x, double min_y, double max_x, double max_y) noexcept {
        min_x_ = min_x; min_y_ = min_y;
        max_x_ = max_x; max_y_ = max_y;
    }

    void set(const Fix& min_x, const Fix& min_y, const Fix& max_x, const Fix& max_y) noexcept {
        min_x_ = min_x; min_y_ = min_y;
        max_x_ = max_x; max_y_ = max_y;
    }

    void set(const Vec2Fix& min, const Vec2Fix& max) noexcept {
        min_x_ = min.x_; min_y_ = min.y_;
        max_x_ = max.x_; max_y_ = max.y_;
    }

    void add(const Vec2Fix& v) noexcept {
        addX(v.x_); addY(v.y_);
    }

    void add(const Vec2Fix* v) noexcept {
        if (v) {
            addX(v->x_); addY(v->y_);
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

    void add(const Fix& x, const Fix& y) noexcept {
        addX(x); addY(y);
    }

    void add(const RectFix& rect) noexcept {
        addX(rect.x_); addX(rect.x2());
        addY(rect.y_); addY(rect.y2());
    }

    void add(const RangeRectFix& r) noexcept {
        addX(r.min_x_); addX(r.max_x_);
        addY(r.min_y_); addY(r.max_y_);
    }

    void limit(const RangeRectFix& max_rect) noexcept {
        if (min_x_ < max_rect.min_x_) min_x_ = max_rect.min_x_;
        if (min_y_ < max_rect.min_y_) min_y_ = max_rect.min_y_;
        if (max_x_ > max_rect.max_x_) max_x_ = max_rect.max_x_;
        if (max_y_ > max_rect.max_y_) max_y_ = max_rect.max_y_;
    }

    void scrollX(const Fix& amount) noexcept { min_x_ += amount; max_x_ += amount; }
    void scrollY(const Fix& amount) noexcept { min_y_ += amount; max_y_ += amount; }
    void scroll(const Fix& x_amount, const Fix& y_amount) noexcept { scrollX(x_amount); scrollY(y_amount); }
    void scrollRight() noexcept { scrollX(width() / 2); }
    void scrollLeft() noexcept { scrollX(-width() / 2); }
    void scrollUp() noexcept { scrollY(height() / 2); }
    void scrollDown() noexcept { scrollY(-height() / 2); }

    void lerp(const RangeRectFix& r, const Fix& t) noexcept {
        RangeRectFix result;
        result.min_x_ = min_x_ + t * (r.min_x_ - min_x_);
        result.min_y_ = min_y_ + t * (r.min_y_ - min_y_);
        result.max_x_ = max_x_ + t * (r.max_x_ - max_x_);
        result.max_y_ = max_y_ + t * (r.max_y_ - max_y_);
        *this = result;
    }

    static RangeRectFix lerp(const RangeRectFix& a, const RangeRectFix& b, const Fix& t) noexcept {
        RangeRectFix result;
        result.min_x_ = a.min_x_ + t * (b.min_x_ - a.min_x_);
        result.min_y_ = a.min_y_ + t * (b.min_y_ - a.min_y_);
        result.max_x_ = a.max_x_ + t * (b.max_x_ - a.max_x_);
        result.max_y_ = a.max_y_ + t * (b.max_y_ - a.max_y_);
        return result;
    }

    [[nodiscard]] bool isInside(const Fix& x, const Fix& y) const noexcept {
        return (x >= min_x_ && x <= max_x_ && y >= min_y_ && y <= max_y_);
    }

    [[nodiscard]] bool isInside(const Vec2Fix& v) const noexcept {
        return (v.x() >= min_x_ && v.x() <= max_x_ && v.y() >= min_y_ && v.y() <= max_y_);
    }
};


template <ScalarType T>
class RangeRect {

public:
    T min_x_{};
    T min_y_{};
    T max_x_{};
    T max_y_{};

public:
    RangeRect() noexcept = default;
    RangeRect(T min_x, T min_y, T max_x, T max_y) noexcept :
        min_x_(min_x), min_y_(min_y), max_x_(max_x), max_y_(max_y) {}

    virtual ~RangeRect() = default;

    [[nodiscard]] virtual const char* className() const noexcept {
        return "RangeRect";
    }

    friend std::ostream& operator << (std::ostream& os, const RangeRect* o) {
        o == nullptr ? os << "RangeRect nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const RangeRect& o) {
        os << std::fixed << o.min_x_ << ", " << o.min_y_ << ", " << o.max_x_ << ", " << o.max_y_ << std::defaultfloat;
        return os;
    }

    RangeRect& operator = (const Vec2<T>& v) {
        min_x_ = max_x_ = v.x_;
        min_y_ = max_y_ = v.y_;
        return *this;
    }

    RangeRect& operator = (const Rect<T>& r) {
        min_x_ = r.width_ >= 0 ? r.x_ : r.x_ + r.width_;
        min_y_ = r.height_ >= 0 ? r.y_ : r.y_ + r.height_;
        max_x_ = r.width_ >= 0 ? r.x_ + r.width_ : r.x_;
        max_y_ = r.height_ >= 0 ? r.y_ + r.height_ : r.y_;
        return *this;
    }

    RangeRect& operator = (const RangeRectFix& r) {
        if constexpr (std::is_same_v<T, double>) {
            min_x_ = r.min_x_.asDouble();
            max_x_ = r.max_x_.asDouble();
            min_y_ = r.min_y_.asDouble();
            max_y_ = r.max_y_.asDouble();
        }
        else if constexpr (std::is_same_v<T, float>) {
            min_x_ = r.min_x_.asFloat();
            max_x_ = r.max_x_.asFloat();
            min_y_ = r.min_y_.asFloat();
            max_y_ = r.max_y_.asFloat();
        }
        else if constexpr (std::is_same_v<T, int64_t>) {
            min_x_ = r.min_x_.asInt64();
            max_x_ = r.max_x_.asInt64();
            min_y_ = r.min_y_.asInt64();
            max_y_ = r.max_y_.asInt64();
        }
        else {
            min_x_ = r.min_x_.asInt32();
            max_x_ = r.max_x_.asInt32();
            min_y_ = r.min_y_.asInt32();
            max_y_ = r.max_y_.asInt32();
        }
        return *this;
    }

    bool operator == (const RangeRect& other) const {
        return
            min_x_ == other.min_x_ && min_y_ == other.min_y_ &&
            max_x_ == other.max_x_ && max_y_ == other.max_y_;
    }

    bool operator != (const RangeRect& other) const {
        return
            min_x_ != other.min_x_ || min_y_ != other.min_y_ ||
            max_x_ != other.max_x_ || max_y_ != other.max_y_;
    }

    RangeRect operator + (const RangeRect& other) const {
        RangeRect result;
        result.min_x_ = min_x_ < other.min_x_ ? min_x_ : other.min_x_;
        result.min_y_ = min_y_ < other.min_y_ ? min_y_ : other.min_y_;
        result.max_x_ = max_x_ > other.max_x_ ? max_x_ : other.max_x_;
        result.max_y_ = max_y_ > other.max_y_ ? max_y_ : other.max_y_;
        return result;
    }

    RangeRect operator + (const Vec2<T>& v) const {
        RangeRect result = *this;
        if (v.x_ < min_x_) { result.min_x_ = v.x_; }
        if (v.y_ < min_y_) { result.min_y_ = v.y_; }
        if (v.x_ > max_x_) { result.max_x_ = v.x_; }
        if (v.y_ > max_y_) { result.max_y_ = v.y_; }
        return result;
    }

    RangeRect operator + (const Rect<T>& r) const {
        RangeRect result;
        T r_min_x = r.width_ > 0 ? r.x_ : r.x_ + r.width_;
        T r_min_y = r.height_ > 0 ? r.y_ : r.y_ + r.height_;
        T r_max_x = r.width_ > 0 ? r.x_ + r.width_ : r.x_;
        T r_max_y = r.height_ > 0 ? r.y_ + r.height_ : r.y_;
        result.min_x_ = min_x_ < r_min_x ? min_x_ : r_min_x;
        result.min_y_ = min_y_ < r_min_y ? min_y_ : r_min_y;
        result.max_x_ = max_x_ > r_max_x ? max_x_ : r_max_x;
        result.max_y_ = max_y_ > r_max_y ? max_y_ : r_max_y;
        return result;
    }

    RangeRect& operator += (const RangeRect& other) {
        min_x_ = min_x_ < other.min_x_ ? min_x_ : other.min_x_;
        min_y_ = min_y_ < other.min_y_ ? min_y_ : other.min_y_;
        max_x_ = max_x_ > other.max_x_ ? max_x_ : other.max_x_;
        max_y_ = max_y_ > other.max_y_ ? max_y_ : other.max_y_;
        return *this;
    }

    RangeRect& operator += (const Vec2<T>& v) {
        if (v.x_ < min_x_) { min_x_ = v.x_; }
        if (v.y_ < min_y_) { min_y_ = v.y_; }
        if (v.x_ > max_x_) { max_x_ = v.x_; }
        if (v.y_ > max_y_) { max_y_ = v.y_; }
        return *this;
    }

    RangeRect& operator += (const Rect<T>& r) {
        T r_min_x = r.width_ > 0 ? r.x_ : r.x_ + r.width_;
        T r_min_y = r.height_ > 0 ? r.y_ : r.y_ + r.height_;
        T r_max_x = r.width_ > 0 ? r.x_ + r.width_ : r.x_;
        T r_max_y = r.height_ > 0 ? r.y_ + r.height_ : r.y_;
        min_x_ = min_x_ < r_min_x ? min_x_ : r_min_x;
        min_y_ = min_y_ < r_min_y ? min_y_ : r_min_y;
        max_x_ = max_x_ > r_max_x ? max_x_ : r_max_x;
        max_y_ = max_y_ > r_max_y ? max_y_ : r_max_y;
        return *this;
    }


    [[nodiscard]] T minX() const noexcept { return min_x_; }
    [[nodiscard]] T maxX() const noexcept { return max_x_; }
    [[nodiscard]] T minY() const noexcept { return min_y_; }
    [[nodiscard]] T maxY() const noexcept { return max_y_; }
    [[nodiscard]] Vec2<T> pos1() const noexcept { return Vec2<T>(min_x_, min_y_); }
    [[nodiscard]] Vec2<T> pos2() const noexcept { return Vec2<T>(max_x_, min_y_); }
    [[nodiscard]] Vec2<T> pos3() const noexcept { return Vec2<T>(max_x_, max_y_); }
    [[nodiscard]] Vec2<T> pos4() const noexcept { return Vec2<T>(min_x_, max_y_); }
    [[nodiscard]] Vec2<T> center() const noexcept { return Vec2<T>(min_x_ + (max_x_ - min_x_) / 2, min_y_ + (max_y_ - min_y_) / 2); }
    [[nodiscard]] T centerX() const noexcept { return min_x_ + (max_x_ - min_x_) / 2; }
    [[nodiscard]] T centerY() const noexcept { return min_y_ + (max_y_ - min_y_) / 2; }
    [[nodiscard]] T width() const noexcept { return max_x_ - min_x_; }
    [[nodiscard]] T height() const noexcept { return max_y_ - min_y_; }
    [[nodiscard]] T shortSide() const noexcept { T w = width(); T h = height(); return w < h ? w : h; }
    [[nodiscard]] T longSide() const noexcept { T w = width(); T h = height(); return w > h ? w : h; }

    [[nodiscard]] Vec2<T> randomPos() const noexcept {
        return Vec2<T>(min_x_ + Random::next(max_x_ - min_x_), min_y_ + Random::next(max_y_ - min_y_));
    }

    [[nodiscard]] Vec2d innerPos(double x, double y) const noexcept {
        return Vec2d(Math::lerp(min_x_, max_x_, x), Math::lerp(min_y_, max_y_, y));
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
        return Rect<T>(min_x_, min_y_, max_x_ - min_x_, max_y_ - min_y_);
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


    void initForMinMaxSearch() noexcept {
        min_x_ = std::numeric_limits<T>::max();
        min_y_ = std::numeric_limits<T>::max();
        max_x_ = std::numeric_limits<T>::lowest();
        max_y_ = std::numeric_limits<T>::lowest();
    }


    void set(T x, T y) noexcept { min_x_ = max_x_ = x; min_y_ = max_y_ = y; }
    void set(const Vec2<T>& v) noexcept { min_x_ = max_x_ = v.x_; min_y_ = max_y_ = v.y_; }
    void set(Vec2<T>* v) noexcept { if (v) { min_x_ = max_x_ = v->x_; min_y_ = max_y_ = v->y_; } }

    void set(T min_x, T min_y, T max_x, T max_y) noexcept {
        min_x_ = min_x;
        min_y_ = min_y;
        max_x_ = max_x;
        max_y_ = max_y;
    }

    void set(const Vec2<T>& min, const Vec2<T>& max) noexcept {
        min_x_ = min.x_;
        max_x_ = max.x_;
        min_y_ = min.y_;
        max_y_ = max.y_;
    }

    void set(T*  values, int32_t n) noexcept {
        if (values && n == 4) {
            min_x_ = values[0];
            min_y_ = values[1];
            max_x_ = values[2];
            max_y_ = values[3];
        }
    }

    void setLonlat(T lon1, T lat1, T lon2, T lat2) noexcept {
        min_x_ = lon1;
        min_y_ = lat1;
        max_x_ = lon2;
        max_y_ = lat2;
    }


    void sanitizeMinMax() noexcept {
        if (min_x_ > max_x_) { swapX(); }
        if (min_y_ > max_y_) { swapY(); }
    }

    void swapX() noexcept {
        T temp = min_x_; min_x_ = max_x_; max_x_ = temp;
    }

    void swapY() noexcept {
        T temp = min_y_; min_y_ = max_y_; max_y_ = temp;
    }

    bool add(const Vec2<T>& v) noexcept {
        bool result = false;
        if (v.x_ < min_x_) { min_x_ = v.x_; result = true; }
        if (v.x_ > max_x_) { max_x_ = v.x_; result = true; }
        if (v.y_ < min_y_) { min_y_ = v.y_; result = true; }
        if (v.y_ > max_y_) { max_y_ = v.y_; result = true; }
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

    bool add(T x, T y) noexcept {
        bool result = false;
        if (x < min_x_) { min_x_ = x; result = true; }
        if (x > max_x_) { max_x_ = x; result = true; }
        if (y < min_y_) { min_y_ = y; result = true; }
        if (y > max_y_) { max_y_ = y; result = true; }
        return result;
    }

    void add(const Rect<T>& rect) noexcept {
        if (rect.x_ < min_x_) min_x_ = rect.x_;
        if (rect.x2() > max_x_) max_x_ = rect.x2();
        if (rect.y_ < min_y_) min_y_ = rect.y_;
        if (rect.y2() > max_y_) max_y_ = rect.y2();
    }

    void add(const RangeRect<T>& rect) noexcept {
        *this += rect;
    }


    void limit(const RangeRect& max_rect) noexcept {
        if (min_x_ < max_rect.min_x_) min_x_ = max_rect.min_x_;
        if (min_y_ < max_rect.min_y_) min_y_ = max_rect.min_y_;
        if (max_x_ > max_rect.max_x_) max_x_ = max_rect.max_x_;
        if (max_y_ > max_rect.max_y_) max_y_ = max_rect.max_y_;
    }

    void adjustUniform(T view_width, T view_height) noexcept {

        T xScale = width() / view_width;    // TODO: check 0!
        T yScale = height() / view_height;

        if (yScale < xScale) {
            T c = centerY();
            T r = height() * (xScale / yScale) / 2;
            min_y_ = c - r;
            max_y_ = c + r;
        }
        else {
            T c = centerX();
            T r = width() * (yScale / xScale) / 2;
            min_x_ = c - r;
            max_x_ = c + r;
        }
    }

    [[nodiscard]] bool contains(const Vec2<T> pos) const noexcept {
        return pos.x_ >= min_x_ &&
               pos.x_ <= max_x_ &&
               pos.y_ >= min_y_ &&
               pos.y_ <= max_y_;
    }

    [[nodiscard]] bool contains(const Vec2<T> pos, T tolerance) const noexcept {
        return pos.x_ >= min_x_ - tolerance &&
               pos.x_ <= max_x_ + tolerance &&
               pos.y_ >= min_y_ - tolerance &&
               pos.y_ <= max_y_ + tolerance;
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
        if (max_x_ < range_rect.min_x_ || range_rect.max_x_ < min_x_) {
            return false;
        }
        if (max_y_ < range_rect.min_y_ || range_rect.max_y_ < min_y_) {
            return false;
        }
        return true;
    }

    void scrollX(T amount) noexcept { min_x_ += amount; max_x_ += amount; }
    void scrollY(T amount) noexcept { min_y_ += amount; max_y_ += amount; }
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
            min_x_ = pivot.x_ + (min_x_ - pivot.x_) / factor;
            max_x_ = pivot.x_ + (max_x_ - pivot.x_) / factor;
            min_y_ = pivot.y_ + (min_y_ - pivot.y_) / factor;
            max_y_ = pivot.y_ + (max_y_ - pivot.y_) / factor;
        }
    }

    void zoomX(T factor) noexcept {
        if (factor != 0) {
            T c = centerX();
            T s = width() / factor / 2;
            min_x_ = c - s;
            max_x_ = c + s;
        }
    }

    void zoomX(T pivot, T factor) noexcept {
        if (factor != 0) {
            min_x_ =  pivot + (min_x_ - pivot) / factor;
            max_x_ =  pivot + (max_x_ - pivot) / factor;
        }
    }

    void zoomY(T factor) noexcept {
        if (factor != 0) {
            T c = centerY();
            T s = height() / factor / 2.0;
            min_y_ = c - s;
            max_y_ = c + s;
        }
    }

    void zoomY(T pivot, T factor) noexcept {
        if (factor != 0) {
            min_y_ =  pivot + (min_y_ - pivot) / factor;
            max_y_ =  pivot + (max_y_ - pivot) / factor;
        }
    }

    void zoomIn() noexcept { zoomInX(); zoomInY(); }

    void zoomInX() noexcept {
        T amount = width() / 4;
        T center = centerX();
        min_x_ = center - amount;
        max_x_ = center + amount;
    }

    void zoomInY() noexcept {
        T amount = height() / 4;
        T center = centerY();
        min_y_ = center - amount;
        max_y_ = center + amount;
    }

    void zoomOut() noexcept { zoomOutX(); zoomOutY(); }

    void zoomOutX() noexcept {
        T amount = width();
        T center = centerX();
        min_x_ = center - amount;
        max_x_ = center + amount;
    }

    void zoomOutY() noexcept {
        T amount = height();
        T center = centerY();
        min_y_ = center - amount;
        max_y_ = center + amount;
    }

    void extend(double f) noexcept {
        double w = width() * f * 0.5;
        double h = height() * f * 0.5;
        if (min_x_ < max_x_) {
            min_x_ -= w;
            max_x_ += w;
        }
        else {
            min_x_ += w;
            max_x_ -= w;
        }
        if (min_y_ < max_y_) {
            min_y_ -= h;
            max_y_ += h;
        }
        else {
            min_y_ += h;
            max_y_ -= h;
        }
    }

    void lerp(const RangeRect<T>& r, double t) noexcept {
        RangeRect<T> result;
        result.min_x_ = min_x_ + t * (r.min_x_ - min_x_);
        result.min_y_ = min_y_ + t * (r.min_y_ - min_y_);
        result.max_x_ = max_x_ + t * (r.max_x_ - max_x_);
        result.max_y_ = max_y_ + t * (r.max_y_ - max_y_);
        *this = result;
    }

    static RangeRect<T> lerp(const RangeRect<T>& a, const RangeRect<T>& b, double t) noexcept {
        RangeRect<T> result;
        result.min_x_ = a.min_x_ + t * (b.min_x_ - a.min_x_);
        result.min_y_ = a.min_y_ + t * (b.min_y_ - a.min_y_);
        result.max_x_ = a.max_x_ + t * (b.max_x_ - a.max_x_);
        result.max_y_ = a.max_y_ + t * (b.max_y_ - a.max_y_);
        return result;
    }

    void writeToFile(File& file) {
        file.writeValue<T>(min_x_);
        file.writeValue<T>(min_y_);
        file.writeValue<T>(max_x_);
        file.writeValue<T>(max_y_);
    }

    void readFromFile(File& file) {
        min_x_ = file.readValue<T>();
        min_y_ = file.readValue<T>();
        max_x_ = file.readValue<T>();
        max_y_ = file.readValue<T>();
    }
};


// Standard types
using RangeRecti = RangeRect<int32_t>;  ///< 32 bit integer
using RangeRectl = RangeRect<int64_t>;  ///< 64 bit integer
using RangeRectf = RangeRect<float>;    ///< 32 bit floating point
using RangeRectd = RangeRect<double>;   ///< 64 bit floating point


} // End of namespace Grain

#endif // GrainRangeRect_hpp
