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

        if (point_type != point_type_) {
            changed = true;

            if (point_type == Bezier::PointType::Linear) {
            }
            else if (point_type == Bezier::PointType::Corner) {
                if (point_type_ == Bezier::PointType::Linear ||
                    point_type_ == Bezier::PointType::Right ||
                    point_type_ == Bezier::PointType::Left) {
                    left_.set(-1, 0);    // TODO:
                    right_.set(1, 0);
                }
            }
            else if (point_type == Bezier::PointType::Smooth1) {
                if (point_type_ == Bezier::PointType::Linear ||
                    point_type_ == Bezier::PointType::Corner ||
                    point_type_ == Bezier::PointType::Right ||
                    point_type_ == Bezier::PointType::Left) {
                    left_.set(-1, 0);    // TODO:
                    right_.set(1, 0);
                }
            }
            else if (point_type == Bezier::PointType::Smooth2) {
                if (point_type_ == Bezier::PointType::Linear ||
                    point_type_ == Bezier::PointType::Corner ||
                    point_type_ == Bezier::PointType::Right ||
                    point_type_ == Bezier::PointType::Left) {
                    left_.set(-1, 0);    // TODO:
                    right_.set(1, 0);
                }
                else if (point_type_ == Bezier::PointType::Smooth1) {
                    left_ = (left_ - right_) * 0.5;
                    right_ = -left_;
                }
            }
            else if (point_type == Bezier::PointType::Right) {
            }
            else if (point_type == Bezier::PointType::Left) {
            }

            point_type_ = point_type;
        }

        if (changed) {
            bezier_value_curve_->mustUpdate();
        }

        return changed;
    }


    double BezierValueCurvePoint::leftDistance() const noexcept {
        return hasLeftControl() ? (left_pos_ - pos_).length() : 0;
    }


    double BezierValueCurvePoint::rightDistance() const noexcept {
        return hasRightControl() ? (right_ - pos_).length() : 0;
    }

    bool BezierValueCurvePoint::hasVisibleControlPoints() const noexcept {
        return
                point_type_ == Bezier::PointType::Corner ||
                point_type_ == Bezier::PointType::Smooth1 ||
                point_type_ == Bezier::PointType::Smooth2;
    }


    bool BezierValueCurvePoint::hasLeftControl() const noexcept {
        return
                point_type_ == Bezier::PointType::Corner ||
                point_type_ == Bezier::PointType::Smooth1 ||
                point_type_ == Bezier::PointType::Smooth2;
    }


    bool BezierValueCurvePoint::hasRightControl() const noexcept {
        return
                point_type_ == Bezier::PointType::Corner ||
                point_type_ == Bezier::PointType::Smooth1 ||
                point_type_ == Bezier::PointType::Smooth2;
    }


    bool BezierValueCurvePoint::usesLeftControl() const noexcept {
        return
                point_type_ == Bezier::PointType::Corner ||
                point_type_ == Bezier::PointType::Smooth1 ||
                point_type_ == Bezier::PointType::Smooth2 ||
                point_type_ == Bezier::PointType::Left;
    }


    bool BezierValueCurvePoint::usesRightControl() const noexcept {
        return
                point_type_ == Bezier::PointType::Corner ||
                point_type_ == Bezier::PointType::Smooth1 ||
                point_type_ == Bezier::PointType::Smooth2 ||
                point_type_ == Bezier::PointType::Right;
    }


    bool BezierValueCurvePoint::isSmooth() const noexcept {
        return point_type_ == Bezier::PointType::Smooth1 || point_type_ == Bezier::PointType::Smooth2;
    }


    bool BezierValueCurvePoint::changeStatus(Status mask, bool flag) noexcept {
        Status old_status = status_;
        status_ = (Status)(flag ? status_ | mask : status_ & ~mask);
        return status_ != old_status;
    }


    void BezierValueCurvePoint::invertSelection() noexcept {
        status_ = (Status)(status_ & Status::Selected ? status_ & ~Status::Selected : status_ | Status::Selected);
    }


    bool BezierValueCurvePoint::setFixed(bool h_flag, bool v_flag) noexcept {
        bool changed = false;
        changed |= changeStatus(Status::FixedX, h_flag);
        changed |= changeStatus(Status::FixedY, v_flag);
        return changed;
    }


    void BezierValueCurvePoint::setPos(const Vec2d& pos) noexcept {
        pos_ = pos;
        curveMustUpdate();
    }


    void BezierValueCurvePoint::setX(double x) noexcept {
        pos_.x_ = x; curveMustUpdate();
    }


    void BezierValueCurvePoint::setY(double y) noexcept {
        pos_.y_ = y; curveMustUpdate();
    }


    void BezierValueCurvePoint::clampX(const double min, const double max) noexcept {
        pos_.clampX(min, max);
        curveMustUpdate();
    }


    void BezierValueCurvePoint::clampY(const double min, const double max) noexcept {
        pos_.clampY(min, max);
        curveMustUpdate();
    }


    void BezierValueCurvePoint::curveMustUpdate() const noexcept {
        if (bezier_value_curve_) {
            bezier_value_curve_->mustUpdate();
        }
    }


    int32_t _BezierValueCurvePoint_compare(const void* a, const void* b) {
        return ((BezierValueCurvePoint*)a)->compare((BezierValueCurvePoint*)b);
    }


    void BezierValueCurvePoint::remember() noexcept {
        remembered_pos_ = pos_;
        remembered_status_ = status_;
    }


    void BezierValueCurvePoint::backToRememberedStatus() noexcept {
        status_ = remembered_status_;
    }


    void BezierValueCurvePoint::startMouseAction(BezierValueCurvePoint::Part part, const Viewport& viewport) noexcept {
        Vec2d posInViewport, leftInViewport, rightInViewport;
        // posInViewport = viewport.posToView(m_pos);
        // leftInViewport = viewport.posToView(m_left_pos);
        // rightInViewport = viewport.posToView(m_right_pos);   TODO: !!!
    }

    BezierValueCurvePoint::Part BezierValueCurvePoint::hit(const Viewport& viewport, const Vec2d& pos, double& min_distance) const noexcept {
        Part part = Part::None;

        /* TODO: !!!!!
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
        */

        return part;
    }


    int32_t BezierValueCurvePoint::compare(const BezierValueCurvePoint* point) const noexcept {
        if (point) {
            if (pos_.x_ > point->pos_.x_) {
                return 1;
            }
            if (pos_.x_ < point->pos_.x_) {
                return -1;
            }
        }

        return 0;
    }


    BezierValueCurve::BezierValueCurve() noexcept {
        points_.reserve(16);
    }


    BezierValueCurve::BezierValueCurve(const BezierValueCurve& bezier_value_curve) noexcept : Object() {
        // TODO: Implement
        #pragma message("BezierValueCurve::BezierValueCurve(const BezierValueCurve& bezier_value_curve) must be implemented!")
    }


    BezierValueCurve::~BezierValueCurve() noexcept {
        delete weighted_samples_;
    }


    bool BezierValueCurve::isValid() const noexcept {
        if (mode_ == Mode::Envelope) {
            bool has_decay_begin = false;

            for (auto& point : points_) {
                if (point.status_ & BezierValueCurvePoint::Status::DecayBegin) {
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

        for (auto& point : points_) {
            if (point.isSelected()) {
                n++;
            }
        }

        return n;
    }


    int32_t BezierValueCurve::decayPointIndex() const noexcept {
        int32_t index = 0;

        for (auto& point : points_) {
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

        for (auto& point : points_) {
            double y = point.pos_.y_;
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

        for (auto& point : points_) {
            if (point.isSelected() || !selected_only) {
                if (point.pos_.x_ < min_x) min_x = point.pos_.x_;
                if (point.pos_.x_ > max_x) max_x = point.pos_.x_;
                if (point.pos_.y_ < min_y) min_y = point.pos_.y_;
                if (point.pos_.y_ > max_y) max_y = point.pos_.y_;
            }
        }

        return { min_x, min_y, max_x - min_x, max_y - min_y };
    }


    const BezierValueCurvePoint* BezierValueCurve::pointAtIndex(int32_t index) const noexcept {
        return index >= 0 && index < static_cast<int32_t>(points_.size()) ? &points_[index] : nullptr;
    }


    BezierValueCurvePoint* BezierValueCurve::mutPointAtIndex(int32_t index) noexcept {
        return index >= 0 && index < static_cast<int32_t>(points_.size()) ? &points_[index] : nullptr;
    }


    BezierValueCurvePoint* BezierValueCurve::firstSelectedPoint() noexcept {
        for (auto& point : points_) {
            if (point.isSelected()) {
                return &point;
            }
        }

        return nullptr;
    }


    BezierValueCurvePoint* BezierValueCurve::decayBeginPoint() noexcept {
        for (auto& point : points_) {
            if (point.isDecayBegin())
                return &point;
        }

        return nullptr;
    }


    void BezierValueCurve::setLimits(double min_x, double max_x, double min_y, double max_y) noexcept {
        limit_min_x_ = min_x;
        limit_max_x_ = max_x;
        limit_min_y_ = min_y;
        limit_max_y_ = max_y;
    }


    int32_t BezierValueCurve::decayBeginIndex() const noexcept {
        int32_t index = 0;
        for (auto& point : points_) {
            if (point.isDecayBegin()) {
                return index;
            }
            index++;
        }

        return -1;
    }


    void BezierValueCurve::setDecayBeginIndex(int32_t decay_begin_index) noexcept {
        int32_t index = 0;
        for (auto& point : points_) {
            point.changeStatus(BezierValueCurvePoint::Status::DecayBegin, index == decay_begin_index);
            index++;
        }
    }


    void BezierValueCurve::selectAllPoints() noexcept {
        for (auto& point : points_) {
            point.select();
        }
    }


    void BezierValueCurve::deselectAllPoints() noexcept {
        for (auto& point : points_) {
            point.deselect();
        }
    }


    void BezierValueCurve::selectPointsInRect(const Rectd& rect) noexcept {
        Rectd selection_rect(rect);
        selection_rect.makePositiveSize();

        for (int32_t index = 0; index < length(); index++) {
            if (auto point = mutPointAtIndex(index)) {
                if (selection_rect.contains(point->pos_)) {
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

            if (c->isSelected() && c->point_type_ != point_type) {
                switch (point_type) {
                    case Bezier::PointType::Linear:
                        break;

                    case Bezier::PointType::Corner:
                        if (c->point_type_ == Bezier::PointType::Linear ||
                            c->point_type_ == Bezier::PointType::Right ||
                            c->point_type_ == Bezier::PointType::Left) {
                            if (l) {
                                c->left_.x_ = (l->pos_.x_ - c->pos_.x_) / 2;
                                c->left_.y_ = 0;
                            }
                            if (r) {
                                c->right_.x_ = (r->pos_.x_ - c->pos_.x_) / 2;
                                c->right_.y_ = 0;
                            }
                        }
                        break;

                    case Bezier::PointType::Smooth1:
                        if (c->point_type_ == Bezier::PointType::Linear ||
                            c->point_type_ == Bezier::PointType::Corner ||
                            c->point_type_ == Bezier::PointType::Right ||
                            c->point_type_ == Bezier::PointType::Left) {
                            if (l) {
                                c->left_.x_ = (l->pos_.x_ - c->pos_.x_) / 2;
                                c->left_.y_ = 0;
                            }
                            if (r) {
                                c->right_.x_ = (r->pos_.x_ - c->pos_.x_) / 2;
                                c->right_.y_ = 0;
                            }
                        }
                        break;

                    case Bezier::PointType::Smooth2:
                        if (c->point_type_ == Bezier::PointType::Linear ||
                            c->point_type_ == Bezier::PointType::Corner ||
                            c->point_type_ == Bezier::PointType::Right ||
                            c->point_type_ == Bezier::PointType::Left) {
                            double length = 0;
                            int32_t n = 0;
                            if (l) {
                                length = c->pos_.x_ - l->pos_.x_;
                                n++;
                            }
                            if (r) {
                                length += r->pos_.x_ - c->pos_.x_;
                                n++;
                            }
                            if (n > 0) {
                                length /= n * 2;
                                c->left_.x_ = -length;
                                c->right_.x_ = length;
                                c->left_.y_ = 0;
                                c->right_.y_ = 0;
                            }
                        }
                        else if (c->point_type_ == Bezier::PointType::Smooth1) {
                            Vec2d v;
                            if (l && r) {
                                v = (-c->left_ + c->right_) * 0.5;
                                c->left_ = -v;
                                c->right_ = v;
                            }
                            if (!l) {
                                c->left_ = -c->right_;
                            }
                            else if (!r) {
                                c->right_ = -c->left_;
                            }
                        }
                        break;

                    default:
                        break;
                }

                c->point_type_ = point_type;
                changed = true;
            }
        }

        return changed;
    }


    bool BezierValueCurve::horizontalCenterSelectedPoints() noexcept {
        bool changed = false;

        BezierValueCurvePoint* a = nullptr;
        BezierValueCurvePoint* b = nullptr;

        for (auto& point : points_) {
            if (a && b) {
                if (b->isSelected() && !a->isSelected() && !point.isSelected()) {
                    b->pos_.x_ = a->pos_.x_ + (point.pos_.x_ - a->pos_.x_) / 2;
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

        for (auto& point : points_) {
            if (a && b) {
                if (b->isSelected() && !a->isSelected() && !point.isSelected()) {
                    b->pos_.y_ = a->pos_.y_ + (point.pos_.y_ - a->pos_.y_) / 2;
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
            case Alignment::Bottom: y = rect.y_; break;
            case Alignment::No: y = 0; break;
            default: return false;
        }

        for (auto& point : points_) {
            if (point.isSelected()) {
                point.pos_.y_ = y;
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
            points_.push(BezierValueCurvePoint());
            auto point = mutLastPoint();
            if (!point) {
                throw ErrorCode::Specific;
            }
            point->bezier_value_curve_ = this;
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
        points_.push(BezierValueCurvePoint());

        auto point = mutLastPoint();
        if (!point) {
            return nullptr;
        }

        point->bezier_value_curve_ = this;
        point->pos_.x_ = x;
        point->pos_.y_ = y;
        point->left_.x_ = lx;
        point->left_.y_ = ly;
        point->right_.x_ = rx;
        point->right_.y_ = ry;
        point->point_type_ = point_type;
        point->status_ = status;

        mustUpdate();

        return point;
    }


    bool BezierValueCurve::removePoint(int32_t index) noexcept {
        if (index >= 0 && index < static_cast<int32_t>(points_.size()) - 1) {
            points_.removeAtIndex(index);
            mustUpdate();
            return true;
        }

        return false;
    }


    int32_t BezierValueCurve::removeSelectedPoints() noexcept {
        int32_t result = 0;

        if (selectedPointsCount() > 0) {
            auto n = static_cast<int32_t>(points_.size());
            for (int32_t i = n - 1; i >= 0; i--) {
                if (points_[i].isSelected()) {
                    points_.removeAtIndex(i);
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
        points_.clear();
    }


    void BezierValueCurve::flipVertical() noexcept {
        if (hasPoints()) {
            double min_y, max_y;
            rangeY(min_y, max_y);
            double offset = min_y + max_y;

            for (auto& point : points_) {
                point.pos_.y_ = offset - point.pos_.y_;
                point.left_.y_ = -point.left_.y_;
                point.right_.y_ = -point.right_.y_;
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
        if (point_a->point_type_ == Bezier::PointType::Linear && point_b->point_type_ == Bezier::PointType::Linear) {
            point_type = Bezier::PointType::Linear;
        }

        auto new_point = addPoint(sub_bezier1.pos_[0].x_, sub_bezier1.pos_[0].y_, 0, 0, 0, 0, point_type);
        if (!new_point) {
            return false;
        }

        if (select) {
            new_point->select();
        }

        new_point->left_ = sub_bezier0.pos_[2] - sub_bezier0.pos_[3];
        new_point->right_ = sub_bezier1.pos_[1] - sub_bezier1.pos_[0];

        point_a->right_ = sub_bezier0.pos_[1] - sub_bezier0.pos_[0];
        point_b->left_ = sub_bezier1.pos_[2] - sub_bezier1.pos_[3];

        if (point_a->point_type_ == Bezier::PointType::Smooth2) {
            point_a->point_type_ = Bezier::PointType::Smooth1;
        }
        if (point_b->point_type_ == Bezier::PointType::Smooth2) {
            point_b->point_type_ = Bezier::PointType::Smooth1;
        }

        mustUpdate();

        return true;
    }


    void BezierValueCurve::rememberAllPoints() noexcept {
        for (auto& point : points_) {
            point.remember();
        }
    }


    bool BezierValueCurve::moveRememberedSelectedPoints(const Vec2d& delta) noexcept {
        bool changed = false;

        for (auto& point : points_) {
            if (point.isSelected()) {
                if (!point.isXFixed()) {
                    double new_x = std::clamp<double>(point.remembered_pos_.x_ + delta.x_, limit_min_x_, limit_max_x_);
                    if (new_x != point.pos_.x_) {
                        point.pos_.x_ = new_x;
                        changed = true;
                    }
                }

                if (!point.isYFixed()) {
                    double new_y = std::clamp<double>(point.remembered_pos_.y_ + delta.y_, limit_min_y_, limit_max_y_);
                    if (new_y != point.pos_.y_) {
                        point.pos_.y_ = new_y;
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
        must_sort_ = true;
        modification_count_++;
    }


    void BezierValueCurve::mustUpdate() noexcept {
        must_sort_ = true;
        must_update_ = true;
        modification_count_++;
    }


    float BezierValueCurve::lookup(float t, int32_t resolution) noexcept {
        _updateWeightedSamples(resolution);
        if (!weighted_samples_) {
            return 0;
        }

        return weighted_samples_->lookup(t);
    }


    bool BezierValueCurve::fillLUT(LUT1* lut) noexcept {
        if (!lut) {
            return false;
        }

        if (!_updateWeightedSamples(lut->resolution())) {
            return false;
        }

        if (!lut->setByWeightedSamples(weighted_samples_)) {
            return false;
        }

        return true;
    }


    bool BezierValueCurve::fillEnvelopeAttackLUT(LUT1* lut) noexcept {
        if (lut && mode_ == Mode::Envelope) {
            int32_t end_index = decayBeginIndex();
            if (end_index >= 0) {
                return _updateLUT(lut, 0, end_index) == ErrorCode::None;
            }
        }

        return false;
    }


    bool BezierValueCurve::fillEnvelopeDecayLUT(LUT1* lut) noexcept {
        if (lut && mode_ == Mode::Envelope) {
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

        double start_x = start_point->pos_.x_;
        double end_x = end_point->pos_.x_;
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

            double x1 = p1->pos_.x_;
            double x2 = p2->pos_.x_;
            double segment_width = x2 - x1;

            double f = segment_width / width;
            t += f;

            int32_t index2 = std::round(t * length);

            Vec2d bp1 = p1->pos_;
            Vec2d bp2 = p1->used_right_pos_;
            Vec2d bp3 = p2->used_left_pos_;
            Vec2d bp4 = p2->pos_;

            float scale = 1.0f / width;
            bp1.x_ = (bp1.x_ - start_x) * scale;
            bp2.x_ = (bp2.x_ - start_x) * scale;
            bp3.x_ = (bp3.x_ - start_x) * scale;
            bp4.x_ = (bp4.x_ - start_x) * scale;

            Bezier bezier(bp1, bp2, bp3, bp4);

            for (int32_t i = index1; i < index2; i++) {
                double x = bp1.x_ + static_cast<double>(i - index1) / (index2 - index1) * (bp4.x_ - bp1.x_);

                double roots[3];
                int32_t n = Math::solveCubicBezier(bp1.x_, bp2.x_, bp3.x_, bp4.x_, x, roots);
                if (n > 0) {
                    double t = roots[0];
                    Vec2d sample = bezier.posOnCurve(t);
                    buffer[i] = sample.y_;
                }

            }

            index1 = index2;
        }


        return ErrorCode::None;
    }


    void BezierValueCurve::_sortPoints() noexcept {
        if (must_sort_) {
            std::sort(points_.begin(), points_.end(), sortPointsCompareFunc);
            must_sort_ = false;
        }
    }


    bool BezierValueCurve::sortPointsCompareFunc(const BezierValueCurvePoint& a, const BezierValueCurvePoint& b) {
        return a.pos_.x_ < b.pos_.x_;
    }


    bool BezierValueCurve::_updateWeightedSamples(int32_t resolution) noexcept {
        return _updateWeightedSamples(0, length() - 1, resolution);
    }


    bool BezierValueCurve::_updateWeightedSamples(int32_t start_point_index, int32_t end_point_index, int32_t resolution) noexcept {
        if (weighted_samples_modification_count_ >= modification_count_ &&
            weighted_samples_resolution_ == resolution &&
            weighted_samples_start_point_index_ == start_point_index &&
            weighted_samples_end_point_index_ == end_point_index) {
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

        if (!weighted_samples_) {
            weighted_samples_ = new (std::nothrow) WeightedSamples(resolution);
        }
        else {
            if (weighted_samples_->setResolution(resolution) != ErrorCode::None) {
                return false;
            }
        }

        if (!weighted_samples_) {
            // GrApp::addError(Gr::ERR_FATAL, "BezierCurve::_updateWeightedSamples()"); TODO: !!!!
            return false;
        }

        weighted_samples_->clear();
        _update();

        auto start_point = pointAtIndex(start_point_index);
        auto end_point = pointAtIndex(end_point_index);
        if (!start_point || !end_point) {
            return false;
        }

        double min_x = start_point ? start_point->pos_.x_ : 0.0;
        double max_x = end_point ? end_point->pos_.x_ : 0.0;
        double width = max_x - min_x;
        if (width < std::numeric_limits<double>::epsilon()) {
            width = std::numeric_limits<double>::epsilon();
        }
        double scale = 1.0 / width;

        for (int32_t i = 1; i < point_n; i++) {

            auto cp1 = pointAtIndex(start_point_index + i - 1);
            auto cp2 = pointAtIndex(start_point_index + i);

            Vec2d bp1 = cp1->pos_;
            Vec2d bp2 = cp1->used_right_pos_;
            Vec2d bp3 = cp2->used_left_pos_;
            Vec2d bp4 = cp2->pos_;

            bp1.x_ = (bp1.x_ - min_x) * scale;
            bp2.x_ = (bp2.x_ - min_x) * scale;
            bp3.x_ = (bp3.x_ - min_x) * scale;
            bp4.x_ = (bp4.x_ - min_x) * scale;

            Bezier bezier(bp1, bp2, bp3, bp4);
            weighted_samples_->addBezier(bezier, 256); // TODO: configurable resolution?
        }

        weighted_samples_->finish();

        weighted_samples_modification_count_ = modification_count_;
        weighted_samples_resolution_ = resolution;
        weighted_samples_start_point_index_ = start_point_index;
        weighted_samples_end_point_index_ = end_point_index;

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

        lut->setByWeightedSamples(weighted_samples_);

        return ErrorCode::None;
    }


    bool BezierValueCurve::_update() noexcept {
        _sortPoints();
        int32_t point_n = length();

        if (must_update_ && point_n > 1) {

            // Pass 1, left to right
            for (int32_t i = 0; i < point_n - 1; i++) {
                auto p0 = mutPointAtIndex(i);
                auto p1 = mutPointAtIndex(i + 1);

                if (p0->right_.x_ < 0) {
                    p0->right_.x_ = 0;
                }

                Vec2d v = p0->right_;
                if (v.x_ > 0) {
                    if (v.x_ > p1->pos_.x_ - p0->pos_.x_) {
                        v.x_ = p1->pos_.x_ - p0->pos_.x_;
                        v.y_ = p0->right_.y_ * v.x_ / p0->right_.x_;
                    }
                }

                p0->right_pos_ = p0->pos_ + p0->right_;
                p0->used_right_pos_ = p0->pos_ + v;
            }

            // Pass 2, right to left
            for (int32_t i = point_n - 1; i > 0; i--) {
                auto p0 = mutPointAtIndex(i);
                auto p1 = mutPointAtIndex(i - 1);

                if (p0->left_.x_ > 0) {
                    p0->left_.x_ = 0;
                }

                Vec2d v = p0->left_;
                if (v.x_ < 0) {
                    if (v.x_ < p1->pos_.x_ - p0->pos_.x_) {
                        v.x_ = p1->pos_.x_ - p0->pos_.x_;
                        v.y_ = p0->left_.y_ * v.x_ / p0->left_.x_;
                    }
                }
                p0->left_pos_ = p0->pos_ + p0->left_;
                p0->used_left_pos_ = p0->pos_ + v;
            }

            // Pass 3, automatic right continuity
            auto p = mutPointAtIndex(0);
            if (p->point_type_ == Bezier::PointType::Right) {
                p->used_right_pos_ = p->pos_;
            }

            for (int32_t i = 1; i < point_n - 1; i++) {
                auto c = mutPointAtIndex(i);

                if (c->point_type_ == Bezier::PointType::Right) {
                    auto l = mutPointAtIndex(i - 1);
                    auto r = mutPointAtIndex(i + 1);

                    Vec2d v = c->pos_ - (l->usesRightControl() ? l->used_right_pos_ : l->pos_);
                    Lined line(0, 0, v.x_, v.y_);
                    double right_pos = (r->pos_.x_ - c->pos_.x_) / 2;

                    Lined top_line(0, 1, right_pos, 1);
                    Vec2d iv;
                    if (line.intersects(top_line, iv)) {
                        if (iv.x_ > 0 && iv.length() < v.length()) {
                            v = iv;
                        }
                    }

                    Lined bottom_line(0, -1, right_pos, -1);
                    if (line.intersects(bottom_line, iv)) {
                        if (iv.x_ > 0 && iv.length() < v.length()) {
                            v = iv;
                        }
                    }

                    Lined right_line(right_pos, -1, right_pos, 1);
                    if (line.intersects(right_line, iv)) {
                        if (iv.x_ > 0 && iv.length() < v.length()) {
                            v = iv;
                        }
                    }

                    c->used_right_pos_ = c->pos_ + v;
                }
            }

            // Pass 4, automatic left continuity
            p = mutPointAtIndex(point_n - 1);
            if (p->point_type_ == Bezier::PointType::Left) {
                p->used_left_pos_ = p->pos_;
            }

            for (int32_t i = point_n - 2; i > 0; i--) {
                auto c = mutPointAtIndex(i);

                if (c->point_type_ == Bezier::PointType::Left) {
                    auto l = mutPointAtIndex(i - 1);
                    auto r = mutPointAtIndex(i + 1);

                    Vec2d v = (r->usesLeftControl() ? r->used_left_pos_ : r->pos_) - c->pos_;
                    Lined line(0, 0, v.x_, v.y_);
                    double left_pos = (l->pos_.x_ - c->pos_.x_) / 2;

                    Vec2d iv;

                    Lined left_line(left_pos, -1, left_pos, 1);
                    if (line.intersects(left_line, iv)) {
                        if (iv.x_ < 0 && iv.length() < v.length()) {
                            v = -iv;
                        }
                    }

                    Lined top_line(left_pos, 1, 0, 1);
                    if (line.intersects(top_line, iv)) {
                        if (iv.x_ < 0 && iv.length() < v.length()) {
                            v = -iv;
                        }
                    }

                    Lined bottom_line(left_pos, -1, 0, -1);
                    if (line.intersects(bottom_line, iv)) {
                        if (iv.x_ < 0 && iv.length() < v.length()) {
                            v = -iv;
                        }
                    }

                    c->used_left_pos_ = c->pos_ - v;
                }
            }

            must_update_ = false;
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

        for (auto& point : points_) {
            if (fit_all_flag || point.isSelected()) {

                min_x = std::min(min_x, point.m_pos.m_x);
                max_x = std::max(max_x, point.m_pos.m_x);
                min_y = std::min(min_y, point.m_pos.y_);
                max_y = std::max(max_y, point.m_pos.y_);

                if (point.hasVisibleControlPoints()) {
                    if (index == 0) {
                        min_x = std::min(min_x, point.m_left_pos.m_x);
                        min_x = std::min(min_x, point.m_right_pos.m_x);
                    }

                    if (index == last_index) {
                        max_x = std::max(max_x, point.m_left_pos.m_x);
                        max_x = std::max(max_x, point.m_right_pos.m_x);
                    }

                    min_y = std::min(min_y, point.m_left_pos.y_);
                    min_y = std::min(min_y, point.m_right_pos.y_);

                    max_y = std::max(max_y, point.m_left_pos.y_);
                    max_y = std::max(max_y, point.m_right_pos.y_);
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
