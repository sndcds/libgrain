//
//  GraphicPathPoint.hpp
//
//  Created by Roald Christesen on from 22.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainGraphicPathPoint_hpp
#define GrainGraphicPathPoint_hpp

#include "Math/Vec2.hpp"
#include "Math/Mat3.hpp"
#include "2d/Quadrilateral.hpp"


namespace Grain {

class GraphicPathPoint {

    friend class GraphicPath;
    friend class GraphicCompoundPath;
    friend class GraphicContext;

protected:
    Vec2d anchor_;
    Vec2d left_;
    Vec2d right_;
    bool left_flag_ = false;
    bool right_flag_ = false;
    double bezier_segment_length_ = -1.0;

public:
    GraphicPathPoint() noexcept = default;
    GraphicPathPoint(const GraphicPathPoint& point) noexcept = default;
    GraphicPathPoint(GraphicPathPoint&& other) noexcept;

    GraphicPathPoint(double x, double y) noexcept;
    GraphicPathPoint(double x, double y, double lx, double ly, double rx, double ry) noexcept;
    GraphicPathPoint(double x, double y, bool left_flag, double lx, double ly, bool right_flag, double rx, double ry) noexcept;
    GraphicPathPoint(const Vec2d& anchor, bool left_flag, const Vec2d& left, bool rightFlag, const Vec2d& right) noexcept;

    virtual ~GraphicPathPoint() = default;

    [[nodiscard]] virtual const char *className() const noexcept {
        return "GraphicPathPoint";
    }

    friend std::ostream& operator << (std::ostream& os, const GraphicPathPoint* o) {
        o == nullptr ? os << "nullptr" : os << *o;
        return os;
    }

    friend std::ostream& operator << (std::ostream& os, const GraphicPathPoint& o) {
        os << "anhor: " << o.anchor_;
        if (o.left_flag_) {
            os << ", left: " << o.left_;
        }
        if (o.right_flag_) {
            os << ", right: " << o.right_;
        }
        if (o.bezier_segment_length_ > 0.0) {
            os << ", length: " << o.bezier_segment_length_;
        }
        return os;
    }


    // Copy assignment operator
    GraphicPathPoint& operator = (const GraphicPathPoint& point) noexcept;

    // Move assignment operator (C++11 and later)
    GraphicPathPoint& operator = (GraphicPathPoint&& point) noexcept;


    [[nodiscard]] Vec2d anchor() const noexcept { return anchor_; }
    [[nodiscard]] Vec2d left() const noexcept { return left_; }
    [[nodiscard]] Vec2d right() const noexcept { return right_; }

    [[nodiscard]] bool usesLeft() const noexcept { return left_flag_; }
    [[nodiscard]] bool usesRight() const noexcept { return right_flag_; }
    [[nodiscard]] bool isCorner() const noexcept { return !left_flag_ && !right_flag_; }

    void setAnchor(const Vec2d& anchor) noexcept { anchor_ = anchor; bezier_segment_length_ = -1.0; }
    void setAnchor(double x, double y) noexcept { anchor_.x_ = x; anchor_.y_ = y; bezier_segment_length_ = -1.0; }
    void setLeft(const Vec2d& left) noexcept { left_ = left; left_flag_ = true; bezier_segment_length_ = -1.0; }
    void setLeft(double x, double y) noexcept { left_.x_ = x; left_.y_ = y; left_flag_ = true; bezier_segment_length_ = -1.0; }
    void setRight(const Vec2d& right) noexcept { right_ = right; right_flag_ = true; bezier_segment_length_ = -1.0; }
    void setRight(double x, double y) noexcept { right_.x_ = x; right_.y_ = y; right_flag_ = true; bezier_segment_length_ = -1.0; }

    void translate(const Vec2d& t) noexcept;
    void translate(double tx, double ty) noexcept;
    void rotate(double angle) noexcept;

    void projectToQuadrilateral(const Quadrilateral& quadrilateral, const Mat3d* matrix = nullptr) noexcept;
};


} // End of namespace Grain

#endif // GrainGraphicPathPoint_hpp
