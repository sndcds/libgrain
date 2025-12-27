//
//  Polygon.hpp
//
//  Created by Roald Christesen on from 06.03.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainPolygon_hpp
#define GrainPolygon_hpp

#include "Math/Vec2.hpp"
#include "2d/Rect.hpp"
#include "2d/RangeRect.hpp"
#include "Type/List.hpp"


namespace Grain {

class GraphicContext;

class Polygon {

    protected:
    List<Vec2d> points_;
    bool closed_ = false;
    bool must_update_ = false;
    double length_ = -1.0;

public:
    explicit Polygon(int32_t point_capacity = 32) noexcept;
    virtual ~Polygon() noexcept;

    [[nodiscard]] virtual const char* className() const noexcept {
        return "Polygon";
    }

    friend std::ostream& operator << (std::ostream& os, const Polygon* o) {
        o == nullptr ? os << "Polygon nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const Polygon& o) {
        os << "points: " << o.points_.size();
        os << ", closed: " << o.closed_;
        os << ", length: " << o.length_;
        return os;
    }


    void _init(int32_t point_capacity) noexcept;
    void _update() noexcept;
    void _updateLength() noexcept;

    [[nodiscard]] bool hasPoints() const noexcept { return pointCount() > 0; }
    [[nodiscard]] int32_t pointCount() const noexcept { return static_cast<int32_t>(points_.size()); }
    [[nodiscard]] int32_t segmentCount() const noexcept;
    [[nodiscard]] List<Vec2d>* pointsListPtr() noexcept { return &points_; }

    [[nodiscard]] int32_t lastPointIndex() const noexcept { return pointCount() - 1; }
    bool pointAtIndex(int32_t index, Vec2d& out_point) noexcept;
    [[nodiscard]] Vec2d* mutPointPtrAtIndex(int32_t index) noexcept;
    [[nodiscard]] Vec2d* mutLastPointPtr() noexcept;

    [[nodiscard]] double length() noexcept;
    [[nodiscard]] RangeRectd bounds() noexcept;
    bool bounds(RangeRectd& out_bounds) noexcept;

    void clear() noexcept;

    bool setCapacity(int32_t capacity) noexcept;

    void addPoint(double x, double y) noexcept;
    void addPoint(const Vec2d& pos) noexcept;

    [[nodiscard]] bool isClosed() const noexcept { return closed_; }
    void close() noexcept { setClosed(true); }
    void setClosed(bool closed) noexcept {
        if (closed_ != closed) {
            closed_ = closed;
            must_update_ = true;
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
