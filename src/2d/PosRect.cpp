//
//  PosRect.cpp
//
//  Created by Roald Christesen on from 23.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "2d/PosRect.hpp"


namespace Grain {

    PosRect& PosRect::operator = (const Rectd &v) {
        origin_x_ = origin_y_ = 0.0f;
        h_spacing_ = v_spacing_ = 2.0f;

        x_ = v.x_;
        y_ = v.y_;
        width_ = v.width_;
        height_ = v.height_;

        return *this;
    }


    void PosRect::set(double x, double y, double width, double height) noexcept {
        origin_x_ = x_ = x;
        origin_y_ = y_ = y;
        width_ = width;
        height_ = height;
    }


    void PosRect::moveRightAutoBreak(double right_bound, int32_t n) noexcept {
        x_ += (width_ + h_spacing_) * n;
        if (x_ > right_bound) {
            moveDownResetX();
        }
    }


    void PosRect::move(MoveMode mode, int32_t colum_n, int32_t row_n) noexcept {
        switch (mode) {
            case MoveMode::Right: moveRight(colum_n); break;
            case MoveMode::RightResetY: moveRightResetY(colum_n); break;
            case MoveMode::Down: moveDown(row_n); break;
            case MoveMode::DownResetX: moveDownResetX(row_n); break;
            case MoveMode::DownResetXRestart: moveDownResetX(row_n); setOrigin(); break;

            default:
                break;
        }
    }


    Rectd PosRect::centeredRect(double width, double height, int32_t row_n, int32_t colum_n) const noexcept {
        if (width < 1) {
            width = columnsWidth(colum_n);
        }
        if (height < 1) {
            height = rowsHeight(row_n);
        }
        Rectd r = spannedRect(row_n, colum_n);
        return Rectd(x_ + (r.width_ - width) / 2.0f, y_ + (r.height_ - height) / 2.0f, width, height);
    }


    Rectd PosRect::centeredRectWithMargin(double margin_top, double margin_right, double margin_bottom, double margin_left, int32_t row_n, int32_t colum_n) const noexcept {
        Rectd r = spannedRect(row_n, colum_n);
        return Rectd(x_ + margin_left, y_ + margin_top, r.width_ - margin_left - margin_right, r.height_ - margin_top + margin_bottom);
    }


    Rectd PosRect::horizontalCenteredRect(double width, int32_t row_n, int32_t colum_n) const noexcept {
        Rectd r = spannedRect(row_n, colum_n);
        return Rectd(x_ + (r.width_ - width) / 2.0f, y_, width, r.height_);
    }


    Rectd PosRect::verticalCenteredRect(double height, int32_t row_n, int32_t colum_n) const noexcept {
        Rectd r = spannedRect(row_n, colum_n);
        return Rectd(x_, y_ + (r.height_ - height) / 2.0f, r.width_, height);
    }


    Rectd PosRect::columnSpannedRect(int32_t colum_n) const noexcept {
        return Rectd(x_, y_, width_ * colum_n + h_spacing_ * (colum_n - 1), height_);
    }


    Rectd PosRect::spannedRect(int32_t row_n, int32_t colum_n) const noexcept {
        return Rectd(x_, y_, width_ * colum_n + h_spacing_ * (colum_n - 1), height_ * row_n + v_spacing_ * (row_n - 1));
    }


}  // End of namespace Grain
