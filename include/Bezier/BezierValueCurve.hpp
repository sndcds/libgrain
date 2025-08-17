//
//  BezierValueCurve.hpp
//
//  Created by Roald Christesen on 18.09.2019
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainBezierValueCurve_hpp
#define GrainBezierValueCurve_hpp

#include "Grain.hpp"
#include "Math/Vec2.hpp"
#include "2d/Line.hpp"
#include "2d/Rect.hpp"
#include "Type/Object.hpp"
#include "Bezier.hpp"
// #include "GrRGB.hpp" !!!!!


namespace Grain {


    class BezierValueCurve;
    class WeightedSamples;
    class File;
    class LUT1;


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
        BezierValueCurve* m_bezier_value_curve = nullptr;

        Vec2d m_pos{};             ///< Coordinate
        Vec2d m_left{};            ///< Left control point relative to m_pos
        Vec2d m_right{};           ///< Right control point relative to m_pos
        Vec2d m_left_pos{};        ///< Left control point absolute coordinate
        Vec2d m_right_pos{};       ///< Right control point absolute coordinate
        Vec2d m_used_left_pos{};   ///< Limited left control point, absolute coordinate
        Vec2d m_used_right_pos{};  ///< Limited right control point, absolute coordinate

        Bezier::PointType m_point_type = Bezier::PointType::Linear;
        Status m_status = Status::None;

        Vec2d m_remembered_pos;
        Status m_remembered_status = Status::None;


    public:
        BezierValueCurvePoint() noexcept : BaseObject() {
            m_bezier_value_curve = nullptr;
            m_point_type = Bezier::PointType::Linear;
            m_status = Status::None;
        }

        [[nodiscard]] const char* className() const noexcept override { return "BezierValueCurvePoint"; }

        friend std::ostream& operator << (std::ostream& os, const BezierValueCurvePoint& o) {
            return os << o.m_pos << " .. " << o.m_left << " .. " << o.m_right;
        }

        // Copy Constructor
        BezierValueCurvePoint(const BezierValueCurvePoint& other) noexcept
                : BaseObject(), // Call base class copy constructor.
                  m_bezier_value_curve(other.m_bezier_value_curve),
                  m_pos(other.m_pos),
                  m_left(other.m_left),
                  m_right(other.m_right),
                  m_left_pos(other.m_left_pos),
                  m_right_pos(other.m_right_pos),
                  m_used_left_pos(other.m_used_left_pos),
                  m_used_right_pos(other.m_used_right_pos),
                  m_point_type(other.m_point_type),
                  m_status(other.m_status),
                  m_remembered_pos(other.m_remembered_pos),
                  m_remembered_status(other.m_remembered_status) {
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
            m_bezier_value_curve = other.m_bezier_value_curve;
            m_pos = other.m_pos;
            m_left = other.m_left;
            m_right = other.m_right;
            m_left_pos = other.m_left_pos;
            m_right_pos = other.m_right_pos;
            m_used_left_pos = other.m_used_left_pos;
            m_used_right_pos = other.m_used_right_pos;
            m_point_type = other.m_point_type;
            m_status = other.m_status;
            m_remembered_pos = other.m_remembered_pos;
            m_remembered_status = other.m_remembered_status;

            return *this;
        }

        // Move Constructor
        BezierValueCurvePoint(BezierValueCurvePoint&& other) noexcept
                : BaseObject(),
                  m_bezier_value_curve(other.m_bezier_value_curve),
                  m_pos(std::move(other.m_pos)),
                  m_left(std::move(other.m_left)),
                  m_right(std::move(other.m_right)),
                  m_left_pos(std::move(other.m_left_pos)),
                  m_right_pos(std::move(other.m_right_pos)),
                  m_used_left_pos(std::move(other.m_used_left_pos)),
                  m_used_right_pos(std::move(other.m_used_right_pos)),
                  m_point_type(other.m_point_type),
                  m_status(other.m_status),
                  m_remembered_pos(std::move(other.m_remembered_pos)),
                  m_remembered_status(other.m_remembered_status) {

            other.m_bezier_value_curve = nullptr; // Reset to default or safe value.
            other.m_status = Status::None; // Optional, reset other members.
        }

        // Move Assignment Operator.
        BezierValueCurvePoint& operator = (BezierValueCurvePoint&& other) noexcept {
            if (this == &other) {
                return *this; // Handle self-assignment.
            }

            // Call base class move assignment operator
            BaseObject::operator = (std::move(other));

            // Move each member from `other`
            m_bezier_value_curve = other.m_bezier_value_curve;
            m_pos = std::move(other.m_pos);
            m_left = std::move(other.m_left);
            m_right = std::move(other.m_right);
            m_left_pos = std::move(other.m_left_pos);
            m_right_pos = std::move(other.m_right_pos);
            m_used_left_pos = std::move(other.m_used_left_pos);
            m_used_right_pos = std::move(other.m_used_right_pos);
            m_point_type = other.m_point_type;
            m_status = other.m_status;
            m_remembered_pos = std::move(other.m_remembered_pos);
            m_remembered_status = other.m_remembered_status;

            other.m_bezier_value_curve = nullptr; // Reset to default or safe value
            other.m_status = Status::None; // Optional, reset other members

            return *this;
        }


        [[nodiscard]] Vec2d pos() const noexcept { return m_pos; }
        [[nodiscard]] Vec2d left() const noexcept { return m_left; }
        [[nodiscard]] Vec2d right() const noexcept { return m_right; }
        [[nodiscard]] Vec2d leftPos() const noexcept { return m_left_pos; }
        [[nodiscard]] Vec2d rightPos() const noexcept { return m_right_pos; }
        [[nodiscard]] Vec2d usedLeftPos() const noexcept { return m_used_left_pos; }
        [[nodiscard]] Vec2d usedRightPos() const noexcept { return m_used_right_pos; }
        [[nodiscard]] Bezier::PointType pointType() const noexcept { return m_point_type; }
        [[nodiscard]] Status status() const noexcept { return m_status; }
        [[nodiscard]] double leftDistance() const noexcept;
        [[nodiscard]] double rightDistance() const noexcept;

        [[nodiscard]] bool isStatus(Status mask, Status status) const noexcept { return (m_status & mask) == status; }
        [[nodiscard]] bool isSelected() const noexcept { return m_status & Status::Selected; }
        [[nodiscard]] bool isDeleteable() const noexcept { return !(m_status & Status::Undeletable); }
        [[nodiscard]] bool isXFixed() const noexcept { return m_status & Status::FixedX; }
        [[nodiscard]] bool isYFixed() const noexcept { return m_status & Status::FixedY; }
        [[nodiscard]] bool isDecayBegin() const noexcept { return m_status & Status::DecayBegin; }

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

        // void startMouseAction(BezierValueCurvePoint::Part part, const Viewport& viewport) noexcept; !!!!!!

        // Part hit(const Viewport& viewport, const Vec2d& pos, double& min_distance) const noexcept; !!!!!!

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
        std::vector<BezierValueCurvePoint> m_points;
        WeightedSamples* m_weighted_samples = nullptr;

        int32_t m_default_resolution = 1000;

        Mode m_mode = Mode::Standard;
        double m_limit_min_x = 0.0;
        double m_limit_max_x = 1000.0;
        double m_limit_min_y = -1000.0;
        double m_limit_max_y = 1000.0;

        int32_t m_fractional_digits = 6;

        bool _m_must_sort = false;
        bool _m_must_update = true;
        int32_t _m_modification_count = 0;

        int32_t _m_weighted_samples_modification_count = 0;
        int32_t _m_weighted_samples_resolution = -1;
        int32_t _m_weighted_samples_start_point_index = -1;
        int32_t _m_weighted_samples_end_point_index = -1;
        int32_t _m_read_decay_point_index = -1;

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
            os << o.m_points.size();
            return os;
        }


        [[nodiscard]] bool isValid() const noexcept;

        [[nodiscard]] int32_t defaultResolution() noexcept { return m_default_resolution; }
        void setDefaultResolution(int32_t resolution) noexcept { m_default_resolution = resolution < 2 ? 2 : resolution; }

        [[nodiscard]] int32_t length() const noexcept { return static_cast<int32_t>(m_points.size()); }
        [[nodiscard]] int32_t lastPointIndex() const noexcept { return static_cast<int32_t>(m_points.size()) - 1; }
        [[nodiscard]] bool hasPoints() const noexcept { return length() > 0; }

        [[nodiscard]] int32_t selectedPointsCount() const noexcept;
        [[nodiscard]] int32_t modificationCount() const noexcept { return _m_modification_count; }

        [[nodiscard]] int32_t decayPointIndex() const noexcept;
        BezierValueCurvePoint* mutDecayPoint() noexcept { return mutPointAtIndex(decayPointIndex()); };

        void rangeY(double& out_min_y, double& out_max_y) const noexcept;
        [[nodiscard]] Rectd bbox(bool selected_only) const noexcept;

        [[nodiscard]] std::vector<BezierValueCurvePoint> points() noexcept { return m_points; }
        [[nodiscard]] int32_t pointCount() const noexcept { return static_cast<int32_t>(m_points.size()); }
        [[nodiscard]] BezierValueCurvePoint* mutPointAtIndex(int32_t index) noexcept;
        [[nodiscard]] const BezierValueCurvePoint* pointAtIndex(int32_t index) const noexcept;
        [[nodiscard]] BezierValueCurvePoint* mutFirstPoint() noexcept { return mutPointAtIndex(0); }
        [[nodiscard]] const BezierValueCurvePoint* firstPoint() const noexcept { return pointAtIndex(0); }
        [[nodiscard]] BezierValueCurvePoint* mutLastPoint() noexcept { return mutPointAtIndex(pointCount() - 1); }
        [[nodiscard]] const BezierValueCurvePoint* lastPoint() const noexcept { return pointAtIndex(pointCount() - 1); }

        [[nodiscard]] BezierValueCurvePoint* firstSelectedPoint() noexcept;
        [[nodiscard]] BezierValueCurvePoint* decayBeginPoint() noexcept;


        [[nodiscard]] Mode mode() const noexcept { return m_mode; }
        void setMode(Mode mode) noexcept { m_mode = mode; }


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
        BezierValueCurvePoint* addPoint(double x, double y, double lx, double ly, double rx, double ry, Bezier::PointType point_type, BezierValueCurvePoint::Status status = BezierValueCurvePoint::Status::None) noexcept;
        bool removePoint(int32_t index) noexcept;
        [[nodiscard]] int32_t removeSelectedPoints() noexcept;
        void removeAllPoints() noexcept { m_points.clear(); }
        void clear() noexcept;
        void flipVertical() noexcept;

        bool split(int32_t segment_index, float t, bool select = false) noexcept;

        void rememberAllPoints() noexcept;
        bool moveRememberedSelectedPoints(const Vec2d& delta) noexcept;

        void mustSort() noexcept;
        void mustUpdate() noexcept;

        [[nodiscard]] float lookup(float t) noexcept { return lookup(t, m_default_resolution); }
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
        bool m_is_enabled = false;
        bool m_shows_keyboard = false;

        float m_alpha = 1;
        float m_fill_alpha = 0;
        float m_point_alpha = 0.8f;
        // RGB m_stroke_color; !!!!!!
        // RGB m_point_colors[kPointColorCount]; !!!!!!
        float m_stroke_width = 1.4f;
        float m_point_radius = 3;
        float m_active_point_radius = 6;
        float m_control_radius = 4;

    public:
        BezierValueCurveDrawSettings() noexcept;

        void enable() noexcept { m_is_enabled = true; }
        void disable() noexcept { m_is_enabled = false; }
        [[nodiscard]] bool isEnabled() const noexcept { return m_is_enabled; }

        [[nodiscard]] bool shouldShowKeyboard() const noexcept { return m_shows_keyboard; }

        [[nodiscard]] float alpha() const noexcept { return m_alpha; }
        [[nodiscard]] float fillAlpha() const noexcept { return m_fill_alpha; }
        [[nodiscard]] float pointAlpha() const noexcept { return m_point_alpha; }
        [[nodiscard]] float pointRadius() const noexcept { return m_point_radius; }
        [[nodiscard]] float activePointRadius() const noexcept { return m_active_point_radius; }
        // RGB strokeColor() const noexcept { return m_stroke_color; } !!!!!!
        // RGB pointColor(BezierValueCurvePoint* point) const noexcept; !!!!!!
        [[nodiscard]] float strokeWidth() const noexcept { return m_stroke_width; }

        void setShowsKeyboard(bool show_keyboard) noexcept { m_shows_keyboard = show_keyboard; }
        void setAlpha(float alpha) noexcept { m_alpha = alpha; }
        void setFillAlpha(float fill_alpha) noexcept { m_fill_alpha = fill_alpha; }
        void setPointAlpha(float point_alpha) noexcept { m_point_alpha = point_alpha; }
        // void setStrokeColor(const RGB& color) noexcept; !!!!!!
        // void setPointColor(const RGB&  color) noexcept { m_stroke_color = color; } !!!!!!
        void setStrokeWidth(float width) noexcept { m_stroke_width = width; }
    };


} // End of namespace Grain

#endif // GrainBezierValueCurve_hpp
