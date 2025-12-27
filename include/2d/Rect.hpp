//
//  Rect.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

//  TODO: Implement methods from olcUTIL_Geometry2D/olcUTIL_Geometry2D.h
//  https://github.com/OneLoneCoder/olcUTIL_Geometry2D/blob/main/olcUTIL_Geometry2D.h

#ifndef GrainRect_hpp
#define GrainRect_hpp

#include "Type/Type.hpp"
#include "Type/Fix.hpp"
#include "Math/Vec2.hpp"
#include "Math/Vec2Fix.hpp"
#include "Math/Random.hpp"
#include "2d/Border.hpp"
#include "Geometry.hpp"

#if defined(__APPLE__) && defined(__MACH__)
#include <CoreGraphics/CoreGraphics.h>
#endif


namespace Grain {

/**
 *  @brief Rectangle.
 *
 *  `Rect` represents a two-dimensional rectangle.
 *  It is defined by its position (x and y), width, and height. The class
 *  provides essential functionalities for working with rectangular areas,
 *  including calculating area, checking for intersections with other rectangles,
 *  and more.
 *
 *  This templated class supports various data types and can be used for
 *  different numerical representations. Predefined specialized versions include
 *  datatypes `int32_t`, `int64_t`, `float` and `double`, named as `Recti`,
 *  `Rectl`, `Rectf` and `Rectd` respectively.
 *
 *  @note This class is often used in graphics applications to represent regions
 *        on a screen or within an image.
 */
template <class T>
class Rect {

public:
    T x_{};        ///< X-coordinate of the rectangle's origin (typically the left edge)
    T y_{};        ///< Y-coordinate of the rectangle's origin (typically the bottom or top edge depending on convention)
    T width_{};    ///< Width of the rectangle
    T height_{};   ///< Height of the rectangle

public:
    /**
     *  @brief Initializes a rectangle at (0, 0) with a default size of 1×1.
     */
    Rect() noexcept : width_(1), height_(1) {}

    /**
     *  @brief Fully initializes a rectangle with position (x, y) and dimensions
     *         (width, height).
     */
    Rect(T x, T y, T width, T height) noexcept : x_(x), y_(y), width_(width), height_(height) {}

    /**
     *  @brief Creates a square rectangle at (0, 0) with both width and height
     *         set to size.
     */
    explicit Rect(T size) noexcept : width_(size), height_(size) {}

    /**
     *  @brief Initializes a rectangle at (0, 0) with the specified width and
     *         height.
     */
    explicit Rect(T width, T height) noexcept : width_(width), height_(height) {}

    /**
     *  @brief Copies a rectangle and applies an inset by inset_size on all sides.
     */
    explicit Rect(const Rect& r, T inset_size) noexcept { *this = r; inset(inset_size); }

    /**
     *  @brief Creates a square centered at center with sides of length radius × 2.
     */
    explicit Rect(const Vec2<T>& center, T radius) noexcept {
        x_ = center.x_ - radius;
        y_ = center.y_ - radius;
        width_ = height_ = radius + radius;  // radius * 2
    }

    /**
     *  @brief Constructs a rectangle centered at center with the given dimensions.
     */
    explicit Rect(const Vec2<T>& center, T width, T height) noexcept {
        x_ = center.x_ - width / 2;
        y_ = center.y_ - height / 2;
        width_ = width;
        height_ = height;
    }

#if defined(__APPLE__) && defined(__MACH__)
    /**
     *  @brief Converts a Core Graphics CGRect to a Rect instance.
     */
    explicit Rect(const CGRect& r) noexcept {
        x_ = r.origin.x;
        y_ = r.origin.y;
        width_ = r.size.width;
        height_ = r.size.height;
    }
#endif

    [[nodiscard]] virtual const char* className() const noexcept {
        return "Rect";
    }

    friend std::ostream& operator << (std::ostream& os, const Rect* o) {
        o == nullptr ? os << "Rect nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const Rect& o) {
        os << o.x_ << ", " << o.y_ << " .. " << o.width_ << " x " << o.height_;
        return os;
    }


    // Operator overloading

    template<typename U>
    requires std::is_convertible_v<U, T>
    Rect<T>& operator = (const Rect<U>& other) {
        x_ = static_cast<T>(other.x_);
        y_ = static_cast<T>(other.y_);
        width_ = static_cast<T>(other.width_);
        height_ = static_cast<T>(other.height_);
        return *this;
    }

#if defined(__APPLE__) && defined(__MACH__)
    Rect& operator = (const CGRect& r) {
        x_ = r.origin.x; y_ = r.origin.y; width_ = r.size.width; height_ = r.size.height; return *this;
    }
#endif


    bool operator == (const Rect& other) const {
        return x_ == other.x_ && y_ == other.y_ && width_ == other.width_ && height_ == other.height_;
    }

    bool operator != (const Rect& other) const {
        return x_ != other.x_ || y_ != other.y_ || width_ != other.width_ || height_ != other.height_;
    }

    Rect operator + (const Vec2<T>& v) const { return Rect(x_ + v.x_, y_ + v.y_, width_, height_); }
    Rect operator - (const Vec2<T>& v) const { return Rect(x_ - v.x_, y_ - v.y_, width_, height_); }
    Rect operator * (const Vec2<T>& v) const { return Rect(x_ * v.x_, y_ * v.y_, width_, height_); }

    Rect& operator += (const Vec2<T>& v) { x_ += v.x_; y_ += v.y_; return *this; }
    Rect& operator -= (const Vec2<T>& v) { x_ -= v.x_; y_ -= v.y_; return *this; }
    Rect& operator *= (const Vec2<T>& v) { x_ *= v.x_; y_ *= v.y_; return *this; }

    Rect& operator += (const Rect& other) {
        T min_x = x_ < other.x_ ? x_ : other.x_;
        T min_y = y_ < other.y_ ? y_ : other.y_;
        T max_x = x2() > other.x2() ? x2() : other.x2();
        T max_y = y2() > other.y2() ? y2() : other.y2();
        x_ = min_x; y_ = min_y; width_ = max_x - min_x; height_ = max_y - min_y;
        return *this;
    }

    [[nodiscard]] T x() const noexcept { return x_; }
    [[nodiscard]] T y() const noexcept { return y_; }
    [[nodiscard]] T x2() const noexcept { return x_ + width_; }
    [[nodiscard]] T y2() const noexcept { return y_ + height_; }
    [[nodiscard]] T width() const noexcept { return width_; }
    [[nodiscard]] T height() const noexcept { return height_; }
    [[nodiscard]] T shortSide() const noexcept { return width_ < height_ ? width_ : height_; }
    [[nodiscard]] T longSide() const noexcept { return width_ > height_ ? width_ : height_; }
    [[nodiscard]] T roundedWidth() const noexcept { return std::round(width_); }
    [[nodiscard]] T roundedHeight() const noexcept { return std::round(height_); }
    [[nodiscard]] double area() const noexcept { return width_ * height_; }

    /**
     *  @brief Computes the aspect ratio of the rectangle (height / width).
     *
     *  The aspect ratio is calculated by dividing the height by the width. If
     *  either dimension is too small (close to zero), it returns a default
     *  aspect ratio of 1.0.
     *
     *  @return The aspect ratio (height / width), or 1.0 if the dimensions are
     *          too small.
     */
    [[nodiscard]] double aspectRatio() const noexcept {
        double w = width();
        double h = height();
        if (w > std::numeric_limits<double>::epsilon() && h > std::numeric_limits<double>::epsilon()) {
            return h / w;
        }
        return 1.0;
    }

    [[nodiscard]] double absRatio() const noexcept {
        double w = width();
        double h = height();
        if (w > h && h > 0.0) {
            return w / h;
        }
        else if (h > w && w > 0.0) {
            return h / w;
        }
        return 1.0;
    }

    /**
     *  @brief Checks if the rectangle is a square.
     *
     *  A rectangle is considered square if its aspect ratio is approximately
     *  1.0, within the floating-point epsilon tolerance.
     *
     *  @return `true` if the rectangle is a square, `false` otherwise.
     */
    [[nodiscard]] bool isSquare() const noexcept {
        return std::fabs(1.0 - aspectRatio()) <= std::numeric_limits<double>::epsilon();
    }

    /**
     *  @brief Checks if the rectangle has positive width and height.
     *     *
     *  @return `true` if the rectangle is usable (both width and height > 0),
     *          `false` otherwise.
     */
    [[nodiscard]] bool usable() const noexcept { return width_ > 0 && height_ > 0; }

    /**
     *  @brief Checks if the rectangle is horizontal.
     *
     *  @return `true` if the rectangle is horizontal (width > height),
     *          `false` otherwise.
     */
    [[nodiscard]] bool isHorizontal() const noexcept {
        return std::fabs(width_) >= std::fabs(height_);
    }

    /**
     *  @brief Checks if the rectangle is vertical.
     *
     *  @return `true` if the rectangle is vertical (height > width),
     *          `false` otherwise.
     */
    [[nodiscard]] bool isVertical() const noexcept {
        return std::fabs(height_) > std::fabs(width_);
    }

    [[nodiscard]] Vec2<T> pos1() const noexcept { return { x_, y_ }; }
    [[nodiscard]] Vec2<T> pos2() const noexcept { return { x_ + width_, y_ }; }
    [[nodiscard]] Vec2<T> pos3() const noexcept { return { x_ + width_, y_ + height_ }; }
    [[nodiscard]] Vec2<T> pos4() const noexcept { return { x_, y_ + height_ }; }
    [[nodiscard]] Vec2<T> pos(int32_t index) const noexcept {
        switch((index % 4 + 4) % 4) {
            case 0: return { x_, y_ };
            case 1: return { x_ + width_, y_ };
            case 2: return { x_ + width_, y_ + height_ };
            default: return { x_, y_ + height_ };
        }
    }

    /**
     *  @brief Gets the X coordinate of the rectangle's center.
     *  @return The X coordinate of the center.
     */
    [[nodiscard]] T centerX() const noexcept {
        return x_ + width_ / T(2);
    }

    /**
     *  @brief Gets the Y coordinate of the rectangle's center.
     *  @return The Y coordinate of the center.
     */
    [[nodiscard]] T centerY() const noexcept {
        return y_ + height_ / T(2);
    }

    /**
     *  @brief Gets the center position of the rectangle.
     *  @return A Vec2<T> representing the center point.
     */
    [[nodiscard]] Vec2<T> center() const noexcept {
        return Vec2(x_ + width_ / T(2), y_ + height_ / T(2));
    }

    /**
     *  @brief Gets the radius of the largest inscribed circle.
     *  @return The radius, defined as half of the shortest rectangle side.
     */
    [[nodiscard]] T radius() const noexcept {
        return shortSide() / T(2);
    }

    /**
     *  @brief Gets the radius of the circumcircle around the rectangle.
     *  @return The circumcircle radius, which is half of the rectangle's
     *          diagonal.
     */
    [[nodiscard]] T circumcircleRadius() {
        return std::sqrt(width_ * width_ + height_ * height_) / T(2);
    }

    /**
     *  @brief Maps a parameter `t` to the x-coordinate based on the rectangle's
     *         position and width.
     *
     *  @param t The parameter to map, typically in the range [0, 1].
     *  @return The mapped x-coordinate.
     */
    [[nodiscard]] double mappedX(double t) const noexcept {
        return static_cast<double>(x_) + t * static_cast<double>(width_);
    }

    /**
     *  @brief Maps a parameter `t` to the y-coordinate based on the rectangle's
     *         position and width.
     *
     *  @param t The parameter to map, typically in the range [0, 1].
     *  @return The mapped y-coordinate.
     */
    [[nodiscard]] double mappedY(double t) const noexcept {
        return static_cast<double>(y_) + t * static_cast<double>(height_);
    }

    /**
     *  @brief Maps two parameters `x` and `y` to the position inside the
     *         rectangle.
     *
     *  This method calculates the position within the rectangle based on the
     *  given `x` and `y` factors, where `x` typically represents a horizontal
     *  factor in the range [0, 1], and `y` represents a vertical factor in the
     *  range [0, 1]. The resulting position is returned as a `Vec2d`,
     *  representing the mapped coordinates.
     *
     *  @param x The horizontal parameter to map, typically in the range [0, 1].
     *  @param y The vertical parameter to map, typically in the range [0, 1].
     *  @return A `Vec2d` representing the mapped position inside the rectangle.
     */
    [[nodiscard]] Vec2d mappedPos(double x, double y) const {
        return {
            static_cast<double>(x_) + x * width(),
            static_cast<double>(x_) + y * height()
        };
    }

    /**
     *  @brief Generates a random position within the rectangle.
     *  @return A Vec2<T> representing a random point inside the rectangle.
     */
    [[nodiscard]] Vec2d randomPos() const noexcept {
        return {
            static_cast<double>(x_) + Random::next(width_),
            static_cast<double>(y_) + Random::next(height_)
        };
    }

    /**
     *  @brief Generates a random rectangle within the current rectangle.
     *
     *  @param min_width Minimum width of the random rectangle.
     *  @param max_width Maximum width of the random rectangle.
     *  @param min_height Minimum height of the random rectangle.
     *  @param max_height Maximum height of the random rectangle.
     *  @return A Rect<T> representing a randomly positioned and sized rectangle
     *                    inside the current one.
     */
    [[nodiscard]] Rect<T> randomRect(T min_width, T max_width, T min_height, T max_height) const noexcept {
        T w = Random::next(min_width, max_width);
        T h = Random::next(min_height, max_height);
        T x = x_ + Random::next(width_ - w);
        T y = y_ + Random::next(height_ - h);
        return Rect<T>(x, y, w, h);
    }

    /**
     *  @brief Adjusts the rectangle to fit within a destination rectangle based
     *        on a given fit mode.
     *
     *  @param dst_rect The destination rectangle to fit into.
     *  @param fit_mode The fitting mode (Cover, Fit, Stretch, or Center).
     *  @return A Rect that has been resized and positioned according to the fit
     *          mode.
     */
    [[nodiscard]] Rect fitRect(const Rect& dst_rect, FitMode fit_mode) const noexcept {
        Rect result;

        switch (fit_mode) {
            case FitMode::Cover:
            {
                double aw = static_cast<double>(dst_rect.width_) / width_;
                double ah = static_cast<double>(dst_rect.height_) / height_;
                if (aw > ah) {
                    result.width_ = width_ * aw;
                    result.height_ = height_ * aw;
                }
                else {
                    result.width_ = width_ * ah;
                    result.height_ = height_ * ah;
                }
            }
                break;

            case FitMode::Fit:
            {
                double aw = static_cast<double>(dst_rect.width_) / width_;
                double ah = static_cast<double>(dst_rect.height_) / height_;
                if (aw > ah) {
                    result.width_ = width_ * ah;
                    result.height_ = height_ * ah;
                }
                else {
                    result.width_ = width_ * aw;
                    result.height_ = height_ * aw;
                }
            }
                break;

            case FitMode::Stretch:
                return *this;

            case FitMode::Center:
                result.width_ = width_;
                result.height_ = height_;
                break;
        }

        result.x_ = dst_rect.centerX() - 0.5 * result.width_;
        result.y_ = dst_rect.centerY() - 0.5 * result.height_;

        return result;
    }

    /**
     *  @brief Gets a centered square inside the rectangle, based on the
     *         shortest side.
     *  @return A Rect representing the centered square.
     */
    [[nodiscard]] Rect centeredSquare() const noexcept {
        Rect result;
        T length = shortSide();
        if (width_ > height_) {
            result.x_ = centerX() - length / 2;
            result.y_ = y_;
        }
        else {
            result.x_ = x_;
            result.y_ = centerY() - length / 2;
        }
        result.width_ = result.height_ = length;
        return result;
    }

    /**
     *  @brief Gets a rectangle adjusted with padding values.
     *
     *  @param top Padding to subtract from the top.
     *  @param right Padding to subtract from the right.
     *  @param bottom Padding to subtract from the bottom.
     *  @param left Padding to subtract from the left.
     *  @return A Rect with adjusted dimensions based on the given paddings.
     */
    [[nodiscard]] Rect paddedRect(T top, T right, T bottom, T left) const noexcept {
        return Rect(x_ + left, y_ + right, width_ - left - right, height_ - top - bottom);
    }

    /**
     *  @brief Gets a rectangle that is a copy of `rect`, positioned inside
     *         `this` rectangle based on normalized factors.
     *
     *  The position of `rect` is determined by `th` and `tw`, where:
     *  - `th` specifies the horizontal placement (0 is left, 1 is right).
     *  - `tv` specifies the vertical placement (0 is top, 1 is bottom).
     *
     *  @param rect The rectangle to be positioned inside `this` rectangle.
     *  @param th The horizontal placement factor (0 to 1).
     *  @param tv The vertical placement factor (0 to 1).
     *  @return A new rectangle positioned as specified.
     */
    [[nodiscard]] Rect rectInRectNormalized(const Rect& rect, double th, double tv) const noexcept {
        double space_h = width_ - rect.width_;
        double space_v = height_ - rect.height_;
        return Rect(x_ + th * space_h, y_ + tv * space_v, rect.width_, rect.height_);
    }

    /**
     *  @brief Returns the position of a point within the rectangle based on the
     *         specified alignment.
     *
     *  @param alignment The alignment type (e.g., TopLeft, Center, BottomRight, etc.).
     *  @return A Vec2<T> representing the aligned position within the rectangle.
     */
    [[nodiscard]] Vec2<T> alignedPos(Alignment alignment) const noexcept {
        Vec2<T> pos;

        switch (alignment) {
            case Alignment::TopLeft:
                pos.x_ = x_;
                pos.y_ = y_;
                break;
            case Alignment::Left:
                pos.x_ = x_;
                pos.y_ = y_ + height_ / 2;
                break;
            case Alignment::BottomLeft:
                pos.x_ = x_;
                pos.y_ = y_ + height_;
                break;
            case Alignment::Top:
                pos.x_ = x_ + width_ / 2;
                pos.y_ = y_;
                break;
            case Alignment::Center:
                pos.x_ = x_ + width_ / 2;
                pos.y_ = y_ + height_ / 2;
                break;
            case Alignment::Bottom:
                pos.x_ = x_ + width_ / 2;
                pos.y_ = y_ + height_;
                break;
            case Alignment::TopRight:
                pos.x_ = x_ + width_;
                pos.y_ = y_;
                break;
            case Alignment::Right:
                pos.x_ = x_ + width_;
                pos.y_ = y_ + height_ / 2;
                break;
            case Alignment::BottomRight:
                pos.x_ = x_ + width_;
                pos.y_ = y_ + height_;
                break;
            default:
                pos.x_ = x_;
                pos.y_ = y_;
                break;
        }

        return pos;
    }

    /**
     *  @brief Gets the rectangular bounds of a grid cell within the current
     *         rectangle.
     *
     *  @param column_count Total number of columns in the grid.
     *  @param row_count Total number of rows in the grid.
     *  @param column_spacing Spacing between columns.
     *  @param row_spacing Spacing between rows.
     *  @param column_index Index of the target column (zero-based).
     *  @param row_index Index of the target row (zero-based).
     *  @param column_span Number of columns spanned by the cell (default is 1).
     *  @param row_span Number of rows spanned by the cell (default is 1).
     *  @param round_flag If true, rounds the computed values.
     *  @return A Rect<T> representing the computed cell's position and dimensions.
     */
    [[nodiscard]] Rect cellRect(
            int32_t column_count,
            int32_t row_count,
            T column_spacing,
            T row_spacing,
            int32_t column_index,
            int32_t row_index,
            int32_t column_span = 1,
            int32_t row_span = 1,
            bool round_flag = false
    ) const noexcept
    {
        Rect<T> rect;

        column_count = Type::maxOf<int32_t>(column_count, 1);
        row_count = Type::maxOf<int32_t>(row_count, 1);
        column_span = Type::maxOf<int32_t>(column_span, 1);
        row_span = Type::maxOf<int32_t>(row_span, 1);

        T total_columns_width = width_ - column_spacing * (column_count - 1);
        T total_rows_height = height_ - row_spacing * (row_count - 1);
        T column_width = total_columns_width / column_count;
        T row_height = total_rows_height / row_count;

        rect.x_ = x_ + (column_width + column_spacing) * column_index;
        rect.y_ = y_ + (row_height + row_spacing) * row_index;
        rect.width_ = column_spacing * (column_span - 1) + column_width * column_span;
        rect.height_ = row_spacing * (row_span - 1) + row_height * row_span;

        if (round_flag) {
            rect.roundValues();
        }

        return rect;
    }

    /**
     *  @brief Fills an array of points with the four corners of the rectangle.
     *
     *  @param out_points A pointer to an array of Vec2<T> where the corner
     *                    points will be stored. If the pointer is nullptr, no
     *                    operation will be performed.
     */
    void points(Vec2<T>* out_points) const noexcept {
        if (out_points) {
            out_points[0].set(x_, y_);
            out_points[1].set(x_ + width_, y_);
            out_points[2].set(x_ + width_, y_ + height_);
            out_points[3].set(x_, y_ + height_);
        }
    }

    void glVertices(T* out_vertices) const noexcept {
        if (out_vertices) {
            out_vertices[0] = out_vertices[6] = x_;
            out_vertices[1] = out_vertices[3] = y_;
            out_vertices[2] = out_vertices[4] = x_ + width_;
            out_vertices[5] = out_vertices[7] = y_ + height_;
        }
    }

    /**
     *  @brief Gets the intersection of the current rectangle with another
     *         rectangle.
     *
     *  @param rect The rectangle to intersect with.
     *  @return A Rect<T> representing the intersection of the two rectangles.
     *                    If there is no intersection, the returned rectangle
     *                    will have zero width and height.
     */
    [[nodiscard]] Rect intersection(const Rect& rect) const noexcept {
        Rect result = *this;
        result.intersect(rect);
        return result;
    }

    /**
     *  @brief Gets an aligned rectangle within the current rectangle,
     *         based on the specified alignment.
     *
     *  @param alignment The alignment option used to position the rectangle.
     *  @param width The width of the aligned rectangle.
     *  @param height The height of the aligned rectangle.
     *
     *  @return A Rect<T> representing the aligned rectangle with the specified
     *          width and height.
     */
    [[nodiscard]] Rect alignedRect(Alignment alignment, T width, T height) const noexcept {
        Rect result(width, height);
        switch (alignment) {
            case Alignment::TopLeft:
            case Alignment::Left:
            case Alignment::BottomLeft:
                result.x_ = x_;
                break;
            case Alignment::Top:
            case Alignment::Center:
            case Alignment::Bottom:
                result.x_ = centerX() - width / 2;
                break;
            case Alignment::TopRight:
            case Alignment::Right:
            case Alignment::BottomRight:
                result.x_ = x2() - width;
                break;
            default:
                break;
        }
        switch (alignment) {
            case Alignment::TopLeft:
            case Alignment::Top:
            case Alignment::TopRight:
                result.y_ = y_;
                break;
            case Alignment::Left:
            case Alignment::Center:
            case Alignment::Right:
                result.y_ = centerY() - height / 2;
                break;
            case Alignment::BottomLeft:
            case Alignment::Bottom:
            case Alignment::BottomRight:
                result.y_ = y2() - height;
                result;
            default:
                break;
        }
        return result;
    }

    /**
     *  @brief Gets a rectangle aligned to the edge of `this` rectangle based
     *         on the given alignment and padding values. The resulting
     *         rectangle is positioned relative to the size of `this` rectangle,
     *         but does not affect `this`'s position.
     *
     *  @param alignment The alignment option used to position the rectangle
     *                   (e.g., Top, Bottom, Left, Right).
     *  @param top The top padding value.
     *  @param right The right padding value.
     *  @param bottom The bottom padding value.
     *  @param left The left padding value.
     *
     *  @return A Rect<T> representing the edge-aligned rectangle with the
     *          specified alignment and padding, relative to the size of `this`
     *          rectangle.
     */
    [[nodiscard]] Rect edgeAlignedRectRelative(Alignment alignment, T top, T right, T bottom, T left) const noexcept {
        Rect result;
        int32_t h = 0;
        int32_t v = 0;

        switch (alignment) {
            case Alignment::Left: h = -1; break;
            case Alignment::Right: h = 1; break;
            case Alignment::Top: v = -1; break;
            case Alignment::Bottom: v = 1; break;
            case Alignment::TopLeft: h = -1; v = -1; break;
            case Alignment::TopRight: h = 1; v = -1; break;
            case Alignment::BottomLeft: h = -1; v = 1; break;
            case Alignment::BottomRight: h = 1; v = 1; break;
            default: break;
        }

        switch (h) {
            case -1: result.x_ = left; result.width_ = right - left; break;
            case 0: result.x_ = left; result.width_ = width_ - left - right; break;
            case 1: result.x_ = width_ - right; result.width_ = right - left; break;
        }

        switch (v) {
            case -1: result.y_ = top; result.height_ = bottom - top; break;
            case 0: result.y_ = top; result.height_ = height_ - top - bottom; break;
            case 1: result.y_ = height_ - bottom; result.height_ = bottom - top; break;
        }

        if (result.width_ < 0) {
            result.width_ = 0;
        }

        if (result.height_ < 0) {
            result.height_ = 0;
        }

        return result;
    }

    /**
     *  @brief Gets an inner rectangle with a specified padding and aspect ratio.
     *
     *  @param padding The padding value to apply around the inner rectangle.
     *  @param aspect_ratio The desired aspect ratio for the inner rectangle
     *                      (width / height).
     *
     *  @return A Rect<T> representing the inner rectangle with the specified
     *          padding and aspect ratio.
     */
    [[nodiscard]] Rect innerRect(T padding, T aspect_ratio) const noexcept {
        Rect result;

        result.x_ = padding;
        result.width_ = width_ - padding * 2;
        result.height_ = result.width_ * aspect_ratio;
        result.y_ = -(result.height_ - height_) / 2;

        if (result.y_ < padding && aspect_ratio != 0) {
            result.y_ = padding;
            result.height_ = height_ - padding * 2;
            result.width_ = result.height_ / aspect_ratio;
            result.x_ = -(result.width_ - width_) / 2;
        }

        return result;
    }


    void zero() noexcept { x_ = y_ = width_ = height_ = 0; }

    void set(T x, T y, T width, T height) noexcept { x_ = x; y_ = y; width_ = width; height_ = height; }
    void set(T x, T y, T size) noexcept { x_ = x; y_ = y; width_ = height_ = size;}
    void set(const Rect& r) noexcept { x_ = r.x_; y_ = r.y_; width_ = r.width_; height_ = r.height_; }
    void set(T width, T height) noexcept { x_ = y_ = 0; width_ = width; height_ = height; }

    void set(const Vec2<T>& center, T radius) noexcept {
        x_ = center.x_ - radius;
        y_ = center.y_ - radius;
        width_ = height_ = radius * 2;
    }

    void setWidth(T width) noexcept { width_ = width; }
    void setWidthFromCenter(T width) noexcept { x_ += (width_ - width) * 0.5; width_ = width; }
    void setWidthFromMax(T width) noexcept { x_ += width_ - width; width_ = width; }

    void setHeight(T height) noexcept { height_ = height; }
    void setHeightFromCenter(T height) noexcept { y_ += (height_ - height) * 0.5; height_ = height; }
    void setHeightFromMax(T height) noexcept { y_ += height_ - height; height_ = height; }

    void setPos(const Vec2<T>& pos) noexcept { x_ = pos.x_; y_ = pos.y_; }
    void setPos(T x, T y) noexcept { x_ = x; y_ = y; }
    void setPos2(const Vec2<T>& pos) { width_ = pos.x_ - x_; height_ = pos.y_ - y_; }
    void setPos2(T x, T y) noexcept { width_ = x - x_; height_ = y - y_; }

    void setSize(T size) noexcept { width_ = size; height_ = size; }
    void setSize(T width, T height) noexcept { width_ = width; height_ = height; }
    void setSizeFromCenter(T width, T height) noexcept { setWidthFromCenter(width); setHeightFromCenter(height); }


    void moveLeft() noexcept { x_ -= width_; }
    void moveRight() noexcept { x_ += width_; }
    void moveUp() noexcept { y_ -= height_; }
    void moveDown() noexcept { y_ += height_; }

    void inset(T size) noexcept {
        x_ += size;
        y_ += size;
        width_ -= size * 2;
        height_ -= size * 2;
    }

    void inset(T h, T v) noexcept {
        x_ += h;
        y_ += v;
        width_ -= h * 2;
        height_ -= v * 2;
    }

    void inset(T top, T right, T bottom, T left) noexcept {
        x_ += left;
        y_ += top;
        width_ -= left + right;
        height_ -= top + bottom;
    }

    void inset(const Border<T>& border) noexcept {
        x_ += border.left_;
        y_ += border.top_;
        width_ -= border.left_ + border.right_;
        height_ -= border.top_ + border.bottom_;
    }

    template<typename U>
    void inset(const Border<U>& border) noexcept
        requires std::is_arithmetic_v<U> {
        x_ += static_cast<T>(border.left_);
        y_ += static_cast<T>(border.top_);
        width_ -= static_cast<T>(border.left_ + border.right_);
        height_ -= static_cast<T>(border.top_ + border.bottom_);
    }

    void insetLeft(T size) noexcept { x_ += size; width_ -= size; }
    void insetRight(T size) noexcept { width_ -= size; }
    void insetTop(T size) noexcept { y_ += size; height_ -= size; }
    void insetBottom(T size) noexcept { height_ -= size; }

    void insetHorizontal(T size) noexcept { x_ += size; width_ -= size * 2; }
    void insetVertical(T size) noexcept { y_ += size; height_ -= size * 2; }

    void expand(T size) noexcept {
        x_ -= size;
        y_ -= size;
        width_ += size * 2;
        height_ += size * 2;

    }

    void expandToFit(const Rect& rect) noexcept {
        if (rect.x2() > width_) {
            width_ = rect.x2();
        }
        if (rect.y2() > height_) {
            height_ = rect.y2();
        }
    }

    void addWidthFromMax(T width) noexcept {
        T new_width = width_ + width;
        x_ += width_ - new_width;
        width_ = new_width;
    }

    void addHeightFromMax(T height) noexcept {
        T new_height = height_ + height;
        y_ += height_ - new_height;
        height_ = new_height;
    }

    void roundValues() noexcept {
        x_ = std::round(x_);
        y_ = std::round(y_);
        width_ = std::round(width_);
        height_ = std::round(height_);
    }

    void translateX(T tx) noexcept { x_ += tx; }
    void translateY(T ty) noexcept { y_ += ty; }
    void translate(T tx, T ty) noexcept { x_ += tx; y_ += ty; }
    void translate(const Vec2<T>& t) noexcept { x_ += t.x_; y_ += t.y_; }

    void flipVertical() noexcept { y_ = -y_ - height_; }

    void scale(T scale) noexcept { x_ *= scale; y_ *= scale; width_ *= scale; height_ *= scale; }
    void scale(T sx, T sy) noexcept { x_ *= sx; y_ *= sy; width_ *= sx; height_ *= sy; }
    void scaleSize(T scale) noexcept { width_ *= scale; height_ *= scale; }
    void scaleSize(T sx, T sy) noexcept { width_ *= sx; height_ *= sy; }
    void scaleWidth(T scale) noexcept { width_ *= scale; }
    void scaleHeight(T scale) noexcept { height_ *= scale; }

    void scaleCentered(T scale) noexcept {
        double new_width = width_ * scale;
        double new_height = height_ * scale;
        x_ -= 0.5 * (new_width - width_);
        y_ -= 0.5 * (new_height - height_);
        width_ = new_width;
        height_ = new_height;
    }

    /**
     *  @brief Aligns the current rectangle within another rectangle based on
     *         a given alignment.
     *
     *  This function updates the position of the current rectangle
     *  (`x_` and `y_`) to align it within the provided rectangle `rect` based
     *  on the specified alignment type.
     *
     *  @param alignment The alignment type to use (e.g., Left, TopLeft, Center, etc.).
     *  @param rect The rectangle within which to align the current rectangle.
     */
    void alignInRect(Alignment alignment, const Rect& rect) noexcept {
        switch (alignment) {
            case Alignment::Left:
            case Alignment::TopLeft:
            case Alignment::BottomLeft:
                x_ = rect.x_;
                break;
            case Alignment::Right:
            case Alignment::TopRight:
            case Alignment::BottomRight:
                x_ = rect.x2() - width_;
                break;
            default:
                x_ = rect.centerX() - width_ / 2;
                break;
        }

        switch (alignment) {
            case Alignment::Top:
            case Alignment::TopLeft:
            case Alignment::TopRight:
                y_ = rect.y_;
                break;
            case Alignment::Bottom:
            case Alignment::BottomLeft:
            case Alignment::BottomRight:
                y_ = rect.y2() - height_;
                break;
            default:
                y_ = rect.centerY() - height_ / 2;
                break;
        }
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
    }

    void avoidNegativeSize() noexcept {
        if (width_ < 0) {
            width_ = 0;
        }
        if (height_ < 0) {
            height_ = 0;
        }
    }

    void makeValidForArea(T width, T height) noexcept {
        // TODO: Test!!!!!
        if (x_ < 0) { width += x_; x_ = 0; }
        if (y_ < 0) { height += y_; y_ = 0; }
        if (x_ >= width) width = 0;
        if (y_ >= height) height = 0;
        if (x_ > 0) width -= x_;
        if (y_ > 0) height -= y_;
    }


    /**
     *  @brief Checks if Rect contains a given position.
     *
     *  This function determines whether the specified position is within the
     *  boundaries of the rectangle represented by this object.
     *
     *  @param pos The position to check for containment within the rectangle.
     *  @return `true` if the specified position is contained within the
     *          rectangle, `false` otherwise.
     */
    [[nodiscard]] bool contains(const Vec2<T>& pos) const noexcept {
        return (pos.x_ >= x_ && pos.x_ < x_ + width_ && pos.y_ >= y_ && pos.y_ < y_ + height_);
    }

    [[nodiscard]] bool contains(const Vec2<T>& pos, T tolerance) const noexcept {
        return (pos.x_ >= x_ - tolerance && pos.x_ < x_ + width_ + tolerance &&
                pos.y_ >= y_ - tolerance && pos.y_ < y_ + height_ + tolerance);
    }

    [[nodiscard]] bool containsX(const T x) const noexcept {
        return (x >= x_ && x < x_ + width_);
    }

    [[nodiscard]] bool containsY(const T y) const noexcept {
        return (y >= y_ && y < y_ + height_);
    }


    /**
     *  @brief Checks if Rect overlaps with another rectangle.
     *
     *  This function determines whether the rectangle represented by this object
     *  overlaps with the specified rectangle.
     *
     *  @param rect The rectangle to check for overlap with.
     *  @return `true` if this rectangle overlaps with the specified rectangle,
     *          `false` otherwise.
     */
    [[nodiscard]] bool overlaps(const Rect& rect) const noexcept {
        int x1 = std::max<T>(x_, rect.x_);
        int y1 = std::max<T>(y_, rect.y_);
        int x2 = std::min<T>(x_ + width_, rect.x_ + rect.width_);
        int y2 = std::min<T>(y_ + height_, rect.y_ + rect.height_);
        return x1 < x2 && y1 < y2;
    }

    [[nodiscard]] bool hitCircle(const Vec2<T>& center, T radius) const noexcept {
        // Find the closest point to the circle within the rectangle
        T closest_x = std::clamp(center.x_, x_, x2());
        T closest_y = std::clamp(center.y_, y_, y2());

        // Calculate the distance between the circle's center and this closest point
        T distance_x = center.x_ - closest_x;
        T distance_y = center.y_ - closest_y;

        // If the distance is less than the circle's radius, an intersection occurs
        T distance_squared = (distance_x * distance_x) + (distance_y * distance_y);

        return distance_squared < (radius * radius);
    }

    [[nodiscard]] double minXDistance(double x) const noexcept {
        double left = x < x_ ? x_ - x : std::numeric_limits<double>::max();
        double right = x > x2() ? x - x2() : std::numeric_limits<double>::max();
        return left < right ? left : right;
    }

    [[nodiscard]] double minYDistance(double y) const noexcept {
        double top = y < y_ ? y_ - y : std::numeric_limits<double>::max();
        double bottom = y > y2() ? y - y2() : std::numeric_limits<double>::max();
        return top < bottom ? top : bottom;
    }


    /**
     *  @brief Updates this rectangle to the intersection of itself with another
     *         rectangle.
     *
     *  This function modifies the current rectangle to represent the
     *  intersection of itself with the specified rectangle. If the rectangles
     *  do not intersect, the dimensions of the current rectangle will be set
     *  to zero.
     *
     *  @param rect The rectangle to intersect with.
     *  @return `true` if the rectangles intersect and the currentcurrent rectangle is
     *          updated, `false` otherwise.
     */
    bool intersect(const Rect& rect) noexcept {
        T x2 = this->x2();
        T y2 = this->y2();
        T rx2 = rect.x2();
        T ry2 = rect.y2();

        if (rect.x_ > x_) { x_ = rect.x_; }
        if (rx2 < x2) { x2 = rx2; }
        if (rect.y_ > y_) { y_ = rect.y_; }
        if (ry2 < y2) { y2 = ry2; }

        width_ = x2 - x_;
        height_ = y2 - y_;

        if (width_ <= 0 || height_ <= 0) {
            width_ = 0;
            height_ = 0;
            return false;
        }
        else {
            return true;
        }
    }

    void clampVec2(Vec2<T>& v) const noexcept {
        T x2 = this->x2();
        T y2 = this->y2();
        if (v.x_ < x_) { v.x_ = x_; } else if (v.x_ > x2) { v.x_ = x2; }
        if (v.y_ < y_) { v.y_ = y_; } else if (v.y_ > y2) { v.y_ = y2; }
    }


    /**
     *  @brief Checks if a point lies inside the specified clipping edge.
     *
     *  @param v The point to test.
     *  @param edge The clipping edge to test against.
     *  @return `true? if the point is inside the clipping boundary, `false`
     *          otherwise.
     */
    [[nodiscard]] bool insideEdge(const Vec2<T>& v, Edge edge) {
        switch (edge) {
            case Edge::Left: return v.x_ >= x_;
            case Edge::Right: return v.x_ <= x_ + width_;
            case Edge::Top: return v.y_ >= y_;
            case Edge::Bottom: return v.y_ <= y_ + height_;
        }
        return false;
    }

    /**
     *  @brief Gets the intersection point of a line segment with a clipping edge.
     *
     *  @param a The starting point of the segment.
     *  @param b The ending point of the segment.
     *  @param edge The clipping edge to intersect with.
     *  @return Vec2d The intersection point.
     */
    [[nodiscard]] Vec2d intersect(const Vec2<T>& a, const Vec2<T>& b, Edge edge) {
        double dx = b.x_ - a.x_;
        double dy = b.y_ - a.y_;

        switch (edge) {
            case Edge::Left: {
                double x = x_;
                double t = (x - a.x_) / dx;
                return Vec2d(x, a.y_ + t * dy);
            }
            case Edge::Right: {
                double x = x_ + width_;
                double t = (x - a.x_) / dx;
                return Vec2d(x, a.y_ + t * dy);
            }
            case Edge::Top: {
                double y = y_;
                double t = (y - a.y_) / dy;
                return Vec2d(a.x_ + t * dx, y);
            }
            case Edge::Bottom: {
                double y = y_ + height_;
                double t = (y - a.y_) / dy;
                return Vec2d(a.x_ + t * dx, y);
            }
        }

        return Vec2<T>(0, 0);  // Fallback
    }

    /**
     *  @brief Clips a polygon edge-by-edge against a single boundary of a
     *         rectangular clipping region.
     *
     *  This function is a subroutine of the Sutherland-Hodgman polygon clipping
     *  algorithm. It clips the given polygon (described by its vertices)
     *  against one of the four rectangle edges (left, right, top, or bottom)
     *  and outputs the resulting polygon segment.
     *
     *  @param[in] vertices Pointer to the array of input polygon vertices.
     *  @param[in] vertex_n The number of vertices in the input polygon.
     *  @param[in] edge The edge of the clipping rectangle to clip against
     *                  (e.g., Edge::Left).
     *  @param[out] out_vertices Pointer to the array where the output vertices
     *                           will be written. This array must be large
     *                           enough to store the clipped vertices.
     *  @return int32_t The number of vertices in the resulting clipped polygon
     *                  segment.
     *
     *  @note This function assumes that the input polygon is convex or already
     *        clipped from previous edges, and operates in-place edge-by-edge.
     *
     *  @see inside(), intersect(), clipPolygonToRect()
     */
    int32_t clipAgainstEdge(const Vec2<T>* vertices, int32_t vertex_n, Edge edge, Vec2<T>* out_vertices) {
        int32_t out_count = 0;
        Vec2<T> prev = vertices[vertex_n - 1];
        bool prev_inside = inside(prev, edge);

        for (auto i = 0; i < vertex_n; i++) {
            Vec2<T> curr = vertices[i];
            bool curr_inside = inside(curr, edge);

            if (curr_inside) {
                if (!prev_inside) {
                    out_vertices[out_count++] = intersect(prev, curr, edge);
                }
                out_vertices[out_count++] = curr;
            }
            else if (prev_inside) {
                out_vertices[out_count++] = intersect(prev, curr, edge);
            }

            prev = curr;
            prev_inside = curr_inside;
        }

        return out_count;
    }

    /**
     *  @brief Clips a polygon to a rectangular region using the
     *         Sutherland-Hodgman algorithm.
     *
     *  This function takes a polygon defined by an array of vertices and clips
     *  it against a rectangular clipping area. It uses the Sutherland-Hodgman
     *  algorithm, processing the polygon against each edge of the rectangle
     *  (left, right, top, bottom) sequentially.
     *
     *  @param[in] vertices Pointer to the array of input polygon vertices.
     *  @param[in] vertex_n Number of vertices in the input polygon.
     *  @param[out] out_vertices Pointer to the output array where the clipped
     *                           polygon vertices will be stored. This array
     *                           must be large enough to hold the resulting
     *                           vertices.
     *  @return int32_t The number of vertices in the resulting clipped polygon.
     *
     *  @note The function uses static buffers internally with a maximum of
     *        256 vertices. If the input exceeds this number, behavior is
     *        undefined.
     *
     *  @see clipAgainstEdge()
     */
    int32_t clipPolygonToRect(const Vec2<T>* vertices, int32_t vertex_n, Vec2<T>* out_vertices) {
        static constexpr int32_t kMax = 256;
        Vec2<T> buffer1[kMax];
        Vec2<T> buffer2[kMax];

        // TODO: Check what to do about `kMax`

        int32_t n = vertex_n;
        std::memcpy(buffer1, vertices, vertex_n * sizeof(Vec2<T>));

        n = clipAgainstEdge(buffer1, n, buffer2, Edge::Left);
        n = clipAgainstEdge(buffer2, n, buffer1, Edge::Right);
        n = clipAgainstEdge(buffer1, n, buffer2, Edge::Top);
        n = clipAgainstEdge(buffer2, n, out_vertices, Edge::Bottom);

        return n;
    }


#if defined(__APPLE__) && defined(__MACH__)
    [[nodiscard]] CGRect cgRect() const noexcept {
        double rx = width_ < 0 ? x_ + width_ : x_;
        double ry = height_ < 0 ? y_ + height_ : y_;
        return CGRectMake(rx, ry, std::fabs(width_), std::fabs(height_));
    }
#endif

};


// Standard types
using Recti = Rect<int32_t>;    ///< 32 bit integer
using Rectl = Rect<int64_t>;    ///< 64 bit integer
using Rectf = Rect<float>;      ///< 32 bit floating point
using Rectd = Rect<double>;     ///< 64 bit floating point


class RectFix {
public:
    Fix x_ = 0;
    Fix y_ = 0;
    Fix width_ = 1;
    Fix height_ = 1;

public:
    RectFix() noexcept = default;
    explicit RectFix(Fix size) noexcept : width_(size), height_(size) {}
    explicit RectFix(Fix width, Fix height) noexcept : width_(width), height_(height) {}
    explicit RectFix(Fix x, Fix y, Fix width, Fix height) noexcept : x_(x), y_(y), width_(width), height_(height) {}

    [[nodiscard]] virtual const char* className() const noexcept { return "RectFix"; }

    friend std::ostream& operator << (std::ostream& os, const RectFix* o) {
        o == nullptr ? os << "RectFix nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const RectFix& o) {
        os << o.x_ << ", " << o.y_ << " | " << o.width_ << " x " << o.height_;
        return os;
    }

    // Operator overloading

    RectFix& operator = (const Rect<int32_t>& r) {
        x_ = r.x_; y_ = r.y_; width_ = r.width_; height_ = r.height_; return *this;
    }

    RectFix& operator = (const Rect<float>& r) {
        x_ = r.x_; y_ = r.y_; width_ = r.width_; height_ = r.height_; return *this;
    }

    RectFix& operator = (const Rect<double>& r) {
        x_ = r.x_; y_ = r.y_; width_ = r.width_; height_ = r.height_; return *this;
    }

    bool operator == (const RectFix& other) const {
        return x_ == other.x_ && y_ == other.y_ && width_ == other.width_ && height_ == other.height_;
    }

    bool operator != (const RectFix& other) const {
        return x_ != other.x_ || y_ != other.y_ || width_ != other.width_ || height_ != other.height_;
    }

    RectFix operator + (const Vec2Fix& v) const { return RectFix(x_ + v.x_, y_ + v.y_, width_, height_); }
    RectFix operator - (const Vec2Fix& v) const { return RectFix(x_ - v.x_, y_ - v.y_, width_, height_); }
    RectFix operator * (const Vec2Fix& v) const { return RectFix(x_ * v.x_, y_ * v.y_, width_, height_); }

    RectFix& operator += (const Vec2Fix& v) { x_ += v.x_; y_ += v.y_; return *this; }
    RectFix& operator -= (const Vec2Fix& v) { x_ -= v.x_; y_ -= v.y_; return *this; }
    RectFix& operator *= (const Vec2Fix& v) { x_ *= v.x_; y_ *= v.y_; return *this; }

    RectFix& operator += (const RectFix& other) {
        RectFix result;
        Fix min_x = x_ < other.x_ ? x_ : other.x_;
        Fix min_y = y_ < other.y_ ? y_ : other.y_;
        Fix max_x = x2() > other.x2() ? x2() : other.x2();
        Fix max_y = y2() > other.y2() ? y2() : other.y2();
        x_ = min_x; y_ = min_y; width_ = max_x - min_x; height_ = max_y - min_y;
        return *this;
    }


    [[nodiscard]] Fix x() const noexcept { return x_; }
    [[nodiscard]] Fix y() const noexcept { return y_; }
    [[nodiscard]] Fix x2() const noexcept { return x_ + width_; }
    [[nodiscard]] Fix y2() const noexcept { return y_ + height_; }
    [[nodiscard]] Fix width() const noexcept { return width_; }
    [[nodiscard]] Fix height() const noexcept { return height_; }
    [[nodiscard]] Fix shortSide() const noexcept { return width_ < height_ ? width_ : height_; }
    [[nodiscard]] Fix longSide() const noexcept { return width_ > height_ ? width_ : height_; }

    [[nodiscard]] bool usable() const noexcept { return width_ > 0 && height_ > 0; }
    [[nodiscard]] bool isHorizontal() const noexcept { return width_ > height_; }
    [[nodiscard]] bool isVertical() const noexcept { return height_ > width_; }

    [[nodiscard]] Fix centerX() const noexcept { return x_ + width_ / 2; }
    [[nodiscard]] Fix centerY() const noexcept { return y_ + height_ / 2; }
    [[nodiscard]] Vec2Fix center() const noexcept { return { x_ + width_ / 2, y_ + height_ / 2 }; }
    [[nodiscard]] Fix radius() const noexcept { return shortSide() / 2; }
    [[nodiscard]] double circumcircleRadius() const {
        double w = width_.asDouble();
        double h = height_.asDouble();
        return std::sqrt(w * w + h * h) / 2;
    }


    void zero() noexcept { x_ = y_ = width_ = height_ = 0; }

    void set(const Fix& x, const Fix& y, const Fix& width, const Fix& height) noexcept {
        x_ = x; y_ = y; width_ = width; height_ = height;
    }
    void set(const Fix& x, const Fix& y, const Fix& size) noexcept {
        x_ = x; y_ = y; width_ = height_ = size;
    }
    void set(const RectFix& r) noexcept {
        x_ = r.x_; y_ = r.y_; width_ = r.width_; height_ = r.height_;
    }
    void set(const Fix& width, const Fix& height) noexcept {
        x_ = y_ = 0; width_ = width; height_ = height;
    }

    void set(const Vec2Fix& center, const Fix& radius) noexcept {
        x_ = center.x_ - radius;
        y_ = center.y_ - radius;
        width_ = height_ = radius * 2;
    }

    void setWidth(const Fix& width) noexcept { width_ = width; }
    void setWidthFromCenter(const Fix& width) noexcept { x_ += (width_ - width) * 0.5; width_ = width; }
    void setWidthFromMax(const Fix& width) noexcept { x_ += width_ - width; width_ = width; }

    void setHeight(const Fix& height) noexcept { height_ = height; }
    void setHeightFromCenter(const Fix& height) noexcept { y_ += (height_ - height) * 0.5; height_ = height; }
    void setHeightFromMax(const Fix& height) noexcept { y_ += height_ - height; height_ = height; }

    void setPos(const Vec2Fix& pos) noexcept { x_ = pos.x_; y_ = pos.y_; }
    void setPos(const Fix& x, const Fix& y) noexcept { x_ = x; y_ = y; }
    void setPos2(const Vec2Fix& pos) { width_ = pos.x_ - x_; height_ = pos.y_ - y_; }
    void setPos2(const Fix& x, const Fix& y) noexcept { width_ = x - x_; height_ = y - y_; }

    void setSize(const Fix& size) noexcept { width_ = size; height_ = size; }
    void setSize(const Fix& width, const Fix& height) noexcept { width_ = width; height_ = height; }
    void setSizeFromCenter(const Fix& width, const Fix& height) noexcept { setWidthFromCenter(width); setHeightFromCenter(height); }


    void moveLeft() noexcept { x_ -= width_; }
    void moveRight() noexcept { x_ += width_; }
    void moveUp() noexcept { y_ -= height_; }
    void moveDown() noexcept { y_ += height_; }

    void inset(const Fix& size) noexcept {
        x_ += size;
        y_ += size;
        width_ -= size * 2;
        height_ -= size * 2;
    }

    void inset(const Fix& top, const Fix& right, const Fix& bottom, const Fix& left) noexcept {
        x_ += left;
        y_ += top;
        width_ -= left + right;
        height_ -= top + bottom;
    }

    void insetLeft(const Fix& size) noexcept { x_ += size; width_ -= size; }
    void insetRight(const Fix& size) noexcept { width_ -= size; }
    void insetTop(const Fix& size) noexcept { y_ += size; height_ -= size; }
    void insetBottom(const Fix& size) noexcept { height_ -= size; }

    void insetFromCenter(const Fix& x_size, const Fix& y_size) noexcept {
        x_ += x_size;
        width_ -= x_size * 2;
        y_ += y_size;
        height_ -= y_size * 2;
    }

    void insetHorizontalFromCenter(const Fix& size) noexcept { x_ += size; width_ -= size * 2; }
    void insetVerticalFromCenter(const Fix& size) noexcept { y_ += size; height_ -= size * 2; }

    void expand(const Fix& size) noexcept {
        x_ -= size;
        y_ -= size;
        width_ += size * 2;
        height_ += size * 2;

    }

    void expandToFit(const RectFix& rect) noexcept {
        if (rect.x2() > width_) {
            width_ = rect.x2();
        }
        if (rect.y2() > height_) {
            height_ = rect.y2();
        }
    }

    void addWidthFromMax(const Fix& width) noexcept {
        Fix new_width = width_ + width;
        x_ += width_ - new_width;
        width_ = new_width;
    }

    void addHeightFromMax(const Fix& height) noexcept {
        Fix new_height = height_ + height;
        y_ += height_ - new_height;
        height_ = new_height;
    }

    void translateX(const Fix& tx) noexcept { x_ += tx; }
    void translateY(const Fix& ty) noexcept { y_ += ty; }
    void translate(const Fix& tx, const Fix& ty) noexcept { x_ += tx; y_ += ty; }
    void translate(const Vec2Fix& t) noexcept { x_ += t.x_; y_ += t.y_; }

    void flipVertical() noexcept { y_ = -y_ - height_; }

    void scale(const Fix& scale) noexcept { width_ *= scale; height_ *= scale; }
    void scaleWidth(const Fix& scale) noexcept { width_ *= scale; }
    void scaleHeight(const Fix& scale) noexcept { height_ *= scale; }

    void scaleCentered(const Fix& scale) noexcept {
        Fix new_width = width_ * scale;
        Fix new_height = height_ * scale;
        x_ -= (new_width - width_) / 2;
        y_ -= (new_height - height_) / 2;
        width_ = new_width;
        height_ = new_height;
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
    }

    void avoidNegativeSize() noexcept {
        if (width_ < 0) {
            width_ = 0;
        }
        if (height_ < 0) {
            height_ = 0;
        }
    }
};


template <class T>
class RemapRect {
public:
    RemapRect() noexcept = default;
    explicit RemapRect(const Rect<T>& src_rect, const Rect<T>& dst_rect, bool flip_y = false) noexcept {
        set(src_rect, dst_rect, flip_y);
    }
    virtual ~RemapRect() noexcept = default;


    [[nodiscard]] virtual const char *className() const noexcept { return "RemapRect"; }

    friend std::ostream& operator << (std::ostream& os, const RemapRect* o) {
        o == nullptr ? os << "RemapRect nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const RemapRect& o) {
        o.log(os, 0, "RemapRect");
        return os;
    }

    void log(std::ostream& os, int32_t indent = 0, const char* label = nullptr) const {
        /* TODO: !!!!! */
    }

    void set(const Rect<T>& src_rect, const Rect<T>& dst_rect, bool flip_y = false) noexcept {
        src_x_ = src_rect.x_;
        src_y_ = src_rect.y_;
        src_width_ = src_rect.width_;
        src_height_ = src_rect.height_;
        dst_x_ = dst_rect.x_;
        dst_y_ = dst_rect.y_;
        dst_width_ = dst_rect.width_;
        dst_height_ = dst_rect.height_;

        // Update aspect ratio
        src_x_a_ = static_cast<double>(src_width_) != 0.0 ? (static_cast<double>(dst_width_) / src_width_) : 1.0;
        if (flip_y) {
            // Flip the Y-axis mapping
            src_y_a_ = static_cast<double>(src_height_) != 0.0 ? -(static_cast<double>(dst_height_) / src_height_) : 1.0;
            dst_y_ += dst_height_;  // Offset Y to account for the flipped origin
        }
        else {
            src_y_a_ = static_cast<double>(src_height_) != 0.0 ? (static_cast<double>(dst_height_) / src_height_) : 1.0;
        }
    }

    void invert() noexcept {
        Rect<T> src_rect = { dst_x_, dst_y_, dst_width_, dst_height_ };
        Rect<T> dst_rect = { src_x_, src_y_, src_width_, src_height_ };
        set(src_rect, dst_rect);
    }


    [[nodiscard]] T mapX(T x) const noexcept {
        return (x - src_x_) * src_x_a_ + dst_x_;
    }

    [[nodiscard]] T inverseMapX(T x) const noexcept {
        return (x - dst_x_) / src_x_a_ + src_x_;
    }

    [[nodiscard]] T mapY(T y) const noexcept {
        return (y - src_y_) * src_y_a_ + dst_y_;
    }

    [[nodiscard]] T inverseMapY(T y) const noexcept {
        return (y - dst_y_) / src_y_a_ + src_y_;
    }

    void mapVec2(Vec2<T>& v) const noexcept {
        v.x_ = (v.x_ - src_x_) * src_x_a_ + dst_x_;
        v.y_ = (v.y_ - src_y_) * src_y_a_ + dst_y_;
    }

    void inverseMapVec2(const Vec2<T>& v) const noexcept {
        v.x_ = (v.x_ - dst_x_) / src_x_a_ + src_x_;
        v.y_ = (v.y_ - dst_y_) / src_y_a_ + src_y_;
    }

    void mapVec2(const Vec2<T>& v, Vec2<T>& out_v) const noexcept {
        out_v.x_ = (v.x_ - src_x_) * src_x_a_ + dst_x_;
        out_v.y_ = (v.y_ - src_y_) * src_y_a_ + dst_y_;
    }

    void inverseMapVec2(const Vec2<T>& v, Vec2<T>& out_v) const noexcept {
        out_v.x_ = (v.x_ - dst_x_) / src_x_a_ + src_x_;
        out_v.y_ = (v.y_ - dst_y_) / src_y_a_ + src_y_;
    }

    void mapVec2(Vec2<T>* v) const noexcept {
        if (v) {
            v->x_ = (v->x_ - src_x_) * src_x_a_ + dst_x_;
            v->y_ = (v->y_ - src_y_) * src_y_a_ + dst_y_;
        }
    }

    void inverseMapVec2(Vec2<T>* v) const noexcept {
        if (v) {
            v->x_ = (v->x_ - dst_x_) / src_x_a_ + src_x_;
            v->y_ = (v->y_ - dst_y_) / src_y_a_ + src_y_;
        }
    }

    void mapRect(Rect<T>& r) const noexcept {
        return mapRect(&r);
    }

    void inverseMapRect(Rect<T>& r) const noexcept {
        return inverseMapRect(&r);
    }

    void mapRect(Rect<T>* r) const noexcept {
        if (r) {
            Vec2<T> p1(r->x_, r->y_);
            Vec2<T> p2(r->x_ + r->width_, r->y_ + r->height_);
            mapVec2(p1);
            mapVec2(p2);
            r->x_ = p1.x_;
            r->y_ = p1.y_;
            r->width_ = p2.x_ - p1.x_;
            r->height_ = p2.y_ - p1.y_;
        }
    }

    void inverseMapRect(Rect<T>* r) const noexcept {
        if (r) {
            Vec2<T> p1(r->x_, r->y_);
            Vec2<T> p2(r->x_ + r->width_, r->y_ + r->height_);
            inverseMapVec2(p1);
            inverseMapVec2(p2);
            r->x_ = p1.x_;
            r->y_ = p1.y_;
            r->width_ = p2.x_ - p1.x;
            r->height_ = p2.y_ - p1.y_;
        }
    }

    RemapRect inverse() const noexcept {
        Rect<T> src_rect = { dst_x_, dst_y_, dst_width_, dst_height_ };
        Rect<T> dst_rect = { src_x_, src_y_, src_width_, src_height_ };
        return RemapRect(src_rect, dst_rect);
    }

protected:
    T src_x_{};
    T src_y_{};
    T src_width_{};
    T src_height_{};
    T dst_x_{};
    T dst_y_{};
    T dst_width_{};
    T dst_height_{};
    double src_x_a_{};     ///< Aspect ratio of source and destination rect in x direction
    double src_y_a_{};     ///< Aspect ratio of source and destination rect in y direction
};


// Standard types
using RemapRectf = RemapRect<float>;    ///< 32 bit floating point
using RemapRectd = RemapRect<double>;   ///< 64 bit floating point


} // End of namespace Grain

#endif // GrainRect_hpp
