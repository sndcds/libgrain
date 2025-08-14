//
//  Polygon.hpp
//
//  Created by Roald Christesen on from 06.03.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 25.07.2025
//

#ifndef GrainPolygon_hpp
#define GrainPolygon_hpp

#include "Grain.hpp"
#include "Math/Vec2.hpp"
#include "2d/Rect.hpp"
#include "RangeRect.hpp"
#include "Type/List.hpp"


namespace Grain {

    class GraphicContext;

    class Polygon {
    protected:
        List<Vec2d> m_points;
        bool m_closed = false;
        bool m_must_update = false;
        double m_length = -1.0;

    public:
        Polygon(int32_t point_capacity = 32) noexcept;
        ~Polygon() noexcept;

        void _init(int32_t point_capacity) noexcept;
        void _update() noexcept;
        void _updateLength() noexcept;

        bool hasPoints() const noexcept { return pointCount() > 0; }
        int32_t pointCount() const noexcept { return (int32_t)m_points.size(); }
        int32_t segmentCount() const noexcept;
        List<Vec2d>* pointsListPtr() noexcept { return &m_points; }

        int32_t lastPointIndex() const noexcept { return pointCount() - 1; }
        bool pointAtIndex(int32_t index, Vec2d& out_point) noexcept;
        Vec2d* mutPointPtrAtIndex(int32_t index) noexcept;
        Vec2d* mutLastPointPtr() noexcept;

        double length() noexcept;
        RangeRectd bounds() noexcept;
        bool bounds(RangeRectd& out_bounds) noexcept;

        void clear() noexcept;

        bool setCapacity(int32_t capacity) noexcept;

        void addPoint(double x, double y) noexcept;
        void addPoint(const Vec2d& pos) noexcept;

        bool isClosed() const noexcept { return m_closed; }
        void close() noexcept { setClosed(true); }
        void setClosed(bool closed) noexcept {
            if (m_closed != closed) {
                m_closed = closed;
                m_must_update = true;
            }
        }

        bool generateEvenlyDistributedPolygon(int32_t point_count, Polygon& out_polygon) noexcept;

        bool point(double t, Vec2d& out_point) noexcept;

        void fill(GraphicContext& gc) noexcept;
        void stroke(GraphicContext& gc) noexcept;
        void addClip(GraphicContext& gc) noexcept;
    };


} // End of namespace Grain

#endif // GrainPolygon_hpp
