//
//  PosRect.hpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainPosRect_hpp
#define GrainPosRect_hpp

#include "2d/Rect.hpp"
#include "Core/Log.hpp"


namespace Grain {

/**
 *  @brief Positioning Rectangle.
 *
 *  The `PosRect` class is designed for generating arrangements of rectangles.
 *  It provides methods to handle padding, spacing, and justification, making it
 *  convenient to create organized layouts of rectangles.
 *
 *  This class is especially useful in scenarios where you need to arrange
 *  multiple rectangles with specific spacing and alignment requirements.
 */
class PosRect : public Rectd {
public:
    double origin_x_ = 0.0;     ///< The origin on x axis
    double origin_y_ = 0.0;     ///< The origin on y axis
    double h_spacing_ = 2.0;    ///< Horozontal space between cells
    double v_spacing_ = 2.0;    ///< Vertical space between cells

    enum class MoveMode {
        None = 0,
        Right,
        RightResetY,
        Down,
        DownResetX,
        DownResetXRestart
    };

public:
    PosRect(double x, double y, double width, double height) noexcept {
        set(x, y, width, height);
    }

    ~PosRect() noexcept override = default;

    [[nodiscard]] const char* className() const noexcept override { return "PosRect"; }

    friend std::ostream& operator << (std::ostream& os, const PosRect* o) {
        o == nullptr ? os << "PosRect nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const PosRect& o) {
        os << o.x_ << ", " << o.y_ << ", " << o.width_ << ", " << o.height_;
        os << " | " << o.origin_x_ << ", " << o.origin_y_;
        os << " | " << o.h_spacing_ << ", " << o.v_spacing_;
        return os;
    }

    PosRect& operator = (const Rect& v);

    void set(const Rectd& rect) noexcept override { set(rect.x_, rect.y_, rect.width_, rect.height_); }
    void set(double x, double y, double width, double height) noexcept override;

    void setOrigin() noexcept { origin_x_ = x_; origin_y_ = y_; }
    void setSpacing(double spacing) noexcept { h_spacing_ = v_spacing_ = spacing; }
    void setSpacing(double horizontal_spacing, double vertical_spacing) noexcept {
        h_spacing_ = horizontal_spacing;
        v_spacing_ = vertical_spacing;
    }
    void setHorizontalSpacing(double spacing) noexcept { h_spacing_ = spacing; }
    void setVerticalSpacing(double spacing) noexcept { v_spacing_ = spacing; }

    void reset() noexcept { x_ = origin_x_; y_ = origin_y_; }
    void resetX() noexcept { x_ = origin_x_; }
    void resetY() noexcept { y_ = origin_y_; }

    void moveUp(int32_t n = 1) noexcept { y_ -= (height_ + v_spacing_) * n; }
    void moveDown(int32_t n = 1) noexcept { y_ += (height_ + v_spacing_) * n; }
    void moveLeft(int32_t n = 1) noexcept { x_ -= (width_ + h_spacing_) * n; }
    void moveRight(int32_t n = 1) noexcept { x_ += (width_ + h_spacing_) * n; }
    void moveDownResetX(int32_t n = 1) noexcept { moveDown(n); resetX(); }
    void moveRightResetY(int32_t n = 1) noexcept { moveRight(n); resetY(); }
    void moveRightAutoBreak(double right_bound, int32_t n = 1) noexcept;

    void move(MoveMode mode, int32_t colum_n = 1, int32_t row_n = 1) noexcept;

    double columnX(int32_t column_index) const noexcept { return origin_x_ + column_index * (width_ + h_spacing_); }
    double rowY(int32_t row_index) const noexcept { return origin_y_ + row_index * (height_ + v_spacing_); }

    double columnsWidth(int32_t colum_n) const noexcept { return width_ * colum_n + h_spacing_ * (colum_n - 1); }
    double rowsHeight(int32_t row_n) const noexcept { return height_ * row_n + v_spacing_ * (row_n - 1); }

    Rectd centeredRect(double width, double height, int32_t row_n = 1, int32_t colum_n = 1) const noexcept;
    Rectd centeredRectWithMargin(double margin_top, double margin_right, double margin_bottom, double margin_left, int32_t row_n = 1, int32_t colum_n = 1) const noexcept;
    Rectd horizontalCenteredRect(double width, int32_t row_n = 1, int32_t colum_n = 1) const noexcept;
    Rectd verticalCenteredRect(double height, int32_t row_n = 1, int32_t colum_n = 1) const noexcept;
    Rectd columnSpannedRect(int32_t colum_n) const noexcept;
    Rectd spannedRect(int32_t row_n, int32_t colum_n) const noexcept;
};


class TableRect : protected Rectd {

protected:
    int col_n_;
    int row_n_;
    double h_spacing_;
    double v_spacing_;

    // Internal computed values
    double total_cols_width_;
    double total_rows_height_;
    double col_width_;
    double row_height_;

public:
    TableRect(const Rectd& rect, int32_t column_n, int32_t row_n, double h_spacing, double v_spacing) {
        x_ = rect.x_;
        y_ = rect.y_;
        width_ = rect.width_;
        height_ = rect.height_;
        setup(column_n, row_n, h_spacing, v_spacing);
    }

    TableRect(const Rectd& rect, int32_t column_n, int32_t row_n) {
        x_ = rect.x_;
        y_ = rect.y_;
        width_ = rect.width_;
        height_ = rect.height_;
        setup(column_n, row_n, 0.0, 0.0);
    }

    ~TableRect() noexcept override = default;

    const char* className() const noexcept override { return "TableRect"; }

    friend std::ostream& operator << (std::ostream& os, const TableRect* o) {
        o == nullptr ? os << "TableRect nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const TableRect& o) {
        o.log(os, 0, "TableRect");
        return os;
    }

    void log(std::ostream& os, int32_t indent = 0, const char* label = nullptr) const {
        Log l(os, indent);
        l.header(label);
        l << "x, y, width, height: " << x_ << ", " << y_ << ", " << width_ << ", " << height_ << l.endl;
        l << "column_n, row_n: " << col_n_ << ", " << row_n_ << l.endl;
        l << "h_spacing, v_spacing: " << h_spacing_ << ", " << v_spacing_ << l.endl;
    }

    void setup(int32_t column_n, int32_t row_n, int32_t h_spacing, int32_t v_spacing) noexcept {
        col_n_ = std::max(column_n, 1);
        row_n_ = std::max(row_n, 1);
        h_spacing_ = h_spacing;
        v_spacing_ = v_spacing;
        total_cols_width_ = width_ - h_spacing_ * (col_n_ - 1);
        total_rows_height_ = height_ - v_spacing_ * (row_n_ - 1);
        col_width_ = total_cols_width_ / col_n_;
        row_height_ = total_rows_height_ / row_n_;
    }

    int32_t columnCount() const noexcept { return col_n_; }
    int32_t rowCount() const noexcept { return row_n_; }
    double columnWidth() const noexcept { return col_width_; }
    double rowHeight() const noexcept { return row_height_; }
    double columnSpacing() const noexcept { return h_spacing_; }
    double rowSpacing() const noexcept { return v_spacing_; }
    double xStep() const noexcept { return col_width_ + h_spacing_; }
    double yStep() const noexcept { return row_height_ + v_spacing_; }

    Rect cellRect(int32_t column_index, int32_t row_index, bool round_flag = false) const noexcept {
        return cellRect(column_index, row_index, 1, 1, round_flag);
    }

    Rect cellRect(int32_t column_index, int32_t row_index, int32_t column_span, int32_t row_span, bool round_flag = false) const noexcept {

        Rectd rect;

        column_span = std::min(column_span, 1);
        row_span = std::min(row_span, 1);

        rect.x_ = x_ + (col_width_ + h_spacing_) * column_index;
        rect.y_ = y_ + (row_height_ + v_spacing_) * row_index;
        rect.width_ = h_spacing_ * (column_span - 1) + col_width_ * column_span;
        rect.height_ = v_spacing_ * (row_span - 1) + row_height_ * row_span;

        if (round_flag == true) {
            rect.roundValues();
        }

        return rect;
    }
};



} // End of namespace Grain

#endif // PosRect
