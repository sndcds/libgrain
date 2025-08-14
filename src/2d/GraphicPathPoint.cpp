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

        m_anchor.m_x = m_left.m_x = m_right.m_x = x;
        m_anchor.m_y = m_left.m_y = m_right.m_y = y;
    }


    /**
     *  @brief Copy constructor.
     */
    GraphicPathPoint::GraphicPathPoint(const GraphicPathPoint& point) noexcept
            : m_anchor(point.m_anchor),
              m_left(point.m_left),
              m_right(point.m_right),
              m_left_flag(point.m_left_flag),
              m_right_flag(point.m_right_flag),
              m_bezier_segment_length(point.m_bezier_segment_length) {
    }


    /**
     *  @brief Move constructor.
     */
    GraphicPathPoint::GraphicPathPoint(GraphicPathPoint&& point) noexcept
            : m_anchor(std::move(point.m_anchor)),
              m_left(std::move(point.m_left)),
              m_right(std::move(point.m_right)),
              m_left_flag(point.m_left_flag),
              m_right_flag(point.m_right_flag),
              m_bezier_segment_length(point.m_bezier_segment_length) {
    }


    GraphicPathPoint::GraphicPathPoint(double x, double y, double lx, double ly, double rx, double ry) noexcept {

        m_anchor.set(x, y);
        m_left.set(lx, ly);
        m_right.set(rx, ry);

        m_left_flag = m_anchor.distance(m_left) > std::numeric_limits<float>::min();
        m_right_flag = m_anchor.distance(m_right) > std::numeric_limits<float>::min();
    }


    GraphicPathPoint::GraphicPathPoint(double x, double y, bool left_flag, double lx, double ly, bool right_flag, double rx, double ry) noexcept {

        m_anchor.set(x, y);
        m_left.set(lx, ly);
        m_right.set(rx, ry);

        m_left_flag = left_flag;
        m_right_flag = right_flag;
    }


    GraphicPathPoint::GraphicPathPoint(const Vec2d& anchor, bool left_flag, const Vec2d& left, bool right_flag, const Vec2d& right) noexcept
            : m_anchor(anchor),
              m_left(left),
              m_right(right),
              m_left_flag(left_flag),
              m_right_flag(right_flag),
              m_bezier_segment_length(0.0) {
    }


    GraphicPathPoint::~GraphicPathPoint() {
    }


    /**
     *  @brief Copy assignment operator.
     */
    GraphicPathPoint& GraphicPathPoint::operator = (const GraphicPathPoint& point) noexcept {

        if (this != &point) {

            // Copy data from 'other' to 'this'
            m_anchor = point.m_anchor;
            m_left = point.m_left;
            m_right = point.m_right;
            m_left_flag = point.m_left_flag;
            m_right_flag = point.m_right_flag;
            m_bezier_segment_length = point.m_bezier_segment_length;
        }

        return *this;
    }


    /**
     *  @brief Move assignment operator.
     */
    GraphicPathPoint& GraphicPathPoint::operator = (GraphicPathPoint&& point) noexcept {

        if (this != &point) {

            // Move data from 'other' to 'this'
            m_anchor = std::move(point.m_anchor);
            m_left = std::move(point.m_left);
            m_right = std::move(point.m_right);
            m_left_flag = point.m_left_flag;
            m_right_flag = point.m_right_flag;
            m_bezier_segment_length = point.m_bezier_segment_length;
        }

        return *this;
    }


    void GraphicPathPoint::translate(const Vec2d& t) noexcept {

        m_anchor.translate(t);
        m_left.translate(t);
        m_right.translate(t);
    }


    void GraphicPathPoint::translate(double tx, double ty) noexcept {

        m_anchor.translate(tx, ty);
        m_left.translate(tx, ty);
        m_right.translate(tx, ty);
    }


    void GraphicPathPoint::rotate(double angle) noexcept {

        m_left.rotate(m_anchor, angle);
        m_right.rotate(m_anchor, angle);
    }


    void GraphicPathPoint::projectToQuadrilateral(const Quadrilateral& quadrilateral, const Mat3d* matrix) noexcept {

        if (matrix != nullptr) {
            matrix->transformVec2(m_anchor);
            matrix->transformVec2(m_left);
            matrix->transformVec2(m_right);
        }

        quadrilateral.project(m_anchor);
        quadrilateral.project(m_left);
        quadrilateral.project(m_right);
    }


} // End of namespace Grain.
