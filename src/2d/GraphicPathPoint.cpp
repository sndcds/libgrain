//
//  GraphicPathPoint.cpp
//
//  Created by Roald Christesen on from 26.01.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "2d/GraphicPathPoint.hpp"
#include "2d/Quadrilateral.hpp"


namespace Grain {


GraphicPathPoint::GraphicPathPoint(double x, double y) noexcept {
    anchor_.x_ = left_.x_ = right_.x_ = x;
    anchor_.y_ = left_.y_ = right_.y_ = y;
}


/**
 *  @brief Move constructor.
 */
GraphicPathPoint::GraphicPathPoint(GraphicPathPoint&& point) noexcept
        : anchor_(std::move(point.anchor_)),
          left_(std::move(point.left_)),
          right_(std::move(point.right_)),
          left_flag_(point.left_flag_),
          right_flag_(point.right_flag_),
          bezier_segment_length_(point.bezier_segment_length_) {
}


GraphicPathPoint::GraphicPathPoint(double x, double y, double lx, double ly, double rx, double ry) noexcept {
    anchor_.set(x, y);
    left_.set(lx, ly);
    right_.set(rx, ry);
    left_flag_ = anchor_.distance(left_) > std::numeric_limits<float>::min();
    right_flag_ = anchor_.distance(right_) > std::numeric_limits<float>::min();
}


GraphicPathPoint::GraphicPathPoint(double x, double y, bool left_flag, double lx, double ly, bool right_flag, double rx, double ry) noexcept {
    anchor_.set(x, y);
    left_.set(lx, ly);
    right_.set(rx, ry);
    left_flag_ = left_flag;
    right_flag_ = right_flag;
}


GraphicPathPoint::GraphicPathPoint(const Vec2d& anchor, bool left_flag, const Vec2d& left, bool right_flag, const Vec2d& right) noexcept
        : anchor_(anchor),
          left_(left),
          right_(right),
          left_flag_(left_flag),
          right_flag_(right_flag),
          bezier_segment_length_(0.0) {
}


/**
 *  @brief Copy assignment operator.
 */
GraphicPathPoint& GraphicPathPoint::operator = (const GraphicPathPoint& point) noexcept {
    if (this != &point) {
        // Copy data from 'other' to 'this'
        anchor_ = point.anchor_;
        left_ = point.left_;
        right_ = point.right_;
        left_flag_ = point.left_flag_;
        right_flag_ = point.right_flag_;
        bezier_segment_length_ = point.bezier_segment_length_;
    }

    return *this;
}


/**
 *  @brief Move assignment operator.
 */
GraphicPathPoint& GraphicPathPoint::operator = (GraphicPathPoint&& point) noexcept {
    if (this != &point) {
        // Move data from 'other' to 'this'
        anchor_ = std::move(point.anchor_);
        left_ = std::move(point.left_);
        right_ = std::move(point.right_);
        left_flag_ = point.left_flag_;
        right_flag_ = point.right_flag_;
        bezier_segment_length_ = point.bezier_segment_length_;
    }

    return *this;
}


void GraphicPathPoint::translate(const Vec2d& t) noexcept {
    anchor_.translate(t);
    left_.translate(t);
    right_.translate(t);
}


void GraphicPathPoint::translate(double tx, double ty) noexcept {
    anchor_.translate(tx, ty);
    left_.translate(tx, ty);
    right_.translate(tx, ty);
}


void GraphicPathPoint::rotate(double angle) noexcept {
    left_.rotate(anchor_, angle);
    right_.rotate(anchor_, angle);
}


void GraphicPathPoint::projectToQuadrilateral(const Quadrilateral& quadrilateral, const Mat3d* matrix) noexcept {
    if (matrix) {
        matrix->transformVec2(anchor_);
        matrix->transformVec2(left_);
        matrix->transformVec2(right_);
    }

    quadrilateral.project(anchor_);
    quadrilateral.project(left_);
    quadrilateral.project(right_);
}


} // End of namespace Grain
