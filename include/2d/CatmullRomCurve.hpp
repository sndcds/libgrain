//
//  CatmullRomCurve.hpp
//
//  Created by Roald Christesen on from 30.07.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
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
    public:
    protected:
        List<Vec2d> m_points;
        float m_alpha = 0.5f;
        double m_point_distance_threshold = 0.01;
        int32_t m_default_res = 1000;

    public:
        CatmullRomCurve() noexcept;
        CatmullRomCurve(int32_t point_capacity) noexcept;
        ~CatmullRomCurve() noexcept;

        friend std::ostream& operator << (std::ostream &os, const CatmullRomCurve &o) {
            os << "CatmullRomCurve:\n";
            os << "  points: " << o.m_points.size() << '\n';
            os << "  segments: " << o.segmentCount() << '\n';
            os << "  alpha: " << o.m_alpha << '\n';
            os << "  point distance threshold: " << o.m_point_distance_threshold << '\n';
            return os;
        }

        ErrorCode addPoint(const Vec2d &point) noexcept;
        ErrorCode extend() noexcept;

        void reset() noexcept {
            m_points.clear();
        }

        bool setPointCapacity(int32_t capacity) noexcept { return m_points.reserve(capacity); }


        Vec2d pointAtIndex(int32_t index) noexcept;

        int32_t lastPointIndex() const noexcept { return (int32_t)m_points.lastIndex(); }
        int32_t pointCount() const noexcept { return (int32_t)m_points.size(); }
        int32_t segmentCount() const noexcept { return (int32_t)m_points.size() - 3; }
        int32_t lastSegmentIndex() const noexcept { return (int32_t)m_points.size() - 4; }

        float alpha() const noexcept { return m_alpha; }
        void setAlpha(float alpha) noexcept { m_alpha = alpha; }

        int32_t defaultResolution() const noexcept { return m_default_res; }
        void setDefaultResolution(int32_t resolution) noexcept { m_default_res = resolution; }

        double rawLength() const noexcept;

        void pointOnCurve(double t, Vec2d &out_point) const noexcept;

        double _getT(double t, float alpha, const Vec2d &p0, const Vec2d &p1) const noexcept;
        void _getPoint(const Vec2d &p0, const Vec2d &p1, const Vec2d &p2, const Vec2d &p3, double t, float alpha, Vec2d &out_point) const noexcept;
        void _getPoint(int32_t segment_index, double t, float alpha, Vec2d &out_point) const noexcept;
    };


} // End of namespace Grain

#endif // GrainCatmullRomCurve_hpp
