//
//  Dimension.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainDimension_hpp
#define GrainDimension_hpp

#include "Grain.hpp"
#include "2d/Rect.hpp"
#include "String/CSVString.hpp"


namespace Grain {

template <class T>
class Dimension {

public:
    T width_{};
    T height_{};

public:
    Dimension() noexcept : width_(static_cast<T>(0)), height_(static_cast<T>(0)) {}
    Dimension(T width, T height) noexcept : width_(width), height_(height) {}
    explicit Dimension(const char* csv, char delimiter = ',') noexcept { setByCSV(csv, delimiter); }
    explicit Dimension(const String& csv, char delimiter = ',') noexcept { setByCSV(csv, delimiter); }

    virtual ~Dimension() = default;


    [[nodiscard]] virtual const char* className() const noexcept {
        return "Dimension";
    }

    friend std::ostream& operator << (std::ostream& os, const Dimension* o) {
        o == nullptr ? os << "Dimension nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const Dimension& o) {
        os << o.width_ << ", " << o.height_;
        return os;
    }

    Dimension& operator = (const Dimension<int32_t>& r) {
        width_ = (T)r.width_; height_ = (T)r.height_; return *this;
    }

    Dimension& operator = (const Dimension<int64_t>& r) {
        width_ = (T)r.width_; height_ = (T)r.height_; return *this;
    }

    Dimension& operator = (const Dimension<float>& r) {
        width_ = (T)r.width_; height_ = (T)r.height_; return *this;
    }

    Dimension& operator = (const Dimension<double>& r) {
        width_ = (T)r.width_; height_ = (T)r.height_; return *this;
    }


    Dimension& operator = (const Rect<T>& v) { width_ = v.width_; height_ = v.height_; return *this; }

    bool operator == (const Dimension& v) const { return width_ == v.width_ && height_ == v.height_; }
    bool operator != (const Dimension& v) const { return width_ != v.width_ || height_ != v.height_; }

    [[nodiscard]] T width() const noexcept { return width_; }
    [[nodiscard]] T height() const noexcept { return height_; }
    [[nodiscard]] T centerX() const noexcept { return width_ / 2; }
    [[nodiscard]] T centerY() const noexcept { return height_ / 2; }
    [[nodiscard]] Vec2<T> center() const noexcept { return Vec2<T>(width_ / 2, height_ / 2); }

    [[nodiscard]] int64_t roundedWidth() noexcept { return static_cast<int64_t>(std::round(width_)); }
    [[nodiscard]] int64_t roundedHeight() noexcept { return static_cast<int64_t>(std::round(height_)); }
    [[nodiscard]] T area() const noexcept { return width_ * height_; }
    [[nodiscard]] double aspectRatio() const noexcept {
        if (std::fabs(width_) > std::numeric_limits<double>::epsilon() &&
            height_ > std::numeric_limits<double>::epsilon()) {
            return static_cast<double>(height_) / static_cast<double>(width_);
        }
        return std::numeric_limits<double>::max();
    }
    [[nodiscard]] bool isLandscape() const noexcept { return aspectRatio() < 1.0; }
    [[nodiscard]] bool isPortrait() const noexcept { return aspectRatio() > 1.0; }
    [[nodiscard]] bool isSquare() const noexcept { return std::fabs(1.0 - aspectRatio() <= std::numeric_limits<float>::epsilon()); }

    void set(T value) noexcept { width_ = value; height_ = value; }
    void set(T width, T height) noexcept { width_ = width; height_ = height; }
    void zero() noexcept { width_ = height_ = 0; }

    bool setByCSV(const char* csv, char delimiter = ',') noexcept {
        if (csv) {
            CSVLineParser csv_line_parser(csv);
            csv_line_parser.setDelimiter(delimiter);
            T values[2]{};
            if (csv_line_parser.values(2, values) == 2) {
                width_ = values[0];
                height_ = values[1];
                return true;
            }
        }
        return false;
    }

    bool setByCSV(const String& csv) noexcept  {
        return setByCSV(csv.utf8());
    }

    void flip() noexcept {
        std::swap(width_, height_);
    }

    void scale(T scale) noexcept {
        width_ *= scale;
        height_ *= scale;
    }
};


// Standard types
using Dimensioni = Dimension<int32_t>;  ///< 32 bit integer
using Dimensionl = Dimension<int64_t>;  ///< 64 bit integer
using Dimensionf = Dimension<float>;    ///< 32 bit floating point
using Dimensiond = Dimension<double>;   ///< 64 bit floating point


} // End of namespace Grain

#endif // GrainDimension_hpp
