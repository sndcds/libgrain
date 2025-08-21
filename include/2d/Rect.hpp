//
//  Rect.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 12.07.2025
//

//  TODO: Implement methods from olcUTIL_Geometry2D/olcUTIL_Geometry2D.h
//  https://github.com/OneLoneCoder/olcUTIL_Geometry2D/blob/main/olcUTIL_Geometry2D.h


#ifndef GrainRect_hpp
#define GrainRect_hpp

#include "Type/Type.hpp"
#include "Geometry.hpp"
#include "Type/Fix.hpp"
#include "Math/Vec2.hpp"
#include "Math/Vec2Fix.hpp"
#include "RectEdges.hpp"
#include "Math/Random.hpp"

#if defined(__APPLE__) && defined(__MACH__)
    #include <CoreGraphics/CoreGraphics.h>
#endif


namespace Grain {


    /**
     *  @brief Rectangle.
     *
     *  `Rect` represents a two-dimensional rectangle.
     *  It is defined by its position (x and y), width, and height. The class provides essential
     *  functionalities for working with rectangular areas, including calculating area, checking
     *  for intersections with other rectangles, and more.
     *
     *  This templated class supports various data types and can be used for different
     *  numerical representations.
     *  Predefined specialized versions include datatypes `int32_t`, `int64_t`, `float`
     *  and `double`, named as `Recti`, `Rectl`, `Rectf` and `Rectd` respectively.
     *
     *  @note This class is often used in graphics applications to represent regions on a screen
     *        or within an image.
     */
    template <class T>
    class Rect {

    public:
        T m_x{};        ///< X-coordinate of the rectangle's origin (typically the left edge)
        T m_y{};        ///< Y-coordinate of the rectangle's origin (typically the bottom or top edge depending on convention)
        T m_width{};    ///< Width of the rectangle
        T m_height{};   ///< Height of the rectangle

    public:
        /**
         *  @brief Initializes a rectangle at (0, 0) with a default size of 1×1.
         */
        Rect() noexcept : m_width(1), m_height(1) {}

        /**
         *  @brief Fully initializes a rectangle with position (x, y) and dimensions (width, height).
         */
        Rect(T x, T y, T width, T height) noexcept : m_x(x), m_y(y), m_width(width), m_height(height) {}

        /**
         *  @brief Creates a square rectangle with both width and height set to size.
         */
        explicit Rect(T size) noexcept : m_width(size), m_height(size) {}

        /**
         *  @brief Initializes a rectangle at (0, 0) with the specified width and height.
         */
        explicit Rect(T width, T height) noexcept : m_width(width), m_height(height) {}

        /**
         *  @brief Copies a rectangle and applies an inset by inset_size on all sides.
         */
        explicit Rect(const Rect& r, T inset_size) noexcept { *this = r; inset(inset_size); }

        /**
         *  @brief Creates a square centered at center with sides of length radius × 2.
         */
        explicit Rect(const Vec2<T>& center, T radius) noexcept {
            m_x = center.m_x - radius;
            m_y = center.m_y - radius;
            m_width = m_height = radius + radius;  // radius * 2
        }

        /**
         *  @brief Constructs a rectangle centered at center with the given dimensions.
         */
        explicit Rect(const Vec2<T>& center, T width, T height) noexcept {
            m_x = center.m_x - width / 2;
            m_y = center.m_y - height / 2;
            m_width = width;
            m_height = height;
        }

        #if defined(__APPLE__) && defined(__MACH__)
            /**
             *  @brief Converts a Core Graphics CGRect to a Rect instance.
             */
            explicit Rect(const CGRect& r) noexcept {
                m_x = r.origin.x;
                m_y = r.origin.y;
                m_width = r.size.width;
                m_height = r.size.height;
            }
        #endif

        [[nodiscard]] virtual const char* className() const noexcept { return "Rect"; }

        friend std::ostream& operator << (std::ostream& os, const Rect* o) {
            o == nullptr ? os << "Rect nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const Rect& o) {
            os << o.m_x << ", " << o.m_y << ", " << o.m_width << ", " << o.m_height;
            return os;
        }


        // Operator overloading

        template<typename U>
        requires std::is_convertible_v<U, T>
        Rect<T>& operator = (const Rect<U>& other) {
            m_x = static_cast<T>(other.m_x);
            m_y = static_cast<T>(other.m_y);
            m_width = static_cast<T>(other.m_width);
            m_height = static_cast<T>(other.m_height);
            return *this;
        }

        #if defined(__APPLE__) && defined(__MACH__)
            Rect& operator = (const CGRect& r) {
                m_x = r.origin.x; m_y = r.origin.y; m_width = r.size.width; m_height = r.size.height; return *this;
            }
        #endif


        bool operator == (const Rect& other) const {
            return m_x == other.m_x && m_y == other.m_y && m_width == other.m_width && m_height == other.m_height;
        }

        bool operator != (const Rect& other) const {
            return m_x != other.m_x || m_y != other.m_y || m_width != other.m_width || m_height != other.m_height;
        }

        Rect operator + (const Vec2<T>& v) const { return Rect(m_x + v.m_x, m_y + v.m_y, m_width, m_height); }
        Rect operator - (const Vec2<T>& v) const { return Rect(m_x - v.m_x, m_y - v.m_y, m_width, m_height); }
        Rect operator * (const Vec2<T>& v) const { return Rect(m_x * v.m_x, m_y * v.m_y, m_width, m_height); }

        Rect& operator += (const Vec2<T>& v) { m_x += v.m_x; m_y += v.m_y; return *this; }
        Rect& operator -= (const Vec2<T>& v) { m_x -= v.m_x; m_y -= v.m_y; return *this; }
        Rect& operator *= (const Vec2<T>& v) { m_x *= v.m_x; m_y *= v.m_y; return *this; }

        Rect& operator += (const Rect& other) {
            Rect result;
            T min_x = m_x < other.m_x ? m_x : other.m_x;
            T min_y = m_y < other.m_y ? m_y : other.m_y;
            T max_x = x2() > other.x2() ? x2() : other.x2();
            T max_y = y2() > other.y2() ? y2() : other.y2();
            m_x = min_x; m_y = min_y; m_width = max_x - min_x; m_height = max_y - min_y;
            return *this;
        }

        [[nodiscard]] T x() const noexcept { return m_x; }
        [[nodiscard]] T y() const noexcept { return m_y; }
        [[nodiscard]] T x2() const noexcept { return m_x + m_width; }
        [[nodiscard]] T y2() const noexcept { return m_y + m_height; }
        [[nodiscard]] T width() const noexcept { return m_width; }
        [[nodiscard]] T height() const noexcept { return m_height; }
        [[nodiscard]] T shortSide() const noexcept { return m_width < m_height ? m_width : m_height; }
        [[nodiscard]] T longSide() const noexcept { return m_width > m_height ? m_width : m_height; }
        [[nodiscard]] T roundedWidth() const noexcept { return std::round(m_width); }
        [[nodiscard]] T roundedHeight() const noexcept { return std::round(m_height); }
        [[nodiscard]] double area() const noexcept { return m_width * m_height; }

        /**
         *  @brief Computes the aspect ratio of the rectangle (height / width).
         *
         *  The aspect ratio is calculated by dividing the height by the width. If either
         *  dimension is too small (close to zero), it returns a default aspect ratio of 1.0.
         *
         *  @return The aspect ratio (height / width), or 1.0 if the dimensions are too small.
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
         *  @brief Checks if the rectangle is in landscape orientation.
         *
         *  A rectangle is considered landscape if its aspect ratio is less than 1.0.
         *
         *  @return `true` if the rectangle is in landscape orientation, `false` otherwise.
         */
        [[nodiscard]] bool isLandscape() const noexcept { return aspectRatio() < 1.0; }

        /**
         *  @brief Checks if the rectangle is in portrait orientation.
         *
         *  A rectangle is considered portrait if its aspect ratio is greater than 1.0.
         *
         *  @return `true` if the rectangle is in portrait orientation, `false` otherwise.
         */
        [[nodiscard]] bool isPortrait() const noexcept { return !isLandscape(); }

        /**
         *  @brief Checks if the rectangle is a square.
         *
         *  A rectangle is considered square if its aspect ratio is approximately 1.0, within
         *  the floating-point epsilon tolerance (`std::numeric_limits<float>::epsilon()`).
         *
         *  @return `true` if the rectangle is a square, `false` otherwise.
         */
        [[nodiscard]] bool isSquare() const noexcept {
            return std::fabs(1.0 - aspectRatio()) <= std::numeric_limits<double>::epsilon();
        }

        /**
         *  @brief Checks if the rectangle has positive width and height.
         *
         *  A rectangle is considered usable if both its width and height are greater than zero.
         *
         *  @return `true` if the rectangle is usable (both width and height > 0), `false` otherwise.
         */
        [[nodiscard]] bool usable() const noexcept { return m_width > 0 && m_height > 0; }

        /**
         *  @brief Checks if the rectangle is horizontal.
         *
         *  A rectangle is considered horizontal if its width is greater than its height.
         *
         *  @return `true` if the rectangle is horizontal (width > height), `false` otherwise.
         */
        [[nodiscard]] bool isHorizontal() const noexcept { return m_width >= m_height; }

        /**
         *  @brief Checks if the rectangle is vertical.
         *
         *  A rectangle is considered vertical if its height is greater than its width.
         *
         *  @return `true` if the rectangle is vertical (height > width), `false` otherwise.
         */
        [[nodiscard]] bool isVertical() const noexcept { return m_height > m_width; }

        [[nodiscard]] Vec2<T> pos1() const noexcept { return { m_x, m_y }; }
        [[nodiscard]] Vec2<T> pos2() const noexcept { return { m_x + m_width, m_y }; }
        [[nodiscard]] Vec2<T> pos3() const noexcept { return { m_x + m_width, m_y + m_height }; }
        [[nodiscard]] Vec2<T> pos4() const noexcept { return { m_x, m_y + m_height }; }
        [[nodiscard]] Vec2<T> pos(int32_t index) const noexcept {
            switch((index % 4 + 4) % 4) {
                case 0: return { m_x, m_y };
                case 1: return { m_x + m_width, m_y };
                case 2: return { m_x + m_width, m_y + m_height };
                default: return { m_x, m_y + m_height };
            }
        }

        /**
         *  @brief Gets the X coordinate of the rectangle's center.
         *  @return The X coordinate of the center.
         */
        [[nodiscard]] T centerX() const noexcept {
            return m_x + m_width / T(2);
        }

        /**
         *  @brief Gets the Y coordinate of the rectangle's center.
         *  @return The Y coordinate of the center.
         */
        [[nodiscard]] T centerY() const noexcept {
            return m_y + m_height / T(2);
        }

        /**
         *  @brief Computes the center position of the rectangle.
         *  @return A Vec2<T> representing the center point.
         */
        [[nodiscard]] Vec2<T> center() const noexcept {
            return Vec2(m_x + m_width / T(2), m_y + m_height / T(2));
        }

        /**
         *  @brief Computes the radius of the largest inscribed circle.
         *  @return The radius, defined as half of the shortest rectangle side.
         */
        [[nodiscard]] T radius() const noexcept {
            return shortSide() / T(2);
        }

        /**
         *  @brief Computes the radius of the circumcircle around the rectangle.
         *  @return The circumcircle radius, which is half of the rectangle's diagonal.
         */
        [[nodiscard]] T circumcircleRadius() {
            return std::sqrt(m_width * m_width + m_height * m_height) / T(2);
        }

        /**
         *  @brief Maps a parameter `t` to the x-coordinate based on the rectangle's position and width.
         *
         *  @param t The parameter to map, typically in the range [0, 1].
         *  @return The mapped x-coordinate.
         */
        [[nodiscard]] double mappedX(double t) const noexcept {
            return static_cast<double>(m_x) + t * static_cast<double>(m_width);
        }

        /**
         *  @brief Maps a parameter `t` to the y-coordinate based on the rectangle's position and width.
         *
         *  @param t The parameter to map, typically in the range [0, 1].
         *  @return The mapped y-coordinate.
         */
        [[nodiscard]] double mappedY(double t) const noexcept {
            return static_cast<double>(m_y) + t * static_cast<double>(m_height);
        }

        /**
         *  @brief Maps two parameters `x` and `y` to the position inside the rectangle.
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
            return { static_cast<double>(m_x) + x * width(), static_cast<double>(m_x) + y * height() };
        }

        /**
         *  @brief Generates a random position within the rectangle.
         *  @return A Vec2<T> representing a random point inside the rectangle.
         */
        [[nodiscard]] Vec2d randomPos() const noexcept {
            return { static_cast<double>(m_x) + Random::next(m_width), static_cast<double>(m_y) + Random::next(m_height) };
        }

        /**
         *  @brief Generates a random rectangle within the currentMillis rectangle.
         *
         *  @param min_width Minimum width of the random rectangle.
         *  @param max_width Maximum width of the random rectangle.
         *  @param min_height Minimum height of the random rectangle.
         *  @param max_height Maximum height of the random rectangle.
         *  @return A Rect<T> representing a randomly positioned and sized rectangle
         *                    inside the currentMillis one.
         */
        [[nodiscard]] Rect<T> randomRect(T min_width, T max_width, T min_height, T max_height) const noexcept {
            T w = Random::next(min_width, max_width);
            T h = Random::next(min_height, max_height);
            T x = m_x + Random::next(m_width - w);
            T y = m_y + Random::next(m_height - h);
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
                    double aw = static_cast<double>(dst_rect.m_width) / m_width;
                    double ah = static_cast<double>(dst_rect.m_height) / m_height;
                    if (aw > ah) {
                        result.m_width = m_width * aw;
                        result.m_height = m_height * aw;
                    }
                    else {
                        result.m_width = m_width * ah;
                        result.m_height = m_height * ah;
                    }
                }
                    break;

                case FitMode::Fit:
                {
                    double aw = static_cast<double>(dst_rect.m_width) / m_width;
                    double ah = static_cast<double>(dst_rect.m_height) / m_height;
                    if (aw > ah) {
                        result.m_width = m_width * ah;
                        result.m_height = m_height * ah;
                    }
                    else {
                        result.m_width = m_width * aw;
                        result.m_height = m_height * aw;
                    }
                }
                    break;

                case FitMode::Stretch:
                    return *this;

                case FitMode::Center:
                    result.m_width = m_width;
                    result.m_height = m_height;
                    break;
            }

            result.m_x = dst_rect.centerX() - 0.5 * result.m_width;
            result.m_y = dst_rect.centerY() - 0.5 * result.m_height;

            return result;
        }

        /**
         *  @brief Creates a centered square inside the rectangle, based on the
         *         shortest side.
         *  @return A Rect representing the centered square.
         */
        [[nodiscard]] Rect centeredSquare() const noexcept {
            Rect result;
            T length = shortSide();
            if (m_width > m_height) {
                result.m_x = centerX() - length / 2;
                result.m_y = m_y;
            }
            else {
                result.m_x = m_x;
                result.m_y = centerY() - length / 2;
            }
            result.m_width = result.m_height = length;
            return result;
        }

        /**
         *  @brief Returns a rectangle adjusted with padding values.
         *
         *  @param top Padding to subtract from the top.
         *  @param right Padding to subtract from the right.
         *  @param bottom Padding to subtract from the bottom.
         *  @param left Padding to subtract from the left.
         *  @return A Rect with adjusted dimensions based on the given paddings.
         */
        [[nodiscard]] Rect paddedRect(T top, T right, T bottom, T left) const noexcept {
            return Rect(m_x + left, m_y + right, m_width - left - right, m_height - top - bottom);
        }

        /**
         *  @brief Returns a rectangle that is a copy of `rect`, positioned inside
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
            double space_h = m_width - rect.m_width;
            double space_v = m_height - rect.m_height;
            return Rect(m_x + th * space_h, m_y + tv * space_v, rect.m_width, rect.m_height);
        }

        /**
         *  @brief Returns the position of a point within the rectangle based on the specified alignment.
         *
         *  @param alignment The alignment type (e.g., TopLeft, Center, BottomRight, etc.).
         *  @return A Vec2<T> representing the aligned position within the rectangle.
         */
        [[nodiscard]] Vec2<T> alignedPos(Alignment alignment) const noexcept {
            Vec2<T> pos;

            switch (alignment) {

                case Alignment::TopLeft:
                    pos.m_x = m_x;
                    pos.m_y = m_y;
                    break;

                case Alignment::Left:
                    pos.m_x = m_x;
                    pos.m_y = m_y + m_height / 2;
                    break;

                case Alignment::BottomLeft:
                    pos.m_x = m_x;
                    pos.m_y = m_y + m_height;
                    break;

                case Alignment::Top:
                    pos.m_x = m_x + m_width / 2;
                    pos.m_y = m_y;
                    break;

                case Alignment::Center:
                    pos.m_x = m_x + m_width / 2;
                    pos.m_y = m_y + m_height / 2;
                    break;

                case Alignment::Bottom:
                    pos.m_x = m_x + m_width / 2;
                    pos.m_y = m_y + m_height;
                    break;

                case Alignment::TopRight:
                    pos.m_x = m_x + m_width;
                    pos.m_y = m_y;
                    break;

                case Alignment::Right:
                    pos.m_x = m_x + m_width;
                    pos.m_y = m_y + m_height / 2;
                    break;

                case Alignment::BottomRight:
                    pos.m_x = m_x + m_width;
                    pos.m_y = m_y + m_height;
                    break;

                default:
                    pos.m_x = m_x;
                    pos.m_y = m_y;
                    break;
            }

            return pos;
        }

        /**
         *  @brief Computes the rectangular bounds of a grid cell within the currentMillis rectangle.
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
        [[nodiscard]] Rect cellRect(int32_t column_count, int32_t row_count, T column_spacing, T row_spacing, int32_t column_index, int32_t row_index, int32_t column_span = 1, int32_t row_span = 1, bool round_flag = false) const noexcept {
            Rect<T> rect;

            column_count = Type::maxOf<int32_t>(column_count, 1);
            row_count = Type::maxOf<int32_t>(row_count, 1);
            column_span = Type::maxOf<int32_t>(column_span, 1);
            row_span = Type::maxOf<int32_t>(row_span, 1);

            T total_columns_width = m_width - column_spacing * (column_count - 1);
            T total_rows_height = m_height - row_spacing * (row_count - 1);
            T column_width = total_columns_width / column_count;
            T row_height = total_rows_height / row_count;

            rect.m_x = m_x + (column_width + column_spacing) * column_index;
            rect.m_y = m_y + (row_height + row_spacing) * row_index;
            rect.m_width = column_spacing * (column_span - 1) + column_width * column_span;
            rect.m_height = row_spacing * (row_span - 1) + row_height * row_span;

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
                out_points[0].set(m_x, m_y);
                out_points[1].set(m_x + m_width, m_y);
                out_points[2].set(m_x + m_width, m_y + m_height);
                out_points[3].set(m_x, m_y + m_height);
            }
        }

        void glVertices(T* out_vertices) const noexcept {
            if (out_vertices) {
                out_vertices[0] = out_vertices[6] = m_x;
                out_vertices[1] = out_vertices[3] = m_y;
                out_vertices[2] = out_vertices[4] = m_x + m_width;
                out_vertices[5] = out_vertices[7] = m_y + m_height;
            }
        }

        /**
         *  @brief Computes the intersection of the currentMillis rectangle with another
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
         *  @brief Computes an aligned rectangle within the currentMillis rectangle,
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
            Rect rect(width, height);
            switch (alignment) {
                case Alignment::TopLeft:
                case Alignment::Left:
                case Alignment::BottomLeft:
                    rect.m_x = m_x;
                    break;
                case Alignment::Top:
                case Alignment::Center:
                case Alignment::Bottom:
                    rect.m_x = centerX() - width / 2;
                    break;
                case Alignment::TopRight:
                case Alignment::Right:
                case Alignment::BottomRight:
                    rect.m_x = x2() - width;
                    break;
                default:
                    break;
            }
            switch (alignment) {
                case Alignment::TopLeft:
                case Alignment::Top:
                case Alignment::TopRight:
                    rect.m_y = m_y;
                    break;
                case Alignment::Left:
                case Alignment::Center:
                case Alignment::Right:
                    rect.m_y = centerY() - height / 2;
                    break;
                case Alignment::BottomLeft:
                case Alignment::Bottom:
                case Alignment::BottomRight:
                    rect.m_y = y2() - height;
                    break;
                default:
                    break;
            }
            return rect;
        }

        /**
         *  @brief Computes a rectangle aligned to the edge of `this` rectangle
         *         based on the given alignment and padding values. The resulting
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

            Rect rect;
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
                case -1: rect.m_x = left; rect.m_width = right - left; break;
                case 0: rect.m_x = left; rect.m_width = m_width - left - right; break;
                case 1: rect.m_x = m_width - right; rect.m_width = right - left; break;
            }

            switch (v) {
                case -1: rect.m_y = top; rect.m_height = bottom - top; break;
                case 0: rect.m_y = top; rect.m_height = m_height - top - bottom; break;
                case 1: rect.m_y = m_height - bottom; rect.m_height = bottom - top; break;
            }

            if (rect.m_width < 0) {
                rect.m_width = 0;
            }

            if (rect.m_height < 0) {
                rect.m_height = 0;
            }

            return rect;
        }

        /**
         *  @brief Computes an inner rectangle with a specified padding and aspect
         *         ratio.
         *
         *  @param padding The padding value to apply around the inner rectangle.
         *  @param aspect_ratio The desired aspect ratio for the inner rectangle
         *                      (width / height).
         *
         *  @return A Rect<T> representing the inner rectangle with the specified
         *          padding and aspect ratio.
         */
        [[nodiscard]] Rect innerRect(T padding, T aspect_ratio) const noexcept {

            Rect rect;

            rect.m_x = padding;
            rect.m_width = m_width - padding * 2;
            rect.m_height = rect.m_width * aspect_ratio;
            rect.m_y = -(rect.m_height - m_height) / 2;

            if (rect.m_y < padding && aspect_ratio != 0) {
                rect.m_y = padding;
                rect.m_height = m_height - padding * 2;
                rect.m_width = rect.m_height / aspect_ratio;
                rect.m_x = -(rect.m_width - m_width) / 2;
            }

            return rect;
        }


        void zero() noexcept { m_x = m_y = m_width = m_height = 0; }

        void set(T x, T y, T width, T height) noexcept { m_x = x; m_y = y; m_width = width; m_height = height; }
        void set(T x, T y, T size) noexcept { m_x = x; m_y = y; m_width = m_height = size;}
        void set(const Rect& r) noexcept { m_x = r.m_x; m_y = r.m_y; m_width = r.m_width; m_height = r.m_height; }
        void set(T width, T height) noexcept { m_x = m_y = 0; m_width = width; m_height = height; }

        void set(const Vec2<T>& center, T radius) noexcept {
            m_x = center.m_x - radius;
            m_y = center.m_y - radius;
            m_width = m_height = radius * 2;
        }

        void setWidth(T width) noexcept { m_width = width; }
        void setWidthFromCenter(T width) noexcept { m_x += (m_width - width) * 0.5; m_width = width; }
        void setWidthFromMax(T width) noexcept { m_x += m_width - width; m_width = width; }

        void setHeight(T height) noexcept { m_height = height; }
        void setHeightFromCenter(T height) noexcept { m_y += (m_height - height) * 0.5; m_height = height; }
        void setHeightFromMax(T height) noexcept { m_y += m_height - height; m_height = height; }

        void setPos(const Vec2<T>& pos) noexcept { m_x = pos.m_x; m_y = pos.m_y; }
        void setPos(T x, T y) noexcept { m_x = x; m_y = y; }
        void setPos2(const Vec2<T>& pos) { m_width = pos.m_x - m_x; m_height = pos.m_y - m_y; }
        void setPos2(T x, T y) noexcept { m_width = x - m_x; m_height = y - m_y; }

        void setSize(T size) noexcept { m_width = size; m_height = size; }
        void setSize(T width, T height) noexcept { m_width = width; m_height = height; }
        void setSizeFromCenter(T width, T height) noexcept { setWidthFromCenter(width); setHeightFromCenter(height); }


        void moveLeft() noexcept { m_x -= m_width; }
        void moveRight() noexcept { m_x += m_width; }
        void moveUp() noexcept { m_y -= m_height; }
        void moveDown() noexcept { m_y += m_height; }

        void inset(T size) noexcept {
            m_x += size;
            m_y += size;
            m_width -= size * 2;
            m_height -= size * 2;
        }

        void inset(T h, T v) noexcept {
            m_x += h;
            m_y += v;
            m_width -= h * 2;
            m_height -= v * 2;
        }

        void inset(T top, T right, T bottom, T left) noexcept {
            m_x += left;
            m_y += top;
            m_width -= left + right;
            m_height -= top + bottom;
        }

        void inset(RectEdges<T> edges) noexcept {
            m_x += edges.m_left;
            m_y += edges.m_top;
            m_width -= edges.m_left + edges.m_right;
            m_height -= edges.m_top + edges.m_bottom;
        }

        template<typename U>
        void inset(const RectEdges<U>& edges) noexcept
            requires std::is_arithmetic_v<U> {
            m_x += static_cast<T>(edges.m_left);
            m_y += static_cast<T>(edges.m_top);
            m_width -= static_cast<T>(edges.m_left + edges.m_right);
            m_height -= static_cast<T>(edges.m_top + edges.m_bottom);
        }

        void insetLeft(T size) noexcept { m_x += size; m_width -= size; }
        void insetRight(T size) noexcept { m_width -= size; }
        void insetTop(T size) noexcept { m_y += size; m_height -= size; }
        void insetBottom(T size) noexcept { m_height -= size; }

        void insetFromCenter(T x_size, T y_size) noexcept {
            m_x += x_size;
            m_width -= x_size * 2;
            m_y += y_size;
            m_height -= y_size * 2;
        }

        void insetHorizontalFromCenter(T size) noexcept { m_x += size; m_width -= size * 2; }
        void insetVerticalFromCenter(T size) noexcept { m_y += size; m_height -= size * 2; }

        void expand(T size) noexcept {
            m_x -= size;
            m_y -= size;
            m_width += size * 2;
            m_height += size * 2;

        }

        void expandToFit(const Rect& rect) noexcept {
            if (rect.x2() > m_width) {
                m_width = rect.x2();
            }
            if (rect.y2() > m_height) {
                m_height = rect.y2();
            }
        }

        void addWidthFromMax(T width) noexcept {
            T new_width = m_width + width;
            m_x += m_width - new_width;
            m_width = new_width;
        }

        void addHeightFromMax(T height) noexcept {
            T new_height = m_height + height;
            m_y += m_height - new_height;
            m_height = new_height;
        }

        void roundValues() noexcept {
            m_x = std::round(m_x);
            m_y = std::round(m_y);
            m_width = std::round(m_width);
            m_height = std::round(m_height);
        }

        void translateX(T tx) noexcept { m_x += tx; }
        void translateY(T ty) noexcept { m_y += ty; }
        void translate(T tx, T ty) noexcept { m_x += tx; m_y += ty; }
        void translate(const Vec2<T>& t) noexcept { m_x += t.m_x; m_y += t.m_y; }

        void flipVertical() noexcept { m_y = -m_y - m_height; }

        void scale(T scale) noexcept { m_x *= scale; m_y *= scale; m_width *= scale; m_height *= scale; }
        void scale(T sx, T sy) noexcept { m_x *= sx; m_y *= sy; m_width *= sx; m_height *= sy; }
        void scaleSize(T scale) noexcept { m_width *= scale; m_height *= scale; }
        void scaleSize(T sx, T sy) noexcept { m_width *= sx; m_height *= sy; }
        void scaleWidth(T scale) noexcept { m_width *= scale; }
        void scaleHeight(T scale) noexcept { m_height *= scale; }

        void scaleCentered(T scale) noexcept {
            double new_width = m_width * scale;
            double new_height = m_height * scale;
            m_x -= 0.5 * (new_width - m_width);
            m_y -= 0.5 * (new_height - m_height);
            m_width = new_width;
            m_height = new_height;
        }

        /**
         *  @brief Aligns the currentMillis rectangle within another rectangle based on
         *         a given alignment.
         *
         *  This function updates the position of the currentMillis rectangle
         *  (`m_x` and `m_y`) to align it within the provided rectangle `rect` based
         *  on the specified alignment type.
         *
         *  @param alignment The alignment type to use (e.g., Left, TopLeft, Center, etc.).
         *  @param rect The rectangle within which to align the currentMillis rectangle.
         */
        void alignInRect(Alignment alignment, const Rect& rect) noexcept {

            switch (alignment) {
                case Alignment::Left:
                case Alignment::TopLeft:
                case Alignment::BottomLeft:
                    m_x = rect.m_x;
                    break;

                case Alignment::Right:
                case Alignment::TopRight:
                case Alignment::BottomRight:
                    m_x = rect.x2() - m_width;
                    break;

                default:
                    m_x = rect.centerX() - m_width / 2;
                    break;
            }

            switch (alignment) {
                case Alignment::Top:
                case Alignment::TopLeft:
                case Alignment::TopRight:
                    m_y = rect.m_y;
                    break;

                case Alignment::Bottom:
                case Alignment::BottomLeft:
                case Alignment::BottomRight:
                    m_y = rect.y2() - m_height;
                    break;

                default:
                    m_y = rect.centerY() - m_height / 2;
                    break;
            }
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
        }

        void avoidNegativeSize() noexcept {
            if (m_width < 0) {
                m_width = 0;
            }
            if (m_height < 0) {
                m_height = 0;
            }
        }

        void makeValidForArea(T width, T height) noexcept {
            // TODO: Test!!!!!
            if (m_x < 0) { width += m_x; m_x = 0; }
            if (m_y < 0) { height += m_y; m_y = 0; }
            if (m_x >= width) width = 0;
            if (m_y >= height) height = 0;
            if (m_x > 0) width -= m_x;
            if (m_y > 0) height -= m_y;
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
            return (pos.m_x >= m_x && pos.m_x < m_x + m_width && pos.m_y >= m_y && pos.m_y < m_y + m_height);
        }

        [[nodiscard]] bool contains(const Vec2<T>& pos, T tolerance) const noexcept {
            return (pos.m_x >= m_x - tolerance && pos.m_x < m_x + m_width + tolerance &&
                    pos.m_y >= m_y - tolerance && pos.m_y < m_y + m_height + tolerance);
        }

        [[nodiscard]] bool containsX(const T x) const noexcept {
            return (x >= m_x && x < m_x + m_width);
        }

        [[nodiscard]] bool containsY(const T y) const noexcept {
            return (y >= m_y && y < m_y + m_height);
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
            int x1 = std::max<T>(m_x, rect.m_x);
            int y1 = std::max<T>(m_y, rect.m_y);
            int x2 = std::min<T>(m_x + m_width, rect.m_x + rect.m_width);
            int y2 = std::min<T>(m_y + m_height, rect.m_y + rect.m_height);
            return x1 < x2 && y1 < y2;
        }

        [[nodiscard]] bool hitCircle(const Vec2<T>& center, T radius) const noexcept {

            // Find the closest point to the circle within the rectangle
            T closest_x = std::clamp(center.m_x, m_x, x2());
            T closest_y = std::clamp(center.m_y, m_y, y2());

            // Calculate the distance between the circle's center and this closest point
            T distance_x = center.m_x - closest_x;
            T distance_y = center.m_y - closest_y;

            // If the distance is less than the circle's radius, an intersection occurs
            T distance_squared = (distance_x * distance_x) + (distance_y * distance_y);

            return distance_squared < (radius * radius);
        }

        [[nodiscard]] double minXDistance(double x) const noexcept {
            double left = x < m_x ? m_x - x : std::numeric_limits<double>::max();
            double right = x > x2() ? x - x2() : std::numeric_limits<double>::max();
            return left < right ? left : right;
        }

        [[nodiscard]] double minYDistance(double y) const noexcept {
            double top = y < m_y ? m_y - y : std::numeric_limits<double>::max();
            double bottom = y > y2() ? y - y2() : std::numeric_limits<double>::max();
            return top < bottom ? top : bottom;
        }


        /**
         *  @brief Updates this rectangle to the intersection of itself with another
         *         rectangle.
         *
         *  This function modifies the currentMillis rectangle to represent the
         *  intersection of itself with the specified rectangle. If the rectangles
         *  do not intersect, the dimensions of the currentMillis rectangle will be set
         *  to zero.
         *
         *  @param rect The rectangle to intersect with.
         *  @return `true` if the rectangles intersect and the currentMillis rectangle is
         *          updated, `false` otherwise.
         */
        bool intersect(const Rect& rect) noexcept {

            T x2 = this->x2();
            T y2 = this->y2();
            T rx2 = rect.x2();
            T ry2 = rect.y2();

            if (rect.m_x > m_x) { m_x = rect.m_x; }
            if (rx2 < x2) { x2 = rx2; }
            if (rect.m_y > m_y) { m_y = rect.m_y; }
            if (ry2 < y2) { y2 = ry2; }

            m_width = x2 - m_x;
            m_height = y2 - m_y;

            if (m_width <= 0 || m_height <= 0) {
                m_width = 0;
                m_height = 0;
                return false;
            }
            else {
                return true;
            }
        }

        void clampVec2(Vec2<T>& v) const noexcept {
            T x2 = this->x2();
            T y2 = this->y2();
            if (v.m_x < m_x) { v.m_x = m_x; } else if (v.m_x > x2) { v.m_x = x2; }
            if (v.m_y < m_y) { v.m_y = m_y; } else if (v.m_y > y2) { v.m_y = y2; }
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
                case Edge::Left:   return v.m_x >= m_x;
                case Edge::Right:  return v.m_x <= m_x + m_width;
                case Edge::Top:    return v.m_y >= m_y;
                case Edge::Bottom: return v.m_y <= m_y + m_height;
            }
            return false;
        }

        /**
         *  @brief Computes the intersection point of a line segment with a
         *         clipping edge.
         *
         *  @param a The starting point of the segment.
         *  @param b The ending point of the segment.
         *  @param edge The clipping edge to intersect with.
         *  @return Vec2d The intersection point.
         */
        [[nodiscard]] Vec2d intersect(const Vec2<T>& a, const Vec2<T>& b, Edge edge) {
            double dx = b.m_x - a.m_x;
            double dy = b.m_y - a.m_y;

            switch (edge) {
                case Edge::Left: {
                    double x = m_x;
                    double t = (x - a.m_x) / dx;
                    return Vec2d(x, a.m_y + t * dy);
                }
                case Edge::Right: {
                    double x = m_x + m_width;
                    double t = (x - a.m_x) / dx;
                    return Vec2d(x, a.m_y + t * dy);
                }
                case Edge::Top: {
                    double y = m_y;
                    double t = (y - a.m_y) / dy;
                    return Vec2d(a.m_x + t * dx, y);
                }
                case Edge::Bottom: {
                    double y = m_y + m_height;
                    double t = (y - a.m_y) / dy;
                    return Vec2d(a.m_x + t * dx, y);
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
                double rx = m_width < 0 ? m_x + m_width : m_x;
                double ry = m_height < 0 ? m_y + m_height : m_y;
                return CGRectMake(rx, ry, std::fabs(m_width), std::fabs(m_height));
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
        Fix m_x = 0;
        Fix m_y = 0;
        Fix m_width = 1;
        Fix m_height = 1;

    public:
        RectFix() noexcept = default;
        explicit RectFix(Fix size) noexcept : m_width(size), m_height(size) {}
        explicit RectFix(Fix width, Fix height) noexcept : m_width(width), m_height(height) {}
        explicit RectFix(Fix x, Fix y, Fix width, Fix height) noexcept : m_x(x), m_y(y), m_width(width), m_height(height) {}

        [[nodiscard]] virtual const char* className() const noexcept { return "RectFix"; }

        friend std::ostream& operator << (std::ostream& os, const RectFix* o) {
            o == nullptr ? os << "RectFix nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const RectFix& o) {
            os << o.m_x << ", " << o.m_y << " | " << o.m_width << " x " << o.m_height;
            return os;
        }

        // Operator overloading

        RectFix& operator = (const Rect<int32_t>& r) {
            m_x = r.m_x; m_y = r.m_y; m_width = r.m_width; m_height = r.m_height; return *this;
        }

        RectFix& operator = (const Rect<float>& r) {
            m_x = r.m_x; m_y = r.m_y; m_width = r.m_width; m_height = r.m_height; return *this;
        }

        RectFix& operator = (const Rect<double>& r) {
            m_x = r.m_x; m_y = r.m_y; m_width = r.m_width; m_height = r.m_height; return *this;
        }

        bool operator == (const RectFix& other) const {
            return m_x == other.m_x && m_y == other.m_y && m_width == other.m_width && m_height == other.m_height;
        }

        bool operator != (const RectFix& other) const {
            return m_x != other.m_x || m_y != other.m_y || m_width != other.m_width || m_height != other.m_height;
        }

        RectFix operator + (const Vec2Fix& v) const { return RectFix(m_x + v.m_x, m_y + v.m_y, m_width, m_height); }
        RectFix operator - (const Vec2Fix& v) const { return RectFix(m_x - v.m_x, m_y - v.m_y, m_width, m_height); }
        RectFix operator * (const Vec2Fix& v) const { return RectFix(m_x * v.m_x, m_y * v.m_y, m_width, m_height); }

        RectFix& operator += (const Vec2Fix& v) { m_x += v.m_x; m_y += v.m_y; return *this; }
        RectFix& operator -= (const Vec2Fix& v) { m_x -= v.m_x; m_y -= v.m_y; return *this; }
        RectFix& operator *= (const Vec2Fix& v) { m_x *= v.m_x; m_y *= v.m_y; return *this; }

        RectFix& operator += (const RectFix& other) {
            RectFix result;
            Fix min_x = m_x < other.m_x ? m_x : other.m_x;
            Fix min_y = m_y < other.m_y ? m_y : other.m_y;
            Fix max_x = x2() > other.x2() ? x2() : other.x2();
            Fix max_y = y2() > other.y2() ? y2() : other.y2();
            m_x = min_x; m_y = min_y; m_width = max_x - min_x; m_height = max_y - min_y;
            return *this;
        }


        [[nodiscard]] Fix x() const noexcept { return m_x; }
        [[nodiscard]] Fix y() const noexcept { return m_y; }
        [[nodiscard]] Fix x2() const noexcept { return m_x + m_width; }
        [[nodiscard]] Fix y2() const noexcept { return m_y + m_height; }
        [[nodiscard]] Fix width() const noexcept { return m_width; }
        [[nodiscard]] Fix height() const noexcept { return m_height; }
        [[nodiscard]] Fix shortSide() const noexcept { return m_width < m_height ? m_width : m_height; }
        [[nodiscard]] Fix longSide() const noexcept { return m_width > m_height ? m_width : m_height; }

        [[nodiscard]] bool usable() const noexcept { return m_width > 0 && m_height > 0; }
        [[nodiscard]] bool isHorizontal() const noexcept { return m_width > m_height; }
        [[nodiscard]] bool isVertical() const noexcept { return m_height > m_width; }

        [[nodiscard]] Fix centerX() const noexcept { return m_x + m_width / 2; }
        [[nodiscard]] Fix centerY() const noexcept { return m_y + m_height / 2; }
        [[nodiscard]] Vec2Fix center() const noexcept { return { m_x + m_width / 2, m_y + m_height / 2 }; }
        [[nodiscard]] Fix radius() const noexcept { return shortSide() / 2; }
        [[nodiscard]] double circumcircleRadius() const {
            double w = m_width.asDouble();
            double h = m_height.asDouble();
            return std::sqrt(w * w + h * h) / 2;
        }


        void zero() noexcept { m_x = m_y = m_width = m_height = 0; }

        void set(const Fix& x, const Fix& y, const Fix& width, const Fix& height) noexcept {
            m_x = x; m_y = y; m_width = width; m_height = height;
        }
        void set(const Fix& x, const Fix& y, const Fix& size) noexcept {
            m_x = x; m_y = y; m_width = m_height = size;
        }
        void set(const RectFix& r) noexcept {
            m_x = r.m_x; m_y = r.m_y; m_width = r.m_width; m_height = r.m_height;
        }
        void set(const Fix& width, const Fix& height) noexcept {
            m_x = m_y = 0; m_width = width; m_height = height;
        }

        void set(const Vec2Fix& center, const Fix& radius) noexcept {
            m_x = center.m_x - radius;
            m_y = center.m_y - radius;
            m_width = m_height = radius * 2;
        }

        void setWidth(const Fix& width) noexcept { m_width = width; }
        void setWidthFromCenter(const Fix& width) noexcept { m_x += (m_width - width) * 0.5; m_width = width; }
        void setWidthFromMax(const Fix& width) noexcept { m_x += m_width - width; m_width = width; }

        void setHeight(const Fix& height) noexcept { m_height = height; }
        void setHeightFromCenter(const Fix& height) noexcept { m_y += (m_height - height) * 0.5; m_height = height; }
        void setHeightFromMax(const Fix& height) noexcept { m_y += m_height - height; m_height = height; }

        void setPos(const Vec2Fix& pos) noexcept { m_x = pos.m_x; m_y = pos.m_y; }
        void setPos(const Fix& x, const Fix& y) noexcept { m_x = x; m_y = y; }
        void setPos2(const Vec2Fix& pos) { m_width = pos.m_x - m_x; m_height = pos.m_y - m_y; }
        void setPos2(const Fix& x, const Fix& y) noexcept { m_width = x - m_x; m_height = y - m_y; }

        void setSize(const Fix& size) noexcept { m_width = size; m_height = size; }
        void setSize(const Fix& width, const Fix& height) noexcept { m_width = width; m_height = height; }
        void setSizeFromCenter(const Fix& width, const Fix& height) noexcept { setWidthFromCenter(width); setHeightFromCenter(height); }


        void moveLeft() noexcept { m_x -= m_width; }
        void moveRight() noexcept { m_x += m_width; }
        void moveUp() noexcept { m_y -= m_height; }
        void moveDown() noexcept { m_y += m_height; }

        void inset(const Fix& size) noexcept {
            m_x += size;
            m_y += size;
            m_width -= size * 2;
            m_height -= size * 2;
        }

        void inset(const Fix& top, const Fix& right, const Fix& bottom, const Fix& left) noexcept {
            m_x += left;
            m_y += top;
            m_width -= left + right;
            m_height -= top + bottom;
        }

        void insetLeft(const Fix& size) noexcept { m_x += size; m_width -= size; }
        void insetRight(const Fix& size) noexcept { m_width -= size; }
        void insetTop(const Fix& size) noexcept { m_y += size; m_height -= size; }
        void insetBottom(const Fix& size) noexcept { m_height -= size; }

        void insetFromCenter(const Fix& x_size, const Fix& y_size) noexcept {
            m_x += x_size;
            m_width -= x_size * 2;
            m_y += y_size;
            m_height -= y_size * 2;
        }

        void insetHorizontalFromCenter(const Fix& size) noexcept { m_x += size; m_width -= size * 2; }
        void insetVerticalFromCenter(const Fix& size) noexcept { m_y += size; m_height -= size * 2; }

        void expand(const Fix& size) noexcept {
            m_x -= size;
            m_y -= size;
            m_width += size * 2;
            m_height += size * 2;

        }

        void expandToFit(const RectFix& rect) noexcept {
            if (rect.x2() > m_width) {
                m_width = rect.x2();
            }
            if (rect.y2() > m_height) {
                m_height = rect.y2();
            }
        }

        void addWidthFromMax(const Fix& width) noexcept {
            Fix new_width = m_width + width;
            m_x += m_width - new_width;
            m_width = new_width;
        }

        void addHeightFromMax(const Fix& height) noexcept {
            Fix new_height = m_height + height;
            m_y += m_height - new_height;
            m_height = new_height;
        }

        void translateX(const Fix& tx) noexcept { m_x += tx; }
        void translateY(const Fix& ty) noexcept { m_y += ty; }
        void translate(const Fix& tx, const Fix& ty) noexcept { m_x += tx; m_y += ty; }
        void translate(const Vec2Fix& t) noexcept { m_x += t.m_x; m_y += t.m_y; }

        void flipVertical() noexcept { m_y = -m_y - m_height; }

        void scale(const Fix& scale) noexcept { m_width *= scale; m_height *= scale; }
        void scaleWidth(const Fix& scale) noexcept { m_width *= scale; }
        void scaleHeight(const Fix& scale) noexcept { m_height *= scale; }

        void scaleCentered(const Fix& scale) noexcept {
            Fix new_width = m_width * scale;
            Fix new_height = m_height * scale;
            m_x -= (new_width - m_width) / 2;
            m_y -= (new_height - m_height) / 2;
            m_width = new_width;
            m_height = new_height;
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
        }

        void avoidNegativeSize() noexcept {
            if (m_width < 0) {
                m_width = 0;
            }
            if (m_height < 0) {
                m_height = 0;
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
            /* TODO: !!!!!
            Log l(os, indent);
            l.header(label);
            l << "src: " << m_src_x << ", " << m_src_y << " | " << m_src_width << " x " << m_src_height;
            l << ", dst " << m_dst_x << ", " << m_dst_y << " | " << m_dst_width << " x " << m_dst_height;
            l << ", aspect ratio x, y: " << m_src_x_a << ", " << m_src_y_a << Log::endl;
             */
        }

        void set(const Rect<T>& src_rect, const Rect<T>& dst_rect, bool flip_y = false) noexcept {
            m_src_x = src_rect.m_x;
            m_src_y = src_rect.m_y;
            m_src_width = src_rect.m_width;
            m_src_height = src_rect.m_height;
            m_dst_x = dst_rect.m_x;
            m_dst_y = dst_rect.m_y;
            m_dst_width = dst_rect.m_width;
            m_dst_height = dst_rect.m_height;

            // Update aspect ratio
            m_src_x_a = static_cast<double>(m_src_width) != 0.0 ? (static_cast<double>(m_dst_width) / m_src_width) : 1.0;
            if (flip_y) {
                // Flip the Y-axis mapping
                m_src_y_a = static_cast<double>(m_src_height) != 0.0 ? -(static_cast<double>(m_dst_height) / m_src_height) : 1.0;
                m_dst_y += m_dst_height;  // Offset Y to account for the flipped origin
            }
            else {
                m_src_y_a = static_cast<double>(m_src_height) != 0.0 ? (static_cast<double>(m_dst_height) / m_src_height) : 1.0;
            }
        }

        void invert() noexcept {
            Rect<T> src_rect = { m_dst_x, m_dst_y, m_dst_width, m_dst_height };
            Rect<T> dst_rect = { m_src_x, m_src_y, m_src_width, m_src_height };
            set(src_rect, dst_rect);
        }


        [[nodiscard]] T mapX(T x) const noexcept {
            return (x - m_src_x) * m_src_x_a + m_dst_x;
        }

        [[nodiscard]] T inverseMapX(T x) const noexcept {
            return (x - m_dst_x) / m_src_x_a + m_src_x;
        }

        [[nodiscard]] T mapY(T y) const noexcept {
            return (y - m_src_y) * m_src_y_a + m_dst_y;
        }

        [[nodiscard]] T inverseMapY(T y) const noexcept {
            return (y - m_dst_y) / m_src_y_a + m_src_y;
        }

        void mapVec2(Vec2<T>& v) const noexcept {
            v.m_x = (v.m_x - m_src_x) * m_src_x_a + m_dst_x;
            v.m_y = (v.m_y - m_src_y) * m_src_y_a + m_dst_y;
        }

        void inverseMapVec2(const Vec2<T>& v) const noexcept {
            v.m_x = (v.m_x - m_dst_x) / m_src_x_a + m_src_x;
            v.m_y = (v.m_y - m_dst_y) / m_src_y_a + m_src_y;
        }

        void mapVec2(const Vec2<T>& v, Vec2<T>& out_v) const noexcept {
            out_v.m_x = (v.m_x - m_src_x) * m_src_x_a + m_dst_x;
            out_v.m_y = (v.m_y - m_src_y) * m_src_y_a + m_dst_y;
        }

        void inverseMapVec2(const Vec2<T>& v, Vec2<T>& out_v) const noexcept {
            out_v.m_x = (v.m_x - m_dst_x) / m_src_x_a + m_src_x;
            out_v.m_y = (v.m_y - m_dst_y) / m_src_y_a + m_src_y;
        }

        void mapVec2(Vec2<T>* v) const noexcept {
            if (v) {
                v->m_x = (v->m_x - m_src_x) * m_src_x_a + m_dst_x;
                v->m_y = (v->m_y - m_src_y) * m_src_y_a + m_dst_y;
            }
        }

        void inverseMapVec2(Vec2<T>* v) const noexcept {
            if (v) {
                v->m_x = (v->m_x - m_dst_x) / m_src_x_a + m_src_x;
                v->m_y = (v->m_y - m_dst_y) / m_src_y_a + m_src_y;
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
                Vec2<T> p1(r->m_x, r->m_y);
                Vec2<T> p2(r->m_x + r->m_width, r->m_y + r->m_height);
                mapVec2(p1);
                mapVec2(p2);
                r->m_x = p1.m_x;
                r->m_y = p1.m_y;
                r->m_width = p2.m_x - p1.m_x;
                r->m_height = p2.m_y - p1.m_y;
            }
        }

        void inverseMapRect(Rect<T>* r) const noexcept {
            if (r) {
                Vec2<T> p1(r->m_x, r->m_y);
                Vec2<T> p2(r->m_x + r->m_width, r->m_y + r->m_height);
                inverseMapVec2(p1);
                inverseMapVec2(p2);
                r->m_x = p1.m_x;
                r->m_y = p1.m_y;
                r->m_width = p2.m_x - p1.m_x;
                r->m_height = p2.m_y - p1.m_y;
            }
        }

        RemapRect inverse() const noexcept {
            Rect<T> src_rect = { m_dst_x, m_dst_y, m_dst_width, m_dst_height };
            Rect<T> dst_rect = { m_src_x, m_src_y, m_src_width, m_src_height };
            return RemapRect(src_rect, dst_rect);
        }

    protected:
        T m_src_x{};
        T m_src_y{};
        T m_src_width{};
        T m_src_height{};
        T m_dst_x{};
        T m_dst_y{};
        T m_dst_width{};
        T m_dst_height{};
        double m_src_x_a{};     ///< Aspect ratio of source and destination rect in x direction
        double m_src_y_a{};     ///< Aspect ratio of source and destination rect in y direction
    };


    // Standard types
    using RemapRectf = RemapRect<float>;    ///< 32 bit floating point
    using RemapRectd = RemapRect<double>;   ///< 64 bit floating point


} // End of namespace Grain

#endif // GrainRect_hpp
