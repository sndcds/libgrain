//
//  CatmullRomCurve.hpp
//
//  Created by Roald Christesen on from 30.07.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainCatmullRomCurve_hpp
#define GrainCatmullRomCurve_hpp

#include "Grain.hpp"
#include "Math/Vec2.hpp"
#include "Type/List.hpp"


namespace Grain {

/**
 *  @brief Centripetal Catmull–Rom spline curve.
 */
class CatmullRomCurve {

protected:
    List<Vec2d> points_;
    float alpha_ = 0.5f;
    double point_distance_threshold_ = 0.01;
    int32_t default_resolution_ = 1000;

public:
    CatmullRomCurve() noexcept;
    explicit CatmullRomCurve(int32_t point_capacity) noexcept;
    ~CatmullRomCurve() noexcept;

    [[nodiscard]] static const char *className() noexcept {
        return "CatmullRomCurve";
    }

    friend std::ostream& operator << (std::ostream& os, const CatmullRomCurve* o) {
        o == nullptr ? os << "CatmullRomCurve nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream &os, const CatmullRomCurve &o) {
        os << "points: " << o.points_.size();
        os << ", segments: " << o.segmentCount();
        os << ", alpha: " << o.alpha_;
        os << ", point distance threshold: " << o.point_distance_threshold_;
        return os;
    }

    ErrorCode addPoint(const Vec2d &point) noexcept;
    ErrorCode extend() noexcept;

    void reset() noexcept {
        points_.clear();
    }

    bool setPointCapacity(int32_t capacity) noexcept { return points_.reserve(capacity); }


    Vec2d pointAtIndex(int32_t index) noexcept;

    [[nodiscard]] int32_t lastPointIndex() const noexcept { return static_cast<int32_t>(points_.lastIndex()); }
    [[nodiscard]] int32_t pointCount() const noexcept { return static_cast<int32_t>(points_.size()); }
    [[nodiscard]] int32_t segmentCount() const noexcept { return pointCount() - 3; }
    [[nodiscard]] int32_t lastSegmentIndex() const noexcept { return pointCount() - 4; }

    [[nodiscard]] float alpha() const noexcept { return alpha_; }
    void setAlpha(float alpha) noexcept { alpha_ = alpha; }

    [[nodiscard]] int32_t defaultResolution() const noexcept { return default_resolution_; }
    void setDefaultResolution(int32_t resolution) noexcept { default_resolution_ = resolution; }

    [[nodiscard]] double rawLength() const noexcept;

    void pointOnCurve(double t, Vec2d &out_point) const noexcept;

    [[nodiscard]] double _getT(double t, float alpha, const Vec2d &p0, const Vec2d &p1) const noexcept;
    void _getPoint(const Vec2d &p0, const Vec2d &p1, const Vec2d &p2, const Vec2d &p3, double t, float alpha, Vec2d &out_point) const noexcept;
    void _getPoint(int32_t segment_index, double t, float alpha, Vec2d &out_point) const noexcept;
};


} // End of namespace Grain

#endif // GrainCatmullRomCurve_hpp
