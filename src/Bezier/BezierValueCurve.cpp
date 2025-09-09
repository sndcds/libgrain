//
//  BezierValueCurve.cpp
//
//  Created by Roald Christesen on 18.09.2019
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Bezier/BezierValueCurve.hpp"
#include "Math/Math.hpp"
#include "Bezier/Bezier.hpp"
#include "DSP/LUT1.hpp"
#include "DSP/WeightedSamples.hpp"
#include "File/File.hpp"
// #include "GrViewport.hpp"
// #include "GrApp.h"

#include <new> // Include this header for placement new


namespace Grain {

    bool BezierValueCurvePoint::setPointType(Bezier::PointType point_type) noexcept {
        bool changed = false;

        if (point_type != m_point_type) {
            changed = true;

            if (point_type == Bezier::PointType::Linear) {
            }
            else if (point_type == Bezier::PointType::Corner) {
                if (m_point_type == Bezier::PointType::Linear ||
                    m_point_type == Bezier::PointType::Right ||
                    m_point_type == Bezier::PointType::Left) {
                    m_left.set(-1, 0);    // TODO:
                    m_right.set(1, 0);
                }
            }
            else if (point_type == Bezier::PointType::Smooth1) {
                if (m_point_type == Bezier::PointType::Linear ||
                    m_point_type == Bezier::PointType::Corner ||
                    m_point_type == Bezier::PointType::Right ||
                    m_point_type == Bezier::PointType::Left) {
                    m_left.set(-1, 0);    // TODO:
                    m_right.set(1, 0);
                }
            }
            else if (point_type == Bezier::PointType::Smooth2) {
                if (m_point_type == Bezier::PointType::Linear ||
                    m_point_type == Bezier::PointType::Corner ||
                    m_point_type == Bezier::PointType::Right ||
                    m_point_type == Bezier::PointType::Left) {
                    m_left.set(-1, 0);    // TODO:
                    m_right.set(1, 0);
                }
                else if (m_point_type == Bezier::PointType::Smooth1) {
                    m_left = (m_left - m_right) * 0.5;
                    m_right = -m_left;
                }
            }
            else if (point_type == Bezier::PointType::Right) {
            }
            else if (point_type == Bezier::PointType::Left) {
            }

            m_point_type = point_type;
        }

        if (changed) {
            m_bezier_value_curve->mustUpdate();
        }

        return changed;
    }


    double BezierValueCurvePoint::leftDistance() const noexcept {
        return hasLeftControl() ? (m_left_pos - m_pos).length() : 0;
    }


    double BezierValueCurvePoint::rightDistance() const noexcept {
        return hasRightControl() ? (m_right - m_pos).length() : 0;
    }

    bool BezierValueCurvePoint::hasVisibleControlPoints() const noexcept {
        return
                m_point_type == Bezier::PointType::Corner ||
                m_point_type == Bezier::PointType::Smooth1 ||
                m_point_type == Bezier::PointType::Smooth2;
    }


    bool BezierValueCurvePoint::hasLeftControl() const noexcept {
        return
                m_point_type == Bezier::PointType::Corner ||
                m_point_type == Bezier::PointType::Smooth1 ||
                m_point_type == Bezier::PointType::Smooth2;
    }


    bool BezierValueCurvePoint::hasRightControl() const noexcept {
        return
                m_point_type == Bezier::PointType::Corner ||
                m_point_type == Bezier::PointType::Smooth1 ||
                m_point_type == Bezier::PointType::Smooth2;
    }


    bool BezierValueCurvePoint::usesLeftControl() const noexcept {
        return
                m_point_type == Bezier::PointType::Corner ||
                m_point_type == Bezier::PointType::Smooth1 ||
                m_point_type == Bezier::PointType::Smooth2 ||
                m_point_type == Bezier::PointType::Left;
    }


    bool BezierValueCurvePoint::usesRightControl() const noexcept {
        return
                m_point_type == Bezier::PointType::Corner ||
                m_point_type == Bezier::PointType::Smooth1 ||
                m_point_type == Bezier::PointType::Smooth2 ||
                m_point_type == Bezier::PointType::Right;
    }


    bool BezierValueCurvePoint::isSmooth() const noexcept {
        return m_point_type == Bezier::PointType::Smooth1 || m_point_type == Bezier::PointType::Smooth2;
    }


    bool BezierValueCurvePoint::changeStatus(Status mask, bool flag) noexcept {
        Status old_status = m_status;
        m_status = (Status)(flag ? m_status | mask : m_status & ~mask);
        return m_status != old_status;
    }


    void BezierValueCurvePoint::invertSelection() noexcept {
        m_status = (Status)(m_status & Status::Selected ? m_status & ~Status::Selected : m_status | Status::Selected);
    }


    bool BezierValueCurvePoint::setFixed(bool h_flag, bool v_flag) noexcept {
        bool changed = false;
        changed |= changeStatus(Status::FixedX, h_flag);
        changed |= changeStatus(Status::FixedY, v_flag);
        return changed;
    }


    void BezierValueCurvePoint::setPos(const Vec2d& pos) noexcept {
        m_pos = pos;
        curveMustUpdate();
    }


    void BezierValueCurvePoint::setX(double x) noexcept {
        m_pos.m_x = x; curveMustUpdate();
    }


    void BezierValueCurvePoint::setY(double y) noexcept {
        m_pos.m_y = y; curveMustUpdate();
    }


    void BezierValueCurvePoint::clampX(const double min, const double max) noexcept {
        m_pos.clampX(min, max);
        curveMustUpdate();
    }


    void BezierValueCurvePoint::clampY(const double min, const double max) noexcept {
        m_pos.clampY(min, max);
        curveMustUpdate();
    }


    void BezierValueCurvePoint::curveMustUpdate() const noexcept {
        if (m_bezier_value_curve) {
            m_bezier_value_curve->mustUpdate();
        }
    }


    int32_t _BezierValueCurvePoint_compare(const void* a, const void* b) {
        return ((BezierValueCurvePoint*)a)->compare((BezierValueCurvePoint*)b);
    }


    void BezierValueCurvePoint::remember() noexcept {
        m_remembered_pos = m_pos;
        m_remembered_status = m_status;
    }


    void BezierValueCurvePoint::backToRememberedStatus() noexcept {
        m_status = m_remembered_status;
    }


    /* TODO: !!!!!
    void BezierValueCurvePoint::startMouseAction(BezierValueCurvePoint::Part part, const Viewport& viewport) noexcept {

        Vec2d posInViewport, leftInViewport, rightInViewport;
        posInViewport = viewport.posToView(m_pos);
        leftInViewport = viewport.posToView(m_left_pos);
        rightInViewport = viewport.posToView(m_right_pos);
    }
    */

    /* TODO: !!!!!
    BezierValueCurvePoint::Part BezierValueCurvePoint::hit(const Viewport& viewport, const Vec2d& pos, double& min_distance) const noexcept {

        Part part = Part::None;

        Vec2d posInView = viewport.posToView(m_pos);
        float distance = posInView.distance(pos);

        if (distance < minDistance) {
            part = Part::Point;
            minDistance = distance;
        }

        if (isSelected()) {
            if (hasLeftControl()) {
                Vec2d leftInView = viewport.posToView(m_left_pos);
                double leftDistance = leftInView.distance(posInView);
                if (leftDistance > 0) {
                    distance = leftInView.distance(pos);
                    if (distance < minDistance) {
                        part = Part::Left;
                        minDistance = distance;
                    }
                }
            }

            if (hasRightControl()) {
                Vec2d rightInView = viewport.posToView(m_right_pos);
                double rightDistance = rightInView.distance(posInView);
                if (rightDistance > 0) {
                    distance = rightInView.distance(pos);
                    if (distance < minDistance) {
                        part = Part::Right;
                        minDistance = distance;
                    }
                }
            }
        }

        return part;
    }
     */


    int32_t BezierValueCurvePoint::compare(const BezierValueCurvePoint* point) const noexcept {
        if (point) {
            if (m_pos.m_x > point->m_pos.m_x) {
                return 1;
            }
            if (m_pos.m_x < point->m_pos.m_x) {
                return -1;
            }
        }

        return 0;
    }


    BezierValueCurve::BezierValueCurve() noexcept {
        m_points.reserve(16);
    }


    BezierValueCurve::BezierValueCurve(const BezierValueCurve& bezier_value_curve) noexcept : Object() {
        // TODO: Implement
        #pragma message("BezierValueCurve::BezierValueCurve(const BezierValueCurve& bezier_value_curve) must be implemented!")
    }


    BezierValueCurve::~BezierValueCurve() noexcept {
        delete m_weighted_samples;
    }


    bool BezierValueCurve::isValid() const noexcept {
        if (m_mode == Mode::Envelope) {
            bool has_decay_begin = false;

            for (auto& point : m_points) {
                if (point.m_status & BezierValueCurvePoint::Status::DecayBegin) {
                    has_decay_begin = true;
                }
            }

            if (!has_decay_begin) {
                return false;
            }
        }

        return true;
    }


    int32_t BezierValueCurve::selectedPointsCount() const noexcept {
        int32_t n = 0;

        for (auto& point : m_points) {
            if (point.isSelected()) {
                n++;
            }
        }

        return n;
    }


    int32_t BezierValueCurve::decayPointIndex() const noexcept {
        int32_t index = 0;

        for (auto& point : m_points) {
            if (point.isDecayBegin()) {
                return index;
            }
            index++;
        }

        return -1;
    }


    void BezierValueCurve::rangeY(double& out_min_y, double& out_max_y) const noexcept {
        out_min_y = DBL_MAX;
        out_max_y = -DBL_MAX;

        for (auto& point : m_points) {
            double y = point.m_pos.m_y;
            if (y < out_min_y)
                out_min_y = y;
            if (y > out_max_y)
                out_max_y = y;
        }
    }


    Rectd BezierValueCurve::bbox(bool selected_only) const noexcept {
        //  _update();    TODO: Is call to _update necessary?

        double min_x = std::numeric_limits<double>::max();
        double min_y = std::numeric_limits<double>::max();
        double max_x = std::numeric_limits<double>::lowest();
        double max_y = std::numeric_limits<double>::lowest();

        for (auto& point : m_points) {
            if (point.isSelected() || !selected_only) {
                if (point.m_pos.m_x < min_x) min_x = point.m_pos.m_x;
                if (point.m_pos.m_x > max_x) max_x = point.m_pos.m_x;
                if (point.m_pos.m_y < min_y) min_y = point.m_pos.m_y;
                if (point.m_pos.m_y > max_y) max_y = point.m_pos.m_y;
            }
        }

        return { min_x, min_y, max_x - min_x, max_y - min_y };
    }


    const BezierValueCurvePoint* BezierValueCurve::pointAtIndex(int32_t index) const noexcept {
        return index >= 0 && index < static_cast<int32_t>(m_points.size()) ? &m_points[index] : nullptr;
    }


    BezierValueCurvePoint* BezierValueCurve::mutPointAtIndex(int32_t index) noexcept {
        return index >= 0 && index < static_cast<int32_t>(m_points.size()) ? &m_points[index] : nullptr;
    }


    BezierValueCurvePoint* BezierValueCurve::firstSelectedPoint() noexcept {
        for (auto& point : m_points) {
            if (point.isSelected()) {
                return &point;
            }
        }

        return nullptr;
    }


    BezierValueCurvePoint* BezierValueCurve::decayBeginPoint() noexcept {
        for (auto& point : m_points) {
            if (point.isDecayBegin())
                return &point;
        }

        return nullptr;
    }


    void BezierValueCurve::setLimits(double min_x, double max_x, double min_y, double max_y) noexcept {
        m_limit_min_x = min_x;
        m_limit_max_x = max_x;
        m_limit_min_y = min_y;
        m_limit_max_y = max_y;
    }


    int32_t BezierValueCurve::decayBeginIndex() const noexcept {
        int32_t index = 0;
        for (auto& point : m_points) {
            if (point.isDecayBegin()) {
                return index;
            }
            index++;
        }

        return -1;
    }


    void BezierValueCurve::setDecayBeginIndex(int32_t decay_begin_index) noexcept {
        int32_t index = 0;
        for (auto& point : m_points) {
            point.changeStatus(BezierValueCurvePoint::Status::DecayBegin, index == decay_begin_index);
            index++;
        }
    }


    void BezierValueCurve::selectAllPoints() noexcept {
        for (auto& point : m_points) {
            point.select();
        }
    }


    void BezierValueCurve::deselectAllPoints() noexcept {
        for (auto& point : m_points) {
            point.deselect();
        }
    }


    void BezierValueCurve::selectPointsInRect(const Rectd& rect) noexcept {
        Rectd selection_rect(rect);
        selection_rect.makePositiveSize();

        for (int32_t index = 0; index < length(); index++) {
            if (auto point = mutPointAtIndex(index)) {
                if (selection_rect.contains(point->m_pos)) {
                    point->select();
                }
                else {
                    point->backToRememberedStatus();
                }
            }
        }
    }


    bool BezierValueCurve::setTypeOfSelectedPoints(Bezier::PointType point_type) noexcept {
        bool changed = false;

        int32_t point_n = length();
        if (point_n < 2) {
            return changed;
        }

        for (int32_t i = 0; i < point_n; i++) {
            auto l = mutPointAtIndex(i - 1);
            auto c = mutPointAtIndex(i);
            auto r = mutPointAtIndex(i + 1);

            if (c->isSelected() && c->m_point_type != point_type) {
                switch (point_type) {
                    case Bezier::PointType::Linear:
                        break;

                    case Bezier::PointType::Corner:
                        if (c->m_point_type == Bezier::PointType::Linear ||
                            c->m_point_type == Bezier::PointType::Right ||
                            c->m_point_type == Bezier::PointType::Left) {
                            if (l) {
                                c->m_left.m_x = (l->m_pos.m_x - c->m_pos.m_x) / 2;
                                c->m_left.m_y = 0;
                            }
                            if (r) {
                                c->m_right.m_x = (r->m_pos.m_x - c->m_pos.m_x) / 2;
                                c->m_right.m_y = 0;
                            }
                        }
                        break;

                    case Bezier::PointType::Smooth1:
                        if (c->m_point_type == Bezier::PointType::Linear ||
                            c->m_point_type == Bezier::PointType::Corner ||
                            c->m_point_type == Bezier::PointType::Right ||
                            c->m_point_type == Bezier::PointType::Left) {
                            if (l) {
                                c->m_left.m_x = (l->m_pos.m_x - c->m_pos.m_x) / 2;
                                c->m_left.m_y = 0;
                            }
                            if (r) {
                                c->m_right.m_x = (r->m_pos.m_x - c->m_pos.m_x) / 2;
                                c->m_right.m_y = 0;
                            }
                        }
                        break;

                    case Bezier::PointType::Smooth2:
                        if (c->m_point_type == Bezier::PointType::Linear ||
                            c->m_point_type == Bezier::PointType::Corner ||
                            c->m_point_type == Bezier::PointType::Right ||
                            c->m_point_type == Bezier::PointType::Left) {
                            double length = 0;
                            int32_t n = 0;
                            if (l) {
                                length = c->m_pos.m_x - l->m_pos.m_x;
                                n++;
                            }
                            if (r) {
                                length += r->m_pos.m_x - c->m_pos.m_x;
                                n++;
                            }
                            if (n > 0) {
                                length /= n * 2;
                                c->m_left.m_x = -length;
                                c->m_right.m_x = length;
                                c->m_left.m_y = 0;
                                c->m_right.m_y = 0;
                            }
                        }
                        else if (c->m_point_type == Bezier::PointType::Smooth1) {
                            Vec2d v;
                            if (l && r) {
                                v = (-c->m_left + c->m_right) * 0.5;
                                c->m_left = -v;
                                c->m_right = v;
                            }
                            if (!l) {
                                c->m_left = -c->m_right;
                            }
                            else if (!r) {
                                c->m_right = -c->m_left;
                            }
                        }
                        break;

                    default:
                        break;
                }

                c->m_point_type = point_type;
                changed = true;
            }
        }

        return changed;
    }


    bool BezierValueCurve::horizontalCenterSelectedPoints() noexcept {
        bool changed = false;

        BezierValueCurvePoint* a = nullptr;
        BezierValueCurvePoint* b = nullptr;

        for (auto& point : m_points) {
            if (a && b) {
                if (b->isSelected() && !a->isSelected() && !point.isSelected()) {
                    b->m_pos.m_x = a->m_pos.m_x + (point.m_pos.m_x - a->m_pos.m_x) / 2;
                    changed = true;
                }
            }
            a = b;
            b = &point;
        }

        if (changed) {
            mustUpdate();
        }

        return changed;
    }


    bool BezierValueCurve::verticalCenterSelectedPoints() noexcept {
        bool changed = false;

        BezierValueCurvePoint* a = nullptr;
        BezierValueCurvePoint* b = nullptr;

        for (auto& point : m_points) {
            if (a && b) {
                if (b->isSelected() && !a->isSelected() && !point.isSelected()) {
                    b->m_pos.m_y = a->m_pos.m_y + (point.m_pos.m_y - a->m_pos.m_y) / 2;
                    changed = true;
                }
            }

            a = b;
            b = &point;
        }

        if (changed) {
            mustUpdate();
        }

        return changed;
    }


    bool BezierValueCurve::alignSelectedPoints(Alignment align) noexcept {
        int32_t n = selectedPointsCount();
        if (align == Alignment::No) {
            if (n < 1) {
                return false;
            }
        }
        else if (n < 2) {
            return false;
        }

        bool changed = false;
        Rectd rect = bbox(true);
        double y = 0.0;

        switch (align) {
            case Alignment::Top: y = rect.y2(); break;
            case Alignment::Center: y = rect.centerY(); break;
            case Alignment::Bottom: y = rect.m_y; break;
            case Alignment::No: y = 0; break;
            default: return false;
        }

        for (auto& point : m_points) {
            if (point.isSelected()) {
                point.m_pos.m_y = y;
                changed = true;
            }
        }

        if (changed) {
            mustUpdate();
        }

        return changed;
    }


    BezierValueCurvePoint* BezierValueCurve::addPoint() noexcept {
        BezierValueCurvePoint* result = nullptr;

        try {
            m_points.push(BezierValueCurvePoint());
            auto point = mutLastPoint();
            if (!point) {
                throw ErrorCode::Specific;
            }
            point->m_bezier_value_curve = this;
            mustUpdate();
            result = point;
        }
        catch (ErrorCode err) {
            // TODO: Messaging!
        }
        catch (const std::exception& e) {
            std::cerr << "BezierValueCurve::addPoint std::exception: " << e.what() << std::endl; // TODO: !!!!
        }

        return result;
    }


    BezierValueCurvePoint* BezierValueCurve::addLinearPoint(double x, double y) noexcept {
            return addPoint(x, y, 0, 0, 0, 0, Bezier::PointType::Linear);
    }


    BezierValueCurvePoint* BezierValueCurve::addPoint(
            double x, double y,
            double lx, double ly,
            double rx, double ry,
            Bezier::PointType point_type,
            BezierValueCurvePoint::Status status
    ) noexcept
    {
        m_points.push(BezierValueCurvePoint());

        auto point = mutLastPoint();
        if (!point) {
            return nullptr;
        }

        point->m_bezier_value_curve = this;
        point->m_pos.m_x = x;
        point->m_pos.m_y = y;
        point->m_left.m_x = lx;
        point->m_left.m_y = ly;
        point->m_right.m_x = rx;
        point->m_right.m_y = ry;
        point->m_point_type = point_type;
        point->m_status = status;

        mustUpdate();

        return point;
    }


    bool BezierValueCurve::removePoint(int32_t index) noexcept {
        if (index >= 0 && index < static_cast<int32_t>(m_points.size()) - 1) {
            m_points.removeAtIndex(index);
            mustUpdate();
            return true;
        }

        return false;
    }


    int32_t BezierValueCurve::removeSelectedPoints() noexcept {
        int32_t result = 0;

        if (selectedPointsCount() > 0) {
            auto n = static_cast<int32_t>(m_points.size());
            for (int32_t i = n - 1; i >= 0; i--) {
                if (m_points[i].isSelected()) {
                    m_points.removeAtIndex(i);
                    result++;
                }
            }

            if (result > 0) {
                mustUpdate();
            }
        }

        return result;
    }


    void BezierValueCurve::clear() noexcept {
        m_points.clear();
    }


    void BezierValueCurve::flipVertical() noexcept {
        if (hasPoints()) {
            double min_y, max_y;
            rangeY(min_y, max_y);
            double offset = min_y + max_y;

            for (auto& point : m_points) {
                point.m_pos.m_y = offset - point.m_pos.m_y;
                point.m_left.m_y = -point.m_left.m_y;
                point.m_right.m_y = -point.m_right.m_y;
            }

            mustUpdate();
        }
    }


    bool BezierValueCurve::split(int32_t segment_index, float t, bool select) noexcept {
        if (segment_index < 0 || segment_index > length() - 1 || t <= 0 || t >= 1) {
            return false;
        }

        _update();

        auto point_a = mutPointAtIndex(segment_index);
        auto point_b = mutPointAtIndex(segment_index + 1);
        if (!point_a || !point_b) {
            return false;
        }

        Bezier bezier(*point_a, *point_b);

        Bezier sub_bezier0, sub_bezier1;
        bezier.split(t, sub_bezier0, sub_bezier1);

        Bezier::PointType point_type = Bezier::PointType::Smooth1;
        if (point_a->m_point_type == Bezier::PointType::Linear && point_b->m_point_type == Bezier::PointType::Linear) {
            point_type = Bezier::PointType::Linear;
        }

        auto new_point = addPoint(sub_bezier1.m_pos[0].m_x, sub_bezier1.m_pos[0].m_y, 0, 0, 0, 0, point_type);
        if (!new_point) {
            return false;
        }

        if (select) {
            new_point->select();
        }

        new_point->m_left = sub_bezier0.m_pos[2] - sub_bezier0.m_pos[3];
        new_point->m_right = sub_bezier1.m_pos[1] - sub_bezier1.m_pos[0];

        point_a->m_right = sub_bezier0.m_pos[1] - sub_bezier0.m_pos[0];
        point_b->m_left = sub_bezier1.m_pos[2] - sub_bezier1.m_pos[3];

        if (point_a->m_point_type == Bezier::PointType::Smooth2) {
            point_a->m_point_type = Bezier::PointType::Smooth1;
        }
        if (point_b->m_point_type == Bezier::PointType::Smooth2) {
            point_b->m_point_type = Bezier::PointType::Smooth1;
        }

        mustUpdate();

        return true;
    }


    void BezierValueCurve::rememberAllPoints() noexcept {
        for (auto& point : m_points) {
            point.remember();
        }
    }


    bool BezierValueCurve::moveRememberedSelectedPoints(const Vec2d& delta) noexcept {
        bool changed = false;

        for (auto& point : m_points) {
            if (point.isSelected()) {
                if (!point.isXFixed()) {
                    double new_x = std::clamp<double>(point.m_remembered_pos.m_x + delta.m_x, m_limit_min_x, m_limit_max_x);
                    if (new_x != point.m_pos.m_x) {
                        point.m_pos.m_x = new_x;
                        changed = true;
                    }
                }

                if (!point.isYFixed()) {
                    double new_y = std::clamp<double>(point.m_remembered_pos.m_y + delta.m_y, m_limit_min_y, m_limit_max_y);
                    if (new_y != point.m_pos.m_y) {
                        point.m_pos.m_y = new_y;
                        changed = true;
                    }
                }
            }
        }

        if (changed) {
            mustUpdate();
        }

        return changed;
    }


    void BezierValueCurve::mustSort() noexcept {
        _m_must_sort = true;
        _m_modification_count++;
    }


    void BezierValueCurve::mustUpdate() noexcept {
        _m_must_sort = true;
        _m_must_update = true;
        _m_modification_count++;
    }


    float BezierValueCurve::lookup(float t, int32_t resolution) noexcept {
        _updateWeightedSamples(resolution);
        if (!m_weighted_samples) {
            return 0;
        }

        return m_weighted_samples->lookup(t);
    }


    bool BezierValueCurve::fillLUT(LUT1* lut) noexcept {
        if (!lut) {
            return false;
        }

        if (!_updateWeightedSamples(lut->resolution())) {
            return false;
        }

        if (!lut->setByWeightedSamples(m_weighted_samples)) {
            return false;
        }

        return true;
    }


    bool BezierValueCurve::fillEnvelopeAttackLUT(LUT1* lut) noexcept {
        if (lut && m_mode == Mode::Envelope) {
            int32_t end_index = decayBeginIndex();
            if (end_index >= 0) {
                return _updateLUT(lut, 0, end_index) == ErrorCode::None;
            }
        }

        return false;
    }


    bool BezierValueCurve::fillEnvelopeDecayLUT(LUT1* lut) noexcept {
        if (lut && m_mode == Mode::Envelope) {
            int32_t begin_index = decayBeginIndex();
            if (begin_index >= 0) {

                int32_t endIndex = length() - 1;
                if (endIndex > begin_index) {
                    return _updateLUT(lut, begin_index, endIndex) == ErrorCode::None;
                }
            }
        }

        return false;
    }


    ErrorCode BezierValueCurve::fillBuffer(int32_t start_index, int32_t end_index, float* buffer, int32_t length) noexcept {
        // TODO: Test, optimization

        if (!buffer) {
            return ErrorCode::NullData;
        }

        if (length < 1) {
            return ErrorCode::BadArgs;
        }

        int32_t point_count = this->length();
        if (point_count < 2) {
            return Error::specific(kErr_ToFewPoints);
        }

        if (start_index < 0 || start_index >= point_count - 1) {
            return Error::specific(kErr_StartIndexOutOfRange);
        }

        if (end_index < start_index + 1 || end_index >= point_count) {
            return Error::specific(kErr_EndIndexOutOfRange);
        }

        int32_t segment_count = end_index - start_index;

        auto start_point = pointAtIndex(start_index);
        auto end_point = pointAtIndex(end_index);

        double start_x = start_point->m_pos.m_x;
        double end_x = end_point->m_pos.m_x;
        double width = end_x - start_x;

        if (width <= 0) {
            return Error::specific(kErrInvalidWidth);
        }

        double t = 0;
        int32_t index1 = 0;
        for (int32_t si = 0; si < segment_count; si++) {
            int32_t pi1 = start_index + si;
            int32_t pi2 = pi1 + 1;

            auto p1 = pointAtIndex(pi1);
            auto p2 = pointAtIndex(pi2);

            double x1 = p1->m_pos.m_x;
            double x2 = p2->m_pos.m_x;
            double segment_width = x2 - x1;

            double f = segment_width / width;
            t += f;

            int32_t index2 = std::round(t * length);

            Vec2d bp1 = p1->m_pos;
            Vec2d bp2 = p1->m_used_right_pos;
            Vec2d bp3 = p2->m_used_left_pos;
            Vec2d bp4 = p2->m_pos;

            float scale = 1.0f / width;
            bp1.m_x = (bp1.m_x - start_x) * scale;
            bp2.m_x = (bp2.m_x - start_x) * scale;
            bp3.m_x = (bp3.m_x - start_x) * scale;
            bp4.m_x = (bp4.m_x - start_x) * scale;

            Bezier bezier(bp1, bp2, bp3, bp4);

            for (int32_t i = index1; i < index2; i++) {
                double x = bp1.m_x + static_cast<double>(i - index1) / (index2 - index1) * (bp4.m_x - bp1.m_x);

                double roots[3];
                int32_t n = Math::solveCubicBezier(bp1.m_x, bp2.m_x, bp3.m_x, bp4.m_x, x, roots);
                if (n > 0) {
                    double t = roots[0];
                    Vec2d sample = bezier.posOnCurve(t);
                    buffer[i] = sample.m_y;
                }

            }

            index1 = index2;
        }


        return ErrorCode::None;
    }


    void BezierValueCurve::_sortPoints() noexcept {
        if (_m_must_sort) {
            std::sort(m_points.begin(), m_points.end(), sortPointsCompareFunc);
            _m_must_sort = false;
        }
    }


    bool BezierValueCurve::sortPointsCompareFunc(const BezierValueCurvePoint& a, const BezierValueCurvePoint& b) {
        return a.m_pos.m_x < b.m_pos.m_x;
    }


    bool BezierValueCurve::_updateWeightedSamples(int32_t resolution) noexcept {
        return _updateWeightedSamples(0, length() - 1, resolution);
    }


    bool BezierValueCurve::_updateWeightedSamples(int32_t start_point_index, int32_t end_point_index, int32_t resolution) noexcept {
        if (_m_weighted_samples_modification_count >= _m_modification_count &&
            _m_weighted_samples_resolution == resolution &&
            _m_weighted_samples_start_point_index == start_point_index &&
            _m_weighted_samples_end_point_index == end_point_index) {
            // Nothing changed since last update.
            return true;
        }

        if (resolution < 1) {
            return false;
        }

        if (length() < 2) {
            return false;
        }

        start_point_index = std::clamp<int32_t>(start_point_index, 0, length() - 1);
        end_point_index = std::clamp<int32_t>(end_point_index, start_point_index + 1, length() - 1);
        int32_t point_n = end_point_index - start_point_index + 1;
        if (point_n < 2) {
            return false;
        }

        if (!m_weighted_samples) {
            m_weighted_samples = new (std::nothrow) WeightedSamples(resolution);
        }
        else {
            if (m_weighted_samples->setResolution(resolution) != ErrorCode::None) {
                return false;
            }
        }

        if (!m_weighted_samples) {
            // GrApp::addError(Gr::ERR_FATAL, "BezierCurve::_updateWeightedSamples()"); TODO: !!!!
            return false;
        }

        m_weighted_samples->clear();
        _update();

        auto start_point = pointAtIndex(start_point_index);
        auto end_point = pointAtIndex(end_point_index);
        if (!start_point || !end_point) {
            return false;
        }

        double min_x = start_point ? start_point->m_pos.m_x : 0.0;
        double max_x = end_point ? end_point->m_pos.m_x : 0.0;
        double width = max_x - min_x;
        if (width < std::numeric_limits<double>::epsilon()) {
            width = std::numeric_limits<double>::epsilon();
        }
        double scale = 1.0 / width;

        for (int32_t i = 1; i < point_n; i++) {

            auto cp1 = pointAtIndex(start_point_index + i - 1);
            auto cp2 = pointAtIndex(start_point_index + i);

            Vec2d bp1 = cp1->m_pos;
            Vec2d bp2 = cp1->m_used_right_pos;
            Vec2d bp3 = cp2->m_used_left_pos;
            Vec2d bp4 = cp2->m_pos;

            bp1.m_x = (bp1.m_x - min_x) * scale;
            bp2.m_x = (bp2.m_x - min_x) * scale;
            bp3.m_x = (bp3.m_x - min_x) * scale;
            bp4.m_x = (bp4.m_x - min_x) * scale;

            Bezier bezier(bp1, bp2, bp3, bp4);
            m_weighted_samples->addBezier(bezier, 256); // TODO: configurable resolution?
        }

        m_weighted_samples->finish();

        _m_weighted_samples_modification_count = _m_modification_count;
        _m_weighted_samples_resolution = resolution;
        _m_weighted_samples_start_point_index = start_point_index;
        _m_weighted_samples_end_point_index = end_point_index;

        return true;
    }


    ErrorCode BezierValueCurve::_updateLUT(LUT1* lut) noexcept {
        return _updateLUT(lut, 0, length() - 1);
    }


    ErrorCode BezierValueCurve::_updateLUT(LUT1* lut, int32_t start_point_index, int32_t end_point_index) noexcept {
        if (!lut) {
            return ErrorCode::NullData;
        }

        if (!_updateWeightedSamples(start_point_index, end_point_index, lut->resolution())) {
            return ErrorCode::Specific;
        }

        lut->setByWeightedSamples(m_weighted_samples);

        return ErrorCode::None;
    }


    bool BezierValueCurve::_update() noexcept {
        _sortPoints();
        int32_t point_n = length();

        if (_m_must_update && point_n > 1) {

            // Pass 1, left to right
            for (int32_t i = 0; i < point_n - 1; i++) {
                auto p0 = mutPointAtIndex(i);
                auto p1 = mutPointAtIndex(i + 1);

                if (p0->m_right.m_x < 0) {
                    p0->m_right.m_x = 0;
                }

                Vec2d v = p0->m_right;
                if (v.m_x > 0) {
                    if (v.m_x > p1->m_pos.m_x - p0->m_pos.m_x) {
                        v.m_x = p1->m_pos.m_x - p0->m_pos.m_x;
                        v.m_y = p0->m_right.m_y * v.m_x / p0->m_right.m_x;
                    }
                }

                p0->m_right_pos = p0->m_pos + p0->m_right;
                p0->m_used_right_pos = p0->m_pos + v;
            }

            // Pass 2, right to left
            for (int32_t i = point_n - 1; i > 0; i--) {
                auto p0 = mutPointAtIndex(i);
                auto p1 = mutPointAtIndex(i - 1);

                if (p0->m_left.m_x > 0) {
                    p0->m_left.m_x = 0;
                }

                Vec2d v = p0->m_left;
                if (v.m_x < 0) {
                    if (v.m_x < p1->m_pos.m_x - p0->m_pos.m_x) {
                        v.m_x = p1->m_pos.m_x - p0->m_pos.m_x;
                        v.m_y = p0->m_left.m_y * v.m_x / p0->m_left.m_x;
                    }
                }
                p0->m_left_pos = p0->m_pos + p0->m_left;
                p0->m_used_left_pos = p0->m_pos + v;
            }

            // Pass 3, automatic right continuity
            auto p = mutPointAtIndex(0);
            if (p->m_point_type == Bezier::PointType::Right) {
                p->m_used_right_pos = p->m_pos;
            }

            for (int32_t i = 1; i < point_n - 1; i++) {
                auto c = mutPointAtIndex(i);

                if (c->m_point_type == Bezier::PointType::Right) {
                    auto l = mutPointAtIndex(i - 1);
                    auto r = mutPointAtIndex(i + 1);

                    Vec2d v = c->m_pos - (l->usesRightControl() ? l->m_used_right_pos : l->m_pos);
                    Lined line(0, 0, v.m_x, v.m_y);
                    double right_pos = (r->m_pos.m_x - c->m_pos.m_x) / 2;

                    Lined top_line(0, 1, right_pos, 1);
                    Vec2d iv;
                    if (line.intersects(top_line, iv)) {
                        if (iv.m_x > 0 && iv.length() < v.length()) {
                            v = iv;
                        }
                    }

                    Lined bottom_line(0, -1, right_pos, -1);
                    if (line.intersects(bottom_line, iv)) {
                        if (iv.m_x > 0 && iv.length() < v.length()) {
                            v = iv;
                        }
                    }

                    Lined right_line(right_pos, -1, right_pos, 1);
                    if (line.intersects(right_line, iv)) {
                        if (iv.m_x > 0 && iv.length() < v.length()) {
                            v = iv;
                        }
                    }

                    c->m_used_right_pos = c->m_pos + v;
                }
            }

            // Pass 4, automatic left continuity
            p = mutPointAtIndex(point_n - 1);
            if (p->m_point_type == Bezier::PointType::Left) {
                p->m_used_left_pos = p->m_pos;
            }

            for (int32_t i = point_n - 2; i > 0; i--) {
                auto c = mutPointAtIndex(i);

                if (c->m_point_type == Bezier::PointType::Left) {
                    auto l = mutPointAtIndex(i - 1);
                    auto r = mutPointAtIndex(i + 1);

                    Vec2d v = (r->usesLeftControl() ? r->m_used_left_pos : r->m_pos) - c->m_pos;
                    Lined line(0, 0, v.m_x, v.m_y);
                    double left_pos = (l->m_pos.m_x - c->m_pos.m_x) / 2;

                    Vec2d iv;

                    Lined left_line(left_pos, -1, left_pos, 1);
                    if (line.intersects(left_line, iv)) {
                        if (iv.m_x < 0 && iv.length() < v.length()) {
                            v = -iv;
                        }
                    }

                    Lined top_line(left_pos, 1, 0, 1);
                    if (line.intersects(top_line, iv)) {
                        if (iv.m_x < 0 && iv.length() < v.length()) {
                            v = -iv;
                        }
                    }

                    Lined bottom_line(left_pos, -1, 0, -1);
                    if (line.intersects(bottom_line, iv)) {
                        if (iv.m_x < 0 && iv.length() < v.length()) {
                            v = -iv;
                        }
                    }

                    c->m_used_left_pos = c->m_pos - v;
                }
            }

            _m_must_update = false;
            return true;
        }

        return false;
    }


    /* !!!!!
    void BezierValueCurve::setViewportToFit(Viewport& viewport, bool fit_all_flag) noexcept {

        if (hasPoints()) {
            return;
        }

        if (selectedPointsCount() < 1) {
            fit_all_flag = true;
        }


        int32_t n = 0;
        double min_x = DBL_MAX, min_y = DBL_MAX, max_x = -DBL_MAX, max_y = -DBL_MAX;

        int32_t index = 0;
        int32_t last_index = length() - 1;

        for (auto& point : m_points) {

            if (fit_all_flag || point.isSelected()) {

                min_x = std::min(min_x, point.m_pos.m_x);
                max_x = std::max(max_x, point.m_pos.m_x);
                min_y = std::min(min_y, point.m_pos.m_y);
                max_y = std::max(max_y, point.m_pos.m_y);

                if (point.hasVisibleControlPoints()) {
                    if (index == 0) {
                        min_x = std::min(min_x, point.m_left_pos.m_x);
                        min_x = std::min(min_x, point.m_right_pos.m_x);
                    }

                    if (index == last_index) {
                        max_x = std::max(max_x, point.m_left_pos.m_x);
                        max_x = std::max(max_x, point.m_right_pos.m_x);
                    }

                    min_y = std::min(min_y, point.m_left_pos.m_y);
                    min_y = std::min(min_y, point.m_right_pos.m_y);

                    max_y = std::max(max_y, point.m_left_pos.m_y);
                    max_y = std::max(max_y, point.m_right_pos.m_y);
                }

                n++;
            }

            index++;
        }

        if (n > 1) {

            double new_x_range = std::clamp<double>(std::fabs(max_x - min_x), 0.1, DBL_MAX);
            double new_y_range = std::clamp<double>(std::fabs(max_y - min_y), 0.1, DBL_MAX);

            min_x -= new_x_range * 0.1;
            max_x += new_x_range * 0.1;
            min_y -= new_y_range * 0.1;
            max_y += new_y_range * 0.1;

            new_x_range = std::fabs(max_x - min_x);
            new_y_range = std::fabs(max_y - min_y);

            // viewport.setRange(minX, maxX, minY, maxY); TODO: !!!!
        }
    }
     */



    BezierValueCurveDrawSettings::BezierValueCurveDrawSettings() noexcept {
    /* TODO !!!!! Implement!
        m_point_colors[(int32_t)Bezier::PointType::Linear].set(1, 1, 1);
        m_point_colors[(int32_t)Bezier::PointType::Corner].set(0.5f, 0.1f, 0.5f);
        m_point_colors[(int32_t)Bezier::PointType::Smooth1].set(0.9f, 0.3f, 0.1f);
        m_point_colors[(int32_t)Bezier::PointType::Smooth2].set(1, 0.8f, 0.2f);
        m_point_colors[(int32_t)Bezier::PointType::Right].set(0.7f, 0.1f, 0.1f);
        m_point_colors[(int32_t)Bezier::PointType::Left].set(0.1f, 0.7f, 0.1f);
        */
    }

    /* TODO !!!!! Implement!
    RGB BezierValueCurveDrawSettings::pointColor(BezierValueCurvePoint *point) const noexcept {
        return point ? m_point_colors[(int32_t)point->pointType()] : RGB();
    }
     */


    /* TODO !!!!! Implement!
    void BezierValueCurveDrawSettings::setStrokeColor(const RGB& color) noexcept {

        m_stroke_color = color;
        m_point_colors[(int32_t)Bezier::PointType::Linear] = color;
    }
     */


} // End of namespace Grain
