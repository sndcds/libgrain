//
//  BezierValueCurve.hpp
//
//  Created by Roald Christesen on 18.09.2019
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainBezierValueCurve_hpp
#define GrainBezierValueCurve_hpp

#include "Grain.hpp"
#include "Math/Vec2.hpp"
#include "2d/Line.hpp"
#include "2d/Rect.hpp"
#include "Type/Object.hpp"
#include "Bezier.hpp"
#include "Type/List.hpp"

// #include "GrRGB.hpp" TODO: !!!!!


namespace Grain {

class BezierValueCurve;
class WeightedSamples;
class File;
class LUT1;
class Viewport;


/**
 *  @brief A single point on a BezierValueCurve.
 */
class BezierValueCurvePoint : public BaseObject {
    friend class Bezier;
    friend class BezierValueCurve;
    friend class Viewport;

public:
    enum Status {
        None = 0x0,
        Selected = 0x1 << 0,
        Undeletable = 0x1 << 1,
        FixedX = 0x1 << 2,
        FixedY = 0x1 << 3,
        DecayBegin = 0x1 << 4,

        Fixed = FixedX | FixedY,
        FixedEnd = FixedX | Undeletable
    };

    enum class Part {
        None,
        Point,
        Left,
        Right
    };

protected:
    BezierValueCurve* bezier_value_curve_ = nullptr;

    Vec2d pos_{};               ///< Coordinate
    Vec2d left_{};              ///< Left control point relative to `pos_`
    Vec2d right_{};             ///< Right control point relative to `pos_`
    Vec2d left_pos_{};          ///< Left control point absolute coordinate
    Vec2d right_pos_{};         ///< Right control point absolute coordinate
    Vec2d used_left_pos_{};     ///< Limited left control point, absolute coordinate
    Vec2d used_right_pos_{};    ///< Limited right control point, absolute coordinate

    Bezier::PointType point_type_ = Bezier::PointType::Linear;
    Status status_ = Status::None;

    Vec2d remembered_pos_;
    Status remembered_status_ = Status::None;

public:
    BezierValueCurvePoint() noexcept : BaseObject() {
        bezier_value_curve_ = nullptr;
        point_type_ = Bezier::PointType::Linear;
        status_ = Status::None;
    }

    [[nodiscard]] const char* className() const noexcept override { return "BezierValueCurvePoint"; }

    friend std::ostream& operator << (std::ostream& os, const BezierValueCurvePoint& o) {
        return os << o.pos_ << " .. " << o.left_ << " .. " << o.right_;
    }

    // Copy Constructor
    BezierValueCurvePoint(const BezierValueCurvePoint& other) noexcept
            : BaseObject(), // Call base class copy constructor.
              bezier_value_curve_(other.bezier_value_curve_),
              pos_(other.pos_),
              left_(other.left_),
              right_(other.right_),
              left_pos_(other.left_pos_),
              right_pos_(other.right_pos_),
              used_left_pos_(other.used_left_pos_),
              used_right_pos_(other.used_right_pos_),
              point_type_(other.point_type_),
              status_(other.status_),
              remembered_pos_(other.remembered_pos_),
              remembered_status_(other.remembered_status_) {
        // Additional copy logic if needed
    }

    // Copy Assignment Operator
    BezierValueCurvePoint& operator = (const BezierValueCurvePoint& other) noexcept {
        if (this == &other) {
            return *this; // Handle self-assignment
        }

        // Call base class assignment operator
        BaseObject::operator = (other);

        // Copy each member from `other`
        bezier_value_curve_ = other.bezier_value_curve_;
        pos_ = other.pos_;
        left_ = other.left_;
        right_ = other.right_;
        left_pos_ = other.left_pos_;
        right_pos_ = other.right_pos_;
        used_left_pos_ = other.used_left_pos_;
        used_right_pos_ = other.used_right_pos_;
        point_type_ = other.point_type_;
        status_ = other.status_;
        remembered_pos_ = other.remembered_pos_;
        remembered_status_ = other.remembered_status_;

        return *this;
    }

    // Move Constructor
    BezierValueCurvePoint(BezierValueCurvePoint&& other) noexcept
            : BaseObject(),
              bezier_value_curve_(other.bezier_value_curve_),
              pos_(std::move(other.pos_)),
              left_(std::move(other.left_)),
              right_(std::move(other.right_)),
              left_pos_(std::move(other.left_pos_)),
              right_pos_(std::move(other.right_pos_)),
              used_left_pos_(std::move(other.used_left_pos_)),
              used_right_pos_(std::move(other.used_right_pos_)),
              point_type_(other.point_type_),
              status_(other.status_),
              remembered_pos_(std::move(other.remembered_pos_)),
              remembered_status_(other.remembered_status_) {

        other.bezier_value_curve_ = nullptr; // Reset to default or safe value.
        other.status_ = Status::None; // Optional, reset other members.
    }

    // Move Assignment Operator.
    BezierValueCurvePoint& operator = (BezierValueCurvePoint&& other) noexcept {
        if (this == &other) {
            return *this; // Handle self-assignment.
        }

        // Call base class move assignment operator
        BaseObject::operator = (std::move(other));

        // Move each member from `other`
        bezier_value_curve_ = other.bezier_value_curve_;
        pos_ = std::move(other.pos_);
        left_ = std::move(other.left_);
        right_ = std::move(other.right_);
        left_pos_ = std::move(other.left_pos_);
        right_pos_ = std::move(other.right_pos_);
        used_left_pos_ = std::move(other.used_left_pos_);
        used_right_pos_ = std::move(other.used_right_pos_);
        point_type_ = other.point_type_;
        status_ = other.status_;
        remembered_pos_ = std::move(other.remembered_pos_);
        remembered_status_ = other.remembered_status_;

        other.bezier_value_curve_ = nullptr; // Reset to default or safe value
        other.status_ = Status::None; // Optional, reset other members

        return *this;
    }


    [[nodiscard]] Vec2d pos() const noexcept { return pos_; }
    [[nodiscard]] Vec2d left() const noexcept { return left_; }
    [[nodiscard]] Vec2d right() const noexcept { return right_; }
    [[nodiscard]] Vec2d leftPos() const noexcept { return left_pos_; }
    [[nodiscard]] Vec2d rightPos() const noexcept { return right_pos_; }
    [[nodiscard]] Vec2d usedLeftPos() const noexcept { return used_left_pos_; }
    [[nodiscard]] Vec2d usedRightPos() const noexcept { return used_right_pos_; }
    [[nodiscard]] Bezier::PointType pointType() const noexcept { return point_type_; }
    [[nodiscard]] Status status() const noexcept { return status_; }
    [[nodiscard]] double leftDistance() const noexcept;
    [[nodiscard]] double rightDistance() const noexcept;

    [[nodiscard]] bool isStatus(Status mask, Status status) const noexcept { return (status_ & mask) == status; }
    [[nodiscard]] bool isSelected() const noexcept { return status_ & Status::Selected; }
    [[nodiscard]] bool isDeleteable() const noexcept { return !(status_ & Status::Undeletable); }
    [[nodiscard]] bool isXFixed() const noexcept { return status_ & Status::FixedX; }
    [[nodiscard]] bool isYFixed() const noexcept { return status_ & Status::FixedY; }
    [[nodiscard]] bool isDecayBegin() const noexcept { return status_ & Status::DecayBegin; }

    [[nodiscard]] bool hasVisibleControlPoints() const noexcept;
    [[nodiscard]] bool hasLeftControl() const noexcept;
    [[nodiscard]] bool hasRightControl() const noexcept;
    [[nodiscard]] bool usesLeftControl() const noexcept;
    [[nodiscard]] bool usesRightControl() const noexcept;

    [[nodiscard]] bool isSmooth() const noexcept;

    bool setPointType(Bezier::PointType point_type) noexcept;
    bool changeStatus(Status status, bool flag) noexcept;
    bool select() noexcept { return changeStatus(Status::Selected, true); }
    bool deselect() noexcept { return changeStatus(Status::Selected, false); }
    void invertSelection() noexcept;
    bool setUndeletable(bool flag) noexcept { return changeStatus(Status::Undeletable, true); }
    bool setFixed(bool h_flag, bool v_flag) noexcept;
    bool setHorizontalFixed(bool flag) noexcept { return changeStatus(Status::FixedX, flag); }
    bool setVerticalFixed(bool flag) noexcept { return changeStatus(Status::FixedX, flag); }

    void setPos(const Vec2d& pos) noexcept;
    void setX(double x) noexcept;
    void setY(double y) noexcept;

    void clampX(double min, double max) noexcept;
    void clampY(double min, double max) noexcept;

    void curveMustUpdate() const noexcept;

    void remember() noexcept;
    void backToRememberedStatus() noexcept;

    void startMouseAction(BezierValueCurvePoint::Part part, const Viewport& viewport) noexcept;

    Part hit(const Viewport& viewport, const Vec2d& pos, double& min_distance) const noexcept;

    [[nodiscard]] int32_t compare(const BezierValueCurvePoint* point) const noexcept;
};


class BezierValueCurve : public Object {
public:
    enum {
        kErr_ToFewPoints,
        kErr_StartIndexOutOfRange,
        kErr_EndIndexOutOfRange,
        kErrInvalidWidth
    };

    enum class Mode {
        Standard = 0,
        Envelope,
        Filter
    };

protected:
    List<BezierValueCurvePoint> points_;
    WeightedSamples* weighted_samples_ = nullptr;

    int32_t default_resolution_ = 1000;

    Mode mode_ = Mode::Standard;
    double limit_min_x_ = 0.0;
    double limit_max_x_ = 1000.0;
    double limit_min_y_ = -1000.0;
    double limit_max_y_ = 1000.0;

    int32_t fractional_digits_ = 6;

    bool must_sort_ = false;
    bool must_update_ = true;
    int32_t modification_count_ = 0;

    int32_t weighted_samples_modification_count_ = 0;
    int32_t weighted_samples_resolution_ = -1;
    int32_t weighted_samples_start_point_index_ = -1;
    int32_t weighted_samples_end_point_index_ = -1;
    int32_t read_decay_point_index_ = -1;

public:
    BezierValueCurve() noexcept;
    BezierValueCurve(const BezierValueCurve& bezier_value_curve) noexcept;
    ~BezierValueCurve() noexcept override;

    [[nodiscard]] const char* className() const noexcept override { return "BezierValueCurve"; }

    friend std::ostream& operator << (std::ostream& os, const BezierValueCurve *o) {
        o == nullptr ? os << "Bezier nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const BezierValueCurve& o) {
        os << o.points_.size();
        return os;
    }


    [[nodiscard]] bool isValid() const noexcept;

    [[nodiscard]] int32_t defaultResolution() noexcept { return default_resolution_; }
    void setDefaultResolution(int32_t resolution) noexcept { default_resolution_ = resolution < 2 ? 2 : resolution; }

    [[nodiscard]] int32_t length() const noexcept { return static_cast<int32_t>(points_.size()); }
    [[nodiscard]] int32_t lastPointIndex() const noexcept { return static_cast<int32_t>(points_.size()) - 1; }
    [[nodiscard]] bool hasPoints() const noexcept { return length() > 0; }

    [[nodiscard]] int32_t selectedPointsCount() const noexcept;
    [[nodiscard]] int32_t modificationCount() const noexcept { return modification_count_; }

    [[nodiscard]] int32_t decayPointIndex() const noexcept;
    BezierValueCurvePoint* mutDecayPoint() noexcept { return mutPointAtIndex(decayPointIndex()); };

    void rangeY(double& out_min_y, double& out_max_y) const noexcept;
    [[nodiscard]] Rectd bbox(bool selected_only) const noexcept;

    [[nodiscard]] List<BezierValueCurvePoint> points() noexcept { return points_; }
    [[nodiscard]] int32_t pointCount() const noexcept { return static_cast<int32_t>(points_.size()); }
    [[nodiscard]] BezierValueCurvePoint* mutPointAtIndex(int32_t index) noexcept;
    [[nodiscard]] const BezierValueCurvePoint* pointAtIndex(int32_t index) const noexcept;
    [[nodiscard]] BezierValueCurvePoint* mutFirstPoint() noexcept { return mutPointAtIndex(0); }
    [[nodiscard]] const BezierValueCurvePoint* firstPoint() const noexcept { return pointAtIndex(0); }
    [[nodiscard]] BezierValueCurvePoint* mutLastPoint() noexcept { return mutPointAtIndex(pointCount() - 1); }
    [[nodiscard]] const BezierValueCurvePoint* lastPoint() const noexcept { return pointAtIndex(pointCount() - 1); }

    [[nodiscard]] BezierValueCurvePoint* firstSelectedPoint() noexcept;
    [[nodiscard]] BezierValueCurvePoint* decayBeginPoint() noexcept;


    [[nodiscard]] Mode mode() const noexcept { return mode_; }
    void setMode(Mode mode) noexcept { mode_ = mode; }


    void setLimits(double min_x, double max_x, double min_y, double max_y) noexcept;
    [[nodiscard]] int32_t decayBeginIndex() const noexcept;
    void setDecayBeginIndex(int32_t decay_begin_index) noexcept;


    bool setTypeOfSelectedPoints(Bezier::PointType point_type) noexcept;
    bool horizontalCenterSelectedPoints() noexcept;
    bool verticalCenterSelectedPoints() noexcept;
    bool alignSelectedPoints(Alignment align) noexcept;

    void selectAllPoints() noexcept;
    void deselectAllPoints() noexcept;
    void selectPointsInRect(const Rectd& rect) noexcept;

    [[nodiscard]] BezierValueCurvePoint* addPoint() noexcept;
    BezierValueCurvePoint* addLinearPoint(double x, double y) noexcept;
    BezierValueCurvePoint* addPoint(double x, double y, double lx, double ly, double rx, double ry, Bezier::PointType point_type, BezierValueCurvePoint::Status status = BezierValueCurvePoint::Status::None) noexcept;
    bool removePoint(int32_t index) noexcept;
    [[nodiscard]] int32_t removeSelectedPoints() noexcept;
    void removeAllPoints() noexcept { points_.clear(); }
    void clear() noexcept;
    void flipVertical() noexcept;

    bool split(int32_t segment_index, float t, bool select = false) noexcept;

    void rememberAllPoints() noexcept;
    bool moveRememberedSelectedPoints(const Vec2d& delta) noexcept;

    void mustSort() noexcept;
    void mustUpdate() noexcept;

    [[nodiscard]] float lookup(float t) noexcept { return lookup(t, default_resolution_); }
    [[nodiscard]] float lookup(float t, int32_t resolution) noexcept;
    bool fillLUT(LUT1* lut) noexcept;
    bool fillEnvelopeAttackLUT(LUT1* lut) noexcept;
    bool fillEnvelopeDecayLUT(LUT1* lut) noexcept;
    ErrorCode fillBuffer(int32_t start_index, int32_t end_index, float* buffer, int32_t length) noexcept;

    bool _updateWeightedSamples(int32_t resolution) noexcept;
    bool _updateWeightedSamples(int32_t start_point_index, int32_t end_point_index, int32_t resolution) noexcept;
    ErrorCode _updateLUT(LUT1* lut) noexcept;
    ErrorCode _updateLUT(LUT1* lut, int32_t start_point_index, int32_t end_point_index) noexcept;


    void _sortPoints() noexcept;
    static bool sortPointsCompareFunc(const BezierValueCurvePoint& a, const BezierValueCurvePoint& b);
    bool _update() noexcept;

    // void setViewportToFit(Viewport& viewport, bool fit_all_flag) noexcept; !!!!!!
};


class BezierValueCurveDrawSettings {

    friend class Viewport; // !!!!!!

public:
    enum {
        kPointColorCount = 6
    };

protected:
    bool is_enabled_ = false;
    bool shows_keyboard_ = false;

    float alpha_ = 1.0f;
    float fill_alpha_ = 0;
    float point_alpha_ = 0.8f;
    // RGB stroke_color_; TODO: !!!!!!
    // RGB point_colors_[kPointColorCount]; TODO: !!!!!!
    float stroke_width_ = 1.4f;
    float point_radius_ = 3;
    float active_point_radius_ = 6;
    float control_radius_ = 4;

public:
    BezierValueCurveDrawSettings() noexcept;

    void enable() noexcept { is_enabled_ = true; }
    void disable() noexcept { is_enabled_ = false; }
    [[nodiscard]] bool isEnabled() const noexcept { return is_enabled_; }

    [[nodiscard]] bool shouldShowKeyboard() const noexcept { return shows_keyboard_; }

    [[nodiscard]] float alpha() const noexcept { return alpha_; }
    [[nodiscard]] float fillAlpha() const noexcept { return fill_alpha_; }
    [[nodiscard]] float pointAlpha() const noexcept { return point_alpha_; }
    [[nodiscard]] float pointRadius() const noexcept { return point_radius_; }
    [[nodiscard]] float activePointRadius() const noexcept { return active_point_radius_; }
    // RGB strokeColor() const noexcept { return stroke_color_; } TODO: !!!!!!
    // RGB pointColor(BezierValueCurvePoint* point) const noexcept; TODO: !!!!!!
    [[nodiscard]] float strokeWidth() const noexcept { return stroke_width_; }

    void setShowsKeyboard(bool show_keyboard) noexcept { shows_keyboard_ = show_keyboard; }
    void setAlpha(float alpha) noexcept { alpha_ = alpha; }
    void setFillAlpha(float fill_alpha) noexcept { fill_alpha_ = fill_alpha; }
    void setPointAlpha(float point_alpha) noexcept { point_alpha_ = point_alpha; }
    // void setStrokeColor(const RGB& color) noexcept; !!!!!!
    // void setPointColor(const RGB&  color) noexcept { stroke_color_ = color; } TODO: !!!!!!
    void setStrokeWidth(float width) noexcept { stroke_width_ = width; }
};


} // End of namespace Grain

#endif // GrainBezierValueCurve_hpp
